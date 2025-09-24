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

#pragma once
#include <string>
#include <FSCore/FSObject.h>
#include <GLLib/GLRenderEngine.h>

class GLContext;

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
	int		mintype;
	int		maxtype;
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
	virtual void SetModel(CGLModel* pm) { m_pModel = pm; }

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
	virtual void Render(GLRenderEngine& re, GLContext& rc) = 0;

	// allow clipping
	bool AllowClipping() { return m_bclip; }
	void AllowClipping(bool b) { m_bclip = b; }

private:
	bool	m_bclip;	// allow the object to be clipped
};
}
