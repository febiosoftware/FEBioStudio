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
#include "LSDynaFile.h"
#include "LSDYNAModel.h"
#include <string>

class LSDynaFileParser
{
public:
	LSDynaFileParser(LSDynaFile& lsfile, LSDYNAModel& lsm) : m_ls(lsfile), m_dyn(lsm), m_err("") {}

	bool ParseFile();

	const char* GetErrorString() const { return m_err.c_str(); }

	void ClearError();

private:
	bool Error(const std::string& err);

private:

protected:
	bool Read_Element_Solid();
	bool Read_Element_Shell();
	bool Read_Element_Shell_Thickness();
	bool Read_Section_Shell();
	bool Read_Section_Solid();
	bool Read_Section_Solid_Title();
	bool Read_Element_Discrete();
	bool Read_Node();
	bool Read_Nodal_Results();
	bool Read_Part();
	bool Read_Part_Contact();
	bool Read_Material();
	bool Read_Mat_Elastic();
	bool Read_Mat_Rigid();
	bool Read_Mat_Viscoelastic();
	bool Read_Mat_Kelvin_Maxwell_Viscoelastic();
	bool Read_Mat_Elastic_Spring_Discrete_Beam();
	bool Read_Mat_Spring();
	bool Read_Mat_Other();
	bool Read_Set_Segment_Title();
	bool Read_Include();
	bool Read_Define_Curve();
	bool Read_Define_Curve_Title();
	bool Read_Parameter();
	bool Read_Parameter_Expression();
	bool Read_Set_Node_List_Title();

private:
	LSDynaFile& m_ls;
	LSDYNAModel& m_dyn;
	std::string	m_err;
};
