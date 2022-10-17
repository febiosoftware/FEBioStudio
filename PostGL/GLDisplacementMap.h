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
#include "GLDataMap.h"
#include <FECore/vec3d.h>
#include <vector>

namespace Post {

//-----------------------------------------------------------------------------
// This datamap maps vector data to nodal displacements
class CGLDisplacementMap : public CGLDataMap
{
	enum { DATA_FIELD, SCALE };

public:
	CGLDisplacementMap(CGLModel* po);

	void Update(int ntime, float dt, bool breset) override;
	void Activate(bool b) override;

	void UpdateState(int ntime, bool breset = false);

	vec3d GetScale() { return m_scl; }
	void SetScale(vec3d f) { m_scl = f; }

public:
	bool UpdateData(bool bsave = true) override;

	void UpdateNodes();

public:
	vec3d				m_scl;		//!< displacement scale factor
	std::vector<vec3f>	m_du;		//!< nodal displacements
	std::vector<int>	m_ntag;
};
}
