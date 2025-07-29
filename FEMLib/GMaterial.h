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

#pragma once
#include <FSCore/FSObject.h>
#include <GLLib/GLMaterial.h>

class FSMaterial;
class FSModel;

// This class defines a material. It stores the material properties (as a 
// FSMaterial class), the material name and other attributes, mostly used for rendering
class GMaterial : public FSObject
{
public:
	GMaterial(FSMaterial* pm = 0);
	~GMaterial(void);

public:

	void SetMaterialProperties(FSMaterial* pm);
	FSMaterial* GetMaterialProperties();
	FSMaterial* TakeMaterialProperties();

	int GetID() { return m_nID; }
	void SetID(int nid)
	{
		m_nID = nid;
		m_nref = (nid >= m_nref ? nid+1:m_nref);
	}

	void SetModel(FSModel* fem) { m_fem = fem; }
	FSModel* GetModel() { return m_fem; }

	GMaterial* Clone();

	const char* GetFullName() const;

	const char* GetTypeString() const;

	void Save(OArchive& ar);
	void Load(IArchive& ar);

	void SetColor(GLColor c) { m_glmat.AmbientDiffuse(c); }
	GLColor GetColor() const { return m_glmat.diffuse; }
	GLMaterial& GetGLMaterial() { return m_glmat; }

	// location where glyph will be rendered (e.g. for rigid bodies)
	vec3d GetPosition() const { return m_pos; }
	void SetPosition(const vec3d& v) { m_pos = v; }

public:
	static void ResetCounter() { m_nref = 1; }
	static void SetCounter(int n) { m_nref = n; }
	static int GetCounter() { return m_nref; }

public:
	// appearance
	int			m_nrender;		// rendering mode

	int		m_ntag;	// used for I/O
	vec3d	m_pos;	// location where glyph will be rendered (e.g. for rigid bodies)

protected:
	GLMaterial	m_glmat;

protected:
	int			m_nID;	//!< unique material ID
	FSMaterial*	m_pm;	//!< the material properties
	FSModel* m_fem = nullptr;

private:
	static	int	m_nref;
};
