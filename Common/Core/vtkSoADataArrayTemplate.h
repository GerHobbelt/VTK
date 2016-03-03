/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkSoADataArrayTemplate.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkSoADataArrayTemplate
// .SECTION Description
// vtkSoADataArrayTemplate is the counterpart of vtkDataArrayTemplate.

#ifndef vtkSoADataArrayTemplate_h
#define vtkSoADataArrayTemplate_h

#include "vtkGenericDataArray.h"
#include "vtkBuffer.h"

template <class ScalarTypeT>
class vtkSoADataArrayTemplate : public vtkTypeTemplate<
                            vtkSoADataArrayTemplate<ScalarTypeT>,
                            vtkGenericDataArray<vtkSoADataArrayTemplate<ScalarTypeT>, ScalarTypeT>
                            >
{
  typedef vtkGenericDataArray<vtkSoADataArrayTemplate<ScalarTypeT>, ScalarTypeT> GenericDataArrayType;
public:
  typedef vtkSoADataArrayTemplate<ScalarTypeT> SelfType;
  typedef typename GenericDataArrayType::ScalarType ScalarType;
  typedef typename GenericDataArrayType::ScalarReturnType ScalarReturnType;

  static vtkSoADataArrayTemplate* New();

  // **************************************************************************
  // Methods that are needed to be implemented by every vtkGenericDataArray
  // subclass.
  // **************************************************************************
  inline ScalarReturnType GetValue(vtkIdType valueIdx) const
    {
    vtkIdType tupleIdx;
    int comp;
    this->GetTupleIndexFromValueIndex(valueIdx, tupleIdx, comp);
    return this->GetComponentValue(tupleIdx, comp);
    }
  inline void GetTupleValue(vtkIdType tupleIdx, ScalarType* tuple) const
    {
    for (int cc=0; cc < this->NumberOfComponents; cc++)
      {
      tuple[cc] = this->Data[cc].GetBuffer()[tupleIdx];
      }
    }
  inline ScalarReturnType GetComponentValue(vtkIdType tupleIdx, int comp) const
    {
    return this->Data[comp].GetBuffer()[tupleIdx];
    }
  inline void SetValue(vtkIdType valueIdx, ScalarType value)
    {
    vtkIdType tupleIdx;
    int comp;
    this->GetTupleIndexFromValueIndex(valueIdx, tupleIdx, comp);
    this->SetComponentValue(tupleIdx, comp, value);
    }
  inline void SetTupleValue(vtkIdType tupleIdx, ScalarType* tuple)
    {
    for (int cc=0; cc < this->NumberOfComponents; ++cc)
      {
      this->Data[cc].GetBuffer()[tupleIdx] = tuple[cc];
      }
    }
  inline void SetComponentValue(vtkIdType tupleIdx, int comp, ScalarType value)
    {
    this->Data[comp].GetBuffer()[tupleIdx] = value;
    }

  // **************************************************************************

  enum DeleteMethod
    {
    VTK_DATA_ARRAY_FREE=vtkBuffer<ScalarType>::VTK_DATA_ARRAY_FREE,
    VTK_DATA_ARRAY_DELETE=vtkBuffer<ScalarType>::VTK_DATA_ARRAY_DELETE
    };

  // Description:
  // Use this API to pass externally allocated memory to this instance. Since
  // vtkSoADataArrayTemplate uses separate contiguous regions for each component,
  // use this API to add arrays for each of the component.
  // \c save: When set to true, vtkSoADataArrayTemplate will not release or realloc the memory
  // even when the AllocatorType is set to RESIZABLE. If needed it will simply
  // allow new memory buffers and "forget" the supplied pointers. When save is
  // set to false, this will be the \c deleteMethod specified to release the
  // array.
  // \c size is specified in number of elements of ScalarType.
  void SetArray(int comp, ScalarType* array, vtkIdType size,
    bool save=false, int deleteMethod=VTK_DATA_ARRAY_FREE);

  // Description:
  // Overridden to allocate pointer for each component.
  virtual void SetNumberOfComponents(int);

  // Description:
  // Call this method before using any of the methods on this array that affect
  // memory allocation. When set to false, any attempt to grow the arrays will
  // raise runtime exceptions. Any attempt to shrink the arrays will have no
  // effect.
  vtkSetMacro(Resizeable, bool);
  vtkGetMacro(Resizeable, bool);
  vtkBooleanMacro(Resizeable, bool);

protected:
  vtkSoADataArrayTemplate();
  ~vtkSoADataArrayTemplate();

  // **************************************************************************
  // Methods that are needed to be implemented by every vtkGenericDataArray
  // subclass.
  // **************************************************************************
  // Implement the memory management interface.
  bool AllocateTuples(vtkIdType numTuples);
  bool ReallocateTuples(vtkIdType numTuples);
  // **************************************************************************

  std::vector<vtkBuffer<ScalarType> > Data;
  bool Resizeable;
  double NumberOfComponentsReciprocal;
private:
  vtkSoADataArrayTemplate(const vtkSoADataArrayTemplate&); // Not implemented.
  void operator=(const vtkSoADataArrayTemplate&); // Not implemented.

  inline void GetTupleIndexFromValueIndex(vtkIdType valueIdx, vtkIdType& tupleIdx, int& comp) const
    {
    tupleIdx = static_cast<vtkIdType>(valueIdx * this->NumberOfComponentsReciprocal);
    comp = valueIdx - (tupleIdx * this->NumberOfComponents);
    }


  friend class vtkGenericDataArray<vtkSoADataArrayTemplate<ScalarTypeT>, ScalarTypeT>;
};

#include "vtkSoADataArrayTemplate.txx"
#endif
