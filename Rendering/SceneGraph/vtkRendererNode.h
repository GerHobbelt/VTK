/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkRendererNode.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkRendererNode - vtkViewNode specialized for vtkRenderers
// .SECTION Description
// State storage and graph traversal for vtkRenderer

#ifndef vtkRendererNode_h
#define vtkRendererNode_h

#include "vtkRenderingSceneGraphModule.h" // For export macro
#include "vtkViewNode.h"

class  vtkCollection;

class VTKRENDERINGSCENEGRAPH_EXPORT vtkRendererNode :
  public vtkViewNode
{
public:
  static vtkRendererNode* New();
  vtkTypeMacro(vtkRendererNode, vtkViewNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  //Description:
  //Build containers for our child nodes.
  virtual void Build(bool prepass);

  //Description:
  //Synchronize our state
  virtual void Synchronize(bool prepass);

protected:
  vtkRendererNode();
  ~vtkRendererNode();

  int Size[2];

private:
  vtkRendererNode(const vtkRendererNode&); // Not implemented.
  void operator=(const vtkRendererNode&); // Not implemented.
};

#endif
