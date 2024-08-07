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
#include <QLabel>

ScriptParser::ScriptParser(FEBioAppDocument* doc) : m_doc(doc)
{
	m_scriptLength = 0;
	m_index = 0;

	Object global;
	global.m_functions["alert"] = [=](const QList<Object>& args) {
		const Object& o = args.at(0);
		QMessageBox::information(m_doc->GetMainWindow(), "FEBio Studio", o.m_val.toString());
		return Object();
		};
	m_vars[""] = global;

	Object app;
	app.m_functions["runModel"] = [=](const QList<Object>& args) {
		m_doc->runModel();
		return Object();
		};
	app.m_functions["stopModel"] = [=](const QList<Object>& args) {
		m_doc->stopModel();
		return Object();
		};
	m_vars["app"] = app;

	FEBioAppWidget* uiWidget = doc->GetUI();
	if (uiWidget)
	{
		Object ui;
		ui.m_functions["getElementById"] = [=](const QList<Object>& args) {

			const Object& o = args.at(0);
			UIElement w = uiWidget->GetElementByID(o.m_val.toString());

			Object ob;
			ob.m_functions["setText"] = [=](const QList<Object>& args) {
				const Object& o = args.at(0);
				QString txt = o.m_val.toString();
				w.setText(txt);
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
	do
	{
		Token token = parseStatement();

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
	else if (token.type == VAR)
	{
		token = parseVar();
	}
	else if (token.type == IF)
	{
		token = parseIfStatement();
	}
	return token;
}

ScriptParser::Token ScriptParser::parseVar()
{
	Token token = nextToken(IDENTIFIER);

	Object o;
	QString varName = token.stringValue;
	m_vars[varName] = o;

	token = nextToken();
	if (token.type == ASSIGNMENT)
	{
		token = nextToken();
		if (token.type == NUMBER)
		{
			m_vars[varName].m_val = token.toNumber();
		}
		else throw QString("Syntax error at position %d").arg(m_index);

		token = nextToken();
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
	else if (token.type == ASSIGNMENT)
	{
		if (m_vars.find(id) == m_vars.end()) throw QString("Unknown variable %s at position %d").arg(id).arg(m_index);
		Object& o = m_vars[id];
		token = nextToken();
		if (token.type == NUMBER)
		{
			o.m_val = token.toNumber();
		}
		else if (token.type == STRING_LITERAL)
		{
			o.m_val = token.stringValue;
		}

		token = nextToken();
	}

	return token;
}

ScriptParser::Token ScriptParser::parseIfStatement()
{
	bool result = parseCondition();
	Token token({ UNKNOWN });
	if (result)
	{
		token = parseBlock();
		if (token == ELSE) token = skipBlock();
	}
	else
	{
		token = skipBlock();
		if (token == ELSE) token = parseBlock();
	}
	return token;
}

bool ScriptParser::parseCondition()
{
	nextToken(LP);

	Token token = nextToken();

	QVariant lvalue;
	if (token == NUMBER)
	{
		lvalue = token.toNumber();
	}
	else if (token == BOOLEAN)
	{
		lvalue = token.toBool();
	}
	else if (token == IDENTIFIER)
	{
		Object& o = m_vars[token.stringValue];
		assert(o.m_val.type() == QVariant::Type::Double);

		lvalue = o.m_val;
	}

	token = nextToken();
	if (token == GT)
	{
		if (lvalue.type() == QVariant::Type::Double)
		{
			token = nextToken();

			if (token.type == NUMBER)
			{
				nextToken(RP);
				return (lvalue.toDouble() > token.toNumber());
			}
		}
		else throw QString("Invalid operation at position %1").arg(m_index);
	}
	else if (token == RP)
	{
		if (lvalue.type() == QVariant::Type::Double)
		{
			return (lvalue.toDouble() != 0.0);
		}
		else if (lvalue.type() == QVariant::Type::Bool)
		{
			return lvalue.toBool();
		}
	}

	throw QString("Error processing condition at position %1").arg(m_index);
	return false;
}

ScriptParser::Token ScriptParser::parseBlock()
{
	Token token = nextToken(LC);

	do {
		token = parseStatement();
		if (token == RC) 
			break;
		else if (token.type != STATEMENT_END)
			throw QString("Unexpected token at position %1").arg(m_index);
	} while (true);

	return nextToken();
}

ScriptParser::Token ScriptParser::skipBlock()
{
	// eat whitespace
	while (m_script[m_index].isSpace())
	{
		m_index++;
		if (m_index >= m_scriptLength) return Token{ SCRIPT_END };
	}

	if (m_script[m_index] != '{') throw QString("missing '{' at position %1").arg(m_index);

	m_index++;
	int l = 1;
	while (l > 0)
	{
		if (m_index >= m_scriptLength) throw QString("Unexpected end of statement");
		QChar c = m_script[m_index++];
		if (c == '{') l++;
		else if (c == '}') l--;
	}

	return nextToken();
}

ScriptParser::Token ScriptParser::parseFunction(ScriptParser::Object& ob, const QString& funcName)
{
	Token token = nextToken();

	QList<Object> args;

	if (token.type == STRING_LITERAL)
	{
		Object o;
		o.m_val = token.stringValue;
		args << o;
		nextToken(RP);
	}
	else if (token.type == IDENTIFIER)
	{
		QString varName = token.stringValue;
		Object o;
		o.m_val = m_vars[varName].m_val.toString();
		args << o;
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
	else if (c == '{') { token = Token({ LC }); m_index++; }
	else if (c == '}') { token = Token({ RC }); m_index++; }
	else if (c == ';') { token = Token({ STATEMENT_END }); m_index++; }
	else if (c == '=') { token = Token({ ASSIGNMENT }); m_index++; }
	else if (c == '<') { token = Token({ LT }); m_index++; }
	else if (c == '>') { token = Token({ GT }); m_index++; }
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
		// see if we are reading a number or a string
		QChar c = m_script[m_index];
		if ((c == '-') || (c == '+') || c.isNumber())
		{
			token.type = NUMBER;

			QString numVal;
			if ((c == '-') || (c == '+')) {
				numVal.push_back(c); m_index++;
			}

			for (; m_index < m_scriptLength; ++m_index)
			{
				c = m_script[m_index];
				if (c.isNumber()) numVal.push_back(c);
				else break;
			}

			if (c == '.')
			{
				numVal.push_back(c);
				for (; m_index < m_scriptLength; ++m_index)
				{
					c = m_script[m_index];
					if (c.isNumber()) numVal.push_back(c);
					else break;
				}

				if ((c == 'e') || (c == 'E'))
				{
					m_index++;
					c = m_script[m_index++];
					if ((c == '+') || (c == '-')) numVal.push_back(c);

					for (; m_index < m_scriptLength; ++m_index)
					{
						c = m_script[m_index];
						if (c.isNumber()) numVal.push_back(c);
						else break;
					}
				}
			}

			token.stringValue = numVal;
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
			else if (token.stringValue == "else")
			{
				token.type = ELSE;
			}
			else if ((token.stringValue == "true") ||
				(token.stringValue == "false"))
			{
				token.type = BOOLEAN;
			}
			else if (token.stringValue == "var")
			{
				token.type = VAR;
			}
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

ScriptParser::Object ScriptParser::callMethod(ScriptParser::Object& var, const QString& func, const QList<Object>& args)
{
	auto it = var.m_functions.find(func);
	if (it == var.m_functions.end()) throw QString("Unknown member function %1").arg(func);
	return it->second(args);
}
