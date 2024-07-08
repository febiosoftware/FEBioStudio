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
#include "ScriptParser.h"
#include "FEBioAppDocument.h"
#include "FEBioAppWidget.h"
#include "../FEBioStudio/MainWindow.h"
#include <QMessageBox>
#include <QPushButton>

ScriptParser::ScriptParser(FEBioAppDocument* doc) : m_doc(doc)
{
	m_scriptLength = 0;
	m_index = 0;

	Object global;
	global.m_functions["alert"] = [=](const QStringList& args) {
		QMessageBox::information(m_doc->GetMainWindow(), "FEBio Studio", args[0]);
		return Object();
		};
	m_vars[""] = global;

	Object app;
	app.m_functions["runModel"] = [=](const QStringList& args) {
		m_doc->runModel();
		return Object();
		};
	app.m_functions["stopModel"] = [=](const QStringList& args) {
		m_doc->stopModel();
		return Object();
		};
	m_vars["app"] = app;

	FEBioAppWidget* uiWidget = doc->GetUI();
	if (uiWidget)
	{
		Object ui;
		ui.m_functions["getElementById"] = [=](const QStringList& args) {
			QWidget* w = uiWidget->GetElementByID(args[0]);

			Object ob;
			ob.m_functions["setText"] = [=](const QStringList& args) {
				QPushButton* pb = dynamic_cast<QPushButton*>(w);
				if (pb) pb->setText(args[0]);
				return Object();
				};

			return ob;
			};
		m_vars["ui"] = ui;
	}
}

ScriptParser::~ScriptParser()
{

}

bool ScriptParser::execute(const QString& script)
{
	m_error.clear();

	m_script = script;
	m_index = 0;
	m_scriptLength = script.length();

	try {
		if (!parseScript())
			return false;
	}
	catch (QString err)
	{
		m_error = err;
		return false;
	}

	return true;
}

bool ScriptParser::parseScript()
{
	Token token({ TokenType::UNKNOWN });
	do
	{
		token = parseStatement();

		if (token.type == SCRIPT_END)
		{
			break;
		}
		else if (token.type != STATEMENT_END)
		{
			return setError("Invalid token");
		}
	} while (true);

	return true;
}

ScriptParser::Token ScriptParser::parseStatement()
{
	Token token = nextToken();
	if (token.type == IDENTIFIER)
	{
		QString id = token.stringValue;
		token = parseIdentifier(id);
	}
	else if (token.type == IF)
	{
		token = parseIfStatement();
	}
	return token;
}

ScriptParser::Token ScriptParser::parseIdentifier(const QString& id)
{
	Token token = nextToken();
	if (token.type == DOT)
	{
		token = nextToken(IDENTIFIER);
		QString member = token.stringValue;

		token = nextToken();
		if (token == LP)
		{
			token = parseFunction(m_vars[id], member);
		}
		else throw QString("Invalid token");
	}
	else if (token.type == LP)
	{
		token = parseFunction(m_vars[""], id);
	}
	return token;
}

ScriptParser::Token ScriptParser::parseIfStatement()
{
	nextToken(LP);
	bool result = parseCondition();
	nextToken(RP);
	Token token({ UNKNOWN });
	if (result)
	{
		token = parseBlock();
		if (token == ELSE) token = skipBlock();
	}
	else
	{
		token = skipBlock();
		if (token == ELSE) parseBlock();
	}
	return Token({ UNKNOWN });
}

bool ScriptParser::parseCondition()
{
	Token token = nextToken();
	return false;
}

ScriptParser::Token ScriptParser::parseBlock()
{
	return Token({ UNKNOWN });
}

ScriptParser::Token ScriptParser::skipBlock()
{
	return Token({ UNKNOWN });
}

ScriptParser::Token ScriptParser::parseFunction(ScriptParser::Object& ob, const QString& funcName)
{
	Token token = nextToken();

	QStringList args;

	if (token.type == STRING_LITERAL)
	{
		QString arg = token.stringValue;
		args << arg;
		nextToken(RP);
	}
	else if (token.type != RP) throw QString("syntax error at position %1").arg(m_index);

	Object returnValue = callMethod(ob, funcName, args);

	token = nextToken();

	if (token == DOT)
	{
		token = nextToken(IDENTIFIER);
		QString member = token.stringValue;

		token = nextToken();
		if (token == LP)
		{
			token = parseFunction(returnValue, member);
		}
		else throw QString("Invalid token");
	}

	return token;
}

ScriptParser::Token ScriptParser::nextToken(TokenType expectedType)
{
	if (m_index >= m_scriptLength) return Token{ SCRIPT_END };

	// eat whitespace
	while (m_script[m_index].isSpace())
	{
		m_index++;
		if (m_index >= m_scriptLength) return Token{ SCRIPT_END };
	}

	Token token({ TokenType::UNKNOWN });
	QChar c = m_script[m_index];
	if (c == '.') { token = Token({ DOT }); m_index++; }
	else if (c == '(') { token = Token({ LP }); m_index++; }
	else if (c == ')') { token = Token({ RP }); m_index++; }
	else if (c == ';') { token = Token({ STATEMENT_END }); m_index++; }
	else if (c == '\'')
	{
		for (m_index++; m_index < m_scriptLength; ++m_index)
		{
			c = m_script[m_index];
			if (c != '\'')
				token.stringValue.push_back(c);
			else break;
		}
		if (c != '\'') throw QString("Missing ' at position %1").arg(m_index);
		token.type = STRING_LITERAL;
		m_index++;
	}
	else
	{
		for (; m_index < m_scriptLength; ++m_index)
		{
			QChar c = m_script[m_index];
			if (c.isLetterOrNumber() || (c == '_')) token.stringValue.push_back(c);
			else
			{
				token.type = IDENTIFIER;
				break;
			}
		}

		// check for keywords
		if (token.stringValue == "if")
		{
			token.type = IF;
		}
	}

	if ((expectedType != UNKNOWN) &&
		(expectedType != token.type))
	{
		throw QString("Invalid token at position %1").arg(m_index);
	}

	return token;
}

QString ScriptParser::errorString() const { return m_error; }

bool ScriptParser::setError(const QString& err) { m_error = err; return false; }

ScriptParser::Object ScriptParser::callMethod(ScriptParser::Object& var, const QString& func, const QStringList& args)
{
	auto it = var.m_functions.find(func);
	if (it == var.m_functions.end()) throw QString("Unknown member function %1").arg(func);
	return it->second(args);
}
