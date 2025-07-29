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
#include "GLCamera.h"
#include <FSCore/FSObject.h>

class CGViewKey : public FSObject
{
public:
	GLCameraTransform transform;
};

//-----------------------------------------------------------------------------
// This class stores viewing information
class CGView : public FSObject
{
public:
	CGView();
	~CGView();

	GLCamera& GetCamera() { return m_cam; }

	void Reset();

	int CameraKeys() { return (int) m_key.size(); }

	CGViewKey& GetKey(int i) { return *m_key[i]; }
	CGViewKey& GetCurrentKey() { return *m_key[m_nkey]; }
	void SetCurrentKey(CGViewKey* pkey);
	void SetCurrentKey(int i);

	CGViewKey* AddCameraKey(GLCameraTransform& t, const std::string& name);

	void DeleteKey(CGViewKey* pt);

	void DeleteAllKeys();

	void PrevKey();
	void NextKey();

	bool OrhographicProjection() { return m_bortho; }

	double GetFOV() { return m_fov; }
	double GetAspectRatio() { return m_ar; }
	double GetNearPlane() { return m_fnear; }
	double GetFarPlane() { return m_ffar; }

public:
	bool	m_bortho;		// orthographic mode
	double	m_fnear;
	double	m_ffar;
	double	m_fov;
	double	m_ar;

protected:
	GLCamera m_cam;	//!< current camera

	std::vector<CGViewKey*>	m_key;	//!< stored camera transformations
	int						m_nkey;	//!< current key
};
