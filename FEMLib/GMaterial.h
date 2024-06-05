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
#include <FSCore/color.h>
#include <MeshLib/IHasItemList.h>

//-----------------------------------------------------------------------------
class FSModel;
class FSMaterial;
class GPartList;

//-----------------------------------------------------------------------------
// This class defines a material. It stores the material properties (as a 
// FSMaterial class), the material name and other attributes, mostly used for rendering
//
class GMaterial : public FSObject, public IHasItemLists
{
public:
	enum {MAX_COLORS = 16};

public:
	GMaterial(FSMaterial* pm = 0);
	~GMaterial(void);

public:

	void SetMaterialProperties(FSMaterial* pm);
	FSMaterial* GetMaterialProperties();
	FSMaterial* TakeMaterialProperties();

	void SetModel(FSModel* ps) { m_ps = ps; }
	FSModel* GetModel() { return m_ps; }

	int GetID() { return m_nID; }
	void SetID(int nid)
	{
		m_nID = nid;
		m_nref = (nid >= m_nref ? nid+1:m_nref);
	}

	GMaterial* Clone();

	const char* GetFullName();

	void Save(OArchive& ar);
	void Load(IArchive& ar);

	GLColor Ambient() { return m_ambient; }
	GLColor Diffuse() { return m_diffuse; }
	GLColor Emission() { return m_emission; }
	GLColor Specular() { return m_specular; }

	void Ambient(GLColor c) { m_ambient = c; }
	void Diffuse(GLColor c) { m_diffuse = c; }
	void Emission(GLColor c) { m_emission = c; }
	void Specular(GLColor c) { m_specular = c; }
	void AmbientDiffuse(GLColor c) { m_ambient = m_diffuse = c; }

	vec3d GetPosition() const { return m_pos; }

	const char* GetTypeString() const override;

public:	// IHasItemList
	int ItemLists() const override { return 1; }
	FEItemListBuilder* GetItemList(int n = 0) override;
	virtual unsigned int GetMeshItemType() const override;

private:
	void SetItemList(FEItemListBuilder* pi, int n = 0) override;
	virtual void SetMeshItemType(unsigned int meshItem){};

public:
	static void ResetCounter() { m_nref = 1; }
	static void SetCounter(int n) { m_nref = n; }
	static int GetCounter() { return m_nref; }

public:
	// appearance
	double	m_shininess;	// shininess factor
	int			m_nrender;		// rendering mode

	int		m_ntag;	// used for I/O
	vec3d	m_pos;	// location where glyph will be rendered (e.g. for rigid bodies)

protected:
	GLColor		m_diffuse;	// diffuse color of material
	GLColor		m_ambient;	// ambient color of material
	GLColor		m_specular;	// specular color of material
	GLColor		m_emission;	// emission color of material

protected:
	int			m_nID;	//!< unique material ID
	FSMaterial*	m_pm;	//!< the material properties
	FSModel*	m_ps;	//!< the model to which this material belongs \todo is this being used?

private:
	GPartList* m_partList;

	static	int	m_nref;
};
