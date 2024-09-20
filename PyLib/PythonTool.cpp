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

#include "PythonTool.h"
#include <FEBioStudio/MainWindow.h>
#include "PythonToolsPanel.h"
#include "../FEBioStudio/PropertyListForm.h"
#include <QBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <iostream>
#include <FEBioStudio/LogPanel.h>
#include <FEBioStudio/Logger.h>

namespace py = pybind11;

void CPythonToolProps::addBoolProperty(const std::string& name, bool v)
{
	CCachedPropertyList::addBoolProperty(v, QString::fromStdString(name));
}

void CPythonToolProps::addIntProperty(const std::string& name, int v)
{
	CCachedPropertyList::addIntProperty(v, QString::fromStdString(name));
}

void CPythonToolProps::addDoubleProperty(const std::string& name, double v)
{
	CCachedPropertyList::addDoubleProperty(v, QString::fromStdString(name));
}

void CPythonToolProps::addVec3Property(const std::string& name, vec3d v)
{
	CCachedPropertyList::addVec3Property(v, QString::fromStdString(name));
}

void CPythonToolProps::addEnumProperty(const std::string& name, const std::string& labels, int v)
{
	QString enumString = QString::fromStdString(labels);
	QStringList enums = enumString.split(';');
	CCachedPropertyList::addEnumProperty(v, QString::fromStdString(name))->setEnumValues(enums);
}

void CPythonToolProps::addStringProperty(const std::string& name, const char* v)
{
	CCachedPropertyList::addStringProperty(v, QString::fromStdString(name));
}

void CPythonToolProps::addResourceProperty(const std::string& name, const char* v)
{
	CCachedPropertyList::addResourceProperty(v, QString::fromStdString(name));
}

CPythonTool::CPythonTool(CMainWindow* wnd, std::string name, int id)
    : CAbstractTool(wnd, name.c_str()), m_id(id)
{
	m_props = nullptr;
}

CPythonTool::~CPythonTool()
{
	delete m_props;
}

void CPythonTool::SetProperties(CPythonToolProps* props)
{
	m_props = props;
}

QWidget* CPythonTool::createUi()
{
	QWidget* w = new QWidget;
	QVBoxLayout* l = new QVBoxLayout();
	QString s = QString::fromStdString(m_props->GetInfo());
	if (!s.isEmpty())
	{
		QLabel* label = new QLabel(s);
		l->addWidget(label);
	}
	CPropertyListForm* ui = new CPropertyListForm;
	ui->setPropertyList(m_props);
	l->addWidget(ui);
	QPushButton* pb = new QPushButton("Run");
	l->addWidget(pb);
	l->addStretch();
	w->setLayout(l);

	QObject::connect(pb, &QPushButton::clicked, this, &CPythonTool::OnApply);
	return w;
}

void CPythonTool::OnApply()
{
	CMainWindow* wnd = GetMainWindow();
	wnd->GetLogPanel()->ShowLog(CLogPanel::PYTHON_LOG);
	CLogger::AddPythonLogEntry(QString(">>> running python tool \"%1\"\n").arg(name()));
	wnd->GetPythonToolsPanel()->GetThread()->SetTool(this);
}

bool CPythonTool::runFunc()
{
    try
    {
        py::dict kwargs;

        for(int prop = 0; prop < m_props->Properties(); prop++)
        {
            CProperty& current = m_props->Property(prop);
            std::string name = current.name.toStdString();
			void* d = current.data(); assert(d);

            switch(current.type)
            {
                case CProperty::Bool:
                    kwargs[name.c_str()] = *((bool*)d);
                    break;
                case CProperty::Int:
                    kwargs[name.c_str()] = *((int*)d);
                    break;
                case CProperty::Float:
                    kwargs[name.c_str()] = *((double*)d);
                    break;
                case CProperty::Vec3:
                    kwargs[name.c_str()] = *((vec3d*)d);
                    break;
                case CProperty::String:
					// TODO: Is this safe? This passes a pointer to a temporary object
                    kwargs[name.c_str()] = ((QString*)d)->toStdString().c_str();
                    break;
				case CProperty::Enum:
					{
						int n = *((int*)d);
						// TODO: Is this safe? This passes a pointer to a temporary object
						kwargs[name.c_str()] = py::make_tuple(n, current.values[n].toStdString().c_str());
					}
                    break;
                case CProperty::Resource:
					// TODO: Is this safe? This passes a pointer to a temporary object
                    kwargs[name.c_str()] = ((QString*)d)->toStdString().c_str();
                    break;

                default:
                    return false;
            };
        }

		auto func = m_props->GetFunction();
        (*func)(**kwargs);
    }
    catch(py::error_already_set &e)
    {
        // Print the error message
        py::print(e.what());

        // Return execution to Python to allow the thread to exit
        e.restore();

        // Clear the error to allow further Python execution. 
        PyErr_Clear();
    }

    return true;
}
