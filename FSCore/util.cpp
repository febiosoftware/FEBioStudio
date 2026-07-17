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
#include "util.h"
#include <math.h>

double bias(double b, double x)
{
	const double f = 1.f / (double)log(0.5);
	return (double)pow(x, log(b) * f);
}

double gain(double g, double x)
{
	if (x < 0.5f)
		return bias(1.f - g, 2.f * x) * 0.5f;
	else
		return 1.f - bias(1.f - g, 2.f - 2.f * x) * 0.5f;
}


vec2d StringToVec2d(const std::string& s)
{
	const char* sz = s.c_str();
	double x = 0.0, y = 0.0;
	if (strcmp(sz, "x") == 0) return vec2d(1, 0);
	if (strcmp(sz, "y") == 0) return vec2d(0, 1);
	if (sz[0] == '{')
		sscanf(sz, "{%lg,%lg}", &x, &y);
	else
		sscanf(sz, "%lg,%lg", &x, &y);
	return vec2d(x, y);
}

vec3d StringToVec3d(const std::string& s)
{
	const char* sz = s.c_str();
	vec3d r(0, 0, 0);
	if (strcmp(sz, "x") == 0) return vec3d(1, 0, 0);
	if (strcmp(sz, "y") == 0) return vec3d(0, 1, 0);
	if (strcmp(sz, "z") == 0) return vec3d(0, 0, 1);
	if (sz[0] == '{')
		sscanf(sz, "{%lg,%lg,%lg}", &r.x, &r.y, &r.z);
	else
		sscanf(sz, "%lg,%lg,%lg", &r.x, &r.y, &r.z);
	return r;
}

vec3f StringToVec3f(const std::string& s)
{
	const char* sz = s.c_str();
	vec3f r(0, 0, 0);
	if (strcmp(sz, "x") == 0) return vec3f(1, 0, 0);
	if (strcmp(sz, "y") == 0) return vec3f(0, 1, 0);
	if (strcmp(sz, "z") == 0) return vec3f(0, 0, 1);
	if (sz[0] == '{')
		sscanf(sz, "{%g,%g,%g}", &r.x, &r.y, &r.z);
	else
		sscanf(sz, "%g,%g,%g", &r.x, &r.y, &r.z);
	return r;
}

mat3d StringToMat3d(const std::string& s)
{
	const char* sz = s.c_str();
	double a[9] = { 0 };
	int n = 0;
	if (sz[0] == '{')
	{
		n = sscanf(sz, "{{%lg,%lg,%lg},{%lg,%lg,%lg},{%lg,%lg,%lg}}", a, a + 1, a + 2, a + 3, a + 4, a + 5, a + 6, a + 7, a + 8);
	}
	else
	{
		n = sscanf(sz, "%lg,%lg,%lg,%lg,%lg,%lg,%lg,%lg,%lg", a, a + 1, a + 2, a + 3, a + 4, a + 5, a + 6, a + 7, a + 8);
	}

	if (n == 1) return mat3d(mat3dd(a[0]));
	else if (n == 3) return mat3d(mat3dd(a[0], a[1], a[2]));
	else return mat3d(a);
}

mat3ds StringToMat3ds(const std::string& s)
{
	const char* sz = s.c_str();
	double a[6] = { 0 };
	if (sz[0] == '{')
		sscanf(sz, "{%lg,%lg,%lg,%lg,%lg,%lg}", a, a + 1, a + 2, a + 3, a + 4, a + 5);
	else
		sscanf(sz, "%lg,%lg,%lg,%lg,%lg,%lg", a, a + 1, a + 2, a + 3, a + 4, a + 5);
	return mat3ds(a[0], a[1], a[2], a[3], a[4], a[5]);
}

vec2i StringToVec2i(const std::string& s)
{
	const char* sz = s.c_str();
	vec2i r;
	sscanf(sz, "%d,%d", &r.x, &r.y);
	return r;
}

std::string VectorIntToString(const std::vector<int>& v)
{
	std::string s;
	for (int i = 0; i < v.size(); ++i)
	{
		s += std::to_string(v[i]);
		if (i != v.size() - 1) s += ",";
	}
	return s;
}

std::vector<int> StringToVectorInt(const std::string& s)
{
	std::vector<int> v;
	if (s.empty()) return v;
	const char* sz = s.c_str();
	while (sz && *sz) {
		const char* ch = strchr(sz, ',');
		int n = atoi(sz);
		v.push_back(n);
		if (ch) sz = ch + 1; else sz = nullptr;
	};
	return v;
}

std::string VectorDoubleToString(const std::vector<double>& v)
{
	std::string s;
	for (int i = 0; i < v.size(); ++i)
	{
		s += std::to_string(v[i]);
		if (i != v.size() - 1) s += ",";
	}
	return s;
}

std::vector<double> StringToVectorDouble(const std::string& s)
{
	std::vector<double> v;
	if (s.empty()) return v;
	const char* sz = s.c_str();
	while (sz && *sz) {
		const char* ch = strchr(sz, ',');
		double f = atof(sz);
		v.push_back(f);
		if (ch) sz = ch + 1; else sz = nullptr;
	};
	return v;
}

std::string Vec2dToString(const vec2d& r)
{
	return "{" + std::to_string(r.x()) + "," + std::to_string(r.y()) + "}";
}

std::string Vec3dToString(const vec3d& r)
{
	return "{" + std::to_string(r.x) + "," + std::to_string(r.y) + "," + std::to_string(r.z) + "}";
}

std::string Vec3fToString(const vec3f& r)
{
	return "{" + std::to_string(r.x) + "," + std::to_string(r.y) + "," + std::to_string(r.z) + "}";
}

std::string Vec2iToString(const vec2i& r)
{
	return "{" + std::to_string(r.x) + "," + std::to_string(r.y) + "}";
}

std::string Mat3dToString(const mat3d& a)
{
	std::string s;
	s += "{";
	for (int i = 0; i < 3; ++i)
	{
		s += "{";
		for (int j = 0; j < 3; ++j)
		{
			s += std::to_string(a(i, j));
			if (j != 2) s += ",";
		}
		s += "}";
		if (i != 2) s += ",";
	}
	s += "}";
	return s;
}

std::string Mat3dsToString(const mat3ds& a)
{
	std::string s;
	s = "{" + std::to_string(a.xx());
	s += "," + std::to_string(a.yy());
	s += "," + std::to_string(a.zz());
	s += "," + std::to_string(a.xy());
	s += "," + std::to_string(a.yz());
	s += "," + std::to_string(a.xz()) + "}";
	return s;
}
