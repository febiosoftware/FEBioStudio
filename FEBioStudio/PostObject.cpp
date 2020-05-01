#include "stdafx.h"
#include "PostObject.h"
#include <PostGL/GLModel.h>

CPostObject::CPostObject(Post::CGLModel* glm) : GMeshObject((FEMesh*)nullptr)
{
	// store the model
	m_glm = glm;

	// Set the FE mesh and update
	SetFEMesh(glm->GetFEModel()->GetFEMesh(0));
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
	FEMesh* mesh = GetFEMesh();
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
	Post::FEPostMesh* postMesh = m_glm->GetActiveState()->GetFEMesh();
	postMesh->UpdateBox();

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
			FENode& ns = postMesh->Node(nd.nid);

			nd.r = ns.r;
		}
		//		mesh->Update();
		mesh->UpdateBoundingBox();
		mesh->UpdateNormals();
	}
}
