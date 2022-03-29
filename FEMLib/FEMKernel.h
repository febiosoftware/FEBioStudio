/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in 
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
#include <vector>
#include <FSCore/FSObject.h>

class FSModel;

class FEClassFactory
{
public:
	FEClassFactory(int module, int superID, int classID, const char* sztype);
	~FEClassFactory();

	virtual FSObject* Create(FSModel* fem) = 0;

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
	FSObject* Create(FSModel* fem) { return new T(fem); }
};

class FEMKernel
{
public:
	static FEMKernel* Instance();

	FSObject* Create(FSModel* fem, int superID, int classID);
	FSObject* Create(FSModel* fem, int superID, const char* szTypeString);

	void RegisterClass(FEClassFactory* fac);

	static std::vector<FEClassFactory*> FindAllClasses(int module, int superID);
	static FEClassFactory* FindClass(int module, int superID, int classID);

    const char* TypeStr(int superID, int classID);

private:
	static FEMKernel*	m_This;
	std::vector<FEClassFactory*>	m_Class;

private:
	FEMKernel(){}
	FEMKernel(const FEMKernel&){}
};

#define REGISTER_FE_CLASS(theClass, theModule, theSuperID, theClassID, theTypeString) \
	FEMKernel::Instance()->RegisterClass(new FEClassFactory_T<theClass>(theModule, theSuperID, theClassID, theTypeString));

template <class T> T* fscore_new(FSModel* fem, int superID, int classID)
{
	return dynamic_cast<T*>(FEMKernel::Instance()->Create(fem, superID, classID));
}

template <class T> T* fscore_new(FSModel* fem, int superID, const char* sztype)
{
	return dynamic_cast<T*>(FEMKernel::Instance()->Create(fem, superID, sztype));
}
