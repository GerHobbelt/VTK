// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
// SPDX-License-Identifier: BSD-3-Clause
#include "vtkCompositeInterpolatedVelocityField.h"

#include "vtkClosestPointStrategy.h"
#include "vtkDataSet.h"
#include "vtkGenericCell.h"
#include "vtkMath.h"
#include "vtkMathUtilities.h"
#include "vtkObjectFactory.h"
#include "vtkPlane.h"
#include "vtkPointData.h"
#include "vtkTriangle.h"

#include <array>

VTK_ABI_NAMESPACE_BEGIN
//------------------------------------------------------------------------------
vtkCompositeInterpolatedVelocityField::DataSetBoundsInformation::DataSetBoundsInformation()
  : DataSet(nullptr)
{
}

//------------------------------------------------------------------------------
vtkCompositeInterpolatedVelocityField::DataSetBoundsInformation::DataSetBoundsInformation(
  vtkDataSet* ds)
  : DataSet(ds)
{
  ds->GetBounds(this->Bounds.data());
}

//------------------------------------------------------------------------------
vtkStandardNewMacro(vtkCompositeInterpolatedVelocityField);

//------------------------------------------------------------------------------
vtkCompositeInterpolatedVelocityField::vtkCompositeInterpolatedVelocityField()
{
  this->SetFindCellStrategy(vtkSmartPointer<vtkClosestPointStrategy>::New());
  this->LastDataSetIndex = 0;
  this->CacheDataSetHit = 0;
  this->CacheDataSetMiss = 0;
}

//------------------------------------------------------------------------------
vtkCompositeInterpolatedVelocityField::~vtkCompositeInterpolatedVelocityField() = default;

//------------------------------------------------------------------------------
// Copy the list of datasets to copy from.
void vtkCompositeInterpolatedVelocityField::CopyParameters(
  vtkAbstractInterpolatedVelocityField* from)
{
  this->Superclass::CopyParameters(from);

  // See if we need to copy our parameters
  vtkCompositeInterpolatedVelocityField* obj =
    vtkCompositeInterpolatedVelocityField::SafeDownCast(from);
  if (!obj)
  {
    return;
  }
  this->DataSetsBoundsInfo = obj->DataSetsBoundsInfo;

  // The weights must be copied as well
  this->Weights.resize(obj->Weights.size());
}

//------------------------------------------------------------------------------
void vtkCompositeInterpolatedVelocityField::AddDataSet(vtkDataSet* dataset, size_t maxCellSize)
{
  if (!dataset)
  {
    vtkErrorMacro(<< "Dataset nullptr!");
    return;
  }

  // insert the dataset (do NOT register the dataset to 'this')
  this->DataSetsBoundsInfo.emplace_back(dataset);

  if (maxCellSize == 0)
  {
    maxCellSize = dataset->GetMaxCellSize();
  }
  if (maxCellSize > this->Weights.size())
  {
    this->Weights.resize(maxCellSize);
  }
}

//------------------------------------------------------------------------------
void vtkCompositeInterpolatedVelocityField::SetLastCellId(vtkIdType c, int dataindex)
{
  if (this->LastCellId != c || this->LastDataSetIndex != dataindex)
  {
    this->LastCellId = c;
    this->LastDataSet = this->DataSetsBoundsInfo[dataindex].DataSet;

    // If the dataset changes, then the cached cell is invalidated. We might as
    // well prefetch the cached cell either way.
    if (this->LastCellId != -1)
    {
      this->LastDataSet->GetCell(this->LastCellId, this->CurrentCell);
    }

    this->LastDataSetIndex = dataindex;
  }
}

//------------------------------------------------------------------------------
int vtkCompositeInterpolatedVelocityField::FunctionValues(double* x, double* f)
{
  vtkDataSet* ds;
  if (!this->LastDataSet && !this->DataSetsBoundsInfo.empty())
  {
    ds = this->DataSetsBoundsInfo[0].DataSet;
    this->LastDataSet = ds;
    this->LastDataSetIndex = 0;
  }
  else
  {
    ds = this->LastDataSet;
  }

  // Use the superclass's method first as it is faster.
  int retVal = this->FunctionValues(ds, x, f);

  if (!retVal)
  {
    this->CacheDataSetMiss++;
    // Okay need to check other datasets since we are outside the current dataset.
    const int datasetsInfoSize = static_cast<int>(this->DataSetsBoundsInfo.size());
    static const double delta[3] = { 0.0, 0.0, 0.0 };
    for (this->LastDataSetIndex = 0; this->LastDataSetIndex < datasetsInfoSize;
         ++this->LastDataSetIndex)
    {
      ds = this->DataSetsBoundsInfo[this->LastDataSetIndex].DataSet;
      if (ds && ds->GetNumberOfPoints() > 0 && ds != this->LastDataSet)
      {
        this->ClearLastCellId();
        const auto& bounds = this->DataSetsBoundsInfo[this->LastDataSetIndex].Bounds;
        retVal = vtkMath::PointIsWithinBounds(x, bounds.data(), delta);
        if (retVal)
        {
          retVal = this->FunctionValues(ds, x, f);
          if (retVal)
          {
            this->LastDataSet = ds;
            return retVal;
          }
        }
      }
    }
    this->LastCellId = -1;
    this->LastDataSetIndex = 0;
    this->LastDataSet = this->DataSetsBoundsInfo[0].DataSet;
    return 0;
  }
  else
  {
    this->CacheDataSetHit++;
  }

  return retVal;
}

//------------------------------------------------------------------------------
int vtkCompositeInterpolatedVelocityField::InsideTest(double* x)
{
  vtkDataSet* ds;
  if (!this->LastDataSet && !this->DataSetsBoundsInfo.empty())
  {
    ds = this->DataSetsBoundsInfo[0].DataSet;
    this->LastDataSet = ds;
    this->LastDataSetIndex = 0;
  }
  else
  {
    ds = this->LastDataSet;
  }

  // Use the superclass's method first as it is faster.
  auto strategy = this->GetDataSetInfo(ds)->Strategy;
  int retVal = this->FindAndUpdateCell(ds, strategy, x);

  if (!retVal)
  {
    this->CacheDataSetMiss++;
    // Okay need to check other datasets since we are outside the current dataset.
    const int datasetsInfoSize = static_cast<int>(this->DataSetsBoundsInfo.size());
    static const double delta[3] = { 0.0, 0.0, 0.0 };
    for (this->LastDataSetIndex = 0; this->LastDataSetIndex < datasetsInfoSize;
         this->LastDataSetIndex++)
    {
      ds = this->DataSetsBoundsInfo[this->LastDataSetIndex].DataSet;
      if (ds && ds->GetNumberOfPoints() > 0 && ds != this->LastDataSet)
      {
        this->ClearLastCellId();
        const auto& bounds = this->DataSetsBoundsInfo[this->LastDataSetIndex].Bounds;
        retVal = vtkMath::PointIsWithinBounds(x, bounds.data(), delta);
        if (retVal)
        {
          strategy = this->GetDataSetInfo(ds)->Strategy;
          retVal = this->FindAndUpdateCell(ds, strategy, x);
          if (retVal)
          {
            this->LastDataSet = ds;
            return retVal;
          }
        }
      }
    }
    this->LastCellId = -1;
    this->LastDataSetIndex = 0;
    this->LastDataSet = this->DataSetsBoundsInfo[0].DataSet;
    return 0;
  }
  else
  {
    this->CacheDataSetHit++;
  }

  return retVal;
}

//------------------------------------------------------------------------------
int vtkCompositeInterpolatedVelocityField::SnapPointOnCell(double* pOrigin, double* pSnap)
{
  if (this->LastDataSet == nullptr)
  {
    return 0;
  }
  auto datasetInfo = this->GetDataSetInfo(this->LastDataSet);
  // Find the closest cell
  if (!this->FindAndUpdateCell(this->LastDataSet, datasetInfo->Strategy, pOrigin))
  {
    return 0;
  }

  // check cell is a 2D cell
  if (this->CurrentCell->GetCellDimension() != 2)
  {
    // not a 2D cell, do not snap
    return 0;
  }

  this->LastDataSet->GetCellPoints(this->LastCellId, this->PointIds);
  if (this->PointIds->GetNumberOfIds() < 3)
  {
    // Not enough points, ignore the cell and do not snap
    return 0;
  }

  // Recover first three points, this assumes the 2D cell is planar
  double p0[3];
  double p1[3];
  double p2[3];
  this->LastDataSet->GetPoint(this->PointIds->GetId(0), p0);
  this->LastDataSet->GetPoint(this->PointIds->GetId(1), p1);
  this->LastDataSet->GetPoint(this->PointIds->GetId(2), p2);

  // Check points are not aligned
  double vec0[3];
  double vec1[3];
  double vec2[3];
  vtkMath::Subtract(p0, p1, vec0);
  vtkMath::Subtract(p0, p2, vec1);
  vtkMath::Cross(vec0, vec1, vec2);
  if (vtkMathUtilities::FuzzyCompare(vec2[0], 0.) && vtkMathUtilities::FuzzyCompare(vec2[1], 0.) &&
    vtkMathUtilities::FuzzyCompare(vec2[2], 0.))
  {
    // Points are aligned, ignore the cell and do not snap
    return 0;
  }

  double normal[3];
  double projected[3];
  vtkTriangle::ComputeNormal(p0, p1, p2, normal);
  vtkNew<vtkPlane> projectionPlane;
  projectionPlane->SetOrigin(p0);
  projectionPlane->SetNormal(normal);
  projectionPlane->ProjectPoint(pOrigin, projected);

  double dist2;
  if (this->CurrentCell->EvaluatePosition(
        projected, nullptr, this->LastSubId, this->LastPCoords, dist2, this->Weights.data()) == 1)
  {
    // Projection is indeed on the cell, use the closest point found earlier
    pSnap[0] = this->LastClosestPoint[0];
    pSnap[1] = this->LastClosestPoint[1];
    pSnap[2] = this->LastClosestPoint[2];
    return 1;
  }
  return 0;
}

//------------------------------------------------------------------------------
void vtkCompositeInterpolatedVelocityField::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Number of DataSets: " << this->DataSetsBoundsInfo.size() << endl;
  os << indent << "Last Dataset Index: " << this->LastDataSetIndex << endl;
  os << indent << "CacheDataSetHit: " << this->CacheDataSetHit << endl;
  os << indent << "CacheDataSetMiss: " << this->CacheDataSetMiss << endl;
}
VTK_ABI_NAMESPACE_END
