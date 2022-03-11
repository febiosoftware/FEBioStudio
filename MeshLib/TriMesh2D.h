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
#include <list>
#include <FECore/vec3d.h>

//-----------------------------------------------------------------------------
// Class that represents a triangle mesh in 2D
//
class TriMesh2D
{
private:
	// forward declaration of node, edge, and face classes
	class NODE;
	class EDGE;
	class FACE;

	// "pointers" to the items
	typedef std::list<NODE>::iterator NODEP;
	typedef std::list<EDGE>::iterator EDGEP;
	typedef std::list<FACE>::iterator FACEP;

	// class representing a vertex
	// only stores the vertex coordinates
	class NODE
	{
	public:
		vec2d	pos;	// spatial position
		int		tag;	// user tag

	public:
		NODE() : tag(0) {}
		explicit NODE(const vec2d& v) : pos(v), tag(0) {}
	};

	class EDGE
	{
	public:
		NODEP	node[2];		
	};

public:
	// constructor. constructs and empty mesh
	TriMesh2D();

private:
};