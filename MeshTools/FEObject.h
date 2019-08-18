#pragma once
#include "ParamBlock.h"
#include <string>

//-----------------------------------------------------------------------------
// Base class for most classes used by PreView
// 
class FEObject : public ParamContainer
{
public:
	FEObject(void);
	virtual ~FEObject(void);

	void SetName(const std::string& name);
	const std::string& GetName() const;

	void Save(OArchive& ar);
	void Load(IArchive& ar);

	// update the object's data
	virtual bool Update(bool b = true) { return true; }

private:
	std::string		m_name;
};
