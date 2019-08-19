#pragma once
#include "ParamBlock.h"
#include <string>

//-----------------------------------------------------------------------------
// Base class for most classes used by FEBio Studio
// 
class FSObject : public ParamContainer
{
public:
	FSObject(void);
	virtual ~FSObject(void);

	void SetName(const std::string& name);
	const std::string& GetName() const;

	void Save(OArchive& ar);
	void Load(IArchive& ar);

	// update the object's data
	virtual bool Update(bool b = true);

	// update parameters
	virtual void UpdateData(bool bsave = true);

private:
	std::string		m_name;
};

