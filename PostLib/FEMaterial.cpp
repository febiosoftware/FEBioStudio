#include "stdafx.h"
#include "FEMaterial.h"
using namespace Post;

FEMaterial::FEMaterial()
{ 
	m_szname[0] = 0; 
	bclip = true; 
	m_nrender = RENDER_MODE_DEFAULT; 
	m_ntransmode = RENDER_TRANS_CONSTANT; 

	benable = true;
	bvisible = true;
	bmesh = true;
	bcast_shadows = true;

	shininess = 0.f;
	transparency = 1.f;

	diffuse = GLColor(200, 200, 200);
	ambient = GLColor(0,0,0);
	specular = GLColor(0,0,0);
	emission = GLColor(0,0,0);
}

const char* FEMaterial::GetName() { return m_szname; }
void FEMaterial::SetName(const char* szname) { strcpy(m_szname, szname); }
