#include "stdafx.h"
#include "GImageObject.h"
#include <qopengl.h>
#include <PostLib/ImageModel.h>
#include <PostLib/3DImage.h>
#include "GLView.h"
#include <PostLib/GLCamera.h>
#include <PostLib/GLContext.h>
#include <PostLib/ImageModel.h>
#include <PostLib/GLImageRenderer.h>
#include "Document.h"

GImageObject::GImageObject()
{
	m_im = nullptr;

	AddDoubleParam(0, "X0", "X0");
	AddDoubleParam(0, "Y0", "Y0");
	AddDoubleParam(0, "Z0", "Z0");

	AddDoubleParam(1, "X1", "X1");
	AddDoubleParam(1, "Y1", "Y1");
	AddDoubleParam(1, "Z1", "Z1");
}

GImageObject::~GImageObject()
{
	delete m_im;
}

bool GImageObject::LoadImageData(const std::string& fileName, int nx, int ny, int nz, BOX box)
{
	Post::C3DImage* im3d = new Post::C3DImage;
	if (im3d->Create(nx, ny, nz) == false)
	{
		delete im3d;
		return false;
	}

	if (im3d->LoadFromFile(fileName.c_str(), 8) == false)
	{
		delete im3d;
		return false;
	}

	// not sure why, but I need to flip the stack
	im3d->FlipZ();

	if (m_im) delete m_im;
	BOUNDINGBOX bbox(box.x0, box.y0, box.z0, box.x1, box.y1, box.z1);
	m_im = new Post::CImageModel(nullptr);
	m_im->Set3DImage(im3d, bbox);

	Update();

	return true;
}

void GImageObject::SetBox(BOX box)
{
	SetFloatValue(0, box.x0);
	SetFloatValue(1, box.y0);
	SetFloatValue(2, box.z0);
	SetFloatValue(3, box.x1);
	SetFloatValue(4, box.y1);
	SetFloatValue(5, box.z1);

	if (m_im)
	{
		BOUNDINGBOX bbox(box.x0, box.y0, box.z0, box.x1, box.y1, box.z1);
		m_im->SetBoundingBox(bbox);
		Update();
	}
}

// get the bounding box
BOX GImageObject::GetBox() const
{
	BOX b;
	b.x0 = GetFloatValue(0);
	b.y0 = GetFloatValue(1);
	b.z0 = GetFloatValue(2);
	b.x1 = GetFloatValue(3);
	b.y1 = GetFloatValue(4);
	b.z1 = GetFloatValue(5);
	return b;
}

void GImageObject::Update()
{
	if (m_im == nullptr) return;
}

// get the image model
Post::CImageModel* GImageObject::GetImageModel()
{
	return m_im;
}

// in PostDoc.cpp
extern vec3f to_vec3f(const vec3d& r);
extern quatd to_quat4f(const quatd& q);

// render the image data
void GImageObject::Render(CGLView* view)
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
	rc.m_q = glcam.GetOrientation();

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glcam.Transform();

	// render the volume image data if present
	Post::CImageModel* img = m_im;
	if (img->IsActive())
	{
		for (int j = 0; j < img->ImageRenderers(); ++j)
		{
			Post::CGLImageRenderer* pir = img->GetImageRenderer(j);
			if (pir && pir->IsActive())
			{
//				if (pir->AllowClipping()) CGLPlaneCutPlot::EnableClipPlanes();
//				else CGLPlaneCutPlot::DisableClipPlanes();
				pir->Render(rc);
			}
		}
	}

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}
