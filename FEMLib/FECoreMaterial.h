#pragma once
#include <FSCore/FSObject.h>
#include <MeshLib/FECoreMesh.h>
#include "FEModelComponent.h"

// material axes generators
#define FE_AXES_LOCAL			0
#define FE_AXES_VECTOR			1
#define FE_AXES_ANGLES          2
#define FE_AXES_CYLINDRICAL		3
#define FE_AXES_SPHERICAL		4

//-----------------------------------------------------------------------------
//! Reference to an element of a mesh
class FEElementRef
{
public:
	FECoreMesh*	m_pmesh;	// mesh to which this element belongs
	int		m_nelem;	// index of element

	vec3d center() 
	{
		FEElement_* pe = m_pmesh->ElementPtr(m_nelem);
		return m_pmesh->ElementCenter(*pe);
	}

	FEElement_* operator -> () {return m_pmesh->ElementPtr(m_nelem);}
	operator FEElement_*(){ return m_pmesh->ElementPtr(m_nelem); }
};

//-----------------------------------------------------------------------------
class FSMaterial;
class GMaterial;

//-----------------------------------------------------------------------------
//! Class describing a material property
//! A material property is a component of a parent material and is represented by a (list of ) material(s) itself
class FSMaterialProperty
{
public:
	enum { NO_FIXED_SIZE = 0 };

	enum Flags {
		EDITABLE = 0x01,			// the property can be edited in the material editor
		NON_EXTENDABLE = 0x02,		// cannot be modified after created in material editor
		REQUIRED = 0x04				// property is required
	};

public:
	FSMaterialProperty();
	FSMaterialProperty(const std::string& name, int nClassID, FSMaterial* parent, int nsize = 1, unsigned int flags = EDITABLE);
	~FSMaterialProperty();

	// clears the material list for this property
	void Clear();

	// set the name of the property
	void SetName(const std::string& name);

	// return the name of the property
	const std::string& GetName();

	// append a material to the material list of this property
	void AddMaterial(FSMaterial* pm);

	// set the material in the property's material list
	void SetMaterial(FSMaterial* pm, int i = 0);

	// remove a material from the list (returns false if pm is not part of the list)
	bool RemoveMaterial(FSMaterial* pm);

	// return a material from the property's material list
	FSMaterial* GetMaterial(int i = 0);

	// return the index of a material (or -1)
	int GetMaterialIndex(FSMaterial* mat);

	// return the material class ID of this property
	int GetClassID() const { return m_nClassID; }

	// return the size of the material list
	int Size() const { return (int) m_mat.size(); }

	// return the max size for this property
	int maxSize() const { return m_maxSize; }

	// return property flags
	unsigned int GetFlags() const { return m_flag; }

	void SetFlags(unsigned int flags) { m_flag = flags; }

	void SetSuperClassID(int superClassID) { m_nsuperClassID = superClassID; }
	int GetSuperClassID() const { return m_nsuperClassID; }

	const std::string& GetDefaultType() { return m_defaultType; }
	void SetDefaultType(const std::string& s) { m_defaultType = s; }

private:
	std::string			m_name;			// name of this property
	std::string			m_defaultType;	// default type string, when type attributed is ommitted.
	int					m_nClassID;		// the material class ID for this property
	int					m_nsuperClassID;// super class ID (used to distinguish between materials and material properties)
	int					m_maxSize;		// max number of properties (0 for no limit)
	unsigned int		m_flag;			// property flags
	FSMaterial*			m_parent;		// parent material this material is a property off
	vector<FSMaterial*>	m_mat;			// list of materials
};

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
	FSMaterial(int ntype);
	virtual ~FSMaterial();

	// return the material type
	int Type() { return m_ntype; }

	// return a string for the material type
	const char* GetTypeString() override;
	void SetTypeString(const char* sz) override;

	// return the class ID this material belongs to
	int ClassID();

	// copy the material
	virtual void copy(FSMaterial* pmat);

	FSMaterial* Clone();

	// fiber stuff
	virtual bool HasFibers() { return false; }
	virtual vec3d GetFiber(FEElementRef& el) { return vec3d(0,0,0); };
	
	// local material axes
	bool HasMaterialAxes() const;
	mat3d GetMatAxes(FEElementRef& el);

	// set the axis material
	virtual void SetAxisMaterial(FSAxisMaterial* Q);

	virtual bool IsRigid();

public:
	// get the number of properties of this material
	int Properties() { return (int) m_Mat.size(); }

	// get a property
	FSMaterialProperty& GetProperty(int i) { return *m_Mat[i]; }

	// find a property by name
	FSMaterialProperty* FindProperty(const std::string& name);

	// find the property by type
	FSMaterialProperty* FindProperty(int ntype);

	// find the property by the material
	FSMaterialProperty* FindProperty(FSMaterial* pm);

	// add a property to the material
	FSMaterialProperty* AddProperty(const std::string& name, int nClassID, int maxSize = 1, unsigned int flags = FSMaterialProperty::EDITABLE);

	// add a material to property with index propID
	int AddProperty(int propID, FSMaterial* pm);

	// add a material to the property with name
	void AddProperty(const std::string& name, FSMaterial* pm);

	// replace a property of the material
	void ReplaceProperty(int propID, FSMaterial* pmat, int matID = 0);
    
	// delete all properties
	void ClearProperties();

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

protected:
	vector<FSMaterialProperty*>	m_Mat;	//!< list of material properties
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
	FSAxisMaterial();

	bool UpdateData(bool bsave) override;

	mat3d GetMatAxes(FEElementRef& el);
};
