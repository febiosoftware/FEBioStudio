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
#include "GObject.h"

//-----------------------------------------------------------------------
class OCC_Data;
class TopoDS_Shape;

//-----------------------------------------------------------------------
// Experimental class for representing OpenCascade objects
class GOCCObject : public GObject
{
public:
	GOCCObject(int type = GOCCOBJECT);

	// create the default mesher for this object
	FEMesher* CreateDefaultMesher() override;

	// build the mesh for visualization
	void BuildGMesh() override;

	// set the shape
	void SetShape(TopoDS_Shape& shape, bool update = true);

	// return the OCC shape object
	TopoDS_Shape& GetShape();

	FSMeshBase* GetEditableMesh() override;
	FSLineMesh* GetEditableLineMesh() override;

	void Save(OArchive& ar) override;
	void Load(IArchive& ar) override;

protected:
	void BuildGObject();

private:
	OCC_Data*	m_occ;
};

//-----------------------------------------------------------------------
// A Bottle built with OCC
class GOCCBottle : public GOCCObject
{
public:
	enum { WIDTH, HEIGHT, THICKNESS };

	GOCCBottle();

	// update the object's geometry
	bool Update(bool b = true) override;

private:
	void MakeBottle(double h, double w, double t);
};

//-----------------------------------------------------------------------
// A Box built with OCC
class GOCCBox : public GOCCObject
{
public:
	GOCCBox();

	// update the object's geometry
	bool Update(bool b = true) override;

private:
	void MakeBox();
};

// merge a list of occ objects into a single object
GOCCObject* MergeOCCObjects(std::vector<GOCCObject*> occlist);
