/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in 
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

#ifdef HAS_PYTHON
 
#ifdef PY_EXTERNAL
#define PY_MODULE_TYPE PYBIND11_MODULE
#else
#define PY_MODULE_TYPE PYBIND11_EMBEDDED_MODULE
#endif

#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <pybind11/stl.h>

#include "PyFBSCore.h"
#include "PyFBSPost.h"
#include "PyFBSMesh.h"
#include "PyFBSGeom.h"
#include <XPLTLib/xpltFileReader.h>

#ifndef PY_EXTERNAL
#include "PyFBSModel.h"
#include <FEBioStudio/FEBioStudio.h>
#include <FEBioStudio/MainWindow.h>
#include <FEBioStudio/ModelDocument.h>
#include <FEBioStudio/PostDocument.h>
#include <FEBioStudio/DocManager.h>
#include <PostGL/GLModel.h>
#include "PyRunContext.h"
#include "PyExceptions.h"
#endif

namespace py = pybind11;

#ifndef PY_EXTERNAL
class CPyOutput
{
public:
	void write(py::str txt)
	{
		std::string s = txt;
		FBS::getMainWindow()->AddPythonLogEntry(QString::fromStdString(s));
	}

	void flush() {}
};

FSModel* GetActiveModel()
{
	CModelDocument* doc = dynamic_cast<CModelDocument*>(PyRunContext::GetDocument());
	if (doc == nullptr)
	{
		throw pyGenericExcept("There is no active model.");
	}
	return (doc ? doc->GetFSModel() : nullptr);
}

Post::CGLModel* GetActivePostModel()
{
	CPostDocument* doc = dynamic_cast<CPostDocument*>(PyRunContext::GetDocument());
	if (doc == nullptr)
	{
		throw pyGenericExcept("There is no active post model.");
	}
	return (doc ? doc->GetGLModel() : nullptr);
}

// Make sure that this class never stores any internal state! 
// Otherwise, it might be better to make it a singleton or a static class.
class PyModelList
{
public:
	PyModelList() {}
	FSModel* get(const std::string& name)
	{
		CMainWindow* wnd = FBS::getMainWindow();
		if (wnd == nullptr) return nullptr;

		CDocManager* docManager = wnd->GetDocManager();
		if (docManager == nullptr) return nullptr;

		for (int i = 0; i < docManager->Documents(); ++i)
		{
			CDocument* doc = docManager->GetDocument(i);
			if (CModelDocument* modelDoc = dynamic_cast<CModelDocument*>(doc))
			{
				if (modelDoc->GetDocTitle() == name) return modelDoc->GetFSModel();
			}
		}

		throw py::value_error("No model with name '" + name + "' found.");

		return nullptr;
	}

	int size()
	{
		CMainWindow* wnd = FBS::getMainWindow();
		if (wnd == nullptr) return 0;

		CDocManager* docManager = wnd->GetDocManager();
		if (docManager == nullptr) return 0;

		int count = 0;
		for (int i = 0; i < docManager->Documents(); ++i)
		{
			CDocument* doc = docManager->GetDocument(i);
			if (dynamic_cast<CModelDocument*>(doc)) count++;
		}

		return count;
	}
};

class PyPostModelList
{
public:
	PyPostModelList() {}
	Post::CGLModel* get(const std::string& name)
	{
		CMainWindow* wnd = FBS::getMainWindow();
		if (wnd == nullptr) return nullptr;

		CDocManager* docManager = wnd->GetDocManager();
		if (docManager == nullptr) return nullptr;

		for (int i = 0; i < docManager->Documents(); ++i)
		{
			CDocument* doc = docManager->GetDocument(i);
			if (CPostDocument* postDoc = dynamic_cast<CPostDocument*>(doc))
			{
				if (postDoc->GetDocTitle() == name) return postDoc->GetGLModel();
			}
		}

		throw py::value_error("No model with name '" + name + "' found.");

		return nullptr;
	}

	int size()
	{
		CMainWindow* wnd = FBS::getMainWindow();
		if (wnd == nullptr) return 0;

		CDocManager* docManager = wnd->GetDocManager();
		if (docManager == nullptr) return 0;

		int count = 0;
		for (int i = 0; i < docManager->Documents(); ++i)
		{
			CDocument* doc = docManager->GetDocument(i);
			if (dynamic_cast<CPostDocument*>(doc)) count++;
		}

		return count;
	}
};

#else 

// TODO: I'm pretty sure this is a memory leak since no one is deleting the FEPostModel
Post::CGLModel* ReadPlotFile(std::string filename)
{
	Post::FEPostModel* model = new Post::FEPostModel;
	xpltFileReader reader(model);

	if (reader.Load(filename.c_str()) == false)
	{
		throw pyGenericExcept("Failed to read plot file.");
	}

	model->SetDisplacementField(BUILD_FIELD(DATA_CLASS::NODE_DATA, 0, 0));

	Post::CGLModel* glm = new Post::CGLModel(model);
	return glm;
}

#endif // PY_EXTERNAL

PY_MODULE_TYPE(fbs, m)
{
	init_FBSCore(m);
    init_FSMesh(m);
	init_FBSPost(m);
	init_FBSGeom(m);

#ifndef PY_EXTERNAL
	py::class_<CPyOutput>(m, "_PyOutput")
		.def(py::init())
		.def("write", &CPyOutput::write)
		.def("flush", &CPyOutput::flush);

	init_FBSModel(m);

	py::class_<PyModelList>(m, "ModelRegistry")
		.def("__len__", &PyModelList::size)
		.def("__getitem__", &PyModelList::get);

	py::class_<PyPostModelList>(m, "PostModelRegistry")
		.def("__len__", &PyPostModelList::size)
		.def("__getitem__", &PyPostModelList::get);

	m.def("active_model", &GetActiveModel, "Returns the active Model instance.", py::return_value_policy::reference);
	m.def("active_post_model", &GetActivePostModel, "Returns the active post::PostModel instance.", py::return_value_policy::reference);

	m.attr("models") = py::cast(PyModelList());
	m.attr("post_models") = py::cast(PyPostModelList());

#else
	post.def("read_plot_file", &ReadPlotFile, "Reads a plot file and returns a post::PostModel object.");
#endif
}

#ifndef PY_EXTERNAL
void init_fbs_python(std::wstring pythonHome)
{
    if(!pythonHome.empty())
    {
        PyConfig config;
        PyConfig_InitPythonConfig(&config);

        // Absolute path to your shipped Python prefix
        PyConfig_SetString(&config, &config.home,
                        pythonHome.c_str());

        py::initialize_interpreter(&config);

        PyConfig_Clear(&config);
    }
    else
    {
        pybind11::initialize_interpreter();
    }

	// setup output
	auto sysm = pybind11::module::import("sys");
	auto output = pybind11::module::import("fbs").attr("_PyOutput");
	sysm.attr("stdout") = output();
	sysm.attr("stderr") = output();
}

void finish_fbs_python()
{
	pybind11::finalize_interpreter();
}
#endif

#endif
