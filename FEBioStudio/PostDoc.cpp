#include "stdafx.h"
#include "PostDoc.h"
#include <XPLTLib/xpltFileReader.h>
#include <PostViewLib/FEModel.h>
#include <PostViewLib/GLContext.h>
#include <PostViewLib/GLCamera.h>
#include <PostGL/GLModel.h>
#include "GLView.h"
#include "Document.h"

class CPostDoc::Imp 
{
public:
	Imp() : fem(nullptr), glm(nullptr) {}
	~Imp() { delete fem; delete glm; }

	void clear()
	{
		delete glm; glm = nullptr;
		delete fem; fem = nullptr;
	}

public:
	Post::CGLModel*	glm;
	Post::FEModel*	fem;
};

CPostDoc::CPostDoc() : imp(new CPostDoc::Imp)
{
}

CPostDoc::~CPostDoc()
{
	delete imp;
}

int CPostDoc::GetStates()
{
	assert(imp->fem);
	return imp->fem->GetStates();
}

Post::FEModel* CPostDoc::GetFEModel()
{
	return imp->fem;
}

void CPostDoc::SetActiveState(int n)
{
	assert(imp->glm);
	imp->glm->setCurrentTimeIndex(n);
	imp->glm->Update(false);
}

void CPostDoc::SetDataField(int n)
{
	imp->glm->GetColorMap()->SetEvalField(n);
	imp->glm->GetColorMap()->Activate(true);
	imp->glm->Update(false);
}

bool CPostDoc::Load(const std::string& fileName)
{
	const char* szfile = fileName.c_str();

	// clear the post doc
	imp->clear();

	// create new FE model
	imp->fem = new Post::FEModel;

	xpltFileReader xplt;

	if (xplt.Load(*imp->fem, szfile) == false)
	{
		delete imp->fem;
		imp->fem = nullptr;
		return false;
	}

	// create new GLmodel
	imp->glm = new Post::CGLModel(imp->fem);

	return true;
}

vec3f to_vec3f(const vec3d& r)
{
	return vec3f((float)r.x, (float)r.y, (float)r.z);
}

quat4f to_quat4f(const quatd& q)
{
	return quat4f((float)q.x, (float)q.y, (float) q.z, (float) q.w);
}

void CPostDoc::Render(CGLView* view)
{
	CGLCamera cam = view->GetCamera();

	// convert PreView camera to PostView camera
	Post::CGLCamera glcam;
	glcam.SetTarget(to_vec3f(cam.Position()));
	glcam.SetLocalTarget(to_vec3f(cam.Target()));
	glcam.SetOrientation(to_quat4f(cam.GetOrientation()));
	glcam.UpdatePosition(true);

	VIEW_SETTINGS& vs = view->GetDocument()->GetViewSettings();

	Post::CGLContext rc;
	rc.m_cam = &glcam;
	rc.m_showOutline = vs.m_bfeat;
	rc.m_showMesh = vs.m_bmesh;

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glcam.Transform();

	imp->glm->Render(rc);

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}
