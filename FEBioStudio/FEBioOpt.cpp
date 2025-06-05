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
#include "FEBioOpt.h"
#include <XML/XMLWriter.h>

bool GenerateFEBioOptimizationFile(const std::string& fileName, FEBioOpt& opt)
{
	const char* szlog[] = { "LOG_DEFAULT", "LOG_NEVER", "LOG_FILE_ONLY", "LOG_SCREEN_ONLY", "LOG_FILE_AND_SCREEN" };
	const char* szprt[] = { "PRINT_ITERATIONS", "PRINT_VERBOSE" };

	XMLWriter xml;

	if (xml.open(fileName.c_str()) == false) return false;

	XMLElement root("febio_optimize");
	root.add_attribute("version", "2.0");
	xml.add_branch(root);

	// print options
	XMLElement ops("Options");
	switch (opt.m_method)
	{
	case 0: ops.add_attribute("type", "levmar"); break;
	case 1: ops.add_attribute("type", "constrained levmar"); break;
	}
	xml.add_branch(ops);
	{
		xml.add_leaf("obj_tol", opt.m_obj_tol);
		xml.add_leaf("f_diff_scale", opt.m_f_diff_scale);
		xml.add_leaf("log_level", szlog[opt.m_outLevel]);
		xml.add_leaf("print_level", szprt[opt.m_printLevel]);
	}
	xml.close_branch();

	// parameters
	XMLElement params("Parameters");
	xml.add_branch(params);
	for (int i = 0; i < (int)opt.m_params.size(); ++i)
	{
		FEBioOpt::Param& pi = opt.m_params[i];
		XMLElement par("param");
		par.add_attribute("name", pi.m_name);
		double v[3] = { pi.m_initVal, pi.m_minVal, pi.m_maxVal };
		par.value(v, 3);
		xml.add_leaf(par);
	}
	xml.close_branch();

	// objective
	XMLElement obj("Objective");
	switch (opt.m_objective)
	{
	case FEBioOpt::DataFit    : obj.add_attribute("type", "data-fit"); break;
	case FEBioOpt::Target     : obj.add_attribute("type", "target"); break;
	case FEBioOpt::ElementData: obj.add_attribute("type", "element-data"); break;
	case FEBioOpt::NodeData   : obj.add_attribute("type", "node-data"); break;
	default:
		return false;
	}

	xml.add_branch(obj);
	{
		if (opt.m_objective == FEBioOpt::DataFit)
		{
			XMLElement fnc("fnc");
			fnc.add_attribute("type", "parameter");
			xml.add_branch(fnc);
			{
				XMLElement p("param");
				p.add_attribute("name", opt.m_objParam);
				xml.add_empty(p);
			}
			xml.close_branch();

			xml.add_branch("data");
			{
				for (int i = 0; i < (int)opt.m_data.size(); ++i)
				{
					FEBioOpt::Data& di = opt.m_data[i];
					double v[2] = { di.m_time, di.m_value };
					xml.add_leaf("pt", v, 2);
				}
			}
			xml.close_branch();
		}
		else if (opt.m_objective == FEBioOpt::Target)
		{
			for (FEBioOpt::TargetVar& var : opt.m_trgVar)
			{
				XMLElement xel("var");
				xel.add_attribute("name", var.m_name.c_str());
				xel.value(var.m_val);
				xml.add_leaf(xel);
			}
		}
		else if (opt.m_objective == FEBioOpt::ElementData)
		{
			XMLElement var("var");
			var.add_attribute("type", opt.m_edVar.c_str());
			xml.add_empty(var);

			xml.add_branch("data");
			{
				XMLElement el("elem");
				int n1 = el.add_attribute("id", 0);
				for (int i = 0; i < (int)opt.m_edData.size(); ++i)
				{
					FEBioOpt::IDValue& di = opt.m_edData[i];
					el.set_attribute(n1, di.m_id);
					el.value(di.m_value);
					xml.add_leaf(el, false);
				}
			}
			xml.close_branch();
		}
		else if (opt.m_objective == FEBioOpt::NodeData)
		{
			XMLElement var("var");
			var.add_attribute("type", opt.m_ndVar.c_str());
			xml.add_empty(var);

			xml.add_branch("data");
			{
				XMLElement el("node");
				int n1 = el.add_attribute("id", 0);
				for (int i = 0; i < (int)opt.m_ndData.size(); ++i)
				{
					FEBioOpt::IDValue& di = opt.m_ndData[i];
					el.set_attribute(n1, di.m_id);
					el.value(di.m_value);
					xml.add_leaf(el, false);
				}
			}
			xml.close_branch();
		}
	}
	xml.close_branch();

	// closing tag
	xml.close_branch();

	xml.close();

	return true;
}
