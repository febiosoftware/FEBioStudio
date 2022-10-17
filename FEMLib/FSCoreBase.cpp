#include "FEBase.h"
#include <FEBioLink/FEBioClass.h>
#include "FEModelComponent.h"
using namespace std;

//-----------------------------------------------------------------------------
FSProperty::FSProperty()
{
	m_parent = nullptr;
	m_npropType = -1;
	m_maxSize = NO_FIXED_SIZE;
	m_nsuperClassID = -1;
//	m_nbaseClassID = -1;
	m_flag = 0;
}

//-----------------------------------------------------------------------------
FSProperty::FSProperty(const std::string& name, int propType, FSCoreBase* parent, int nsize, unsigned int flags) : m_parent(parent)
{
	m_npropType = propType;
	m_name = name;
	m_flag = flags;
	m_maxSize = nsize;
	m_nsuperClassID = -1;
//	m_nbaseClassID = -1;
	if (nsize > 0)
	{
		m_cmp.assign(nsize, nullptr);
	}
}

//-----------------------------------------------------------------------------
FSProperty::~FSProperty()
{
	Clear();
}

//-----------------------------------------------------------------------------
void FSProperty::SetName(const std::string& name)
{
	m_name = name;
}

//-----------------------------------------------------------------------------
const std::string& FSProperty::GetName() const
{
	return m_name;
}

//-----------------------------------------------------------------------------
void FSProperty::SetLongName(const std::string& longName)
{
	m_longName = longName;
}

//-----------------------------------------------------------------------------
const std::string& FSProperty::GetLongName()
{
	return m_longName;
}

//-----------------------------------------------------------------------------
void FSProperty::Clear()
{
	for (int i = 0; i < (int)m_cmp.size(); ++i) { delete m_cmp[i]; m_cmp[i] = 0; }
	if (m_maxSize == NO_FIXED_SIZE) m_cmp.clear();
}

//-----------------------------------------------------------------------------
bool FSProperty::IsRequired() const
{
	return ((GetFlags() & FSProperty::REQUIRED) != 0);
}

//-----------------------------------------------------------------------------
bool FSProperty::IsPreferred() const
{
	return ((GetFlags() & FSProperty::PREFERRED) != 0);
}

//-----------------------------------------------------------------------------
bool FSProperty::IsTopLevel() const
{
	return ((GetFlags() & FSProperty::TOPLEVEL) != 0);
}

//-----------------------------------------------------------------------------
void FSProperty::AddComponent(FSCoreBase* pm)
{
	if (pm)
	{
		pm->SetParent(m_parent);
		pm->SetFlags(GetFlags());
	}
	if (m_maxSize == NO_FIXED_SIZE)
		m_cmp.push_back(pm);
	else
	{
		// find a zero component
		for (int i = 0; i < (int)m_cmp.size(); ++i)
		{
			if (m_cmp[i] == 0) { m_cmp[i] = pm; return; }
		}

		// TODO: I only get here for 1D point-functions, but not sure 
		//       for any other reason. 
		if (m_cmp.size() == 1)
		{
			delete m_cmp[0];
			m_cmp[0] = pm;
			return;
		}
		assert(false);
	}
}

//-----------------------------------------------------------------------------
void FSProperty::SetComponent(FSCoreBase* pm, int i)
{
	//	if (pm) assert(pm->ClassID() & m_nClassID);
	if (pm)
	{
		pm->SetParent(m_parent);
		pm->SetFlags(GetFlags());
	}
	if (m_cmp.empty() == false)
	{
		if (m_cmp[i] != pm)
		{
			delete m_cmp[i];
			m_cmp[i] = pm;
		}
	}
}

//-----------------------------------------------------------------------------
// remove a material from the list (returns false if pm is not part of the list)
bool FSProperty::RemoveComponent(FSCoreBase* pm)
{
	// find the component
	for (int i = 0; i < (int)m_cmp.size(); ++i)
	{
		if (m_cmp[i] == pm)
		{
			if (maxSize() == NO_FIXED_SIZE)
			{
				m_cmp.erase(m_cmp.begin() + i);
			}
			else m_cmp[i] = nullptr;
			delete pm;
			return true;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
FSCoreBase* FSProperty::GetComponent(int i)
{
	if ((i < 0) || (i >= (int)m_cmp.size())) return 0;
	return m_cmp[i];
}

//-----------------------------------------------------------------------------
const FSCoreBase* FSProperty::GetComponent(int i) const
{
	if ((i < 0) || (i >= (int)m_cmp.size())) return 0;
	return m_cmp[i];
}

//-----------------------------------------------------------------------------
int FSProperty::GetComponentIndex(FSCoreBase* pc)
{
	for (int i = 0; i < (int)m_cmp.size(); ++i)
	{
		if (m_cmp[i] == pc) return i;
	}
	return -1;
}

// Set a size (new components will be set to nullptr)
void FSProperty::SetSize(int newSize)
{
	assert(newSize >= 0);
	int oldSize = m_cmp.size();
	if (newSize == m_cmp.size()) return;
	if (newSize == 0) Clear();
	if (newSize < m_cmp.size())
	{
		for (int i = newSize; i < oldSize; i++) delete m_cmp[i];
		m_cmp.resize(newSize);
	}
	else
	{
		FSModelComponent* parent = dynamic_cast<FSModelComponent*>(m_parent); assert(parent);
		m_cmp.resize(newSize);
		for (int i = oldSize; i < newSize; ++i)
		{
			m_cmp[i] = nullptr;
			if ((IsRequired() || IsPreferred()) && (m_defaultType.empty() == false))
			{
				FSModel* fem = parent->GetFSModel();
				FSModelComponent* pmci = FEBio::CreateClass(GetSuperClassID(), m_defaultType, fem, IsTopLevel());
				SetComponent(pmci, i);
			}
		}
	}
}

//=============================================================================
FSCoreBase::FSCoreBase()
{
	m_classId = -1;
	m_flags = FSProperty::TOPLEVEL;
}

void FSCoreBase::SetFlags(unsigned int flags)
{
	m_flags = flags;
}

unsigned int FSCoreBase::Flags() const
{
	return m_flags;
}

size_t FSCoreBase::Properties() const
{
	return m_prop.size();
}

FSProperty& FSCoreBase::GetProperty(int i)
{
	return *m_prop[i];
}

const FSProperty& FSCoreBase::GetProperty(int i) const
{
	return *m_prop[i];
}

FSCoreBase* FSCoreBase::GetProperty(int i, int j)
{
	return m_prop[i]->GetComponent(j);
}

//-----------------------------------------------------------------------------
// set the class ID
void FSCoreBase::SetClassID(int nid)
{
	m_classId = nid;
}

// get the class ID
int FSCoreBase::GetClassID() const
{
	return m_classId;
}

//-----------------------------------------------------------------------------
// delete all properties
void FSCoreBase::ClearProperties()
{
	vector<FSProperty*>::iterator it;
	for (it = m_prop.begin(); it != m_prop.end(); ++it) (*it)->Clear();
}

//-----------------------------------------------------------------------------
// Add a component to the material
FSProperty* FSCoreBase::AddProperty(const std::string& name, int nClassID, int maxSize, unsigned int flags)
{
	FSProperty* m = new FSProperty(name, nClassID, this, maxSize, flags);
	m->SetLongName(name);
	m_prop.push_back(m);
	return m;
}

//-----------------------------------------------------------------------------
void FSCoreBase::AddProperty(const std::string& name, FSCoreBase* pm)
{
	FSProperty* p = FindProperty(name);
	assert(p);
	if (p) p->AddComponent(pm);
}

//-----------------------------------------------------------------------------
int FSCoreBase::AddProperty(int propID, FSCoreBase* pm)
{
	FSProperty& p = GetProperty(propID);
	p.AddComponent(pm);
	return (p.Size() - 1);
}

//-----------------------------------------------------------------------------
// Replace the material of a component
void FSCoreBase::ReplaceProperty(int propID, FSCoreBase* pm, int classID)
{
	m_prop[propID]->SetComponent(pm, classID);
}

//-----------------------------------------------------------------------------
FSProperty* FSCoreBase::FindProperty(const std::string& name)
{
	int n = (int)m_prop.size();
	for (int i = 0; i < n; ++i)
	{
		FSProperty* pm = m_prop[i];
		if (pm->GetName() == name) return pm;
	}
	return 0;
}

//-----------------------------------------------------------------------------
const FSProperty* FSCoreBase::FindProperty(const std::string& name) const
{
	int n = (int)m_prop.size();
	for (int i = 0; i < n; ++i)
	{
		FSProperty* pm = m_prop[i];
		if (pm->GetName() == name) return pm;
	}
	return 0;
}

//-----------------------------------------------------------------------------
// find the property by type
FSProperty* FSCoreBase::FindProperty(int ntype)
{
	int n = (int)m_prop.size();
	for (int i = 0; i < n; ++i)
	{
		FSProperty* pm = m_prop[i];
		if (pm->GetPropertyType() == ntype) return pm;
	}
	return 0;
}

//-----------------------------------------------------------------------------
// find the property by the material
FSProperty* FSCoreBase::FindProperty(FSCoreBase* pm)
{
	int NP = Properties();
	for (int i = 0; i < NP; ++i)
	{
		FSProperty& p = GetProperty(i);
		int nmat = p.Size();
		for (int j = 0; j < nmat; ++j)
		{
			if (p.GetComponent(j) == pm) return &p;
		}
	}
	return 0;
}
