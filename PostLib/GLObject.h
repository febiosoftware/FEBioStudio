#pragma once
#include <string>
#include <FSCore/FSObject.h>


class CGLContext;

namespace Post {

// enum for setting range types
enum Range_Type {
	RANGE_DYNAMIC = 0,
	RANGE_STATIC  = 1,
	RANGE_USER    = 2
};

struct DATA_RANGE
{
	float	min;
	float	max;
	int		ntype;
	bool	valid;
};

class CGLModel;

//-----------------------------------------------------------------------------
// This class is the base class for anything that affects what's get rendered
// to the 3D view. 
//
class CGLObject : public FSObject
{
public:
	CGLObject(CGLModel* mdl = 0);
	virtual ~CGLObject();

	// update contents
	virtual void Update(int ntime, float dt, bool breset) {}
	virtual void Update() {}

	// (de-)activate
	virtual void Activate(bool bact) { m_bactive = bact; }
	bool IsActive() { return m_bactive; }

	CGLModel* GetModel() { return m_pModel; }
	void SetModel(CGLModel* pm) { m_pModel = pm; }

	virtual void ChangeName(const std::string& name);

protected:
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
