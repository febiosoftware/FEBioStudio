#pragma once
#include <vector>
#include <MeshTools/FEObject.h>
using namespace std;

class FEModel;

class FEClassFactory
{
public:
	FEClassFactory(int module, int superID, int classID, const char* sztype);
	~FEClassFactory();

	virtual FEObject* Create(FEModel* fem) = 0;

	int GetModule() const { return m_Module; }
	int GetSuperID() const { return m_SuperID; }
	int GetClassID() const { return m_ClassID; }

	const char* GetTypeStr() const { return m_szType; }

private:
	int	m_Module;		// The module this class belongs to
	int	m_SuperID;		// The super-class (i.e. category) this class belongs to
	int	m_ClassID;		// class ID (must be unique within each super class)
	const char*	m_szType;	// type string
};

template <class T> class FEClassFactory_T : public FEClassFactory
{
public:
	FEClassFactory_T(int module, int superID, int classID, const char* sztype) : FEClassFactory(module, superID, classID, sztype){}
	FEObject* Create(FEModel* fem) { return new T(fem); }
};

class FEMKernel
{
public:
	static FEMKernel* Instance();

	FEObject* Create(FEModel* fem, int superID, int classID);

	void RegisterClass(FEClassFactory* fac);

	static vector<FEClassFactory*> FindAllClasses(int module, int superID);

private:
	static FEMKernel*	m_This;
	vector<FEClassFactory*>	m_Class;	

private:
	FEMKernel(){}
	FEMKernel(const FEMKernel&){}
};

#define REGISTER_FE_CLASS(theClass, theModule, theSuperID, theClassID, theType) \
	FEMKernel::Instance()->RegisterClass(new FEClassFactory_T<theClass>(theModule, theSuperID, theClassID, theType));

template <class T> T* fecore_new(FEModel* fem, int superID, int classID)
{
	return dynamic_cast<T*>(FEMKernel::Instance()->Create(fem, superID, classID));
}
