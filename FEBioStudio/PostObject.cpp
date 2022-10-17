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

#include "stdafx.h"
#include "PostObject.h"
#include <PostGL/GLModel.h>

CPostObject::CPostObject(Post::CGLModel* glm) : GMeshObject((FSMesh*)nullptr)
{
	// store the model
	m_glm = glm;

	// Set the FE mesh and update
	SetFEMesh(glm->GetFSModel()->GetFEMesh(0));
	Update(true);
}

CPostObject::~CPostObject()
{
	// The mesh is owned by the CGLModel
	// so we have to set the mesh to zero here, otherwise the GObject baseclass will try to 
	// delete it as well
	SetFEMesh(nullptr);
}

BOX CPostObject::GetBoundingBox()
{
	FSMesh* mesh = GetFEMesh();
	if (mesh) return mesh->GetBoundingBox();
	else return BOX();
}

// is called whenever the selection has changed
void CPostObject::UpdateSelection()
{
	m_glm->UpdateSelectionLists();
}

void CPostObject::UpdateMesh()
{
	Post::FEState* state = m_glm->GetActiveState();
	if (state == nullptr) return;
	Post::FEPostMesh* postMesh = state->GetFEMesh();
	if (postMesh == nullptr) return;

	postMesh->UpdateBoundingBox();

	if (GetFEMesh() != postMesh)
	{
		SetFEMesh(postMesh);
		BuildGMesh();
	}
	else
	{
		GLMesh* mesh = GetRenderMesh(); assert(mesh);

		for (int i = 0; i < mesh->Nodes(); ++i)
		{
			GMesh::NODE& nd = mesh->Node(i);
			FSNode& ns = postMesh->Node(nd.nid);

			nd.r = ns.r;
		}
		//		mesh->Update();
		mesh->UpdateBoundingBox();
		mesh->UpdateNormals();
	}
}
