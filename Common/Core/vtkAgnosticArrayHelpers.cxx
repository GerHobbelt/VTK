/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkAgnosticArrayHelpers.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkAgnosticArrayHelpers.h"

#include "vtkObjectFactory.h"
#include "vtkSoAArrayTemplate.h"

//============================================================================
namespace
{
  //--------------------------------------------------------------------------
  template <class ArrayDestType, class ArraySourceType>
  void vtkSetTuple(ArrayDestType* dest, vtkIdType destTuple,
                     ArraySourceType* source, vtkIdType sourceTuple)
      {
      for (int cc=0, max=dest->GetNumberOfComponents(); cc < max; ++cc)
        {
        dest->Begin(destTuple)[cc] =
          static_cast<typename ArrayDestType::ScalarType>(source->Begin(sourceTuple)[cc]);
        }
      }

  //-------------------------------------------------------------------------
  template <class ArraySourceType>
  void vtkGetTuple(ArraySourceType* source, vtkIdType tuple, double* buffer)
      {
      for (int cc=0, max=source->GetNumberOfComponents(); cc < max; ++cc)
        {
        buffer[cc] = static_cast<double>(source->Begin(tuple)[cc]);
        }
      }

}
//============================================================================


//----------------------------------------------------------------------------
vtkAgnosticArrayHelpers::vtkAgnosticArrayHelpers()
{
}

//----------------------------------------------------------------------------
vtkAgnosticArrayHelpers::~vtkAgnosticArrayHelpers()
{
}

//----------------------------------------------------------------------------
void vtkAgnosticArrayHelpers::SetTuple(
  vtkAbstractArray* dest, vtkIdType destTuple,
  vtkAbstractArray* source, vtkIdType sourceTuple)
{
  if (source->GetDataType() != dest->GetDataType())
    {
    vtkGenericWarningMacro("Input and output array data types do not match.");
    return;
    }
  if (dest->GetNumberOfComponents() != source->GetNumberOfComponents())
    {
    vtkGenericWarningMacro("Input and output component sizes do not match.");
    return;
    }

  vtkAgnosticArrayMacro2(dest, source,
    vtkSetTuple(ARRAY1, destTuple, ARRAY2, sourceTuple));
}

//----------------------------------------------------------------------------
void vtkAgnosticArrayHelpers::GetTuple(vtkAbstractArray* source, vtkIdType tuple, double* buffer)
{
  vtkAgnosticArrayMacro(source, vtkGetTuple(ARRAY, tuple, buffer));
}

//----------------------------------------------------------------------------
void vtkAgnosticArrayHelpers::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
