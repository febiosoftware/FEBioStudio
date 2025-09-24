#pragma once
#include <FSCore/FSObject.h>
#include <MeshLib/FSCoreMesh.h>
#include "FEModelComponent.h"

enum MaterialAxesGeneratorType {
	AXES_LOCAL,
	AXES_VECTOR,
	AXES_ANGLES,
	AXES_CYLINDRICAL,
	AXES_SPHERICAL
};

//-----------------------------------------------------------------------------
//! Reference to an element of a mesh
class FEElementRef
{
public:
	FSCoreMesh*	m_pmesh;	// mesh to which this element belongs
	int		m_nelem;	// index of element

	vec3d center() const 
	{
		const FSElement_* pe = m_pmesh->ElementPtr(m_nelem);
		return m_pmesh->ElementCenter(*pe);
	}

	FSElement_* operator -> () {return m_pmesh->ElementPtr(m_nelem);}
	const FSElement_* operator -> () const {return m_pmesh->ElementPtr(m_nelem);}

	operator FSElement_*(){ return m_pmesh->ElementPtr(m_nelem); }
};

//-----------------------------------------------------------------------------
class FSMaterial;
class GMaterial;

//-----------------------------------------------------------------------------
// TODO: I had to wrap this in a material so that I can show it in the 
// MaterialPropsView
class FSAxisMaterial;

//-----------------------------------------------------------------------------
//! FSMaterial is the base class for all materials
//! It essentially collects material parameters (via the FEParamContainer base class) and
//! contains a list of material properties. 
class FSMaterial : public FSModelComponent
{
public:
	FSMaterial(int ntype, FSModel* fem);
	virtual ~FSMaterial();

	// return the material type
	int Type() const { return m_ntype; }

	// return a string for the material type
	const char* GetTypeString() const override;
	void SetTypeString(const std::string& s) override;

	// return the class ID this material belongs to
	int ClassID();

	// copy the material
	virtual void copy(FSMaterial* pmat);

	virtual FSMaterial* Clone();

	// fiber stuff
	virtual bool HasFibers() { return false; }
	virtual vec3d GetFiber(FEElementRef& el) { return vec3d(0,0,0); };
	
	// local material axes
	virtual bool HasMaterialAxes() const;
	virtual mat3d GetMatAxes(FEElementRef& el) const;

	// set the axis material
	virtual void SetAxisMaterial(FSAxisMaterial* Q);

	virtual bool IsRigid();

public:
	FSMaterial* GetMaterialProperty(int propId, int index = 0);

public:
	void SetParentMaterial(FSMaterial* pmat);
	const FSMaterial* GetParentMaterial() const;
	const FSMaterial* GetAncestor() const;

	GMaterial* GetOwner() const;
	void SetOwner(GMaterial* owner);

public:
	void Save(OArchive& ar);
	void Load(IArchive& ar);

protected:
	int			m_ntype;		// material type
	FSMaterial*	m_parent;		// parent material (if this material is a property)
	GMaterial*	m_owner;		// the owner of the material
	
public:
	// local material axes
	FSAxisMaterial*	m_axes;
};

//-----------------------------------------------------------------------------
// The only purpose of this class is to set the superclass ID to FEMATERIALPROPERTY_ID
// Note that there is a different FSMaterialProperty defined below. 
class FSMaterialProp : public FSMaterial
{
public:
	FSMaterialProp(int ntype, FSModel* fem);
};

//-----------------------------------------------------------------------------
// TODO: I had to wrap this in a material so that I can show it in the 
// MaterialPropsView
class FSAxisMaterial : public FSMaterial
{
public:
	int		m_naopt;		// axes option
	int		m_n[3];			// local node numbering
	vec3d	m_a;			// axes vector a
	vec3d	m_d;			// axes vector d
    double  m_theta;        // axes angle theta
    double  m_phi;          // axes angle phi

	// cylindrical/spherical options
	vec3d	m_center;
	vec3d	m_axis;
	vec3d	m_vec;

public:
	FSAxisMaterial(FSModel* fem);

	bool UpdateData(bool bsave) override;

	mat3d GetMatAxes(FEElementRef& el);
};

//======================================================================================
enum FSMaterialPropertyType {
	FE_FEBIO_MATERIAL_PROPERTY = 1
};

class FSMaterialProperty : public FSModelComponent
{
public:
	FSMaterialProperty(FSModel* fem, int ntype);

	int Type() const;

	void Save(OArchive& ar);
	void Load(IArchive& ar);

	virtual bool HasFibers() { return false; }
	virtual vec3d GetFiber(FEElementRef& el) { return vec3d(0, 0, 0); }

private:
	int	m_ntype;
};

class FEBioMaterialProperty : public FSMaterialProperty
{
public:
	FEBioMaterialProperty(FSModel* fem);

	void Save(OArchive& ar);
	void Load(IArchive& ar);

	bool HasFibers() override;
	vec3d GetFiber(FEElementRef& el) override;
};
