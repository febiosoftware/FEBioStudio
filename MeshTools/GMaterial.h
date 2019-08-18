#pragma once
#include "FEObject.h"

//-----------------------------------------------------------------------------
class FEModel;
class FEMaterial;

//-----------------------------------------------------------------------------
// This class defines a material. It stores the material properties (as a 
// FEMaterial class), the material name and other attributes, mostly used for rendering
//
class GMaterial : public FEObject
{
public:
	GMaterial(FEMaterial* pm = 0);
	~GMaterial(void);

public:

	void SetMaterialProperties(FEMaterial* pm);
	FEMaterial* GetMaterialProperties();

	void SetModel(FEModel* ps) { m_ps = ps; }

	int GetID() { return m_nID; }
	void SetID(int nid)
	{
		m_nID = nid;
		m_nref = (nid >= m_nref ? nid+1:m_nref);
	}

	static void ResetRefs() { m_nref = 1; }

	const char* GetFullName();

	void Save(OArchive& ar);
	void Load(IArchive& ar);

	GLCOLOR Ambient() { return m_ambient; }
	GLCOLOR Diffuse() { return m_diffuse; }
	GLCOLOR Emission() { return m_emission; }
	GLCOLOR Specular() { return m_specular; }

	void Ambient(GLCOLOR c) { m_ambient = c; }
	void Diffuse(GLCOLOR c) { m_diffuse = c; }
	void Emission(GLCOLOR c) { m_emission = c; }
	void Specular(GLCOLOR c) { m_specular = c; }
	void AmbientDiffuse(GLCOLOR c) { m_ambient = m_diffuse = c; }

public:
	// appearance
	double	m_shininess;	// shininess factor
	int			m_nrender;		// rendering mode

	int		m_ntag;	// used for I/O

protected:
	GLCOLOR		m_diffuse;	// diffuse color of material
	GLCOLOR		m_ambient;	// ambient color of material
	GLCOLOR		m_specular;	// specular color of material
	GLCOLOR		m_emission;	// emission color of material

protected:
	int			m_nID;	//!< unique material ID
	FEMaterial*	m_pm;	//!< the material properties
	FEModel*	m_ps;	//!< the model to which this material belongs \todo is this being used?
	static	int	m_nref;
};
