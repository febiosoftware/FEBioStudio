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
#include <FSCore/math3d.h>
#include <string>

class FSMesh;

// Extrudes (sweeps) a linear shell mesh (TRI3/QUAD4 elements) in a straight direction into a solid mesh
// of PENTA6/HEX8 elements. The shell mesh is not modified.
class ShellExtruder
{
public:
	enum DirectionMode {
		AVERAGE_NORMAL,	// one direction: the area-weighted average of the shell's normals
		LOCAL_NORMAL,	// each node moves along its own (averaged) normal
		USER_DIRECTION	// one direction: m_direction (in the same coordinates as the shell mesh)
	};

public:
	ShellExtruder();

	DirectionMode	m_mode;
	vec3d			m_direction;	// used for USER_DIRECTION
	bool			m_flip;			// reverse the direction
	double			m_distance;		// total extrusion distance (negative extrudes against the direction)
	int				m_layers;		// number of layers (elements through the thickness)
	double			m_bias;			// ratio of successive layer thicknesses (1 = uniform)
	bool			m_autoPartition;// partition the resulting surface into faces based on the crease angle
	double			m_creaseAngle;	// crease (smoothing) angle in degrees

	// Build the solid mesh. Returns nullptr on failure and sets the error message.
	FSMesh* Apply(const FSMesh& shell);

	const std::string& GetErrorString() const { return m_err; }

private:
	std::string	m_err;
};
