#pragma once
#include <string>

namespace Post {

class CGLContext;
class CGLModel;
class CPropertyList;

//-----------------------------------------------------------------------------
// This class is the base class for anything that affects what's get rendered
// to the 3D view. 
//
class CGLObject
{
public:
	CGLObject(CGLModel* mdl = 0);
	virtual ~CGLObject();

	// update contents
	virtual void Update(int ntime, float dt, bool breset) {}
	virtual void Update() {}

	// get the name
	const std::string& GetName() const;
	void SetName(const std::string& szname);

	// (de-)activate
	virtual void Activate(bool bact) { m_bactive = bact; }
	bool IsActive() { return m_bactive; }

	CGLModel* GetModel() { return m_pModel; }
	void SetModel(CGLModel* pm) { m_pModel = pm; }

	virtual CPropertyList* propertyList() { return 0; }

protected:
	std::string		m_name;
	bool			m_bactive;
	CGLModel*		m_pModel;
};

//-----------------------------------------------------------------------------
// This is the base class for anything that can draw itself to the 3D view
//
class CGLVisual : public CGLObject
{
public:
	CGLVisual(CGLModel* mdl = 0) : CGLObject(mdl) { m_bclip = true; }

	// render the object to the 3D view
	virtual void Render(CGLContext& rc) = 0;

	// allow clipping
	bool AllowClipping() { return m_bclip; }
	void AllowClipping(bool b) { m_bclip = b; }

private:
	bool	m_bclip;	// allow the object to be clipped
};
}
