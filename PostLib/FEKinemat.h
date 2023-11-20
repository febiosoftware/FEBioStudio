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
#include <vector>
#include <FSCore/math3d.h>

namespace Post {
	class FEPostModel;
}

//-----------------------------------------------------------------------------
//! This class implements a tool to apply kinematics data to a model
class FEKinemat
{
public:
	struct KINE
	{
		double m[16];

		vec3d translate();
		mat3d rotate();

		vec3d apply(const vec3d& r);
	};

	class STATE
	{
	public:
		std::vector<KINE>	D;

	public:
		STATE(){}
		STATE(const STATE& s) { D = s.D; }
		STATE& operator = (const STATE& s) { D = s.D; return (*this); }
	};

public:
	FEKinemat();

	bool Apply(Post::FEPostModel* fem, const char* szkine);
	void SetRange(int n0, int n1, int ni);

	int States() const;

	STATE& GetState(int i) { return m_State[i]; }

	bool IsKineValid() const;

	bool ReadKine(const char* szkine);

protected:
	bool BuildStates(Post::FEPostModel* glm);

protected:
	int	m_n0, m_n1, m_ni;
	std::vector<STATE>	m_State;
	bool	m_isKineValid;
};
