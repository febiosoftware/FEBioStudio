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
#include "FEBioAppDocument.h"
#include "FEBioAppWidget.h"
#include "../FEBioStudio/MainWindow.h"
#include <QMessageBox>
#include <QPushButton>
#include <QLabel>
#include "JSParser.h"
#include "JSInterpreter.h"

FEBioAppScript::FEBioAppScript(FEBioAppDocument* doc) : m_doc(doc)
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

bool FEBioAppScript::runScript()
{
	std::string script = m_script.toStdString();
	JSParser parser;
	if (!parser.parse(script))
	{
		m_error = QString::fromStdString(parser.errorString());
		return false;
	}

	FEBioAppWidget* uiWidget = m_doc->GetUI();

	JSInterpreter interpreter;
	interpreter.init();

	JSObject& global = interpreter.addVar("");
	global.m_functions["alert"] = [=](const std::list<JSObject>& args, JSObject& ret) {
		QMessageBox::warning(uiWidget, "Alert", ObjectListToString(args));
		};

	JSObject& app = interpreter.addVar("app");
	app.m_functions["runModel"] = [=](const std::list<JSObject>& args, JSObject& ret) {
			m_doc->runModel();
		};

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

	try {
		interpreter.run(parser.GetAST());
	}
	catch (std::runtime_error e)
	{
		m_error = QString::fromStdString(e.what());
		uiWidget->error(m_error);
		return false;
	}

	return true;
}
