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

#include "FECoreMaterial.h"
#include "FEMaterialFactory.h"
#include "FEMaterial.h"
#include "FEMKernel.h"
#include <FECore/fecore_enum.h>
#include <FECore/units.h>
#include <FEBioLink/FEBioInterface.h>
#include <exception>
#include <sstream>
using namespace std;

//=============================================================================
// FSAxisMaterial
//=============================================================================

FSAxisMaterial::FSAxisMaterial(FSModel* fem) : FSMaterial(0, fem)
{
	m_naopt = -1;
	m_n[0] = 0; m_n[1] = 1; m_n[2] = 2;
	m_a = vec3d(1, 0, 0);
	m_d = vec3d(0, 1, 0);
    m_theta = 0;
    m_phi = 90;

	m_center = vec3d(0, 0, 0);
	m_axis = vec3d(0, 0, 1);

	AddIntParam(0, "Axes")->SetEnumNames("(none)\0local\0vector\0angles\0\0");
	AddIntParam(0, "n0");
	AddIntParam(0, "n1");
	AddIntParam(0, "n2");
	AddVecParam(vec3d(1,0,0), "a");
	AddVecParam(vec3d(0,1,0), "d");
    AddScienceParam(0 , UNIT_DEGREE, "theta");
    AddScienceParam(90, UNIT_DEGREE, "phi");

	AddVecParam(m_center, "center");
	AddVecParam(m_axis, "axis");
	AddVecParam(m_vec, "vector");

	for (int i = 1; i < Parameters(); ++i) GetParam(i).SetState(0);
}

bool FSAxisMaterial::UpdateData(bool bsave)
{
	for (int i = 1; i < Parameters(); ++i) GetParam(i).SetState(0);

	if (bsave)
	{
		int oldopt = m_naopt;

		m_naopt = GetIntValue(0) - 1;
		switch (m_naopt)
		{
		case -1: break;
		case FE_AXES_LOCAL: 
			GetParam(1).SetState(Param_ALLFLAGS);
			GetParam(2).SetState(Param_ALLFLAGS);
			GetParam(3).SetState(Param_ALLFLAGS);
			m_n[0] = GetIntValue(1);
			m_n[1] = GetIntValue(2);
			m_n[2] = GetIntValue(3);
			break;
		case FE_AXES_VECTOR:
			GetParam(4).SetState(Param_ALLFLAGS);
			GetParam(5).SetState(Param_ALLFLAGS);
			m_a = GetVecValue(4);
			m_d = GetVecValue(5);
			break;
        case FE_AXES_ANGLES:
            GetParam(6).SetState(Param_ALLFLAGS);
            GetParam(7).SetState(Param_ALLFLAGS);
            m_theta = GetFloatValue(6);
            m_phi = GetFloatValue(7);
            break;
		case FE_AXES_CYLINDRICAL:
			GetParam(8).SetState(Param_ALLFLAGS);
			GetParam(9).SetState(Param_ALLFLAGS);
			GetParam(10).SetState(Param_ALLFLAGS);
			m_center = GetVecValue(8);
			m_axis   = GetVecValue(9);
			m_vec    = GetVecValue(10);
			break;
		case FE_AXES_SPHERICAL:
			GetParam(8).SetState(Param_ALLFLAGS);
			GetParam(10).SetState(Param_ALLFLAGS);
			m_center = GetVecValue(8);
			m_vec = GetVecValue(10);			
			break;
		}

		return (oldopt != m_naopt);
	}
	else
	{
		SetIntValue(0, m_naopt + 1);
		
		SetIntValue(1, m_n[0]); if (m_naopt == 0) GetParam(1).SetState(Param_ALLFLAGS);
		SetIntValue(2, m_n[1]); if (m_naopt == 0) GetParam(2).SetState(Param_ALLFLAGS);
		SetIntValue(3, m_n[2]); if (m_naopt == 0) GetParam(3).SetState(Param_ALLFLAGS);
		
		SetVecValue(4, m_a); if (m_naopt == 1) GetParam(4).SetState(Param_ALLFLAGS);
		SetVecValue(5, m_d); if (m_naopt == 1) GetParam(5).SetState(Param_ALLFLAGS);

        SetFloatValue(6, m_theta); if (m_naopt == 2) GetParam(6).SetState(Param_ALLFLAGS);
        SetFloatValue(7, m_phi); if (m_naopt == 2) GetParam(7).SetState(Param_ALLFLAGS);

		// cylindrical
		SetVecValue(8, m_center); if (m_naopt == FE_AXES_CYLINDRICAL) GetParam(8).SetState(Param_ALLFLAGS);
		SetVecValue(9, m_axis); if (m_naopt == FE_AXES_CYLINDRICAL) GetParam(9).SetState(Param_ALLFLAGS);
		SetVecValue(10, m_vec); if (m_naopt == FE_AXES_CYLINDRICAL) GetParam(10).SetState(Param_ALLFLAGS);

		// spherical
		SetVecValue(8, m_center); if (m_naopt == FE_AXES_SPHERICAL) GetParam(8).SetState(Param_ALLFLAGS);
		SetVecValue(10, m_vec); if (m_naopt == FE_AXES_SPHERICAL) GetParam(10).SetState(Param_ALLFLAGS);

	}
	return false;
}

mat3d FSAxisMaterial::GetMatAxes(FEElementRef& el)
{
    switch (m_naopt)
    {
        case FE_AXES_LOCAL:
        {
            FSCoreMesh* pm = el.m_pmesh;
            vec3d r1 = pm->Node(el->m_node[m_n[0] - 1]).r;
            vec3d r2 = pm->Node(el->m_node[m_n[1] - 1]).r;
            vec3d r3 = pm->Node(el->m_node[m_n[2] - 1]).r;
            vec3d a = r2 - r1;
            vec3d d = r3 - r1;
            vec3d c = a^d;
            vec3d b = c^a;
            a.Normalize();
            b.Normalize();
            c.Normalize();
            mat3d Q;
            Q.zero();
            Q[0][0] = a.x; Q[0][1] = b.x; Q[0][2] = c.x;
            Q[1][0] = a.y; Q[1][1] = b.y; Q[1][2] = c.y;
            Q[2][0] = a.z; Q[2][1] = b.z; Q[2][2] = c.z;
            
            return Q;
        }
            break;
        case FE_AXES_VECTOR:
        {
            vec3d a = m_a;
            vec3d d = m_d;
            vec3d c = a^d;
            vec3d b = c^a;
            a.Normalize();
            b.Normalize();
            c.Normalize();
            mat3d Q;
            Q.zero();
            Q[0][0] = a.x; Q[0][1] = b.x; Q[0][2] = c.x;
            Q[1][0] = a.y; Q[1][1] = b.y; Q[1][2] = c.y;
            Q[2][0] = a.z; Q[2][1] = b.z; Q[2][2] = c.z;
            
            return Q;
        }
            break;
        case FE_AXES_ANGLES:
        {
            double theta = m_theta*PI/180;
            double phi = m_phi*PI/180;
            mat3d Q;
            Q.zero();
            Q[0][0] = sin(phi)*cos(theta); Q[0][1] = -sin(theta); Q[0][2] = -cos(phi)*cos(theta);
            Q[1][0] = sin(phi)*sin(theta); Q[1][1] = cos(theta);  Q[1][2] = -cos(phi)*sin(theta);
            Q[2][0] = cos(phi);            Q[2][1] = 0;           Q[2][2] = sin(phi);
            
            return Q;
        }
		break;
		case FE_AXES_CYLINDRICAL:
		{
			// we'll use the element center as the reference point
			FSCoreMesh* pm = el.m_pmesh;
			int n = el->Nodes();
			vec3d p(0, 0, 0);
			for (int i = 0; i < n; ++i) p += pm->NodePosition(el->m_node[i]);
			p /= (double)n;

			// find the vector to the axis
			vec3d b = (p - m_center) - m_axis * (m_axis*(p - m_center)); b.Normalize();

			// setup the rotation vector
			vec3d x_unit(vec3d(1, 0, 0));
			quatd q(x_unit, b);

			// rotate the reference vector
			vec3d r(m_vec); r.Normalize();
			q.RotateVector(r);

			// setup a local coordinate system with r as the x-axis
			vec3d d(0, 1, 0);
			q.RotateVector(d);
			if (fabs(d*r) > 0.99)
			{
				d = vec3d(0, 0, 1);
				q.RotateVector(d);
			}

			// find basis vectors
			vec3d e1 = r;
			vec3d e3 = (e1 ^ d); e3.Normalize();
			vec3d e2 = e3 ^ e1;

			// setup rotation matrix
			mat3d Q;
			Q[0][0] = e1.x; Q[0][1] = e2.x; Q[0][2] = e3.x;
			Q[1][0] = e1.y; Q[1][1] = e2.y; Q[1][2] = e3.y;
			Q[2][0] = e1.z; Q[2][1] = e2.z; Q[2][2] = e3.z;

			return Q;
		}
		break;
		case FE_AXES_SPHERICAL:
		{
			// we'll use the element center as the reference point
			FSCoreMesh* pm = el.m_pmesh;
			int n = el->Nodes();
			vec3d a(0, 0, 0);
			for (int i = 0; i < n; ++i) a += pm->NodePosition(el->m_node[i]);
			a /= (double)n;

			a -= m_center;
			a.Normalize();

			// setup the rotation vector
			vec3d x_unit(1, 0, 0);
			quatd q(x_unit, a);

			vec3d v = m_vec;
			v.Normalize();
			q.RotateVector(v);
			a = v;

			vec3d d(0, 1, 0);
			d.Normalize();
			if (fabs(a*d) > .99)
			{
				d = vec3d(0, 0, 1);
				d.Normalize();
			}

			vec3d c = a ^ d;
			vec3d b = c ^ a;

			a.Normalize();
			b.Normalize();
			c.Normalize();

			mat3d Q;
			Q[0][0] = a.x; Q[0][1] = b.x; Q[0][2] = c.x;
			Q[1][0] = a.y; Q[1][1] = b.y; Q[1][2] = c.y;
			Q[2][0] = a.z; Q[2][1] = b.z; Q[2][2] = c.z;

			return Q;
		}
		break;
    }

	mat3d Q;
	Q.unit();
	return Q;
}

//=============================================================================
// FSMaterial
//=============================================================================

//-----------------------------------------------------------------------------
FSMaterial::FSMaterial(int ntype, FSModel* fem) : FSModelComponent(fem), m_ntype(ntype)
{
	m_parent = 0;
	m_owner = 0;
	m_axes = nullptr;
	m_superClassID = FEMATERIAL_ID;
}

//-----------------------------------------------------------------------------
FSMaterial::~FSMaterial()
{
	ClearProperties();
	delete m_axes;
}

//-----------------------------------------------------------------------------
int FSMaterial::ClassID()
{
	FEMaterialFactory& MF = *FEMaterialFactory::GetInstance();
	return MF.ClassID(this);
}

//-----------------------------------------------------------------------------
const char* FSMaterial::GetTypeString() const
{
	FEMaterialFactory& MF = *FEMaterialFactory::GetInstance();
	return MF.TypeStr(this);
}

//-----------------------------------------------------------------------------
void FSMaterial::SetTypeString(const std::string& s)
{
	assert(false);
}

//-----------------------------------------------------------------------------
void FSMaterial::SetParentMaterial(FSMaterial* pmat)
{
	assert((m_parent==0) || (m_parent == pmat));
	m_parent = pmat;
}

//-----------------------------------------------------------------------------
const FSMaterial* FSMaterial::GetParentMaterial() const
{
	return m_parent;
}

//-----------------------------------------------------------------------------
const FSMaterial* FSMaterial::GetAncestor() const
{
	if (m_parent) return m_parent->GetAncestor();
	return this;
}

//-----------------------------------------------------------------------------
GMaterial* FSMaterial::GetOwner() const 
{ 
	return m_owner; 
}

//-----------------------------------------------------------------------------
void FSMaterial::SetOwner(GMaterial* owner)
{
	m_owner = owner;
}

//-----------------------------------------------------------------------------
FSMaterial* FSMaterial::GetMaterialProperty(int propId, int index)
{
	return dynamic_cast<FSMaterial*>(GetProperty(propId).GetComponent(index));
}

//-----------------------------------------------------------------------------
void FSMaterial::copy(FSMaterial* pm)
{
	// make sure these materials are the same
	assert(m_ntype == pm->m_ntype);

	// copy this material's parameters first
	GetParamBlock() = pm->GetParamBlock();

	// copy the individual components
	// TODO: Probably should move this logic to base class
	ClearProperties();
	int NC = (int) pm->Properties();
	for (int i=0; i<NC; ++i)
	{
		FSProperty& mcs = pm->GetProperty(i);
		FSProperty& mcd = *AddProperty(mcs.GetName(), mcs.GetPropertyType(), mcs.maxSize(), mcs.GetFlags());

		if ((mcs.Size() == 1)&&(mcd.Size() == 1))
		{
			if (mcs.GetComponent())
			{
				FSMaterial* pmj = dynamic_cast<FSMaterial*>(mcs.GetComponent(0));
				FSMaterial* pm = FEMaterialFactory::Create(GetFSModel(), pmj->Type());
				pm->copy(pmj);
				mcd.SetComponent(pm);
			}
			else mcs.SetComponent(nullptr);
		}
		else
		{
			for (int j=0; j<mcs.Size(); ++j)
			{
				FSMaterial* pmj = dynamic_cast<FSMaterial*>(mcs.GetComponent(j));
				FSMaterial* pm = FEMaterialFactory::Create(GetFSModel(), pmj->Type());
				pm->copy(pmj);
				mcd.AddComponent(pm);
			}
		}
	}
}

//-----------------------------------------------------------------------------
FSMaterial* FSMaterial::Clone()
{
	FSMaterial* pmCopy = FEMaterialFactory::Create(GetFSModel(), Type());
	pmCopy->copy(this);
	return pmCopy;
}

//-----------------------------------------------------------------------------
bool FSMaterial::HasMaterialAxes() const
{
	return (m_axes != nullptr);
}

//-----------------------------------------------------------------------------
mat3d FSMaterial::GetMatAxes(FEElementRef& el) const
{
	mat3d Q = m_axes ? m_axes->GetMatAxes(el) : mat3d(1, 0, 0, 0, 1, 0, 0, 0, 1);
	const FSMaterial* parentMat = GetParentMaterial();
	if (parentMat)
	{
		mat3d Qp = parentMat->GetMatAxes(el);
		Q = Qp * Q;
	}
	return Q;
}

//-----------------------------------------------------------------------------
// set the axis material
void FSMaterial::SetAxisMaterial(FSAxisMaterial* Q)
{
	if (m_axes) delete m_axes;
	m_axes = Q;
}

//-----------------------------------------------------------------------------
bool FSMaterial::IsRigid()
{
	return false;
}

//-----------------------------------------------------------------------------
// Save the material data to the archive
void FSMaterial::Save(OArchive& ar)
{
	// save the name if there is one
	string name = GetName();
	if (name.empty() == false)
	{
		ar.WriteChunk(CID_MAT_NAME, name);
	}

	// save material parameters (if any)
	if (Parameters() > 0)
	{
		ar.BeginChunk(CID_MAT_PARAMS);
		{
			ParamContainer::Save(ar);
		}
		ar.EndChunk();
	}

	// write the material properties (if any)
	if (Properties() != 0)
	{
		ar.BeginChunk(CID_MAT_PROPERTY);
		{
			int n = (int) Properties();
			for (int i=0; i<n; ++i)
			{
				FSProperty& mpi = GetProperty(i);

				// store the property name
				ar.WriteChunk(CID_MAT_PROPERTY_NAME, mpi.GetName());

				ar.BeginChunk(CID_MATERIAL_COMPONENT);
				{
					for (int j = 0; j < mpi.Size(); ++j)
					{
						FSModelComponent* pc = dynamic_cast<FSModelComponent*>(mpi.GetComponent(j));
						if (pc)
						{
							string typeStr = pc->GetTypeString();
							ar.WriteChunk(CID_MATERIAL_COMPONENT_TYPE, typeStr);
							ar.BeginChunk(CID_MATERIAL_COMPONENT_DATA);
							{
								pc->Save(ar);
							}
							ar.EndChunk();
						}
					}
				}
				ar.EndChunk();
			}
		}
		ar.EndChunk();
	}

	if (m_axes)
	{
		ar.BeginChunk(CID_MAT_AXES);
		{
			ar.WriteChunk(0, m_axes->m_naopt);
			ar.WriteChunk(1, m_axes->m_n, 3);
			ar.WriteChunk(2, m_axes->m_a);
			ar.WriteChunk(3, m_axes->m_d);
            ar.WriteChunk(4, m_axes->m_theta);
            ar.WriteChunk(5, m_axes->m_phi);
		}
		ar.EndChunk();
	}
}

//-----------------------------------------------------------------------------
// Load the material data from the archive
void FSMaterial::Load(IArchive &ar)
{
	TRACE("FSMaterial::Load");

	FSModel* fem = GetFSModel();

	char szname[256];
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case CID_MAT_NAME: { ar.read(szname); SetName(szname); } break;
		case CID_MAT_PARAMS: ParamContainer::Load(ar); break;
		case CID_MAT_AXES:
			{
				FSAxisMaterial* axes = new FSAxisMaterial(GetFSModel());
				while (IArchive::IO_OK == ar.OpenChunk())
				{
					int nid = (int)ar.GetChunkID();
					switch (nid)
					{
					case 0: ar.read(axes->m_naopt); break;
					case 1: ar.read(axes->m_n, 3); break;
					case 2: ar.read(axes->m_a); break;
					case 3: ar.read(axes->m_d); break;
                    case 4: ar.read(axes->m_theta); break;
                    case 5: ar.read(axes->m_phi); break;
					}
					ar.CloseChunk();
				}
				axes->UpdateData(false);
				axes->UpdateData(true);

				SetAxisMaterial(axes);
			}
			break;
        case CID_MAT_PROPERTY:
			{
				FSProperty* prop = 0;
				while (IArchive::IO_OK == ar.OpenChunk())
				{
					int nid = (int) ar.GetChunkID();
					if (nid == CID_MAT_PROPERTY_NAME) 
					{
						ar.read(szname);
						prop = FindProperty(szname);
						assert(prop);
						prop->Clear();
					}
					else if (nid == CID_MAT_PROPERTY_MAT)
					{
						int n = 0;
						while (IArchive::IO_OK == ar.OpenChunk())
						{
							int nid = ar.GetChunkID();
                            FSMaterial* pm = 0;

                            switch (nid)
                            {
                            case FE_FIBEREXPPOW_COUPLED_OLD     : pm = new FSFiberExpPowOld(fem); break;
                            case FE_FIBEREXPPOW_UNCOUPLED_OLD   : pm = new FSFiberExpPowUncoupledOld(fem); break;
                            case FE_FIBERPOWLIN_COUPLED_OLD     : pm = new FSFiberPowLinOld(fem); break;
                            case FE_FIBERPOWLIN_UNCOUPLED_OLD   : pm = new FSFiberPowLinUncoupledOld(fem); break;
                            case FE_ACTIVE_CONTRACT_UNI_OLD     : pm = new FSPrescribedActiveContractionUniaxialOld(fem); break;
                            case FE_ACTIVE_CONTRACT_TISO_OLD    : pm = new FSPrescribedActiveContractionTransIsoOld(fem); break;
                            case FE_ACTIVE_CONTRACT_UNI_UC_OLD  : pm = new FSPrescribedActiveContractionUniaxialUCOld(fem); break;
                            case FE_ACTIVE_CONTRACT_TISO_UC_OLD : pm = new FSPrescribedActiveContractionTransIsoUCOld(fem); break;
                            default:
                                pm = FEMaterialFactory::Create(fem, nid);
                            }
							assert(pm);

							// Add the component to the property.
							// Note that we need to do this before we actually load the material!
							if (prop)
							{
								if (prop->maxSize() == FSProperty::NO_FIXED_SIZE)
									prop->AddComponent(pm);
								else prop->SetComponent(pm, n);
							}

							pm->Load(ar);

							// see if we need to convert this material
							pm = nullptr;
							if (nid == FE_TRANS_MOONEY_RIVLIN_OLD)
							{
								FSTransMooneyRivlin* pnewMat = new FSTransMooneyRivlin(fem);
								pnewMat->Convert(dynamic_cast<FSTransMooneyRivlinOld*>(pm));
								pm = pnewMat;
							}
							else if (nid == FE_TRANS_VERONDA_WESTMANN_OLD)
							{
								FSTransVerondaWestmann* pnewMat = new FSTransVerondaWestmann(fem);
								pnewMat->Convert(dynamic_cast<FSTransVerondaWestmannOld*>(pm));
								pm = pnewMat;
							}
							else if (nid == FE_COUPLED_TRANS_ISO_MR_OLD)
							{
								FSCoupledTransIsoMooneyRivlin* pnewMat = new FSCoupledTransIsoMooneyRivlin(fem);
								pnewMat->Convert(dynamic_cast<FSCoupledTransIsoMooneyRivlinOld*>(pm));
								pm = pnewMat;
							}
							else if (nid == FE_ACTIVE_CONTRACT_UNI_OLD)
							{
								FSPrescribedActiveContractionUniaxial* pnewMat = new FSPrescribedActiveContractionUniaxial(fem);
								pnewMat->Convert(dynamic_cast<FSPrescribedActiveContractionUniaxialOld*>(pm));
								pm = pnewMat;
							}
							else if (nid == FE_ACTIVE_CONTRACT_TISO_OLD)
							{
								FSPrescribedActiveContractionTransIso* pnewMat = new FSPrescribedActiveContractionTransIso(fem);
								pnewMat->Convert(dynamic_cast<FSPrescribedActiveContractionTransIsoOld*>(pm));
								pm = pnewMat;
							}
							else if (nid == FE_ACTIVE_CONTRACT_UNI_UC_OLD)
							{
								FSPrescribedActiveContractionUniaxialUC* pnewMat = new FSPrescribedActiveContractionUniaxialUC(fem);
								pnewMat->Convert(dynamic_cast<FSPrescribedActiveContractionUniaxialUCOld*>(pm));
								pm = pnewMat;
							}
							else if (nid == FE_ACTIVE_CONTRACT_TISO_UC_OLD)
							{
								FSPrescribedActiveContractionTransIsoUC* pnewMat = new FSPrescribedActiveContractionTransIsoUC(fem);
								pnewMat->Convert(dynamic_cast<FSPrescribedActiveContractionTransIsoUCOld*>(pm));
								pm = pnewMat;
							}
							else if (nid == FE_FIBEREXPPOW_COUPLED_OLD)
							{
								FSFiberExpPow* pnewMat = new FSFiberExpPow(fem);
								pnewMat->Convert(dynamic_cast<FSFiberExpPowOld*>(pm));
								pm = pnewMat;
							}
							else if (nid == FE_FIBEREXPPOW_UNCOUPLED_OLD)
							{
								FSFiberExpPowUncoupled* pnewMat = new FSFiberExpPowUncoupled(fem);
								pnewMat->Convert(dynamic_cast<FSFiberExpPowUncoupledOld*>(pm));
								pm = pnewMat;
							}
							else if (nid == FE_FIBERPOWLIN_COUPLED_OLD)
							{
								FSFiberPowLin* pnewMat = new FSFiberPowLin(fem);
								pnewMat->Convert(dynamic_cast<FSFiberPowLinOld*>(pm));
								pm = pnewMat;
							}
							else if (nid == FE_FIBERPOWLIN_UNCOUPLED_OLD)
							{
								FSFiberPowLinUncoupled* pnewMat = new FSFiberPowLinUncoupled(fem);
								pnewMat->Convert(dynamic_cast<FSFiberPowLinUncoupledOld*>(pm));
								pm = pnewMat;
							}

							if (pm && prop)
							{
								prop->SetComponent(pm, n);
							}
							n++;

							ar.CloseChunk();
						}
					}
					else if (nid == CID_MAT_PROPERTY_MATPROP)
					{
						int n = 0;
						while (IArchive::IO_OK == ar.OpenChunk())
						{
							int nid = ar.GetChunkID();
							FSMaterialProperty* pm = fscore_new<FSMaterialProperty>(fem, FEMATERIALPROP_ID, nid); assert(pm);
							if (pm)
							{
								pm->Load(ar);

								if (prop)
								{
									if (prop->maxSize() == FSProperty::NO_FIXED_SIZE)
										prop->AddComponent(pm);
									else prop->SetComponent(pm, n);
									n++;
								}
							}
							ar.CloseChunk();
						}
					}
					else if (CID_MATERIAL_COMPONENT)
					{
						string typeString;
						while (IArchive::IO_OK == ar.OpenChunk())
						{
							int cid = ar.GetChunkID();
							switch (cid)
							{
							case CID_MATERIAL_COMPONENT_TYPE: ar.read(typeString); break;
							case CID_MATERIAL_COMPONENT_DATA:
								assert(prop);
								if (prop)
								{
									FSModelComponent* pmc = FEBio::CreateFSClass(prop->GetSuperClassID(), -1, fem); assert(pmc);
									pmc->SetFlags(prop->GetFlags());
									try {
										pmc->Load(ar);
										prop->AddComponent(pmc);
									}
									catch (...)
									{
										delete pmc;
									}
								}
								break;
							}

							ar.CloseChunk();
						}
					}
					else if (ar.Version() < 0x00020000)
					{
						// Note that some materials are considered obsolete. Since they are no longer registered
						// we need to check for them explicitly.
						FSMaterial* pm = 0;
						switch (nid)
						{
						case FE_TRANS_MOONEY_RIVLIN_OLD   : pm = new FSTransMooneyRivlinOld(fem); break;
						case FE_TRANS_VERONDA_WESTMANN_OLD: pm = new FSTransVerondaWestmannOld(fem); break;
						case FE_COUPLED_TRANS_ISO_MR_OLD  : pm = new FSCoupledTransIsoMooneyRivlinOld(fem); break;
						default:
							pm = FEMaterialFactory::Create(fem, nid);
						}
						assert(pm);
						if (pm) 
						{
							pm->Load(ar);
							prop->AddComponent(pm);
						}
					}
					ar.CloseChunk();
				}
			}
			break;
		}
		ar.CloseChunk();
	}
}

//===============================================================================================
FSMaterialProp::FSMaterialProp(int ntype, FSModel* fem) : FSMaterial(ntype, fem)
{
	SetSuperClassID(FEMATERIALPROP_ID);
}

//===============================================================================================
FSMaterialProperty::FSMaterialProperty(FSModel* fem, int ntype) : FSModelComponent(fem)
{
	m_ntype = ntype;
	SetSuperClassID(FEMATERIALPROP_ID);
}

int FSMaterialProperty::Type() const
{
	return m_ntype;
}

void FSMaterialProperty::Save(OArchive& ar)
{
	// save the name if there is one
	string name = GetName();
	if (name.empty() == false)
	{
		ar.WriteChunk(CID_FEOBJ_NAME, name);
	}

	string info = GetInfo();
	if (info.empty() == false)
	{
		ar.WriteChunk(CID_FEOBJ_INFO, info);
	}
	ar.BeginChunk(CID_FEOBJ_PARAMS);
	{
		ParamContainer::Save(ar);
	}
	ar.EndChunk();

	// write the material properties (if any)
	if (Properties() != 0)
	{
		int n = (int)Properties();
		for (int i = 0; i < n; ++i)
		{
			FSProperty& mpi = GetProperty(i);
			ar.BeginChunk(CID_MAT_PROPERTY);
			{
				// store the property name
				ar.WriteChunk(CID_MAT_PROPERTY_NAME, mpi.GetName());

				for (int j = 0; j < mpi.Size(); ++j)
				{
					FSModelComponent* pc = dynamic_cast<FSModelComponent*>(mpi.GetComponent(j));
					if (pc)
					{
						ar.BeginChunk(CID_MATERIAL_COMPONENT);
						{
							string typeStr = pc->GetTypeString();
							ar.WriteChunk(CID_MATERIAL_COMPONENT_TYPE, typeStr);
							ar.BeginChunk(CID_MATERIAL_COMPONENT_DATA);
							{
								pc->Save(ar);
							}
							ar.EndChunk();
						}
						ar.EndChunk();
					}
				}
			}
			ar.EndChunk();
		}
	}
}

void FSMaterialProperty::Load(IArchive& ar)
{
	TRACE("FSMaterialProperty::Load");
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case CID_FEOBJ_NAME: { string name; ar.read(name); SetName(name); } break;
		case CID_FEOBJ_INFO: { string info; ar.read(info); SetInfo(info); } break;
		case CID_FEOBJ_PARAMS: ParamContainer::Load(ar); break;
		case CID_MAT_PROPERTY:
		{
			FSProperty* prop = nullptr;
			while (IArchive::IO_OK == ar.OpenChunk())
			{
				switch (ar.GetChunkID())
				{
				case CID_MAT_PROPERTY_NAME:
				{
					string name; 
					ar.read(name);
					prop = FindProperty(name);
					if (prop == nullptr) ar.log("WARNING: Property %s could not be found\n", name.c_str());
					if (prop) prop->Clear();
				}
				break;
				case CID_MATERIAL_COMPONENT:
				{
					if (prop)
					{
						FSModelComponent* pmc = nullptr;
						while (IArchive::IO_OK == ar.OpenChunk())
						{
							switch (ar.GetChunkID())
							{
							case CID_MATERIAL_COMPONENT_TYPE:
							{
								string type; ar.read(type);
								pmc = FEBio::CreateFSClass(prop->GetSuperClassID(), -1, GetFSModel()); assert(pmc);
							}
							break;
							case CID_MATERIAL_COMPONENT_DATA:
							{
								if (pmc)
								{
									pmc->Load(ar);
									if (prop) prop->AddComponent(pmc);
								}
							}
							break;
							}
							ar.CloseChunk();
						}
					}
				}
				break;
				}
				ar.CloseChunk();
			}
		}
		break;
		}
		ar.CloseChunk();
	}
}

//===============================================================================================
FEBioMaterialProperty::FEBioMaterialProperty(FSModel* fem) : FSMaterialProperty(fem, FE_FEBIO_MATERIAL_PROPERTY)
{

}

void FEBioMaterialProperty::Save(OArchive& ar)
{
	ar.BeginChunk(CID_FEBIO_META_DATA);
	{
		SaveClassMetaData(this, ar);
	}
	ar.EndChunk();

	ar.BeginChunk(CID_FEBIO_BASE_DATA);
	{
		FSMaterialProperty::Save(ar);
	}
	ar.EndChunk();
}

void FEBioMaterialProperty::Load(IArchive& ar)
{
	TRACE("FEBioMaterial::Load");
	bool errorFlag = false;
	while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		case CID_FEBIO_META_DATA:
		{
			try {
				LoadClassMetaData(this, ar);
			}
			catch (std::runtime_error e)
			{
				ar.log(e.what());
				errorFlag = true;
			}
		}
		break;
		case CID_FEBIO_BASE_DATA:
			if (!errorFlag) FSMaterialProperty::Load(ar); 
			break;
		default:
			assert(false);
		}
		ar.CloseChunk();
	}

	if (errorFlag) throw std::runtime_error("Failed to load component");

	// We call this to make sure that the FEBio class has the same parameters
	UpdateData(true);
}

bool FEBioMaterialProperty::HasFibers()
{
	return (FindProperty("fiber") != nullptr);
}

vec3d FEBioMaterialProperty::GetFiber(FEElementRef& el)
{
	vec3d v(0, 0, 0);
	FSProperty* pm = FindProperty("fiber");
	FSVec3dValuator* fiber = dynamic_cast<FSVec3dValuator*>(pm->GetComponent());
	if (fiber)
	{
		// evaluate the fiber direction
		v = fiber->GetFiberVector(el);
	}
	return v;
}
