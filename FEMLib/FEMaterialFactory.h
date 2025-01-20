#pragma once
#include <list>
#include <vector>
#include <string>

class FSMaterial;
class FSModel;

enum MaterialFlags
{	
	TOPLEVEL = 1,		// can be defined as a top-level material. Otherwise, it is a component of another material
};

//-----------------------------------------------------------------------------
// The FEMatDescriptor is a utility class that helps in the dynamic creation
// of material objects.
//
class FEMatDescriptor
{

public:
	FEMatDescriptor(
			int nmodule,
			int ntype, 
			int nclass, 
			const char* szname,
			unsigned int flags);

	virtual ~FEMatDescriptor(){}

	virtual FSMaterial* Create(FSModel* fem) = 0;

	int GetModule() const { return m_nModule; }

	int GetClassID() const { return m_nClass; }

	int GetTypeID() const { return m_nType; }
	const char* GetTypeString() const { return m_szname; }

	unsigned int GetFlags() const { return m_flags; }

protected:
	int		m_nModule;	// Module
	int		m_nType;	// material type
	int		m_nClass;	// material category
	const char*	m_szname;
	unsigned int m_flags;
};

template <typename T> class FEMatDescriptor_T : public FEMatDescriptor
{
public:
	FEMatDescriptor_T(int nmodule, int ntype, int nclass, const char* szname, unsigned int flags) : FEMatDescriptor(nmodule, ntype, nclass, szname, flags) {}
	FSMaterial* Create(FSModel* fem) override { return new T(fem); }
};

typedef std::list<FEMatDescriptor*>::iterator FEMatDescIter;

//-----------------------------------------------------------------------------
class FEMatCategory
{
public:
	FEMatCategory(const std::string& name, int module, int catID) : m_name(name), m_module(module), m_catID(catID) {}
	FEMatCategory(const FEMatCategory& c) : m_name(c.m_name), m_module(c.m_module), m_catID(c.m_catID) {}

	const std::string& GetName() const { return m_name; }
	int GetID() const { return m_catID; }
	int GetModule() const { return m_module; }

private:
	std::string		m_name;
	int				m_catID;
	int				m_module;
};

//-----------------------------------------------------------------------------
// The material factory class manages the available materials. Each material
// must be registered with the material manager in order to take advantage of
// the provided facilities.
//
class FEMaterialFactory
{
public:
	// get an instance of a material class
	static FEMaterialFactory* GetInstance()
	{
		if (m_pFac == 0) m_pFac = new FEMaterialFactory;
		return m_pFac;
	}

	static void RegisterMaterial(FEMatDescriptor* pd);

	int Materials() { return (int)m_Desc.size(); }
	FEMatDescIter FirstMaterial() { return m_Desc.begin(); }

	// create a material from its ID
	static FSMaterial* Create(FSModel* fem, int nid);

	// create a material from its name
	static FSMaterial* Create(FSModel* fem, const char* szname, int classId = -1);

	// return the type string of the material
    static const char* TypeStr(int nid);
	static const char* TypeStr(const FSMaterial* pm);

	// return the class ID of the material
	static int ClassID(FSMaterial* pm);

	// return a list of material types for a given material class
	static std::list<FEMatDescriptor*> Enumerate(int matClass);

	static void AddCategory(const std::string& name, int module, int catID);

	static int Categories();

	static FEMatCategory& GetCategory(int i);

public:
	FEMatDescriptor* Find(int nid);
	FEMatDescriptor* Find(const char* szname, int superClassId = -1);
	FEMatDescriptor* AtIndex(int index);

private:
	FEMaterialFactory(void);
	~FEMaterialFactory(void);

protected:
	static FEMaterialFactory*	m_pFac;

	std::list<FEMatDescriptor*>		m_Desc;
	std::vector<FEMatCategory>		m_Cat;
};

//-----------------------------------------------------------------------------

class FERegisterMaterial
{
public:
	FERegisterMaterial(FEMatDescriptor* pd)
	{
		FEMaterialFactory::RegisterMaterial(pd);
	}
};

//-----------------------------------------------------------------------------
// The following macros should be used to register a material with the material factory.
// To register a material, take the following steps.
// 1) add the DECLARE_REGISTERED macro to the material class declaration
// 2) add the REGISTER_MATERIAL macro to the material class definition

// The DECLARE_REGISTERED macro sets up the mechanism to do the material registration
#define DECLARE_REGISTERED(theClass) \
public: \
	static FERegisterMaterial	m_##theClass##_rm;

// the REGISTER_MATERIAL does the actual material registration
#define REGISTER_MATERIAL(theClass, theModule, theType, theCategory, theName, theFlags) \
	FERegisterMaterial theClass::m_##theClass##_rm(new FEMatDescriptor_T<theClass>(theModule, theType, theCategory, theName, theFlags));
