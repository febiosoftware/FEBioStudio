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
#include "FEBioAppScript.h"
#include "FEBioApp.h"
#include "FEBioAppWidget.h"
#include <QMessageBox>
#include "JSParser.h"
#include "JSInterpreter.h"
#include <FEBioLib/FEBioModel.h>
#include <FECore/FEMaterial.h>

FEBioAppScript::FEBioAppScript(FEBioApp* app) : m_app(app)
{

}

FEBioAppScript::~FEBioAppScript()
{

}

bool FEBioAppScript::run(const QString& script)
{
	m_error.clear();
	if (script.isEmpty()) return true;

	m_script = script;

	try {
		runScript();
	}
	catch (std::runtime_error e)
	{
		m_error = e.what();
		return false;
	}

	return true;
}

QString ObjectListToString(const std::list<JSObject>& args)
{
	QString s;
	bool first = true;
	for (auto& it : args)
	{
		if (!first) s += " ";
		else first = false;
		s += QString::fromStdString(it.toString());
	}
	return s;
}

void FEBioAppScript::initJSModules(JSInterpreter& interpreter)
{
	FEBioAppWidget* uiWidget = m_app->GetUI();

	// global module
	JSObject& global = interpreter.addVar("");
	global.m_functions["alert"] = [=](const std::list<JSObject>& args, JSObject& ret) {
		QMessageBox::warning(uiWidget, "Alert", ObjectListToString(args));
		};

	global.m_functions["print"] = [=](const std::list<JSObject>& args, JSObject& ret) {
		uiWidget->print(ObjectListToString(args));
		};

	// app module
	JSObject& app = interpreter.addVar("app");
	app.m_functions["runModel"] = [=](const std::list<JSObject>& args, JSObject& ret) {
		m_app->runModel();
		};
	app.m_functions["stopModel"] = [=](const std::list<JSObject>& args, JSObject& ret) {
		m_app->stopModel();
		};

	// ui module
	JSObject& ui = interpreter.addVar("ui");
	ui.m_functions["getElementById"] = [=](const std::list<JSObject>& args, JSObject& ret) {

		const JSObject& o = *args.begin();
		UIElement w = uiWidget->GetElementByID(QString::fromStdString(o.toString()));

		JSObject ob;
		ob.m_functions["setText"] = [=](const std::list<JSObject>& args, JSObject& ret) {
			w.setText(ObjectListToString(args));
			};

		ret = ob;
		};

	// fem module
	FEBioModel* febioModel = m_app->GetFEBioModel();
	JSObject& fem = interpreter.addVar("fem");
	ObjectValue* v = new ObjectValue;
	ArrayValue* mat = new ArrayValue(febioModel->Materials());
	for (int i = 0; i < mat->size(); ++i)
	{
		FEMaterial* pm = febioModel->GetMaterial(i);
		ObjectValue* m = new ObjectValue;
		FEParameterList& pl = pm->GetParameterList();
		auto it = pl.first();
		for (int j = 0; j < pm->Parameters(); ++j, ++it)
		{
			FEParam& pj = *it;
			switch (pj.type())
			{
			case FE_PARAM_DOUBLE: { JSObject o(pj.value<double>()); m->addProperty(pj.name(), o); } break;
			case FE_PARAM_INT: { JSObject o((double)pj.value<int>()); m->addProperty(pj.name(), o); } break;
			case FE_PARAM_BOOL: { JSObject o(pj.value<bool>()); m->addProperty(pj.name(), o); } break;
			case FE_PARAM_DOUBLE_MAPPED:
			{
				FEParamDouble& v = pj.value<FEParamDouble>();
				if (v.isConst())
				{
					JSObject o(v.constValue());
					m->addProperty(pj.name(), o);
				}
			}
			break;
			}
		}
		mat->at(i).SetValue(m);
	}
	v->addProperty("material", JSObject(mat));
	fem.SetValue(v);
}

bool FEBioAppScript::runScript()
{
	std::string script = m_script.toStdString();
	JSParser parser;
	if (!parser.parse(script))
	{
		m_error = QString::fromStdString(parser.errorString());
		return false;
	}

	JSInterpreter interpreter;
	interpreter.init();

	initJSModules(interpreter);

	try {
		interpreter.run(parser.GetAST());
	}
	catch (std::runtime_error e)
	{
		m_error = QString::fromStdString(e.what());
		m_app->GetUI()->error(m_error);
		return false;
	}

	return true;
}
