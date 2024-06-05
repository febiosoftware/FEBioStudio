#pragma once
#include <FSCore/FSObject.h>


//-----------------------------------------------------------------------------
class FSCoreBase;

//-----------------------------------------------------------------------------
//! Class describing a material property
//! A material property is a component of a parent material and is represented by a (list of ) material(s) itself
class FSProperty
{
public:
	enum { NO_FIXED_SIZE = 0 };

	// NOTE: Should match flags defined in FECore\FEProperty.h
	//       These flags are not serialized.
	enum Flags {
//		OPTIONAL   = 0x00, (had to comment out. Causing compilation error for some files.)
		REQUIRED   = 0x01,		// the property is required (default)
		PREFERRED  = 0x02,		// the property is not required, but a default should be allocated when possible.
		REFERENCE  = 0x04,		// references another class in the model
		FIXED      = 0x08,		// fixed properties are fixed type class members
		TOPLEVEL   = 0x10,		// This is a "top-level" property. 
	};

public:
	FSProperty();
	FSProperty(const std::string& name, int propType, FSCoreBase* parent, int nsize = 1, unsigned int flags = 0);
	~FSProperty();

	FSProperty(const FSProperty& p);

	// clears the component list for this property
	void Clear();

	// get the parent
	FSCoreBase* GetParent() { return m_parent; }

	// set the name of the property
	void SetName(const std::string& name);

	// return the name of the property
	const std::string& GetName() const;

	// set the name of the property
	void SetLongName(const std::string& name);

	// return the name of the property
	const std::string& GetLongName();

	// append a component to the component list of this property
	void AddComponent(FSCoreBase* pm);

	// set the component in the property's component list
	void SetComponent(FSCoreBase* pm, int i = 0);

	// remove a component from the list (returns false if pm is not part of the list)
	bool RemoveComponent(FSCoreBase* pm);

	// return a component from the property's component list
	FSCoreBase* GetComponent(int i = 0);
	const FSCoreBase* GetComponent(int i = 0) const;

	// return the index of a component (or -1)
	int GetComponentIndex(FSCoreBase* mat);

	// return the property type
	int GetPropertyType() const { return m_npropType; }

	// return the size of the component list
	int Size() const { return (int)m_cmp.size(); }

	// Set a size (new components will be set to nullptr)
	void SetSize(int newSize);

	// return the max size for this property
	int maxSize() const { return m_maxSize; }

	// return property flags
	unsigned int GetFlags() const { return m_flag; }

	void SetFlags(unsigned int flags) { m_flag = flags; }

	void AddFlag(unsigned int flag) { m_flag |= flag; }

	void SetSuperClassID(int superClassID) { m_nsuperClassID = superClassID; }
	int GetSuperClassID() const { return m_nsuperClassID; }

//	void SetBaseClassID(int baseID) { m_nbaseClassID = baseID; }
//	int GetBaseClassID() const { return m_nbaseClassID; }

	const std::string& GetDefaultType() { return m_defaultType; }
	void SetDefaultType(const std::string& s) { m_defaultType = s; }

	bool IsRequired() const;
	bool IsPreferred() const;
	bool IsTopLevel() const;

private:
	std::string			m_name;			// name of this property
	std::string			m_longName;		// decorative name of this property
	std::string			m_defaultType;	// default type string, when type attributed is ommitted.

	int					m_nsuperClassID;// super class ID
	int					m_npropType;	// this is used as the base class ID for FEBio classes

	int					m_maxSize;		// max number of properties (0 for no limit)
	unsigned int		m_flag;			// property flags
	FSCoreBase*			m_parent;		// parent object this class is a property off
	std::vector<FSCoreBase*>	m_cmp;			// list of materials
};

//-----------------------------------------------------------------------------
// Base class for components of an FSModel
// TODO: Should I rename this to something else, like FSAbstractModelComponent or something.
class FSCoreBase : public FSObject
{
public:
	FSCoreBase();

public:
	// get number of properties
	size_t Properties() const;

	// delete all properties
	void ClearProperties();

	// set the class ID
	void SetClassID(int nid);

	// get the class ID
	int GetClassID() const;

	// get a property
	FSProperty& GetProperty(int i);
	const FSProperty& GetProperty(int i) const;

	// get the class of a property
	FSCoreBase* GetProperty(int i, int j);

	// find a property by name
	FSProperty* FindProperty(const std::string& name);
	const FSProperty* FindProperty(const std::string& name) const;

	// find the property by type
	FSProperty* FindProperty(int ntype);

	// find the property by the class
	FSProperty* FindProperty(FSCoreBase* pm);

	// add a property to the material
	FSProperty* AddProperty(const std::string& name, int nClassID, int maxSize = 1, unsigned int flags = 0);

	// add a component to property with index propID
	int AddProperty(int propID, FSCoreBase* pm);

	// add a material to the property with name
	void AddProperty(const std::string& name, FSCoreBase* pm);

	// replace a property of the class
	void ReplaceProperty(int propID, FSCoreBase* pmat, int matID = 0);

	void SetFlags(unsigned int);
	unsigned int Flags() const;

private:
	int		m_classId;				//!< The (FEBio) class ID
	unsigned int	m_flags;
	std::vector<FSProperty*>	m_prop;		//!< list of properties
};
