#pragma once
#include <list>

//-----------------------------------------------------------------------------
// forward declarations
class CObject;

//-----------------------------------------------------------------------------
typedef enum {
	CLASS_OBJECT,
	CLASS_CONNECTOR,
	CLASS_MODIFIER,
	CLASS_FEMODIFIER,
	CLASS_MESHER,
	CLASS_SKETCH,
	CLASS_DISCRETE,
	CLASS_SURFACE_MODIFIER
} Class_Type;

//-----------------------------------------------------------------------------
// A Class descriptor assists the framework in managing objects. It provides
// facilities for creating objects and displaying their properties in the GUI
class ClassDescriptor
{
public:
	ClassDescriptor(Class_Type ntype, const char* szname, const char* szres, unsigned int flag = 0);
	virtual ~ClassDescriptor();

	virtual CObject* Create() = 0;

	virtual bool IsType(CObject* po) = 0;

public:
	const char* GetName() const { return m_szname; }
	const char* GetResourceName() const { return m_szres; }
	Class_Type GetType() const { return m_ntype; }
	int GetCount() const { return m_ncount; }

	void SetFlag(unsigned int n) { m_flag = n; }
	unsigned int Flag() const { return m_flag; }

protected:
	Class_Type	m_ntype;	// class type
	const char*	m_szname;	// name of class
	const char*	m_szres;	// resource name
	int			m_ncount;	// count how many objects of this type were created
	unsigned int	m_flag;
};

typedef std::list<ClassDescriptor*>::iterator Class_Iterator;

//-----------------------------------------------------------------------------
template <class theClass> class ClassDescriptor_T : public ClassDescriptor
{
public:
	ClassDescriptor_T(Class_Type ntype, const char* szname,  const char* szres, unsigned int flag) : ClassDescriptor(ntype, szname, szres, flag){}
	CObject* Create() { m_ncount++; return new theClass(); }

	bool IsType(CObject* po) { return (dynamic_cast<theClass*>(po) != 0); }
};

//-----------------------------------------------------------------------------
// Kernel object that manages the class descriptors
class ClassKernel
{
public:
	static ClassKernel* GetInstance();

	static void RegisterClassDescriptor(ClassDescriptor* pcd);

	static Class_Iterator FirstCD();
	static Class_Iterator LastCD ();

private:
	static ClassKernel*	m_pInst;
	
	std::list<ClassDescriptor*>	m_CD;	// list of class descriptors

private:
	ClassKernel(){}
	ClassKernel(const ClassKernel&){}
};

//-----------------------------------------------------------------------------
// Helper class for registering classes with the framework. 
// Create a static variable of this class and it will call the kernel with the class descriptor
class RegisterPrvClass
{
public:
	RegisterPrvClass(ClassDescriptor* pcd) { ClassKernel::RegisterClassDescriptor(pcd); }
};

//-----------------------------------------------------------------------------
// Helper macro for registering a class with the framework.
#define REGISTER_PREVIEW_CLASS(theClass, theType, theName, theFlag) \
	RegisterPrvClass _##theClass##_rc(new ClassDescriptor_T<theClass>(theType, theName, 0, theFlag));

#define REGISTER_PREVIEW_CLASS2(theClass, theType, theName, theResource, theFlag) \
	RegisterPrvClass _##theClass##_rc(new ClassDescriptor_T<theClass>(theType, theName, theResource, theFlag));
