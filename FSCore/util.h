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
#include "math3d.h"
#include <vector>
#include <string>

double bias(double g, double x);

double gain(double g, double x);

std::string Vec2dToString(const vec2d& r);
std::string Vec3dToString(const vec3d& r);
std::string Vec3fToString(const vec3f& r);
vec2d StringToVec2d(const std::string& s);
vec3d StringToVec3d(const std::string& s);
vec3f StringToVec3f(const std::string& s);
mat2d StringToMat2d(const std::string& s);
mat3d StringToMat3d(const std::string& s);
mat3ds StringToMat3ds(const std::string& s);
std::string Mat2dToString(const mat2d& a);
std::string Mat3dToString(const mat3d& a);
std::string Mat3dsToString(const mat3ds& a);
std::string Vec2iToString(const vec2i& r);
vec2i StringToVec2i(const std::string& s);
std::vector<int> StringToVectorInt(const std::string& s);
std::string VectorIntToString(const std::vector<int>& v);
std::vector<double> StringToVectorDouble(const std::string& s);
std::string VectorDoubleToString(const std::vector<double>& v);
