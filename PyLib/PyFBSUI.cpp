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

#include "PyFBSUI.h"
#ifdef HAS_PYTHON
#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <pybind11/stl.h>
#include <FEBioStudio/FEBioStudio.h>
#include <FEBioStudio/MainWindow.h>
#include <FEBioStudio/PropertyList.h>
#include <PyLib/PythonToolsPanel.h>

namespace py = pybind11;
using namespace pybind11::literals;

void openFile(const char *fileName)
{
	FBS::getMainWindow()->OpenFile(fileName);
}

CCachedPropertyList* CreatePropertyListFromPythonDict(py::dict d)
{
	CCachedPropertyList* p = new CCachedPropertyList;

	for (auto item : d) {
		py::object& key = static_cast<py::object&>(item.first);
		py::object& value = static_cast<py::object&>(item.second);

		std::string key_str = py::str(key);
		QString propName = QString::fromStdString(key_str);

		if (py::isinstance<py::bool_>(value)) {
			bool v = value.cast<bool>();
			p->addBoolProperty(v, propName);
		}
		else if (py::isinstance<py::int_>(value)) {
			int v = value.cast<int>();
			p->addIntProperty(v, propName);
		}
		else if (py::isinstance<py::float_>(value)) {
			double v = value.cast<double>();
			p->addDoubleProperty(v, propName);
		}
		else if (py::isinstance<py::str>(value)) {
			std::string v = value.cast<std::string>();
			size_t n = v.find(":", 0);
			if (n != std::string::npos)
			{
				std::string typeStr = v.substr(0, n);
				if (typeStr == "@url")
				{
					std::string url = v.substr(n+1);
					p->addResourceProperty(QString::fromStdString(url), propName);
				}
				else
					p->addStringProperty(QString::fromStdString(v), propName);
			}
			else 
				p->addStringProperty(QString::fromStdString(v), propName);
		}
		else if (py::isinstance<py::list>(value)) {
			auto l = value.cast<py::list>();

			QStringList enumVals;
			for (const auto& item : l) {
				std::string name = py::cast<std::string>(item);
				enumVals.push_back(QString::fromStdString(name));
			}
			p->addEnumProperty(0, propName)->setEnumValues(enumVals);
		}
		else {
			py::object type = py::type::of(value);
			std::string type_str = py::str(type.attr("__name__"));

			if (type_str == "vec3d")
			{
				// Extract attributes
				double x = value.attr("x").cast<double>();
				double y = value.attr("y").cast<double>();
				double z = value.attr("z").cast<double>();

				p->addVec3Property(vec3d(x, y, z), propName);
			}
			else if (type_str == "color")
			{
				// Extract attributes
				uint8_t r = value.attr("r").cast<uint8_t>();
				uint8_t g = value.attr("g").cast<uint8_t>();
				uint8_t b = value.attr("b").cast<uint8_t>();

				p->addColorProperty(QColor(r, g, b), propName);
			}
			else
				assert(false);
		}
	}

	return p;
}

void PyAddTool(std::string toolName, py::dict props, py::function func, std::string infoStr)
{
	CCachedPropertyList* p = CreatePropertyListFromPythonDict(props);

	std::string func_name = py::str(func.attr("__name__"));
	std::string mod_name = py::str(func.attr("__module__"));

	QString fncName = QString::fromStdString(func_name);
	QString modName = QString::fromStdString(mod_name);
	CProperty* prop = p->addStringProperty(fncName, "_function_"); prop->setFlags(0); // make this hidden and uneditable
	prop = p->addStringProperty(modName, "_module_"); prop->setFlags(0); // make this hidden and uneditable

	auto wnd = FBS::getMainWindow();
//	auto panel = wnd->GetPythonToolsPanel();

	QString name = QString::fromStdString(toolName);
	QString info = QString::fromStdString(infoStr);

//	QMetaObject::invokeMethod(panel, "addPythonTool", Q_ARG(QString, name), Q_ARG(CCachedPropertyList*, p), Q_ARG(QString, info));
}

class CPyOutput
{
public:
	CPyOutput() {}

	void write(const char* txt)
	{
		FBS::getMainWindow()->AddPythonLogEntry(txt);
	}

	void flush() {}
};

void init_FBSUI(py::module& m)
{
    py::module ui = m.def_submodule("ui", "Module used to interact with the FEBio Studio GUI");

//	py::module panels = ui.def_submodule("panels", "Module used for interacting with FBS panels");
//	py::module pytools = panels.def_submodule("pytools", "Module used for interacting with Python panel");

    py::class_<CPyOutput>(ui, "PyOutput")
        .def(py::init())
        .def("write", &CPyOutput::write)
        .def("flush", &CPyOutput::flush);

    ui.def("openFile", openFile);

//	pytools.def("AddTool", PyAddTool, py::arg("toolName"), py::arg("props"), py::arg("func"), py::arg("infoStr") = "");
}

#else
void init_FBSUI(pybind11::module_& m) {}
#endif