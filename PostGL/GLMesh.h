/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in 
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

#pragma once
#include "PostLib/ColorMap.h"
#include <MathLib/math3d.h>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#elif LINUX

#else
#include <Windows.h>
#include <GL/gl.h>
#endif

// rendering flags
#define GLX_FACES	0x01
#define GLX_LINES	0x02
#define GLX_NODES   0x04

class CGLMesh  
{
public:
	class CNode
	{
	public:
		vec3f	m_pos0;	// nodal position at state 0 (use for smoothing groups) 
		vec3f	m_pos;	// nodal position
		int		m_nref;	// reference value (i.e. nodal ID of FE mesh)
		bool	m_bsel;
		float	m_val;	// the value at this node
	};

	class CLine
	{
	public:
		int		m_node[2];
		GLColor	m_col[2];
	};

	class CFace
	{
	public:
		bool HasEdge(int n1, int n2)
		{
			if ((m_node[0] == n1) && (m_node[1] == n2)) return true;
			if ((m_node[1] == n1) && (m_node[0] == n2)) return true;

			if ((m_node[1] == n1) && (m_node[2] == n2)) return true;
			if ((m_node[2] == n1) && (m_node[1] == n2)) return true;

			if ((m_node[2] == n1) && (m_node[3] == n2)) return true;
			if ((m_node[3] == n1) && (m_node[2] == n2)) return true;

			if ((m_node[3] == n1) && (m_node[0] == n2)) return true;
			if ((m_node[0] == n1) && (m_node[3] == n2)) return true;

			return false;
		}

		CFace()
		{
			m_pFace[0] = m_pFace[1] = m_pFace[2] = 0;
		}

		int Nodes() { return (m_node[3]==m_node[2]?3:4); }

	public:
		int		m_node[4];	// indices to nodes
		CFace*	m_pFace[4];	// pointer to neighbour faces

		vec3f	m_VNorm[4];	// normals at nodes
		vec3f	m_FNorm;	// face normal
		vec3f	m_FNorm0;	// normal at state 0 (use for auto smoothing)
		GLColor	m_Col[4];	// nodal colors
		float	m_tex[4];	// nodal 1D-texture coordinates

		int		m_nID;		// id of face
		int		m_nref;		// reference value (i.e. element ID)
		bool	m_bsel;		// selection status
	};

	void SubSample();

public:
	CGLMesh();
	CGLMesh(CGLMesh& mesh);
	virtual ~CGLMesh();
	void CleanUp();

	bool Create(int nodes, int faces, int lines = 0);

	int Nodes() { return m_nNodes; }
	int Faces() { return m_nFaces; }
	int Lines() { return m_nLines; }

	CNode* Node(int i) { return m_pNode + i; }

	CFace* Face(int i) { return m_pFace + i; }
	GLColor* FaceColor(int i) { return m_pFace[i].m_Col; }

	CLine* Line(int i) { return m_pLine + i; }

	void SetColor(Byte r, Byte g, Byte b);

	void Update();

	void Render(int flag, bool bsmooth)
	{
		if (flag & GLX_FACES  ) RenderFaces(bsmooth);
		if (flag & GLX_LINES  ) RenderLines();
		if (flag & GLX_NODES  ) RenderNodes();
	}

	void RenderShadowVolume(vec3f n, double inf);

	bool AutoSmooth() const { return m_bAutoSmooth; }
	void AutoSmooth(bool bauto) { m_bAutoSmooth = bauto; }

	void SetDiffuse(float r, float g, float b, float a) 
	{
		m_diffuse[0] = r; m_diffuse[1] = g; m_diffuse[2] = b; m_diffuse[3] = a;
	}
	void SetAmbient(float r, float g, float b, float a) 
	{
		m_ambient[0] = r; m_ambient[1] = g; m_ambient[2] = b; m_ambient[3] = a;
	}
	void SetSpecular(float r, float g, float b, float a) 
	{
		m_specular[0] = r; m_specular[1] = g; m_specular[2] = b; m_specular[3] = a;
	}
	void SetEmission(float r, float g, float b, float a) 
	{
		m_emission[0] = r; m_emission[1] = g; m_emission[2] = b; m_emission[3] = a;
	}
	void SetShininess(float shine) { m_shininess = (shine > 1 ? 128 : (int)(shine*128)); }
	void SetTransparency(float trans) { m_transparency = trans; }
	void SetDiffuse  (GLColor c) { SetDiffuse  (c.r/255.f, c.g/255.f, c.b/255.f, c.a/255.f); }
	void SetAmbient  (GLColor c) { SetAmbient  (c.r/255.f, c.g/255.f, c.b/255.f, c.a/255.f); }
	void SetSpecular (GLColor c) { SetSpecular (c.r/255.f, c.g/255.f, c.b/255.f, c.a/255.f); }
	void SetEmission (GLColor c) { SetEmission (c.r/255.f, c.g/255.f, c.b/255.f, c.a/255.f); }
	void SetShininess(GLint  s ) { m_shininess = s; } 

	void SetDiffuse  (float a[4]) { m_diffuse [0] = a[0]; m_diffuse [1] = a[1]; m_diffuse [2] = a[2]; m_diffuse [3] = a[3]; }
	void SetAmbient  (float a[4]) { m_ambient [0] = a[0]; m_ambient [1] = a[1]; m_ambient [2] = a[2]; m_ambient [3] = a[3]; }
	void SetSpecular (float a[4]) { m_specular[0] = a[0]; m_specular[1] = a[1]; m_specular[2] = a[2]; m_specular[3] = a[3]; }
	void SetEmission (float a[4]) { m_emission[0] = a[0]; m_emission[1] = a[1]; m_emission[2] = a[2]; m_emission[3] = a[3]; }

	GLfloat* Diffuse  () { return m_diffuse;   }
	GLfloat* Ambient  () { return m_ambient;   }
	GLfloat* Specular () { return m_specular;  }
	GLfloat* Emission () { return m_emission;  }
	GLint    Shininess() { return m_shininess; }
	GLfloat	 Transparency() { return m_transparency; }

	bool Use1DTexture() { return m_bUseTex; }
	void Use1DTexture(bool btex) { m_bUseTex = btex; }

	void Clip(bool bclip) { m_bclip = bclip; }
	bool Clip() { return m_bclip; }

	void Project(vec3f norm, float rot);

	bool IsVisible() { return (m_transparency > 0.f); }

	bool CastShadow() { return m_bshadow; }
	void CastShadow(bool bcast) { m_bshadow = bcast; }

protected:
	void RenderFaces(bool bsmooth);
	void RenderLines();
	void RenderNodes();

	void SetMaterialProps()
	{
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, m_ambient);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, m_specular);
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, m_emission);
		glMateriali(GL_FRONT_AND_BACK, GL_SHININESS, m_shininess);
	}

protected:
	CNode*		m_pNode;	// nodal coordinates array
	int			m_nNodes;	// number of nodes

	CFace*		m_pFace;	// face array
	int			m_nFaces;	// number of faces

	CLine*		m_pLine;	// line array
	int			m_nLines;	// number of lines

	bool		m_bAutoSmooth;	// apply smoothing groups automatically

	// material properties
	GLfloat		m_diffuse[4];	// diffure color
	GLfloat		m_ambient[4];	// ambient color
	GLfloat		m_specular[4];	// specular color
	GLfloat		m_emission[4];	// emission color
	GLint		m_shininess;	// shininess
	GLfloat		m_transparency;	// transparency
	bool		m_bshadow;		// This object casts shadows (or not)

	bool		m_bUseTex;
	bool		m_bclip;		// use clip plane or not
};

typedef CGLMesh::CFace	CGLFace;
typedef CGLMesh::CNode	CGLNode;
typedef CGLMesh::CLine	CGLLine;
