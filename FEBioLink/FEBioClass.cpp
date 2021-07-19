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
#include "FEBioClass.h"
#include "FEBioModule.h"
#include <FECore/FECoreKernel.h>
#include <FECore/FECoreBase.h>
#include <FECore/FEModelParam.h>
#include <FEBioLib/FEBioModel.h>
using namespace FEBio;



std::map<int, const char*> idmap;
void initMap()
{
	idmap.clear();

	idmap[FEINVALID_ID] = "FEINVALID_ID";
	idmap[FEOBJECT_ID] = "FEOBJECT_ID";
	idmap[FETASK_ID] = "FETASK_ID";
	idmap[FESOLVER_ID] = "FESOLVER_ID";
	idmap[FEMATERIAL_ID] = "FEMATERIAL_ID";
	idmap[FEMATERIALPROP_ID] = "FEMATERIALPROP_ID";
	idmap[FEBODYLOAD_ID] = "FEBODYLOAD_ID";
	idmap[FESURFACELOAD_ID] = "FESURFACELOAD_ID";
	idmap[FEEDGELOAD_ID] = "FEEDGELOAD_ID";
	idmap[FENODALLOAD_ID] = "FENODALLOAD_ID";
	idmap[FENLCONSTRAINT_ID] = "FENLCONSTRAINT_ID";
	idmap[FEPLOTDATA_ID] = "FEPLOTDATA_ID";
	idmap[FEANALYSIS_ID] = "FEANALYSIS_ID";
	idmap[FESURFACEPAIRINTERACTION_ID] = "FESURFACEPAIRINTERACTION_ID";
	idmap[FENODELOGDATA_ID] = "FENODELOGDATA_ID";
	idmap[FEFACELOGDATA_ID] = "FEFACELOGDATA_ID";
	idmap[FEELEMLOGDATA_ID] = "FEELEMLOGDATA_ID";
	idmap[FEOBJLOGDATA_ID] = "FEOBJLOGDATA_ID";
	idmap[FEBC_ID] = "FEBC_ID";
	idmap[FEGLOBALDATA_ID] = "FEGLOBALDATA_ID";
	idmap[FERIGIDOBJECT_ID] = "FERIGIDOBJECT_ID";
	idmap[FENLCLOGDATA_ID] = "FENLCLOGDATA_ID";
	idmap[FECALLBACK_ID] = "FECALLBACK_ID";
	idmap[FEDOMAIN_ID] = "FEDOMAIN_ID";
	idmap[FEIC_ID] = "FEIC_ID";
	idmap[FEDATAGENERATOR_ID] = "FEDATAGENERATOR_ID";
	idmap[FELOADCONTROLLER_ID] = "FELOADCONTROLLER_ID";
	idmap[FEMODEL_ID] = "FEMODEL_ID";
	idmap[FEMODELDATA_ID] = "FEMODELDATA_ID";
	idmap[FESCALARGENERATOR_ID] = "FESCALARGENERATOR_ID";
	idmap[FEVECTORGENERATOR_ID] = "FEVECTORGENERATOR_ID";
	idmap[FEMAT3DGENERATOR_ID] = "FEMAT3DGENERATOR_ID";
	idmap[FEMAT3DSGENERATOR_ID] = "FEMAT3DSGENERATOR_ID";
	idmap[FEFUNCTION1D_ID] = "FEFUNCTION1D_ID";
	idmap[FELINEARSOLVER_ID] = "FELINEARSOLVER_ID";
	idmap[FEMESHADAPTOR_ID] = "FEMESHADAPTOR_ID";
	idmap[FEMESHADAPTORCRITERION_ID] = "FEMESHADAPTORCRITERION_ID";
	idmap[FERIGIDBC_ID] = "FERIGIDBC_ID";
	idmap[FERIGIDLOAD_ID] = "FERIGIDLOAD_ID";
	idmap[FENEWTONSTRATEGY_ID] = "FENEWTONSTRATEGY_ID";
	idmap[FEITEMLIST_ID] = "FEITEMLIST_ID";
	idmap[FETIMECONTROLLER_ID] = "FETIMECONTROLLER_ID";
	idmap[FEEIGENSOLVER_ID] = "FEEIGENSOLVER_ID";
	idmap[FESURFACEPAIRINTERACTIONNL_ID] = "FESURFACEPAIRINTERACTIONNL_ID";
	idmap[FEDATARECORD_ID] = "FEDATARECORD_ID";
}

// dummy model used for allocating temporary FEBio classes.
static FEBioModel febioModel;

static std::map<std::string, int> classIndex;

int baseClassIndex(const char* sz)
{
	if (sz == nullptr) return -1;

	int n = -1;
	auto it = classIndex.find(sz);
	if (it == classIndex.end())
	{
		n = classIndex.size();
		classIndex[sz] = n;
	}
	else n = classIndex[sz];

	return n;
}

int FEBio::GetBaseClassIndex(const std::string& baseClassName)
{
	return baseClassIndex(baseClassName.c_str());
}

bool in_vector(const vector<int>& v, int n)
{
	for (int j = 0; j < v.size(); ++j)
	{
		if (v[j] == n) return true;
	}
	return false;
}

std::vector<FEBio::FEBioClassInfo> FEBio::FindAllClasses(int mod, int superId, int baseClassId, unsigned int flags)
{
	vector<FEBio::FEBioClassInfo> facs;

	bool includeModuleDependencies = (flags & ClassSearchFlags::IncludeModuleDependencies);
	bool includeFECoreClasses      = (flags & ClassSearchFlags::IncludeFECoreClasses);

	FECoreKernel& fecore = FECoreKernel::GetInstance();
	vector<int> mods;
	if ((mod != -1) && includeModuleDependencies)
	{
		mods = fecore.GetModuleDependencies(mod - 1);
	}
	if ((mod != -1) && includeFECoreClasses) 
	{
		mods.push_back(0);
	}

	for (int i = 0; i < fecore.FactoryClasses(); ++i)
	{
		const FECoreFactory* fac = fecore.GetFactoryClass(i);
		int facmod = fac->GetModuleID();

		int baseId = baseClassIndex(fac->GetBaseClassName());

		if (((superId     == -1) || (fac->GetSuperClassID() == superId)) && 
			((baseClassId == -1) || (baseId == baseClassId)))
		{
			if ((mod == -1) || (mod == facmod) || in_vector(mods, facmod))
			{
				const char* szmod = fecore.GetModuleName(fac->GetModuleID() - 1);
				FEBio::FEBioClassInfo febc = { fac->GetTypeStr(), szmod, (unsigned int) i };
				facs.push_back(febc);
			}
		}
	}

	return facs;
}

int FEBio::GetClassId(int superClassId, const std::string& typeStr)
{
	FECoreKernel& fecore = FECoreKernel::GetInstance();
	return fecore.GetFactoryIndex(superClassId, typeStr.c_str());
}

FEBioParam& FEBioClass::AddParameter(const std::string& paramName, int paramType, const QVariant& val)
{
	FEBioParam p;
	p.m_type = paramType;
	p.m_name = paramName;
	p.m_val  = val;
	m_Param.push_back(p);
	return m_Param[m_Param.size() - 1];
}

void FEBioClass::AddProperty(const std::string& propName, int superClassId, int baseClassId, bool required)
{
	FEBioProperty prop;
	prop.m_name = propName;
	prop.m_brequired = required;
	prop.m_baseClassId = baseClassId;
	prop.m_superClassId = superClassId;
	m_Props.push_back(prop);
}

QVariant vec3d_to_qvariant(const vec3d& v)
{
	QList<QVariant> val;
	val.push_back(v.x);
	val.push_back(v.y);
	val.push_back(v.z);
	return val;
}

QVariant mat3d_to_qvariant(const mat3d& m)
{
	QList<QVariant> val;
	for (int i = 0; i < 3; ++i)
		for (int j = 0; j < 3; ++j) val.push_back(m[i][j]);
	return val;
}

QVariant mat3ds_to_qvariant(const mat3ds& m)
{
	QList<QVariant> val;
	val.push_back(m.xx());
	val.push_back(m.yy());
	val.push_back(m.zz());
	val.push_back(m.xy());
	val.push_back(m.yz());
	val.push_back(m.xz());
	return val;
}

FEBioClass* FEBio::CreateFEBioClass(int classId)
{
	// Get the kernel
	FECoreKernel& fecore = FECoreKernel::GetInstance();

	// find the factory
	const FECoreFactory* fac = fecore.GetFactoryClass(classId);
	if (fac == nullptr) return nullptr;

	// try to create a temporary FEBio object
	unique_ptr<FECoreBase> pc(fac->Create(&febioModel)); assert(pc);
	if (pc == nullptr) return nullptr;

	const char* sztype = fac->GetTypeStr();

	// create the interface class
	FEBioClass* feb = new FEBioClass;
	feb->SetSuperClassID(fac->GetSuperClassID());
	feb->SetTypeString(sztype);

	// copy parameter info
	FEParameterList& pl = pc->GetParameterList();
	int params = pl.Parameters();
	FEParamIterator it = pl.first();
	for (int i = 0; i < params; ++i, ++it)
	{
		FEParam& p = *it;
		if ((p.GetFlags() & FE_PARAM_HIDDEN) == 0)
		{
			int ndim = p.dim();
			switch (p.type())
			{
			case FE_PARAM_INT:
			{
				FEBioParam& param = feb->AddParameter(p.name(), p.type(), p.value<int>());
				param.m_flags = p.GetFlags();
				if (p.enums())
				{
					param.m_enums = p.enums();
				}
			}
			break;
			case FE_PARAM_BOOL: feb->AddParameter(p.name(), p.type(), p.value<bool>()); break;
			case FE_PARAM_DOUBLE:
			{
				if (ndim == 1)
					feb->AddParameter(p.name(), p.type(), p.value<double>());
				else if (ndim == 3)
				{
					vec3d v(0, 0, 0);
					v.x = p.value<double>(0);
					v.y = p.value<double>(1);
					v.z = p.value<double>(2);
					feb->AddParameter(p.name(), FE_PARAM_VEC3D, vec3d_to_qvariant(v));
				}
				else if (ndim > 3)
				{
					vector<double> v(ndim);
					for (int i = 0; i < ndim; ++i) v[i] = p.value<double>(i);
					feb->AddParameter(p.name(), FE_PARAM_STD_VECTOR_DOUBLE, QVariant::fromValue(v));
				}
			}
			break;
			case FE_PARAM_VEC3D: feb->AddParameter(p.name(), p.type(), vec3d_to_qvariant(p.value<vec3d>())); break;
			case FE_PARAM_MAT3D: feb->AddParameter(p.name(), p.type(), mat3d_to_qvariant(p.value<mat3d>())); break;
			case FE_PARAM_STD_STRING: feb->AddParameter(p.name(), p.type(), QString::fromStdString(p.value<std::string>())); break;
			case FE_PARAM_DOUBLE_MAPPED:
			{
				if (ndim == 1)
				{
					FEParamDouble& v = p.value<FEParamDouble>();
					feb->AddParameter(p.name(), p.type(), v.constValue());
				}
				else if (ndim == 3)
				{
					vec3d v(0, 0, 0);
					v.x = p.value<FEParamDouble>(0).constValue();
					v.y = p.value<FEParamDouble>(1).constValue();
					v.z = p.value<FEParamDouble>(2).constValue();
					feb->AddParameter(p.name(), FE_PARAM_VEC3D, vec3d_to_qvariant(v));
				}
				else assert(false);
			}
			break;
			case FE_PARAM_VEC3D_MAPPED:
			{
				vec3d v(0, 0, 0); // TODO: Grab const value from FEParamVec3d
				QVariant val = vec3d_to_qvariant(v);
				feb->AddParameter(p.name(), p.type(), val);
			}
			break;
			case FE_PARAM_MAT3D_MAPPED:
			{
				if (strcmp(p.name(), "mat_axis") != 0)
				{
					FEParamMat3d& v = p.value<FEParamMat3d>();
					mat3d M = v.constValue();
					QVariant val = mat3d_to_qvariant(M);
					feb->AddParameter(p.name(), p.type(), val);
				}
			}
			break;
			case FE_PARAM_MAT3DS_MAPPED:
			{
				mat3ds M; M.unit(); // TODO: Grab const value from FEParamMat3d
				QVariant val = mat3ds_to_qvariant(M);
				feb->AddParameter(p.name(), p.type(), val);
			}
			break;
			case FEBIO_PARAM_STD_VECTOR_INT:
			{
				std::vector<int>& v = p.value<std::vector<int> >();
				QVariant val = QVariant::fromValue(v);
				FEBioParam& param = feb->AddParameter(p.name(), p.type(), val);
				if (p.enums()) param.m_enums = p.enums();
			}
			break;
			case FEBIO_PARAM_STD_VECTOR_DOUBLE:
			{
				std::vector<double>& v = p.value<std::vector<double> >();
				QVariant val = QVariant::fromValue(v);
				FEBioParam& param = feb->AddParameter(p.name(), p.type(), val);
			}
			break;
			case FE_PARAM_DATA_ARRAY:
			{
				// Don't know how to handle this.
			}
			break;
			case FE_PARAM_STD_VECTOR_VEC2D:
			{
				// don't know how to handle this.
			}
			break;
			default:
				assert(false);
			}
		}

		if (feb->Parameters() > 0)
		{
			FEBioParam& febParam = feb->GetParameter(feb->Parameters() - 1);
			if (p.units()) febParam.m_szunit = p.units();
		}
	}

	// copy properties
	int props = pc->PropertyClasses();
	for (int i = 0; i < props; ++i)
	{
		FEProperty* prop = pc->PropertyClass(i);
		const char* sz = prop->GetClassName();

		// lookup the base class ID.
		int n = baseClassIndex(sz);

		// add it
		feb->AddProperty(prop->GetName(), prop->GetSuperClassID(), n, prop->IsRequired());
	}

	// all done!
	return feb;
}


vector<FEBio::FEBioModule>	FEBio::GetAllModules()
{
	// Get the kernel
	FECoreKernel& fecore = FECoreKernel::GetInstance();

	vector<FEBio::FEBioModule> mods;
	for (int i = 0; i < fecore.Modules(); ++i)
	{
		FEBio::FEBioModule mod;
		mod.m_szname = fecore.GetModuleName(i);
		mod.m_szdesc = fecore.GetModuleDescription(i);
		mod.m_id = i + 1;
		mods.push_back(mod);
	}

	return mods;
}

const char* FEBio::GetModuleName(int moduleId)
{
	FECoreKernel& fecore = FECoreKernel::GetInstance();
	const char* szmodName = fecore.GetModuleNameFromId(moduleId); assert(szmodName);
	return szmodName;
}

int FEBio::GetModuleId(const std::string& moduleName)
{
	vector<FEBio::FEBioModule> modules = GetAllModules();
	for (int i = 0; i < modules.size(); ++i)
	{
		FEBioModule& mod = modules[i];
		string modName(mod.m_szname);
		if (modName == moduleName) return mod.m_id;
	}
	assert(false);
	return -1;
}

void FEBio::SetActiveModule(int moduleID)
{
	FECoreKernel& fecore = FECoreKernel::GetInstance();
	fecore.SetActiveModule(moduleID);
}

int FEBio::GetActiveModule()
{
	FECoreKernel& fecore = FECoreKernel::GetInstance();
	return fecore.GetActiveModule();
}

class FBSLogStream : public LogStream
{
public:
	FBSLogStream(FEBioOutputHandler* outputHandler) : m_outputHandler(outputHandler) {}

	void print(const char* sz) override
	{
		if (m_outputHandler) m_outputHandler->write(sz);
	}

private:
	FEBioOutputHandler* m_outputHandler;
};

static bool terminateRun = false;

bool interrup_cb(FEModel* fem, unsigned int nwhen, void* pd)
{
	if (terminateRun)
	{
		terminateRun = false;
		throw std::exception("terminated febio run");
	}
	return true;
}

void FEBio::TerminateRun()
{
	terminateRun = true;
}

bool FEBio::runModel(const std::string& fileName, FEBioOutputHandler* outputHandler)
{
	terminateRun = false;

	FEBioModel fem;

	// attach the output handler
	if (outputHandler)
	{
		fem.GetLogFile().SetLogStream(new FBSLogStream(outputHandler));
	}

	// attach a callback to interrupt
	fem.AddCallback(interrup_cb, CB_ALWAYS, nullptr);

	try {

		// try to read the input file
		if (fem.Input(fileName.c_str()) == false)
		{
			return false;
		}

		// do model initialization
		if (fem.Init() == false)
		{
			return false;
		}

		// solve the model
		return fem.Solve();
	}
	catch (...)
	{

	}

	return false;
}

const char* FEBio::GetSuperClassString(int superClassID)
{
	if (idmap.empty()) initMap();
	if (idmap.find(superClassID) != idmap.end())
		return idmap[superClassID];
	else
		return nullptr;
}

std::map<int, const char*> FEBio::GetSuperClassMap()
{
	if (idmap.empty()) initMap();
	return idmap;
}
