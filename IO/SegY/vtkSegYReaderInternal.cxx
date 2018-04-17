/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkSegYReaderInternal.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkSegYReaderInternal.h"

#include "vtkArrayData.h"
#include "vtkFloatArray.h"
#include "vtkImageData.h"
#include "vtkMath.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkSegYBinaryHeaderBytesPositions.h"
#include "vtkSegYIOUtils.h"
#include "vtkSegYTraceReader.h"
#include "vtkSmartPointer.h"
#include "vtkStructuredGrid.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <iterator>
#include <map>
#include <set>

namespace {
  const int FIRST_TRACE_START_POS = 3600;  // this->Traces start after 3200 + 400 file header
  double decodeMultiplier(short multiplier)
  {
    return
      (multiplier < 0) ?
      (-1.0 / multiplier)
      : (multiplier > 0 ? multiplier : 1.0);
  }

};

//-----------------------------------------------------------------------------
vtkSegYReaderInternal::vtkSegYReaderInternal() :
  SampleInterval(0), FormatCode(0), SampleCountPerTrace(0)
{
  this->BinaryHeaderBytesPos = new vtkSegYBinaryHeaderBytesPositions();
  this->VerticalCRS = 0;
  this->TraceReader = new vtkSegYTraceReader();
}

//-----------------------------------------------------------------------------
vtkSegYReaderInternal::~vtkSegYReaderInternal()
{
  delete this->BinaryHeaderBytesPos;
  delete this->TraceReader;
  for (auto trace : this->Traces)
    delete trace;
}

//-----------------------------------------------------------------------------
void vtkSegYReaderInternal::SetXYCoordBytePositions(int x, int y)
{
  this->TraceReader->SetXYCoordBytePositions(x, y);
}

//-----------------------------------------------------------------------------
void vtkSegYReaderInternal::SetVerticalCRS(int v)
{
  this->VerticalCRS = v > 0 ? 1 : 0;
}

//-----------------------------------------------------------------------------
bool vtkSegYReaderInternal::LoadFromFile(std::string path)
{
  this->In.open(path, std::ifstream::binary);
  if (!this->In)
  {
    std::cerr << "File not found:" << path << std::endl;
    return false;
  }

  this->ReadHeader();
  this->LoadTraces();

  this->In.close();
  return true;
}

//-----------------------------------------------------------------------------
void vtkSegYReaderInternal::LoadTraces()
{
  int traceStartPos = FIRST_TRACE_START_POS;
  int fileSize = vtkSegYIOUtils::Instance()->getFileSize(this->In);
  while (traceStartPos + 240 < fileSize)
  {
    vtkSegYTrace* pTrace = new vtkSegYTrace();
    this->TraceReader->ReadTrace(traceStartPos, this->In, this->FormatCode, pTrace);
    this->Traces.push_back(pTrace);
  }
}

//-----------------------------------------------------------------------------
bool vtkSegYReaderInternal::ReadHeader()
{
  this->SampleInterval = vtkSegYIOUtils::Instance()->readShortInteger(
    this->BinaryHeaderBytesPos->SampleInterval, this->In);
  this->FormatCode = vtkSegYIOUtils::Instance()->readShortInteger(
    this->BinaryHeaderBytesPos->FormatCode, this->In);
  this->In.seekg(this->BinaryHeaderBytesPos->MajorVersion, this->In.beg);
  unsigned char majorVersion = vtkSegYIOUtils::Instance()->readUChar(this->In);
  unsigned char minorVersion = vtkSegYIOUtils::Instance()->readUChar(this->In);
  this->SampleCountPerTrace = vtkSegYIOUtils::Instance()->readShortInteger(
    this->BinaryHeaderBytesPos->NumSamplesPerTrace, this->In);
  short tracesPerEnsemble = vtkSegYIOUtils::Instance()->readShortInteger(
    this->BinaryHeaderBytesPos->NumberTracesPerEnsemble, this->In);
  short ensembleType = vtkSegYIOUtils::Instance()->readShortInteger(
    this->BinaryHeaderBytesPos->EnsembleType, this->In);
  short measurementSystem = vtkSegYIOUtils::Instance()->readShortInteger(
    this->BinaryHeaderBytesPos->MeasurementSystem, this->In);
  int byteOrderingDetection = vtkSegYIOUtils::Instance()->readLongInteger(
    this->BinaryHeaderBytesPos->ByteOrderingDetection, this->In);
  return true;
}

//-----------------------------------------------------------------------------
bool vtkSegYReaderInternal::Is3DComputeParameters(
  int* extent, double* origin, double* spacing)
{
  this->ReadHeader();
  int traceStartPos = FIRST_TRACE_START_POS;
  int fileSize = vtkSegYIOUtils::Instance()->getFileSize(this->In);
  int crosslineFirst, crosslineSecond, inlineFirst, inlineSecond,
    inlineNumber, crosslineNumber;
  double coordFirst[3], coordSecondX[3], coordSecondY[3], d[3];
  int xCoord, yCoord;
  short coordMultiplier;
  int prevTraceStartPos = traceStartPos;
  int crosslineCount = 0;
  if (traceStartPos + 240 < fileSize)
  {
    this->TraceReader->ReadInlineCrossline
      (traceStartPos, this->In, this->FormatCode,
       &inlineFirst, &crosslineFirst,
       &xCoord, &yCoord, &coordMultiplier);
    double coordinateMultiplier = decodeMultiplier(coordMultiplier);
    coordFirst[0] = xCoord * coordinateMultiplier;
    coordFirst[1] = yCoord * coordinateMultiplier;
    coordFirst[2] = 0;
    ++crosslineCount;
  }
  int traceSize = traceStartPos - prevTraceStartPos;
  if (traceStartPos + 240 < fileSize)
  {
    this->TraceReader->ReadInlineCrossline
      (traceStartPos, this->In, this->FormatCode,
       &inlineNumber, &crosslineSecond,
       &xCoord, &yCoord, &coordMultiplier);
    double coordinateMultiplier = decodeMultiplier(coordMultiplier);
    coordSecondX[0] = xCoord * coordinateMultiplier;
    coordSecondX[1] = yCoord * coordinateMultiplier;
    coordSecondX[2] = 0;
    ++crosslineCount;
  }
  vtkMath::Subtract(coordFirst, coordSecondX, d);
  float xStep = vtkMath::Norm(d);
  while(inlineFirst == inlineNumber && traceStartPos + 240 < fileSize)
  {
    this->TraceReader->ReadInlineCrossline
      (traceStartPos, this->In, this->FormatCode,
       &inlineNumber, &crosslineNumber,
       &xCoord, &yCoord, &coordMultiplier);
    ++crosslineCount;
  }
  if (traceStartPos + 240 < fileSize)
  {
    // we read a crossline from the next inline
    --crosslineCount;
  }
  int inlineCount = (fileSize - FIRST_TRACE_START_POS) / traceSize / crosslineCount;
  auto e = {
    0, crosslineCount - 1,
    0, inlineCount - 1,
    0, this->SampleCountPerTrace - 1,
  };
  std::copy(e.begin(), e.end(), extent);
  if (traceStartPos + 240 >= fileSize)
  {
    // this is a 2D dataset
    return false;
  }
  inlineSecond = inlineNumber;
  double coordinateMultiplier = decodeMultiplier(coordMultiplier);
  coordSecondY[0] = xCoord * coordinateMultiplier;
  coordSecondY[1] = yCoord * coordinateMultiplier;
  coordSecondY[2] = 0;
  vtkMath::Subtract(coordFirst, coordSecondY, d);
  float yStep = vtkMath::Norm(d);

  // The samples are uniformly placed at sample interval depths
  // Dividing by 1000.0 to convert from microseconds to milliseconds.
  float zStep = this->SampleInterval / 1000.0;
  std::array<double, 3> o = {coordFirst[0],
                             coordFirst[1],
                             - zStep * (this->SampleCountPerTrace - 1)};
  std::copy(o.begin(), o.end(), origin);
  auto s = {xStep, yStep, zStep};
  std::copy(s.begin(), s.end(), spacing);
  return true;
}




//-----------------------------------------------------------------------------
void vtkSegYReaderInternal::ExportData3D(vtkImageData* imageData,
                                 int* extent, double* origin, double* spacing)
{
  imageData->SetExtent(extent);
  imageData->SetOrigin(origin);
  imageData->SetSpacing(spacing);
  int* dims = imageData->GetDimensions();

  vtkNew<vtkFloatArray> scalars;
  scalars->SetNumberOfComponents(1);
  scalars->SetNumberOfTuples(dims[0] * dims[1] * dims[2]);
  scalars->SetName("trace");
  imageData->GetPointData()->SetScalars(scalars);
  for (int k = 0; k < this->SampleCountPerTrace; ++k)
  {
    for (int j = 0; j < dims[1]; ++j)
    {
      for (int i = 0; i < dims[0]; ++i)
      {
        vtkSegYTrace* trace = this->Traces[j * dims[0] + i];
        scalars->SetValue(k * dims[1] * dims[0] + j * dims[0] + i,
                          trace->Data[k]);
      }
    }
  }
}


//-----------------------------------------------------------------------------
void vtkSegYReaderInternal::AddScalars(vtkStructuredGrid* grid)
{
  vtkSmartPointer<vtkFloatArray> outScalars =
    vtkSmartPointer<vtkFloatArray>::New();
  outScalars->SetName("trace");
  outScalars->SetNumberOfComponents(1);

  int crosslineCount = this->Traces.size();

  outScalars->Allocate(crosslineCount * this->SampleCountPerTrace);

  int j = 0;
  for (int k = 0; k < this->SampleCountPerTrace; k++)
  {
    for (unsigned int i = 0; i < this->Traces.size(); i++)
    {
      outScalars->InsertValue(j++, this->Traces[i]->Data[k]);
    }
  }

  grid->GetPointData()->SetScalars(outScalars);
  grid->GetPointData()->SetActiveScalars("trace");
}

//-----------------------------------------------------------------------------
void vtkSegYReaderInternal::ExportData2D(vtkStructuredGrid* grid)
{
  if (!grid)
  {
    return;
  }
  grid->SetDimensions(this->Traces.size(), 1, this->SampleCountPerTrace);
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();

  int sign = this->VerticalCRS == 0 ? -1 : 1;
  for (int k = 0; k < this->SampleCountPerTrace; k++)
  {
    for (unsigned int i = 0; i < this->Traces.size(); i++)
    {
      auto trace = this->Traces[i];
      double coordinateMultiplier = decodeMultiplier(trace->CoordinateMultiplier);
      float x = trace->XCoordinate * coordinateMultiplier;
      float y = trace->YCoordinate * coordinateMultiplier;

      // The samples are uniformly placed at sample interval depths
      // Dividing by 1000.0 to convert from microseconds to milliseconds.
      float z = sign * k * (trace->SampleInterval / 1000.0);
      points->InsertNextPoint(x, y, z);
    }
  }

  grid->SetPoints(points);
  this->AddScalars(grid);
}
