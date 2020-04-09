#pragma once
#include <string>
#include <vector>

// this class describes an FEBio optimization problem
class FEBioOpt
{
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

public:
	FEBioOpt()
	{
		method = 0;
		obj_tol = 1e-4;
		f_diff_scale = 0.001;
		outLevel = 0;
		printLevel = 0;
	}

	FEBioOpt(const FEBioOpt& op)
	{
		method = op.method;
		obj_tol = op.obj_tol;
		f_diff_scale = op.f_diff_scale;
		m_params = op.m_params;
		m_data = op.m_data;
		m_objParam = op.m_objParam;
	}

	FEBioOpt& operator = (const FEBioOpt& op)
	{
		method = op.method;
		obj_tol = op.obj_tol;
		f_diff_scale = op.f_diff_scale;
		m_params = op.m_params;
		m_data = op.m_data;
		m_objParam = op.m_objParam;

		return *this;
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
	int		method;			// optimization method
	double	obj_tol;		// objective tolerance
	double	f_diff_scale;	// forward difference scale factor
	int		outLevel;		// output level
	int		printLevel;		// print level

	std::string		m_objParam;	// the objective parameter

	std::vector<Param>		m_params;
	std::vector<Data>		m_data;
};
