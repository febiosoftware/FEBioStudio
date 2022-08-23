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
#include <FSCore/FSObject.h>
#include <vector>

namespace Post {

class LineDataModel;
class FEPostModel;

//-----------------------------------------------------------------------------
struct LINESEGMENT
{
	// nodal coordinates
	vec3f	m_r0;
	vec3f	m_r1;

	// tangents
	vec3d	m_t0;
	vec3d	m_t1;

	// values
	float	m_val[2];
	float	m_user_data[2];
	int		m_elem[2];

	// segment ID
	int	m_segId;

	// flags to identify ends
	int m_end[2];
};

class LineData
{
public:
	LineData() {}

	int Lines() const { return (int)m_Line.size(); }
	LINESEGMENT& Line(int n) { return m_Line[n]; }
	const LINESEGMENT& Line(int n) const { return m_Line[n]; }

	void Add(LINESEGMENT& line) { m_Line.push_back(line); }

	void processLines();

	void AddLine(vec3f a, vec3f b, float data_a = 0.f, float data_b = 0.f, int el0 = -1, int el1 = -1)
	{
		vec3f t = b - a; t.Normalize();

		LINESEGMENT L;
		L.m_segId = 0;
		L.m_r0 = a;
		L.m_r1 = b;
		L.m_t0 = L.m_t1 = to_vec3d(t);
		L.m_user_data[0] = data_a;
		L.m_user_data[1] = data_b;
		L.m_elem[0] = el0;
		L.m_elem[1] = el1;
		L.m_end[0] = L.m_end[1] = 0;
		m_Line.push_back(L);
	}

private:
	std::vector<LINESEGMENT>	m_Line;
};

class LineDataSource : public FSObject
{
public:
	LineDataSource(LineDataModel* mdl);
	virtual ~LineDataSource() {}

	virtual bool Load(const char* szfile) = 0;
	virtual bool Reload() = 0;

	LineDataModel* GetLineDataModel() { return m_mdl; }

private:
	LineDataModel* m_mdl;
};

class LineDataModel
{
public:
	LineDataModel(FEPostModel* fem);
	~LineDataModel();

	void Clear();

	LineData& GetLineData(size_t n) { return m_line[n]; }

	int States() const { return (int)m_line.size(); }

	FEPostModel* GetFEModel() { return m_fem; }

	void SetLineDataSource(LineDataSource* src) { m_src = src; }
	LineDataSource* GetLineDataSource() { return m_src; }

	bool Reload()
	{
		if (m_src) return m_src->Reload();
		else return false;
	}

private:
	LineDataSource* m_src;
	FEPostModel* m_fem;
	std::vector<LineData>	m_line;
};


class Ang2LineDataSource : public Post::LineDataSource
{
	enum {
		MP_FILENAME
	};

public:
	Ang2LineDataSource(Post::LineDataModel* mdl);

	bool UpdateData(bool bsave) override;

	bool Load(const char* szfile) override;

	bool Reload() override;

private:
	int ReadAng2Format(const char* szfile, Post::LineDataModel& lineData);
	bool ReadOldFormat(const char* szfile, Post::LineDataModel& lineData);

private:
	std::string		m_fileName;
};

} // namespace Post
