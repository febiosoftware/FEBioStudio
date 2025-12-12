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
#include <FECore/FETransform.h>
#include <vector>

class GObject;


class GICPRegistration
{
public:
	GICPRegistration();

	// returns the transform from registring source to target
	Transform Register(GObject* ptrg, GObject* psrc);
	Transform Register(const std::vector<vec3d>& trg, const std::vector<vec3d>& src);

	void SetMaxIterations(int n) { m_maxiter = n; }
	void SetTolerance(double tol) { m_tol = tol; }
	void SetOutputLevel(int n) { m_outputLevel = n; }

	int Iterations() const { return m_iters; }
	double RelativeError() const { return m_err; }
	int OutputLevel() const { return m_outputLevel; }

private:
	Transform Register(const std::vector<vec3d>& P0, const std::vector<vec3d>& Y, double* err);
	void ApplyTransform(const std::vector<vec3d>& P0, const Transform& Q, std::vector<vec3d>& P);

private:
	double	m_tol;
	int		m_maxiter;
	int		m_outputLevel;

	int		m_iters;
	double	m_err;
};
