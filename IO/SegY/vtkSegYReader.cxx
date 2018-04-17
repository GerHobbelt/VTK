/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkSegYReader.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#include "vtkImageData.h"
#include "vtkInformationVector.h"
#include "vtkInformation.h"
#include "vtkObjectFactory.h"
#include "vtkSegYReaderInternal.h"
#include "vtkSmartPointer.h"
#include "vtkSegYReader.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkStructuredGrid.h"

#include <algorithm>
#include <iostream>
#include <iterator>


vtkStandardNewMacro(vtkSegYReader);

//-----------------------------------------------------------------------------
vtkSegYReader::vtkSegYReader()
{
  this->SetNumberOfInputPorts(0);
  this->Reader = new vtkSegYReaderInternal();
  this->FileName = nullptr;
  this->Is3D = false;
  this->DataOrigin[0] = this->DataOrigin[1] = this->DataOrigin[2] = 0.0;
  this->DataSpacing[0] = this->DataSpacing[1] = this->DataSpacing[2] = 1.0;
  this->DataExtent[0] = this->DataExtent[2] = this->DataExtent[4] = 0;
  this->DataExtent[1] = this->DataExtent[3] = this->DataExtent[5] = 0;

  this->XYCoordMode = VTK_SEGY_SOURCE;
  this->StructuredGrid = 0;
  this->XCoordByte = 73;
  this->YCoordByte = 77;

  this->VerticalCRS = VTK_SEGY_VERTICAL_HEIGHTS;

}

//-----------------------------------------------------------------------------
vtkSegYReader::~vtkSegYReader()
{
  delete this->Reader;
}

//-----------------------------------------------------------------------------
void vtkSegYReader::SetXYCoordModeToSource()
{
  this->SetXYCoordMode(VTK_SEGY_SOURCE);
}

//-----------------------------------------------------------------------------
void vtkSegYReader::SetXYCoordModeToCDP()
{
  this->SetXYCoordMode(VTK_SEGY_CDP);
}

//-----------------------------------------------------------------------------
void vtkSegYReader::SetXYCoordModeToCustom()
{
  this->SetXYCoordMode(VTK_SEGY_CUSTOM);
}


//-----------------------------------------------------------------------------
void vtkSegYReader::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
}


//-----------------------------------------------------------------------------
int vtkSegYReader::RequestData(vtkInformation* vtkNotUsed(request),
                               vtkInformationVector** vtkNotUsed(inputVector),
                               vtkInformationVector* outputVector)
{
  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  if (!outInfo)
  {
    return 0;
  }

  vtkDataObject* output = outInfo->Get(vtkDataObject::DATA_OBJECT());
  if (!output)
  {
    return 0;
  }

  this->Reader->SetVerticalCRS(this->VerticalCRS);
  switch (this->XYCoordMode)
  {
  case VTK_SEGY_SOURCE:
    {
      this->Reader->SetXYCoordBytePositions(72, 76);
      break;
    }
  case VTK_SEGY_CDP:
    {
      this->Reader->SetXYCoordBytePositions(180, 184);
      break;
    }
  case VTK_SEGY_CUSTOM:
    {
      this->Reader->SetXYCoordBytePositions(this->XCoordByte - 1,
                                            this->YCoordByte - 1);
      break;
    }
  default:
    {
      vtkErrorMacro(<< "Unknown value for XYCoordMode " << this->XYCoordMode);
      return 1;
    }
  }
  this->Reader->LoadTraces();
  this->UpdateProgress(0.5);
  if (this->Is3D && ! this->StructuredGrid)
  {
    this->Reader->ExportData(vtkImageData::SafeDownCast(output),
                             this->DataExtent, this->DataOrigin, this->DataSpacing);
  }
  else
  {
    vtkStructuredGrid* grid = vtkStructuredGrid::SafeDownCast(output);
    this->Reader->ExportData(grid, this->DataExtent);
    grid->Squeeze();
  }
  this->Reader->In.close();
  std::cout << "RequestData" << std::endl;
  return 1;
}

//-----------------------------------------------------------------------------
int vtkSegYReader::RequestInformation(vtkInformation * vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *outputVector)
{
  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  if (!outInfo)
  {
    vtkErrorMacro("Invalid output information object");
    return 0;
  }

  std::cout << "DataExtent: ";
  std::copy(this->DataExtent, this->DataExtent + 6, std::ostream_iterator<int>(std::cout, " "));
  outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(),
               this->DataExtent, 6);
  std::cout << std::endl;
  if (this->Is3D && ! this->StructuredGrid)
  {
    std::cout << "DataOrigin: ";
    std::copy(this->DataOrigin, this->DataOrigin + 3, std::ostream_iterator<double>(std::cout, " "));
    std::cout << "\nDataSpacing: ";
    std::copy(this->DataSpacing, this->DataSpacing + 3, std::ostream_iterator<double>(std::cout, " "));
    std::cout << std::endl;
    outInfo->Set(vtkDataObject::ORIGIN(), this->DataOrigin, 3);
    outInfo->Set(vtkDataObject::SPACING(), this->DataSpacing, 3);
  }
  return 1;
}

//----------------------------------------------------------------------------
int vtkSegYReader::FillOutputPortInformation(
  int vtkNotUsed(port), vtkInformation* info)
{
  const char* outputTypeName =
    (this->Is3D  && ! this->StructuredGrid) ? "vtkImageData" : "vtkStructuredGrid";
  info->Set(vtkDataObject::DATA_TYPE_NAME(), outputTypeName);
  std::cout << "FillOutputPortInformation" << std::endl;
  return 1;
}

//----------------------------------------------------------------------------
int vtkSegYReader::RequestDataObject(
  vtkInformation*,
  vtkInformationVector** vtkNotUsed(inputVector),
  vtkInformationVector* outputVector)
{
  vtkInformation* info = outputVector->GetInformationObject(0);
  vtkDataSet *output = vtkDataSet::SafeDownCast(
    info->Get(vtkDataObject::DATA_OBJECT()));

  if (!this->FileName)
  {
    vtkErrorMacro("Requires valid input file name") ;
    return 0;
  }

  this->Reader->In.open(this->FileName, std::ifstream::binary);
  if (!this->Reader->In)
  {
    std::cerr << "File not found:" << this->FileName << std::endl;
    return 0;
  }
  this->Is3D = this->Reader->Is3DComputeParameters(
    this->DataExtent, this->DataOrigin, this->DataSpacing);
  const char* outputTypeName = this->Is3D ? "vtkImageData" : "vtkStructuredGrid";

  if (!output || !output->IsA(outputTypeName))
  {
    vtkDataSet* newOutput = nullptr;
    if (this->Is3D && ! this->StructuredGrid)
    {
      newOutput = vtkImageData::New();
    }
    else
    {
      newOutput = vtkStructuredGrid::New();
    }
    info->Set(vtkDataObject::DATA_OBJECT(), newOutput);
    newOutput->Delete();
  }
  std::cout << "RequestDataObject" << std::endl;
  return 1;
}
