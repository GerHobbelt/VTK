#!/usr/bin/env python
from vtkmodules.vtkCommonCore import (
    vtkDoubleArray,
    vtkLookupTable,
)
from vtkmodules.vtkCommonDataModel import (
    vtkHyperTreeGridNonOrientedCursor,
    vtkUniformHyperTreeGrid,
)
from vtkmodules.vtkFiltersGeneral import vtkShrinkFilter
from vtkmodules.vtkFiltersHyperTree import (
    vtkHyperTreeGridAxisReflection,
    vtkHyperTreeGridDepthLimiter,
    vtkHyperTreeGridGeometry,
    vtkHyperTreeGridThreshold,
)
from vtkmodules.vtkRenderingCore import (
    vtkActor,
    vtkCamera,
    vtkDataSetMapper,
    vtkRenderWindow,
    vtkRenderWindowInteractor,
    vtkRenderer,
)
import vtkmodules.vtkInteractionStyle
import vtkmodules.vtkRenderingFreeType
import vtkmodules.vtkRenderingOpenGL2

htg = vtkUniformHyperTreeGrid()
htg.Initialize()

scalarArray = vtkDoubleArray()
scalarArray.SetName('scalar')
scalarArray.SetNumberOfValues(0)
htg.GetCellData().AddArray(scalarArray)
htg.GetCellData().SetActiveScalars('scalar')

htg.SetDimensions([4, 3, 3])
htg.SetBranchFactor(2)

# Rectilinear grid coordinates
xValues = vtkDoubleArray()
xValues.SetNumberOfValues(4)
xValues.SetValue(0, -1)
xValues.SetValue(1, 0)
xValues.SetValue(2, 1)
xValues.SetValue(3, 2)
htg.SetXCoordinates(xValues);

yValues = vtkDoubleArray()
yValues.SetNumberOfValues(3)
yValues.SetValue(0, -1)
yValues.SetValue(1, 0)
yValues.SetValue(2, 1)
htg.SetYCoordinates(yValues);

zValues = vtkDoubleArray()
zValues.SetNumberOfValues(4)
zValues.SetValue(0, -1)
zValues.SetValue(1, 0)
zValues.SetValue(2, 1)
zValues.SetValue(3, 2)
htg.SetZCoordinates(zValues);

# Let's split the various trees
cursor = vtkHyperTreeGridNonOrientedCursor()
offsetIndex = 0

# ROOT CELL 0-5
for iHT in range(6):
  htg.InitializeNonOrientedCursor(cursor, iHT, True)
  cursor.SetGlobalIndexStart(offsetIndex)
  idx = cursor.GetGlobalNodeIndex()
  scalarArray.InsertTuple1(idx, iHT+1)
  offsetIndex += cursor.GetTree().GetNumberOfVertices()

# ROOT CELL 6
htg.InitializeNonOrientedCursor(cursor, 6, True)
cursor.SetGlobalIndexStart(offsetIndex)
idx = cursor.GetGlobalNodeIndex()
scalarArray.InsertTuple1(idx, 7)
cursor.SubdivideLeaf()

# ROOT CELL 6/[0-7]
for ichild in range(8):
  cursor.ToChild(ichild)
  idx = cursor.GetGlobalNodeIndex()
  scalarArray.InsertTuple1(idx, 13+ichild)
  cursor.ToParent()

offsetIndex += cursor.GetTree().GetNumberOfVertices()

# ROOT CELL 7
htg.InitializeNonOrientedCursor(cursor, 7, True)
cursor.SetGlobalIndexStart(offsetIndex)
idx = cursor.GetGlobalNodeIndex()
scalarArray.InsertTuple1(idx, 8)

offsetIndex += cursor.GetTree().GetNumberOfVertices()

# ROOT CELL 8
htg.InitializeNonOrientedCursor(cursor, 8, True)
cursor.SetGlobalIndexStart(offsetIndex)
idx = cursor.GetGlobalNodeIndex()
scalarArray.InsertTuple1(idx, 9)

cursor.SubdivideLeaf()

# ROOT CELL 8/[0-7]
for ichild in range(8):
  cursor.ToChild(ichild)
  idx = cursor.GetGlobalNodeIndex()
  scalarArray.InsertTuple1(idx, 21+ichild)
  cursor.ToParent()

offsetIndex += cursor.GetTree().GetNumberOfVertices()

# ROOT CELL 9
htg.InitializeNonOrientedCursor(cursor, 9, True)
cursor.SetGlobalIndexStart(offsetIndex)
idx = cursor.GetGlobalNodeIndex()
scalarArray.InsertTuple1(idx, 10)

offsetIndex += cursor.GetTree().GetNumberOfVertices()

# ROOT CELL 10
htg.InitializeNonOrientedCursor(cursor, 10, True)
cursor.SetGlobalIndexStart(offsetIndex)
idx = cursor.GetGlobalNodeIndex()
scalarArray.InsertTuple1(idx, 11)

cursor.SubdivideLeaf()

# ROOT CELL 10/[0-7]
for ichild in range(8):
  cursor.ToChild(ichild)
  idx = cursor.GetGlobalNodeIndex()
  scalarArray.InsertTuple1(idx, 29+ichild)
  cursor.ToParent()

cursor.ToChild(7)
cursor.SubdivideLeaf()
# ROOT CELL 4/3/[0-3]
for ichild in range(8):
  cursor.ToChild(ichild)
  idx = cursor.GetGlobalNodeIndex()
  scalarArray.InsertTuple1(idx, 37+ichild)
  cursor.ToParent()

cursor.ToChild(4)
cursor.SubdivideLeaf()

# ROOT CELL 4/3/0/[0-3]
for ichild in range(8):
  cursor.ToChild(ichild)
  idx = cursor.GetGlobalNodeIndex()
  scalarArray.InsertTuple1(idx, 46+ichild)
  cursor.ToParent()

offsetIndex += cursor.GetTree().GetNumberOfVertices()

# ROOT CELL 11
htg.InitializeNonOrientedCursor(cursor, 11, True)
cursor.SetGlobalIndexStart(offsetIndex)
idx = cursor.GetGlobalNodeIndex()
scalarArray.InsertTuple1(idx, 12)

print('#',scalarArray.GetNumberOfTuples())
print('DataRange: ',scalarArray.GetRange())

isFilter = False

# Depth Limiter Filter
depth = None
if True:
  print('With Depth Limiter Filter (HTG)')
  depth = vtkHyperTreeGridDepthLimiter()
  depth.SetInputData(htg)
  depth.SetDepth(2)
  isFilter = True
else:
  print('Without Depth Limiter Filter (HTG)')
  depth = htg

# Threshold Filter
threshold = None
if True:
  print('With Threshold Filter (HTG)')
  threshold = vtkHyperTreeGridThreshold()
  if isFilter:
    threshold.SetInputConnection(depth.GetOutputPort())
  else:
    threshold.SetInputData(depth)
  threshold.ThresholdBetween(9,9999)
  isFilter = True
else:
  print('No Threshold Filter (HTG)')
  threshold = depth

# Axis reflection
reflection = None
if True:
  print('With AxisReflection Filter (HTG)')
  reflection = vtkHyperTreeGridAxisReflection()
  if isFilter:
    reflection.SetInputConnection(threshold.GetOutputPort())
  else:
    reflection.SetInputData(threshold)
  reflection.SetPlaneToY()
  reflection.SetCenter(0)
  isFilter = True
else:
  print('No AxisReflection Filter (HTG)')
  reflection = threshold

# Geometries
geometry = vtkHyperTreeGridGeometry()
if isFilter:
  geometry.SetInputConnection(reflection.GetOutputPort())
else:
  geometry.SetInputData(reflection)
print('With Geometry Filter (HTG to NS)')

# Shrink Filter
if True:
  print('With Shrink Filter (NS)')
  # In 3D, the shrink shouldn't be done on the geometry because it only represents the skin
  shrink = vtkShrinkFilter()
  shrink.SetInputConnection(geometry.GetOutputPort())
  shrink.SetShrinkFactor(.8)
else:
  print('No Shrink Filter (NS)')
  shrink = geometry

# LookupTable
lut = vtkLookupTable()
lut.SetHueRange(0.66, 0)
lut.UsingLogScale()
lut.Build()

# Mappers
mapper = vtkDataSetMapper()
mapper.SetInputConnection(geometry.GetOutputPort())

mapper.SetLookupTable(lut)
mapper.SetColorModeToMapScalars()
mapper.SetScalarModeToUseCellFieldData()
mapper.SelectColorArray('scalar')
dataRange = [1,53] # Forced for compare with 3DMask
mapper.SetScalarRange(dataRange[0], dataRange[1])

# Actors
actor1 = vtkActor()
actor1.SetMapper(mapper)

actor2 = vtkActor()
actor2.SetMapper(mapper)
actor2.GetProperty().SetColor(0, 0, 0)
actor2.GetProperty().SetRepresentationToWireframe()

# Camera
bd = htg.GetBounds()
camera = vtkCamera()
camera.SetClippingRange(1., 100.)
focal = []
for i in range(3):
  focal.append(bd[ 2 * i ] + (bd[ 2 * i + 1 ] - bd[ 2 * i]) / 2.)
camera.SetFocalPoint(focal)
camera.SetPosition(focal[0]+4, focal[1]+3, focal[2] + 6.)

# Renderer
renderer = vtkRenderer()
renderer.SetActiveCamera(camera)
renderer.AddActor(actor1)
renderer.AddActor(actor2)

# Render window
renWin = vtkRenderWindow()
renWin.AddRenderer(renderer)
renWin.SetSize(600, 400)

# Render window interactor
iren = vtkRenderWindowInteractor()
iren.SetRenderWindow(renWin)

# render the image
renWin.Render()
# iren.Start()

# prevent the tk window from showing up then start the event loop
# --- end of script --
