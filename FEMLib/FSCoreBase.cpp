#include "FEBase.h"

//-----------------------------------------------------------------------------
FSProperty::FSProperty()
{
	m_parent = nullptr;
	m_nClassID = -1;
	m_maxSize = NO_FIXED_SIZE;
	m_nsuperClassID = -1;
	m_flag = EDITABLE;
}

//-----------------------------------------------------------------------------
FSProperty::FSProperty(const std::string& name, int nClassID, FSCoreBase* parent, int nsize, unsigned int flags) : m_parent(parent)
{
	m_nClassID = nClassID;
	m_name = name;
	m_flag = flags;
	m_maxSize = nsize;
	m_nsuperClassID = -1;
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
const std::string& FSProperty::GetName()
{
	return m_name;
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
void FSProperty::AddComponent(FSCoreBase* pm)
{
	if (pm) pm->SetParent(m_parent);
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
	if (pm) pm->SetParent(m_parent);
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
			m_cmp.erase(m_cmp.begin() + i);
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
int FSProperty::GetComponentIndex(FSCoreBase* pc)
{
	for (int i = 0; i < (int)m_cmp.size(); ++i)
	{
		if (m_cmp[i] == pc) return i;
	}
	return -1;
}

//=============================================================================
FSCoreBase::FSCoreBase()
{

}

size_t FSCoreBase::Properties() const
{
	return m_prop.size();
}

FSProperty& FSCoreBase::GetProperty(int i)
{
	return *m_prop[i];
}

FSCoreBase* FSCoreBase::GetProperty(int i, int j)
{
	return m_prop[i]->GetComponent(j);
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
// find the property by type
FSProperty* FSCoreBase::FindProperty(int ntype)
{
	int n = (int)m_prop.size();
	for (int i = 0; i < n; ++i)
	{
		FSProperty* pm = m_prop[i];
		if (pm->GetClassID() == ntype) return pm;
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
