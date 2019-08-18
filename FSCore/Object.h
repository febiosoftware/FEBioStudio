#pragma once
#include <MeshTools/ParamBlock.h>
#include <string>

//-----------------------------------------------------------------------------
// Base class for most classes used by FEBio Studio
// 
class CObject : public ParamContainer
{
public:
	CObject(void);
	virtual ~CObject(void);

	void SetName(const std::string& name);
	const std::string& GetName() const;

	void Save(OArchive& ar);
	void Load(IArchive& ar);

	// update the object's data
	virtual bool Update(bool b = true) { return true; }

private:
	std::string		m_name;
};

