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
#include <GeomLib/GObject.h>

class FEMultiQuadMesh;

//-----------------------------------------------------------------------------
//! The GMultiBox class is the geometry that is used to create a multi-block mesh. 
//! A multi-block mesh is basically a mesh that is composed of rectangular blocks. 
//! The user can edit the number of partitions by splitting edges, faces and parts.
//
class GMultiPatch : public GObject
{
public:
	//! constructor
	GMultiPatch();
	GMultiPatch(GObject* po);

	bool DeletePart(GPart* pg) override;

	FEMeshBase* GetEditableMesh() override;

	GObject* Clone() override;

	bool Merge(GMultiPatch& mb);

private:
	void BuildObject(FEMultiQuadMesh& mb);
};
