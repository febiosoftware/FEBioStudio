/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2021 University of Utah, The Trustees of Columbia University in
the City of New York, and others.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

#include "GPrimitive.h"
#include <MeshTools/FEGregoryPatch.h>
#include <MeshTools/FETetGenMesher.h>
#include <MeshLib/FSMesh.h>

//=============================================================================
// GPrimitive
//=============================================================================

// get the editable mesh
FSMeshBase* GPrimitive::GetEditableMesh() { return GetFEMesh(); }
FSLineMesh* GPrimitive::GetEditableLineMesh() { return GetFEMesh(); }

//-----------------------------------------------------------------------------
extern GObject* BuildObject(int ntype);

//-----------------------------------------------------------------------------
GObject* GPrimitive::Clone()
{
	// create a new copy of this primitive
	GObject* po = BuildObject(GetType());
	assert(po);
	if (po == 0) return 0;

	// copy transform
	po->CopyTransform(this);

	// copy color
	po->SetMaterial(GetMaterial());

	// copy parameters
	po->GetParamBlock() = GetParamBlock();

	// update the object
	po->Update();

	return po;
}

//=============================================================================
// GGregoryPatch
//=============================================================================

GGregoryPatch::GGregoryPatch(FSMesh* pm) : GShellPrimitive(GGREGORY_PATCH) 
{ 
	SetFEMesh(pm); 
}


void GGregoryPatch::UpdateMesh()
{
	FEGregoryPatch& m = dynamic_cast<FEGregoryPatch&>(*GetFEMesh());

	// reposition the nodes
	for (int i=0; i<Nodes(); ++i)
	{
		GNode& n = *Node(i);
		FEGregoryPatch::GNode& gn = m.GetNode(i);
		gn.m_r = n.LocalPosition();
	}

	// rebuild the FE mesh
	m.BuildPatchData();
}
