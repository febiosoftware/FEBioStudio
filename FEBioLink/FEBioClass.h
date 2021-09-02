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
#include <string>
#include <map>
#include <QVariant>
#include <MathLib/math3d.h>

namespace FEBio {

	// NOTE: This is an exact copy of the FEParamType enum in FEBio (defined in FEParam.h)!
	//       Make sure that this remains so! 
	enum FEBIO_PARAM_TYPE {
		FEBIO_PARAM_INVALID,
		FEBIO_PARAM_INT,
		FEBIO_PARAM_BOOL,
		FEBIO_PARAM_DOUBLE,
		FEBIO_PARAM_VEC2D,
		FEBIO_PARAM_VEC3D,
		FEBIO_PARAM_MAT3D,
		FEBIO_PARAM_MAT3DS,
		FEBIO_PARAM_IMAGE_3D,
		FEBIO_PARAM_STRING,
		FEBIO_PARAM_DATA_ARRAY,
		FEBIO_PARAM_TENS3DRS,
		FEBIO_PARAM_STD_STRING,
		FEBIO_PARAM_STD_VECTOR_INT,
		FEBIO_PARAM_STD_VECTOR_DOUBLE,
		FEBIO_PARAM_STD_VECTOR_VEC2D,
		FEBIO_PARAM_STD_VECTOR_STRING,
		FEBIO_PARAM_DOUBLE_MAPPED,
		FEBIO_PARAM_VEC3D_MAPPED,
		FEBIO_PARAM_MAT3D_MAPPED,
		FEBIO_PARAM_MAT3DS_MAPPED,
		FEBIO_PARAM_MATERIALPOINT
	};

	struct FEBioClassInfo
	{
		const char* sztype;
		const char* szmod;
		unsigned int	classId;
	};

	enum ClassSearchFlags {
		IncludeModuleDependencies = 0x01,
		IncludeFECoreClasses = 0x02,
		AllFlags = 0xFF
	};

	std::vector<FEBioClassInfo> FindAllClasses(int mod, int superId, int baseClassId = -1, unsigned int flags = ClassSearchFlags::AllFlags);
	int GetClassId(int superClassId, const std::string& typeStr);

	int GetBaseClassIndex(const std::string& baseClassName);

	class FEBioParam
	{
	public:
		FEBioParam() { m_enums = nullptr; m_type = -1; m_flags = 0; m_szunit = nullptr; }
		FEBioParam(const FEBioParam& p)
		{
			m_name = p.m_name;
			m_longName = p.m_longName;
			m_type = p.m_type;
			m_val  = p.m_val;
			m_flags = p.m_flags;
			m_enums = p.m_enums;
			m_szunit = p.m_szunit;
		}
		void operator = (const FEBioParam& p)
		{
			m_name = p.m_name;
			m_longName = p.m_longName;
			m_type = p.m_type;
			m_val = p.m_val;
			m_flags = p.m_flags;
			m_enums = p.m_enums;
			m_szunit = p.m_szunit;
		}

		int type() const { return m_type; }
		const std::string& name() const { return m_name; }
		const std::string& longName() const { return m_longName; }

	public:
		std::string		m_name;
		std::string		m_longName;
		int				m_type;
		const char*		m_enums;	// enum values, only for int parameters
		const char*		m_szunit;
		unsigned int	m_flags;
		QVariant		m_val;
	};

	class FEBioClass;

	class FEBioProperty
	{
	public:
		FEBioProperty() { m_baseClassId = -1; m_superClassId = -1; m_brequired = false; m_isArray = false; }
		FEBioProperty(const FEBioProperty& p)
		{
			m_baseClassId = p.m_baseClassId;
			m_superClassId = p.m_superClassId;
			m_brequired = p.m_brequired;
			m_isArray = p.m_isArray;
			m_name = p.m_name;
			m_comp = p.m_comp;
		}
		void operator = (const FEBioProperty& p)
		{
			m_baseClassId = p.m_baseClassId;
			m_superClassId = p.m_superClassId;
			m_brequired = p.m_brequired;
			m_isArray = p.m_isArray;
			m_name = p.m_name;
			m_comp = p.m_comp;
		}

	public:
		int	m_baseClassId;	// Id that identifies the base class of the property (this is an index!)
		int	m_superClassId;	// Id that identifies the super class (this is an enum!)
		bool	m_brequired;
		bool	m_isArray;
		std::string	m_name;
		std::vector<FEBioClass>	m_comp;
	};

	class FEBioClass
	{
	public:
		FEBioClass() { m_febClass = nullptr; }
		FEBioClass(const FEBioClass& c)
		{
			m_superClassID = c.m_superClassID;
			m_typeString = c.m_typeString;
			m_Param = c.m_Param;
			m_Props = c.m_Props;
			m_febClass = c.m_febClass;
		}
		void operator = (const FEBioClass& c)
		{
			m_superClassID = c.m_superClassID;
			m_typeString = c.m_typeString;
			m_Param = c.m_Param;
			m_Props = c.m_Props;
			m_febClass = c.m_febClass;
		}

	public:
		std::string TypeString() const { return m_typeString; }
		void SetTypeString(const std::string& s) { m_typeString = s; }

		int Parameters() const { return (int)m_Param.size(); }
		FEBioParam& AddParameter(const std::string& paramName, const std::string& paramLongName, int paramType, const QVariant& val);
		FEBioParam& GetParameter(int i) { return m_Param[i]; }

		int Properties() const { return (int)m_Props.size(); }
		FEBioProperty& AddProperty(const std::string& propName, int superClassId, int baseClassId = -1, bool required = false, bool isArray = false);
		FEBioProperty& GetProperty(int i) { return m_Props[i]; }

		void SetSuperClassID(int scid) { m_superClassID = scid; }
		int GetSuperClassID() const { return m_superClassID; }

		void SetFEBioClass(void* febClass) { m_febClass = febClass; }
		void* GetFEBioClass() { return m_febClass; }

	private:
		int				m_superClassID;
		std::string		m_typeString;
		std::vector<FEBioParam>		m_Param;
		std::vector<FEBioProperty>	m_Props;
		void* m_febClass;	// pointer to FEBio class
	};

	FEBioClass* CreateFEBioClass(int classId);

	class FEBioOutputHandler
	{
	public:
		FEBioOutputHandler() {}
		virtual ~FEBioOutputHandler() {}
		virtual void write(const char* sztxt) = 0;
	};

	bool runModel(const std::string& fileName, FEBioOutputHandler* outputHandler = nullptr);
	void TerminateRun();

	const char* GetSuperClassString(int superClassID);

	std::map<int, const char*> GetSuperClassMap();

	vec3d GetMaterialFiber(void* vec3dvaluator);

	void DeleteClass(void* p);
}
