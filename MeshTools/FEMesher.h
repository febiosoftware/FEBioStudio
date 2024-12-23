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
#include <FSCore/FSThreadedTask.h>

class GObject;
class FSMesh;

// mesher types
// Don't change the values!
enum FEMesherType {
	Default_Mesher = 0,
	TetGen_Mesher = 1,
	Shell_Mesher = 2,
	NetGen_OCC_Mesher = 3,
	NetGen_STL_Mesher = 4
};

//-----------------------------------------------------------------------------
// The FEMesher class takes a geometry object and converts it to a finite
// element mesh.
//

class FEMesher : public FSThreadedTask
{
	enum {PARAMS};

public:
	// constructor
	FEMesher();

	// desctructor
	virtual ~FEMesher();

	// build the mesh
	virtual FSMesh*	BuildMesh() = 0;

	// save/load
	void Save(OArchive& ar);
	void Load(IArchive& ar);

	// set the error message
	void SetErrorMessage(const std::string& err);
	std::string GetErrorMessage() const;

	int Type() const;

protected:
	void SetType(int ntype);

public:
	static FEMesher* Create(GObject* po, int classType);

protected:
	std::string		m_error;

	int	m_ntype;
};
