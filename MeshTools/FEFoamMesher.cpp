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

#include "FEFoamMesher.h"
#include <GeomLib/GFoamObject.h>
#include "FoamMesh.h"

FEFoamMesher::FEFoamMesher()
{
	AddIntParam(5, "seeds");
	AddIntParam(10, "nx");
	AddIntParam(10, "ny");
	AddIntParam(10, "nz");
	AddDoubleParam(0.5, "ref")->SetFloatRange(0, 1);
}

FSMesh* FEFoamMesher::BuildMesh(GObject* po)
{
	GFoamObject* foamObj = dynamic_cast<GFoamObject*>(po);
	if (foamObj == nullptr) return nullptr;

	FoamGen foam;
	foam.m_w = foamObj->GetFloatValue(GFoamObject::WIDTH);
	foam.m_h = foamObj->GetFloatValue(GFoamObject::HEIGHT);
	foam.m_d = foamObj->GetFloatValue(GFoamObject::DEPTH);

	foam.m_nseed = GetIntValue(0);
	foam.m_nx = GetIntValue(1);
	foam.m_ny = GetIntValue(2);
	foam.m_nz = GetIntValue(3);
	foam.m_ref = GetFloatValue(4);

	FSMesh* mesh = foam.Create();

	return mesh;
}
