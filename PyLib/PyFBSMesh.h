/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in
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

#include <functional>
#include <MeshLib/FSMesh.h>
#include <MeshLib/FSFace.h>

namespace pybind11
{
	class module_;
}

template <typename MeshType, typename ItemType>
class PyMeshItemRef {
public:

	using GetFunc = std::function<ItemType& (MeshType*, int)>;

	PyMeshItemRef(MeshType* mesh, int index, GetFunc getFunc) : m_mesh(mesh), m_index(index), m_getFunc(getFunc) {}

	int index() const { return m_index; }

	int id() const { return m_getFunc(m_mesh, m_index).GetID(); }

	ItemType& item() { return m_getFunc(m_mesh, m_index); }

private:
	MeshType* m_mesh = nullptr;
	int m_index = -1;
	GetFunc m_getFunc;
};

class PyFaceRef : public PyMeshItemRef<FSMesh, FSFace> {
public:
	PyFaceRef(FSMesh* mesh, int index) : PyMeshItemRef<FSMesh, FSFace>(mesh, index, [](FSMesh* m, int i) -> FSFace& { return m->Face(i); }) {}
};

void init_FSMesh(pybind11::module_& m);
