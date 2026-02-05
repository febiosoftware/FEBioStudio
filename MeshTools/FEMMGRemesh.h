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
#include "FEModifier.h"
#include "FESurfaceModifier.h"

//-----------------------------------------------------------------------------
//! This modifier refines a tet mesh using MMG.
class MMGRemesh : public FEModifier
{
	enum {
		ELEM_SIZE,
		HMIN,
		HAUSDORFF,
		HGRAD,
		ANGLE,
		SELECTED_ONLY,
		PRESERVE_SURFACE
	};

public:
	MMGRemesh();
	FSMesh* Apply(FSMesh* pm) override;
	FSMesh* Apply(FSGroup* pg) override;

private:
	FSMesh* RemeshTET4(FSMesh* pm);
	FSMesh* RemeshTRI3(FSMesh* pm);
};

//-----------------------------------------------------------------------------
//! This modifier refines a tet mesh using MMG.
class MMGSurfaceRemesh : public FESurfaceModifier
{
	enum {
		ELEM_SIZE,
		HMIN,
		HAUSDORFF,
		HGRAD,
		ANGLE,
		SELECTED_ONLY
	};

public:
	MMGSurfaceRemesh();
	FSSurfaceMesh* Apply(FSSurfaceMesh* pm) override;
    void SetElementSize(double h) { SetFloatValue(ELEM_SIZE, h); }
    void SetElementMinSize(double hmin) { SetFloatValue(HMIN, hmin); }
    void SetHausdorf(double haus) { SetFloatValue(HAUSDORFF, haus); }
    void SetGradation(double hgrad) { SetFloatValue(HGRAD, hgrad); }
};

//-----------------------------------------------------------------------------
//! This modifier refines a tet mesh using MMG.
class MMG2DRemesh : public FESurfaceModifier
{
	enum {
		ELEM_SIZE,
		HMIN,
		HAUSDORFF,
		HGRAD,
		SELECTED_ONLY
	};

public:
	MMG2DRemesh();
	FSSurfaceMesh* Apply(FSSurfaceMesh* pm) override;
};
