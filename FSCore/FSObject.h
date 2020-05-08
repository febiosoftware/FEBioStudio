#pragma once
#include "ParamBlock.h"
#include <string>

//-----------------------------------------------------------------------------
// Base class for most classes used by FEBio Studio
// 
class FSObject : public ParamContainer
{
public:
	FSObject(FSObject* parent = nullptr);
	virtual ~FSObject(void);

	void SetName(const std::string& name);
	const std::string& GetName() const;

	void SetInfo(const std::string& info);
	const std::string& GetInfo() const;

	void Save(OArchive& ar);
	void Load(IArchive& ar);

	// update the object's data
	virtual bool Update(bool b = true);

	// update parameters
	// return true if parameter list was modified
	virtual bool UpdateData(bool bsave = true);

public:
	void SetParent(FSObject* parent);
	FSObject* GetParent();
	const FSObject* GetParent() const;
	virtual size_t RemoveChild(FSObject* po);
	virtual void InsertChild(size_t pos, FSObject* po);

private:
	std::string		m_name;
	std::string		m_info;
	FSObject*		m_parent;
};

