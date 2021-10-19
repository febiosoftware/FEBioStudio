#pragma once
#include <vector>
#include <FSCore/FSObject.h>
//using namespace std;

class FEModel;

class FEClassFactory
{
public:
	FEClassFactory(int module, int superID, int classID, const char* sztype, const char* helpURL);
	~FEClassFactory();

	virtual FSObject* Create(FEModel* fem) = 0;

	int GetModule() const { return m_Module; }
	int GetSuperID() const { return m_SuperID; }
	int GetClassID() const { return m_ClassID; }

	const char* GetTypeStr() const { return m_szType; }
	const char* GetHelpURL() const { return m_helpURL; }

private:
	int	m_Module;		// The module this class belongs to
	int	m_SuperID;		// The super-class (i.e. category) this class belongs to
	int	m_ClassID;		// class ID (must be unique within each super class)
	const char*	m_szType;	// type string
	const char* m_helpURL;	// optional help URL
};

template <class T> class FEClassFactory_T : public FEClassFactory
{
public:
	FEClassFactory_T(int module, int superID, int classID, const char* sztype, const char* helpURL = "") : FEClassFactory(module, superID, classID, sztype, helpURL){}
	FSObject* Create(FEModel* fem) { return new T(fem); }
};

class FEMKernel
{
public:
	static FEMKernel* Instance();

	FSObject* Create(FEModel* fem, int superID, int classID);
	FSObject* Create(FEModel* fem, int superID, const char* szTypeString);

	void RegisterClass(FEClassFactory* fac);

	static vector<FEClassFactory*> FindAllClasses(int module, int superID);
	static FEClassFactory* FindClass(int module, int superID, int classID);

private:
	static FEMKernel*	m_This;
	vector<FEClassFactory*>	m_Class;	

private:
	FEMKernel(){}
	FEMKernel(const FEMKernel&){}
};

#define REGISTER_FE_CLASS(theClass, theModule, theSuperID, theClassID, ...) \
	FEMKernel::Instance()->RegisterClass(new FEClassFactory_T<theClass>(theModule, theSuperID, theClassID, __VA_ARGS__));

template <class T> T* fecore_new(FEModel* fem, int superID, int classID)
{
	return dynamic_cast<T*>(FEMKernel::Instance()->Create(fem, superID, classID));
}

template <class T> T* fecore_new(FEModel* fem, int superID, const char* sztype)
{
	return dynamic_cast<T*>(FEMKernel::Instance()->Create(fem, superID, sztype));
}
