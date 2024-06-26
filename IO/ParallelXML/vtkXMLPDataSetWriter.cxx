// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
// SPDX-License-Identifier: BSD-3-Clause
#include "vtkXMLPDataSetWriter.h"

#include "vtkCallbackCommand.h"
#include "vtkDataSet.h"
#include "vtkImageData.h"
#include "vtkInformation.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkRectilinearGrid.h"
#include "vtkStructuredGrid.h"
#include "vtkUnstructuredGrid.h"
#include "vtkXMLPImageDataWriter.h"
#include "vtkXMLPPolyDataWriter.h"
#include "vtkXMLPRectilinearGridWriter.h"
#include "vtkXMLPStructuredGridWriter.h"
#include "vtkXMLPUnstructuredGridWriter.h"

VTK_ABI_NAMESPACE_BEGIN
vtkStandardNewMacro(vtkXMLPDataSetWriter);

//------------------------------------------------------------------------------
vtkXMLPDataSetWriter::vtkXMLPDataSetWriter() = default;

//------------------------------------------------------------------------------
vtkXMLPDataSetWriter::~vtkXMLPDataSetWriter() = default;

//------------------------------------------------------------------------------
void vtkXMLPDataSetWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//------------------------------------------------------------------------------
vtkDataSet* vtkXMLPDataSetWriter::GetInput()
{
  return static_cast<vtkDataSet*>(this->Superclass::GetInput());
}

//------------------------------------------------------------------------------
int vtkXMLPDataSetWriter::WriteInternal()
{
  vtkAlgorithmOutput* input = this->GetInputConnection(0, 0);
  vtkXMLPDataWriter* writer = nullptr;

  // Create a writer based on the data set type.
  switch (this->GetInput()->GetDataObjectType())
  {
    case VTK_UNIFORM_GRID:
    case VTK_IMAGE_DATA:
    case VTK_STRUCTURED_POINTS:
    {
      vtkXMLPImageDataWriter* w = vtkXMLPImageDataWriter::New();
      w->SetInputConnection(input);
      writer = w;
    }
    break;
    case VTK_STRUCTURED_GRID:
    {
      vtkXMLPStructuredGridWriter* w = vtkXMLPStructuredGridWriter::New();
      w->SetInputConnection(input);
      writer = w;
    }
    break;
    case VTK_RECTILINEAR_GRID:
    {
      vtkXMLPRectilinearGridWriter* w = vtkXMLPRectilinearGridWriter::New();
      w->SetInputConnection(input);
      writer = w;
    }
    break;
    case VTK_UNSTRUCTURED_GRID:
    {
      vtkXMLPUnstructuredGridWriter* w = vtkXMLPUnstructuredGridWriter::New();
      w->SetInputConnection(input);
      writer = w;
    }
    break;
    case VTK_POLY_DATA:
    {
      vtkXMLPPolyDataWriter* w = vtkXMLPPolyDataWriter::New();
      w->SetInputConnection(input);
      writer = w;
    }
    break;
  }

  // Make sure we got a valid writer for the data set.
  if (!writer)
  {
    vtkErrorMacro("Cannot write dataset type: " << this->GetInput()->GetDataObjectType());
    return 0;
  }

  // Copy the settings to the writer.
  writer->SetDebug(this->GetDebug());
  writer->SetFileName(this->GetFileName());
  writer->SetByteOrder(this->GetByteOrder());
  writer->SetCompressor(this->GetCompressor());
  writer->SetBlockSize(this->GetBlockSize());
  writer->SetDataMode(this->GetDataMode());
  writer->SetEncodeAppendedData(this->GetEncodeAppendedData());
  writer->SetHeaderType(this->GetHeaderType());
  writer->SetIdType(this->GetIdType());
  writer->SetWriteTimeValue(this->GetWriteTimeValue());
  writer->SetNumberOfPieces(this->GetNumberOfPieces());
  writer->SetGhostLevel(this->GetGhostLevel());
  writer->SetStartPiece(this->GetStartPiece());
  writer->SetEndPiece(this->GetEndPiece());
  writer->SetWriteSummaryFile(this->WriteSummaryFile);
  writer->AddObserver(vtkCommand::ProgressEvent, this->InternalProgressObserver);

  // Try to write.
  int result = writer->Write();

  // Cleanup.
  writer->RemoveObserver(this->InternalProgressObserver);
  writer->Delete();
  return result;
}

//------------------------------------------------------------------------------
const char* vtkXMLPDataSetWriter::GetDataSetName()
{
  return "DataSet";
}

//------------------------------------------------------------------------------
const char* vtkXMLPDataSetWriter::GetDefaultFileExtension()
{
  return "vtk";
}

//------------------------------------------------------------------------------
vtkXMLWriter* vtkXMLPDataSetWriter::CreatePieceWriter(int)
{
  return nullptr;
}
//------------------------------------------------------------------------------
int vtkXMLPDataSetWriter::FillInputPortInformation(int vtkNotUsed(port), vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet");
  return 1;
}
VTK_ABI_NAMESPACE_END
