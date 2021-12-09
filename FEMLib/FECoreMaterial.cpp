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
#include "enums.h"
#include <FECore/units.h>

//=============================================================================
// FSMaterialProperty
//=============================================================================

//-----------------------------------------------------------------------------
FSMaterialProperty::FSMaterialProperty()
{
	m_parent = 0;
	m_nClassID = -1;
	m_maxSize = NO_FIXED_SIZE;
	m_nsuperClassID = FE_MATERIAL;
	m_flag = EDITABLE;
}

//-----------------------------------------------------------------------------
FSMaterialProperty::FSMaterialProperty(const std::string& name, int nClassID, FSMaterial* parent, int nsize, unsigned int flags) : m_parent(parent)
{
	m_nClassID = nClassID;
	m_name = name;
	m_flag = flags;
	m_maxSize = nsize;
	m_nsuperClassID = FE_MATERIAL;
	if (nsize > 0)
	{
		m_mat.assign(nsize, 0);
	}
}

//-----------------------------------------------------------------------------
FSMaterialProperty::~FSMaterialProperty()
{
	Clear();
}

//-----------------------------------------------------------------------------
void FSMaterialProperty::SetName(const std::string& name)
{
	m_name = name;
}

//-----------------------------------------------------------------------------
const std::string& FSMaterialProperty::GetName()
{ 
	return m_name; 
}

//-----------------------------------------------------------------------------
void FSMaterialProperty::Clear()
{
	for (int i = 0; i<(int)m_mat.size(); ++i) { delete m_mat[i]; m_mat[i] = 0; }
	if (m_maxSize == NO_FIXED_SIZE) m_mat.clear();
}

//-----------------------------------------------------------------------------
void FSMaterialProperty::AddMaterial(FSMaterial* pm)
{
	if (pm) pm->SetParentMaterial(m_parent);
	if (m_maxSize == NO_FIXED_SIZE)
		m_mat.push_back(pm);
	else
	{
		// find a zero component
		for (int i=0; i<(int)m_mat.size(); ++i)
		{
			if (m_mat[i] == 0) { m_mat[i] = pm; return; }
		}

		// TODO: I only get here for 1D point-functions, but not sure 
		//       for any other reason. 
		if (m_mat.size() == 1)
		{
			delete m_mat[0];
			m_mat[0] = pm;
			return;
		}
		assert(false);
	}
}

//-----------------------------------------------------------------------------
void FSMaterialProperty::SetMaterial(FSMaterial* pm, int i) 
{ 
//	if (pm) assert(pm->ClassID() & m_nClassID);
	if (pm) pm->SetParentMaterial(m_parent);
	if (m_mat.empty() == false)
	{
		if (m_mat[i] != pm)
		{
			delete m_mat[i];
			m_mat[i] = pm;
		}
	}
}

//-----------------------------------------------------------------------------
// remove a material from the list (returns false if pm is not part of the list)
bool FSMaterialProperty::RemoveMaterial(FSMaterial* pm)
{
	// find the material
	for (int i=0; i<(int)m_mat.size(); ++i)
	{
		if (m_mat[i] == pm)
		{
			m_mat.erase(m_mat.begin() + i);
			delete pm;
			return true;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
FSMaterial* FSMaterialProperty::GetMaterial(int i)
{	
	if ((i<0) || (i>=(int)m_mat.size())) return 0;
	return m_mat[i]; 
}

//-----------------------------------------------------------------------------
int FSMaterialProperty::GetMaterialIndex(FSMaterial* mat)
{
	for (int i=0; i<(int)m_mat.size(); ++i)
	{
		if (m_mat[i] == mat) return i;
	}
	return -1;
}

//=============================================================================
// FEAxisMaterial
//=============================================================================

FEAxisMaterial::FEAxisMaterial() : FSMaterial(0)
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

bool FEAxisMaterial::UpdateData(bool bsave)
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

mat3d FEAxisMaterial::GetMatAxes(FEElementRef& el)
{
    switch (m_naopt)
    {
        case FE_AXES_LOCAL:
        {
            FECoreMesh* pm = el.m_pmesh;
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
			FECoreMesh* pm = el.m_pmesh;
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
			FECoreMesh* pm = el.m_pmesh;
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
FSMaterial::FSMaterial(int ntype) : m_ntype(ntype)
{
	m_parent = 0;
	m_owner = 0;
	m_axes = nullptr;
	m_superClassID = FE_MATERIAL;
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
const char* FSMaterial::GetTypeString()
{
	FEMaterialFactory& MF = *FEMaterialFactory::GetInstance();
	return MF.TypeStr(this);
}

//-----------------------------------------------------------------------------
void FSMaterial::SetTypeString(const char* sz)
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
// delete all material properties
void FSMaterial::ClearProperties()
{
	vector<FSMaterialProperty*>::iterator it;
	for (it = m_Mat.begin(); it != m_Mat.end(); ++it) (*it)->Clear();
}

//-----------------------------------------------------------------------------
// Add a component to the material
FSMaterialProperty* FSMaterial::AddProperty(const std::string& name, int nClassID, int maxSize, unsigned int flags)
{
	FSMaterialProperty* m = new FSMaterialProperty(name, nClassID, this, maxSize, flags);
	m_Mat.push_back(m);
	return m;
}

//-----------------------------------------------------------------------------
void FSMaterial::AddProperty(const std::string& name, FSMaterial* pm)
{
	FSMaterialProperty* p = FindProperty(name);
	assert(p);
	if (p) p->AddMaterial(pm);
}

//-----------------------------------------------------------------------------
int FSMaterial::AddProperty(int propID, FSMaterial* pm)
{
	FSMaterialProperty& p = GetProperty(propID);
	p.AddMaterial(pm);
	return (p.Size() - 1);
}

//-----------------------------------------------------------------------------
// Replace the material of a component
void FSMaterial::ReplaceProperty(int propID, FSMaterial* pm, int matID)
{
	m_Mat[propID]->SetMaterial(pm, matID);
}

//-----------------------------------------------------------------------------
FSMaterialProperty* FSMaterial::FindProperty(const std::string& name)
{
	int n = (int) m_Mat.size();
	for (int i=0; i<n; ++i)
	{
		FSMaterialProperty* pm = m_Mat[i];
		if (pm->GetName() == name) return pm;
	}
	return 0;
}

//-----------------------------------------------------------------------------
// find the property by type
FSMaterialProperty* FSMaterial::FindProperty(int ntype)
{
	int n = (int)m_Mat.size();
	for (int i = 0; i<n; ++i)
	{
		FSMaterialProperty* pm = m_Mat[i];
		if (pm->GetClassID() == ntype) return pm;
	}
	return 0;
}

//-----------------------------------------------------------------------------
// find the property by the material
FSMaterialProperty* FSMaterial::FindProperty(FSMaterial* pm)
{
	int NP = Properties();
	for (int i=0; i<NP; ++i)
	{
		FSMaterialProperty& p = GetProperty(i);
		int nmat = p.Size();
		for (int j=0; j<nmat; ++j)
		{
			if (p.GetMaterial(j) == pm) return &p;
		}
	}
	return 0;
}

//-----------------------------------------------------------------------------
void FSMaterial::copy(FSMaterial* pm)
{
	// make sure these materials are the same
	assert(m_ntype == pm->m_ntype);

	// copy this material's parameters first
	GetParamBlock() = pm->GetParamBlock();

	// copy the individual components
	int NC = (int) pm->m_Mat.size();
	m_Mat.resize(NC);
	for (int i=0; i<NC; ++i)
	{
		FSMaterialProperty& mcd = GetProperty(i);
		FSMaterialProperty& mcs = pm->GetProperty(i);

		if ((mcs.Size() == 1)&&(mcd.Size() == 1))
		{
			if (mcs.GetMaterial())
			{
				FSMaterial* pm = FEMaterialFactory::Create(mcs.GetMaterial()->Type());
				pm->copy(mcs.GetMaterial());
				mcd.SetMaterial(pm);
			}
			else mcs.SetMaterial(0);
		}
		else
		{
			for (int j=0; j<mcs.Size(); ++j)
			{
				FSMaterial* pmj = mcs.GetMaterial(j);
				FSMaterial* pm = FEMaterialFactory::Create(pmj->Type());
				pm->copy(pmj);
				mcd.AddMaterial(pm);
			}
		}
	}
}

//-----------------------------------------------------------------------------
FSMaterial* FSMaterial::Clone()
{
	FSMaterial* pmCopy = FEMaterialFactory::Create(Type());
	pmCopy->copy(this);
	return pmCopy;
}

//-----------------------------------------------------------------------------
bool FSMaterial::HasMaterialAxes() const
{
	return (m_axes != nullptr);
}

//-----------------------------------------------------------------------------
mat3d FSMaterial::GetMatAxes(FEElementRef& el)
{
	return (m_axes ? m_axes->GetMatAxes(el) : mat3d(1,0,0, 0,1,0, 0,0,1));
}

//-----------------------------------------------------------------------------
// set the axis material
void FSMaterial::SetAxisMaterial(FEAxisMaterial* Q)
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
	if (!m_Mat.empty())
	{
		ar.BeginChunk(CID_MAT_PROPERTY);
		{
			int n = (int) m_Mat.size();
			for (int i=0; i<n; ++i)
			{
				FSMaterialProperty& mpi = GetProperty(i);

				// store the property name
				ar.WriteChunk(CID_MAT_PROPERTY_NAME, mpi.GetName());

				// store the property data
				ar.BeginChunk(CID_MAT_PROPERTY_MAT);
				{
					for (int j = 0; j<mpi.Size(); ++j)
					{
						FSMaterial* pm = mpi.GetMaterial(j);
						if (pm)
						{
							ar.BeginChunk(pm->Type());
							{
								pm->Save(ar);
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
				FEAxisMaterial* axes = new FEAxisMaterial;
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
				FSMaterialProperty* prop = 0;
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
                            case FE_FIBEREXPPOW_COUPLED_OLD     : pm = new FEFiberExpPowOld; break;
                            case FE_FIBEREXPPOW_UNCOUPLED_OLD   : pm = new FEFiberExpPowUncoupledOld; break;
                            case FE_FIBERPOWLIN_COUPLED_OLD     : pm = new FEFiberPowLinOld; break;
                            case FE_FIBERPOWLIN_UNCOUPLED_OLD   : pm = new FEFiberPowLinUncoupledOld; break;
                            case FE_ACTIVE_CONTRACT_UNI_OLD     : pm = new FEPrescribedActiveContractionUniaxialOld; break;
                            case FE_ACTIVE_CONTRACT_TISO_OLD    : pm = new FEPrescribedActiveContractionTransIsoOld; break;
                            case FE_ACTIVE_CONTRACT_UNI_UC_OLD  : pm = new FEPrescribedActiveContractionUniaxialUCOld; break;
                            case FE_ACTIVE_CONTRACT_TISO_UC_OLD : pm = new FEPrescribedActiveContractionTransIsoUCOld; break;
                            default:
                                pm = FEMaterialFactory::Create(nid);
								pm->SetSuperClassID(prop->GetSuperClassID());
                            }
							assert(pm);
							pm->Load(ar);

							if (nid == FE_TRANS_MOONEY_RIVLIN_OLD)
							{
								FETransMooneyRivlin* pnewMat = new FETransMooneyRivlin;
								pnewMat->Convert(dynamic_cast<FETransMooneyRivlinOld*>(pm));
								pm = pnewMat;
							}
							else if (nid == FE_TRANS_VERONDA_WESTMANN_OLD)
							{
								FETransVerondaWestmann* pnewMat = new FETransVerondaWestmann;
								pnewMat->Convert(dynamic_cast<FETransVerondaWestmannOld*>(pm));
								pm = pnewMat;
							}
							else if (nid == FE_COUPLED_TRANS_ISO_MR_OLD)
							{
								FECoupledTransIsoMooneyRivlin* pnewMat = new FECoupledTransIsoMooneyRivlin;
								pnewMat->Convert(dynamic_cast<FECoupledTransIsoMooneyRivlinOld*>(pm));
								pm = pnewMat;
							}
							else if (nid == FE_ACTIVE_CONTRACT_UNI_OLD)
							{
								FEPrescribedActiveContractionUniaxial* pnewMat = new FEPrescribedActiveContractionUniaxial;
								pnewMat->Convert(dynamic_cast<FEPrescribedActiveContractionUniaxialOld*>(pm));
								pm = pnewMat;
							}
							else if (nid == FE_ACTIVE_CONTRACT_TISO_OLD)
							{
								FEPrescribedActiveContractionTransIso* pnewMat = new FEPrescribedActiveContractionTransIso;
								pnewMat->Convert(dynamic_cast<FEPrescribedActiveContractionTransIsoOld*>(pm));
								pm = pnewMat;
							}
							else if (nid == FE_ACTIVE_CONTRACT_UNI_UC_OLD)
							{
								FEPrescribedActiveContractionUniaxialUC* pnewMat = new FEPrescribedActiveContractionUniaxialUC;
								pnewMat->Convert(dynamic_cast<FEPrescribedActiveContractionUniaxialUCOld*>(pm));
								pm = pnewMat;
							}
							else if (nid == FE_ACTIVE_CONTRACT_TISO_UC_OLD)
							{
								FEPrescribedActiveContractionTransIsoUC* pnewMat = new FEPrescribedActiveContractionTransIsoUC;
								pnewMat->Convert(dynamic_cast<FEPrescribedActiveContractionTransIsoUCOld*>(pm));
								pm = pnewMat;
							}
							else if (nid == FE_FIBEREXPPOW_COUPLED_OLD)
							{
								FEFiberExpPow* pnewMat = new FEFiberExpPow;
								pnewMat->Convert(dynamic_cast<FEFiberExpPowOld*>(pm));
								pm = pnewMat;
							}
							else if (nid == FE_FIBEREXPPOW_UNCOUPLED_OLD)
							{
								FEFiberExpPowUncoupled* pnewMat = new FEFiberExpPowUncoupled;
								pnewMat->Convert(dynamic_cast<FEFiberExpPowUncoupledOld*>(pm));
								pm = pnewMat;
							}
							else if (nid == FE_FIBERPOWLIN_COUPLED_OLD)
							{
								FEFiberPowLin* pnewMat = new FEFiberPowLin;
								pnewMat->Convert(dynamic_cast<FEFiberPowLinOld*>(pm));
								pm = pnewMat;
							}
							else if (nid == FE_FIBERPOWLIN_UNCOUPLED_OLD)
							{
								FEFiberPowLinUncoupled* pnewMat = new FEFiberPowLinUncoupled;
								pnewMat->Convert(dynamic_cast<FEFiberPowLinUncoupledOld*>(pm));
								pm = pnewMat;
							}

							if (prop)
							{
								if (prop->maxSize() == FSMaterialProperty::NO_FIXED_SIZE)
									prop->AddMaterial(pm);
								else prop->SetMaterial(pm, n);
								n++;
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
						case FE_TRANS_MOONEY_RIVLIN_OLD   : pm = new FETransMooneyRivlinOld; break;
						case FE_TRANS_VERONDA_WESTMANN_OLD: pm = new FETransVerondaWestmannOld; break;
						case FE_COUPLED_TRANS_ISO_MR_OLD  : pm = new FECoupledTransIsoMooneyRivlinOld; break;
						default:
							pm = FEMaterialFactory::Create(nid);
						}
						assert(pm);
						if (pm) 
						{
							pm->Load(ar);
							prop->AddMaterial(pm);
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
