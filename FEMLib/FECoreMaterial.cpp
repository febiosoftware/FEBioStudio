#include "FECoreMaterial.h"
#include "FEMaterialFactory.h"
#include "FEMaterial.h"
#include <FSCore/paramunit.h>

//=============================================================================
// FEMaterialProperty
//=============================================================================

//-----------------------------------------------------------------------------
FEMaterialProperty::FEMaterialProperty()
{
	m_parent = 0;
	m_nClassID = -1;
	m_maxSize = NO_FIXED_SIZE;
}

//-----------------------------------------------------------------------------
FEMaterialProperty::FEMaterialProperty(const std::string& name, int nClassID, FEMaterial* parent, int nsize, unsigned int flags) : m_parent(parent)
{
	m_nClassID = nClassID;
	m_name = name;
	m_flag = flags;
	m_maxSize = nsize;
	if (nsize > 0)
	{
		m_mat.assign(nsize, 0);
	}
}

//-----------------------------------------------------------------------------
FEMaterialProperty::~FEMaterialProperty()
{
	Clear();
}

//-----------------------------------------------------------------------------
void FEMaterialProperty::SetName(const std::string& name)
{
	m_name = name;
}

//-----------------------------------------------------------------------------
const std::string& FEMaterialProperty::GetName()
{ 
	return m_name; 
}

//-----------------------------------------------------------------------------
void FEMaterialProperty::Clear()
{
	for (int i = 0; i<(int)m_mat.size(); ++i) { delete m_mat[i]; m_mat[i] = 0; }
	if (m_maxSize == NO_FIXED_SIZE) m_mat.clear();
}

//-----------------------------------------------------------------------------
void FEMaterialProperty::AddMaterial(FEMaterial* pm)
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
void FEMaterialProperty::SetMaterial(FEMaterial* pm, int i) 
{ 
	if (pm) assert(pm->ClassID() & m_nClassID);
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
bool FEMaterialProperty::RemoveMaterial(FEMaterial* pm)
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
FEMaterial* FEMaterialProperty::GetMaterial(int i)
{	
	if ((i<0) || (i>=(int)m_mat.size())) return 0;
	return m_mat[i]; 
}

//-----------------------------------------------------------------------------
int FEMaterialProperty::GetMaterialIndex(FEMaterial* mat)
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

FEAxisMaterial::FEAxisMaterial() : FEMaterial(0)
{
	m_naopt = -1;
	m_n[0] = 0; m_n[1] = 1; m_n[2] = 2;
	m_a = vec3d(1, 0, 0);
	m_d = vec3d(0, 1, 0);
    m_theta = 0;
    m_phi = 90;

	AddIntParam(0, "Axes")->SetEnumNames("(none)\0local\0vector\0angles\0\0");
	AddIntParam(0, "n0");
	AddIntParam(0, "n1");
	AddIntParam(0, "n2");
	AddVecParam(vec3d(1,0,0), "a");
	AddVecParam(vec3d(0,1,0), "d");
    AddScienceParam(0 , UNIT_DEGREE, "theta");
    AddScienceParam(90, UNIT_DEGREE, "phi");

	for (int i = 1; i < Parameters(); ++i) GetParam(i).SetState(0);
}

bool FEAxisMaterial::UpdateData(bool bsave)
{
	if (bsave)
	{
		int oldopt = m_naopt;

		for (int i = 1; i < Parameters(); ++i) GetParam(i).SetState(0);

		m_naopt = GetIntValue(0) - 1;
		switch (m_naopt)
		{
		case -1: break;
		case 0: 
			GetParam(1).SetState(Param_ALLFLAGS);
			GetParam(2).SetState(Param_ALLFLAGS);
			GetParam(3).SetState(Param_ALLFLAGS);
			m_n[0] = GetIntValue(1);
			m_n[1] = GetIntValue(2);
			m_n[2] = GetIntValue(3);
			break;
		case 1:
			GetParam(4).SetState(Param_ALLFLAGS);
			GetParam(5).SetState(Param_ALLFLAGS);
			m_a = GetVecValue(4);
			m_d = GetVecValue(5);
			break;
        case 2:
            GetParam(6).SetState(Param_ALLFLAGS);
            GetParam(7).SetState(Param_ALLFLAGS);
            m_theta = GetFloatValue(6);
            m_phi = GetFloatValue(7);
            break;
		}

		return (oldopt != m_naopt);
	}
	else
	{
		SetIntValue(0, m_naopt + 1);
		SetIntValue(1, m_n[0]);
		SetIntValue(2, m_n[1]);
		SetIntValue(3, m_n[2]);
		SetVecValue(4, m_a);
		SetVecValue(5, m_d);
        SetFloatValue(6, m_theta);
        SetFloatValue(7, m_phi);
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
    }

	mat3d Q;
	Q.unit();
	return Q;
}

//=============================================================================
// FEMaterial
//=============================================================================

//-----------------------------------------------------------------------------
FEMaterial::FEMaterial(int ntype) : m_ntype(ntype), m_axes(ntype != 0 ? new FEAxisMaterial : nullptr)
{
	m_parent = 0;
	m_owner = 0;
	m_hasMatAxes = false;
}

//-----------------------------------------------------------------------------
FEMaterial::~FEMaterial()
{
	ClearProperties();
	delete m_axes;
}

//-----------------------------------------------------------------------------
int FEMaterial::ClassID()
{
	FEMaterialFactory& MF = *FEMaterialFactory::GetInstance();
	return MF.ClassID(this);
}

//-----------------------------------------------------------------------------
const char* FEMaterial::TypeStr()
{
	FEMaterialFactory& MF = *FEMaterialFactory::GetInstance();
	return MF.TypeStr(this);
}

//-----------------------------------------------------------------------------
void FEMaterial::SetParentMaterial(FEMaterial* pmat)
{
	assert((m_parent==0) || (m_parent == pmat));
	m_parent = pmat;
}

//-----------------------------------------------------------------------------
const FEMaterial* FEMaterial::GetParentMaterial() const
{
	return m_parent;
}

//-----------------------------------------------------------------------------
const FEMaterial* FEMaterial::GetAncestor() const
{
	if (m_parent) return m_parent->GetAncestor();
	return this;
}

//-----------------------------------------------------------------------------
GMaterial* FEMaterial::GetOwner() const 
{ 
	return m_owner; 
}

//-----------------------------------------------------------------------------
void FEMaterial::SetOwner(GMaterial* owner)
{
	m_owner = owner;
}

//-----------------------------------------------------------------------------
// delete all material properties
void FEMaterial::ClearProperties()
{
	vector<FEMaterialProperty*>::iterator it;
	for (it = m_Mat.begin(); it != m_Mat.end(); ++it) (*it)->Clear();
}

//-----------------------------------------------------------------------------
// Add a component to the material
void FEMaterial::AddProperty(const std::string& name, int nClassID, int maxSize, unsigned int flags)
{
	FEMaterialProperty* m = new FEMaterialProperty(name, nClassID, this, maxSize, flags);
	m_Mat.push_back(m);
}

//-----------------------------------------------------------------------------
void FEMaterial::AddProperty(const std::string& name, FEMaterial* pm)
{
	FEMaterialProperty* p = FindProperty(name);
	assert(p);
	if (p) p->AddMaterial(pm);
}

//-----------------------------------------------------------------------------
int FEMaterial::AddProperty(int propID, FEMaterial* pm)
{
	FEMaterialProperty& p = GetProperty(propID);
	p.AddMaterial(pm);
	return (p.Size() - 1);
}

//-----------------------------------------------------------------------------
// Replace the material of a component
void FEMaterial::ReplaceProperty(int propID, FEMaterial* pm, int matID)
{
	m_Mat[propID]->SetMaterial(pm, matID);
}

//-----------------------------------------------------------------------------
FEMaterialProperty* FEMaterial::FindProperty(const std::string& name)
{
	int n = (int) m_Mat.size();
	for (int i=0; i<n; ++i)
	{
		FEMaterialProperty* pm = m_Mat[i];
		if (pm->GetName() == name) return pm;
	}
	return 0;
}

//-----------------------------------------------------------------------------
// find the property by type
FEMaterialProperty* FEMaterial::FindProperty(int ntype)
{
	int n = (int)m_Mat.size();
	for (int i = 0; i<n; ++i)
	{
		FEMaterialProperty* pm = m_Mat[i];
		if (pm->GetClassID() == ntype) return pm;
	}
	return 0;
}

//-----------------------------------------------------------------------------
// find the property by the material
FEMaterialProperty* FEMaterial::FindProperty(FEMaterial* pm)
{
	int NP = Properties();
	for (int i=0; i<NP; ++i)
	{
		FEMaterialProperty& p = GetProperty(i);
		int nmat = p.Size();
		for (int j=0; j<nmat; ++j)
		{
			if (p.GetMaterial(j) == pm) return &p;
		}
	}
	return 0;
}

//-----------------------------------------------------------------------------
void FEMaterial::copy(FEMaterial* pm)
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
		FEMaterialProperty& mcd = GetProperty(i);
		FEMaterialProperty& mcs = pm->GetProperty(i);

		if ((mcs.Size() == 1)&&(mcd.Size() == 1))
		{
			if (mcs.GetMaterial())
			{
				FEMaterial* pm = FEMaterialFactory::Create(mcs.GetMaterial()->Type());
				pm->copy(mcs.GetMaterial());
				mcd.SetMaterial(pm);
			}
			else mcs.SetMaterial(0);
		}
		else
		{
			for (int j=0; j<mcs.Size(); ++j)
			{
				FEMaterial* pmj = mcs.GetMaterial(j);
				FEMaterial* pm = FEMaterialFactory::Create(pmj->Type());
				pm->copy(pmj);
				mcd.AddMaterial(pm);
			}
		}
	}
}

//-----------------------------------------------------------------------------
bool FEMaterial::HasMaterialAxes() const
{
	return m_hasMatAxes;
}

//-----------------------------------------------------------------------------
mat3d FEMaterial::GetMatAxes(FEElementRef& el)
{
	return (m_axes ? m_axes->GetMatAxes(el) : mat3d(1,0,0, 0,1,0, 0,0,1));
}

//-----------------------------------------------------------------------------
// Save the material data to the archive
void FEMaterial::Save(OArchive& ar)
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
				FEMaterialProperty& mpi = GetProperty(i);

				// store the property name
				ar.WriteChunk(CID_MAT_PROPERTY_NAME, mpi.GetName());

				// store the property data
				ar.BeginChunk(CID_MAT_PROPERTY_MAT);
				{
					for (int j = 0; j<mpi.Size(); ++j)
					{
						FEMaterial* pm = mpi.GetMaterial(j);
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
void FEMaterial::Load(IArchive &ar)
{
	TRACE("FEMaterial::Load");

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
			if (m_axes)
			{
				while (IArchive::IO_OK == ar.OpenChunk())
				{
					int nid = (int)ar.GetChunkID();
					switch (nid)
					{
					case 0: ar.read(m_axes->m_naopt); break;
					case 1: ar.read(m_axes->m_n, 3); break;
					case 2: ar.read(m_axes->m_a); break;
					case 3: ar.read(m_axes->m_d); break;
                    case 4: ar.read(m_axes->m_theta); break;
                    case 5: ar.read(m_axes->m_phi); break;
					}
					ar.CloseChunk();
				}
				m_axes->UpdateData(false);
				m_axes->UpdateData(true);
			}
			}
			break;
        case CID_MAT_PROPERTY:
			{
				FEMaterialProperty* prop = 0;
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

							FEMaterial* pm = FEMaterialFactory::Create(nid);
							assert(pm);
							pm->Load(ar);
							if (prop)
							{
								if (prop->maxSize() == FEMaterialProperty::NO_FIXED_SIZE)
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
						FEMaterial* pm = 0;
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
