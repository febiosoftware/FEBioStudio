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
#include <list>

//-----------------------------------------------------------------------------
// forward declarations
class FSObject;

//-----------------------------------------------------------------------------
typedef enum {
	CLASS_OBJECT,
	CLASS_CONNECTOR,
	CLASS_MODIFIER,
	CLASS_FEMODIFIER,
	CLASS_MESHER,
	CLASS_SKETCH,
	CLASS_DISCRETE,
	CLASS_SURFACE_MODIFIER,
	CLASS_IMAGE_FILTER,
	CLASS_PLOT,
	CLASS_FILE_READER
} Class_Type;

//-----------------------------------------------------------------------------
// A Class descriptor assists the framework in managing objects. It provides
// facilities for creating objects and displaying their properties in the GUI
class ClassDescriptor
{
public:
	ClassDescriptor(Class_Type ntype, int cid, const char* szname, const char* szres, unsigned int flag = 0);
	virtual ~ClassDescriptor();

	virtual FSObject* CreateInstance() = 0;

	FSObject* Create();

	virtual bool IsType(FSObject* po) = 0;

public:
	const char* GetName() const { return m_szname; }
	const char* GetResourceName() const { return m_szres; }
	Class_Type GetType() const { return m_ntype; }
	int GetCount() const { return m_ncount; }

	void SetFlag(unsigned int n) { m_flag = n; }
	unsigned int Flag() const { return m_flag; }

	void SetClassId(int cid) { m_classId = cid; }
	int GetClassId() const { return m_classId; }

protected:
	Class_Type	m_ntype;	// class type
	const char*	m_szname;	// name of class
	const char*	m_szres;	// resource name
	int			m_ncount;	// count how many objects of this type were created
	int			m_classId;
	unsigned int	m_flag;
};

typedef std::list<ClassDescriptor*>::iterator Class_Iterator;

//-----------------------------------------------------------------------------
template <class theClass> class ClassDescriptor_T : public ClassDescriptor
{
public:
	ClassDescriptor_T(Class_Type ntype, int cid, const char* szname,  const char* szres, unsigned int flag) : ClassDescriptor(ntype, cid, szname, szres, flag){}
	FSObject* CreateInstance() { m_ncount++; return new theClass(); }

	bool IsType(FSObject* po) { return (dynamic_cast<theClass*>(po) != 0); }
};

template <class T> class ClassDescriptorWithCtorArg : public ClassDescriptor
{
public:
	ClassDescriptorWithCtorArg(Class_Type ntype, int cid, const char* szname, const char* szres, unsigned int flag) : ClassDescriptor(ntype, cid, szname, szres, flag) {}

	void SetConstructorArgument(T* arg) { m_arg = arg; }

protected:
	T* m_arg;
};

template <class theClass, class ctorArg> class ClassDescriptor_T2 : public ClassDescriptorWithCtorArg<ctorArg>
{
public:
	ClassDescriptor_T2(Class_Type ntype, int cid, const char* szname, const char* szres, unsigned int flag) : ClassDescriptorWithCtorArg<ctorArg>(ntype, cid, szname, szres, flag) {}

	using ClassDescriptor::m_ncount;
	using ClassDescriptorWithCtorArg<ctorArg>::m_arg;
	
	FSObject* CreateInstance() { m_ncount++; return new theClass(*m_arg); }

	bool IsType(FSObject* po) { return (dynamic_cast<theClass*>(po) != 0); }
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

	static FSObject* CreateClass(Class_Type classType, const char* typeStr);
	static FSObject* CreateClassFromID(Class_Type classType, int cid);

	static ClassDescriptor* FindClassDescriptor(Class_Type classType, const char* typeStr);

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
#define REGISTER_CLASS(theClass, theType, theName, theFlag) \
	RegisterPrvClass _##theClass##_rc(new ClassDescriptor_T<theClass>(theType, -1, theName, 0, theFlag));

#define REGISTER_CLASS2(theClass, theType, theName, theResource, theFlag) \
	RegisterPrvClass _##theClass##_rc(new ClassDescriptor_T<theClass>(theType, -1, theName, theResource, theFlag));

#define REGISTER_CLASS3(theClass, theType, theClassId, theName, theResource, theFlag) \
	RegisterPrvClass _##theClass##_rc(new ClassDescriptor_T<theClass>(theType, theClassId, theName, theResource, theFlag));

#define REGISTER_CLASS4(theClass, theType, theName, theCtorArg) \
	RegisterPrvClass _##theClass##_rc(new ClassDescriptor_T2<theClass, theCtorArg>(theType, -1, theName, 0, 0));

namespace FSCore {

	template <class T> T* CreateClass(Class_Type classType, const char* sztype)
	{
		FSObject* po = ClassKernel::CreateClass(classType, sztype);
		if (po)
		{
			T* pt = dynamic_cast<T*>(po);
			if (pt == nullptr)
			{
				delete po;
				return nullptr;
			}
			else return pt;
		}
		else return nullptr;
	}

	template <class T, class A> T* CreateClass(Class_Type classType, const char* sztype, A* ctorArg)
	{
		ClassDescriptorWithCtorArg<A>* pcd = dynamic_cast<ClassDescriptorWithCtorArg<A>*>(ClassKernel::FindClassDescriptor(classType, sztype));
		if (pcd)
		{
			pcd->SetConstructorArgument(ctorArg);
			FSObject* po = pcd->Create();
			if (po)
			{
				T* pt = dynamic_cast<T*>(po);
				if (pt == nullptr)
				{
					delete po;
					return nullptr;
				}
				else return pt;
			}
			else return nullptr;
		}
		else return nullptr;
	}

	template <class T> T* CreateClassFromID(Class_Type classType, int cid)
	{
		FSObject* po = ClassKernel::CreateClassFromID(classType, cid);
		if (po)
		{
			T* pt = dynamic_cast<T*>(po);
			if (pt == nullptr)
			{
				delete po;
				return nullptr;
			}
			else return pt;
		}
		else return nullptr;
	}
}
