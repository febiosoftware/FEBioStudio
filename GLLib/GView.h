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

//-----------------------------------------------------------------------------
// This class stores viewing information
class CGView : public FSObject
{
public:
    enum UiView
    {
        MODEL_VIEW, SLICE_VIEW, TIME_VIEW_2D
    };

public:
	CGView();
	~CGView();

	CGLCamera& GetCamera() { return m_cam; }

	void Reset();

	int CameraKeys() { return (int) m_key.size(); }

	GLCameraTransform& GetKey(int i) { return *m_key[i]; }
	GLCameraTransform& GetCurrentKey() { return *m_key[m_nkey]; }
	void SetCurrentKey(GLCameraTransform* pkey);
	void SetCurrentKey(int i);

	GLCameraTransform* AddCameraKey(GLCameraTransform& t);

	void DeleteKey(GLCameraTransform* pt);

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
    UiView imgView;

protected:
	CGLCamera m_cam;	//!< current camera

	std::vector<GLCameraTransform*>	m_key;	//!< stored camera transformations
	int								m_nkey;	//!< current key
};
