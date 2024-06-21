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
#include <string>
#include <vector>

// this class describes an FEBio optimization problem
class FEBioOpt
{
public:
	enum Objective
	{
		DataFit,
		Target,
		ElementData,
		NodeData
	};

public:
	class Param
	{
	public:
		Param(const std::string& paramName) : m_name(paramName)
		{
			m_initVal = 0.0;
			m_minVal = 0.0;
			m_maxVal = 0.0;
		}

		Param(const Param& p)
		{
			m_name = p.m_name;
			m_initVal = p.m_initVal;
			m_minVal = p.m_minVal;
			m_maxVal = p.m_maxVal;
		}

		Param& operator = (const Param& p)
		{
			m_name = p.m_name;
			m_initVal = p.m_initVal;
			m_minVal = p.m_minVal;
			m_maxVal = p.m_maxVal;

			return *this;
		}

	public:
		std::string	m_name;
		double		m_initVal;
		double		m_minVal;
		double		m_maxVal;
	};

	class Data
	{
	public:
		double	m_time;
		double	m_value;
	};

	class TargetVar
	{
	public:
		std::string	m_name;
		double		m_val;
	};

public:
	FEBioOpt()
	{
		m_method = 0;
		m_obj_tol = 1e-4;
		m_f_diff_scale = 0.001;
		m_outLevel = 0;
		m_printLevel = 0;
		m_objective = Objective::DataFit;
	}

	void AddParameter(const Param& p)
	{
		m_params.push_back(p);
	}

	void AddData(double timeVal, double value)
	{
		Data a;
		a.m_time = timeVal;
		a.m_value = value;
		m_data.push_back(a);
	}

public:
	int		m_method;			// optimization method
	double	m_obj_tol;		// objective tolerance
	double	m_f_diff_scale;	// forward difference scale factor
	int		m_outLevel;		// output level
	int		m_printLevel;		// print level
	int		m_objective;	// objective option

	// parameters to optimize
	std::vector<Param>		m_params;

	// data-fit model
	std::string		m_objParam;	// the objective parameter
	std::vector<Data>		m_data;

	// target model
	std::vector<TargetVar>	m_trgVar;

	// element-data model
	std::string m_edVar;

	// node-data model
	std::string m_ndVar;
};

bool GenerateFEBioOptimizationFile(const std::string& fileName, FEBioOpt& opt);
