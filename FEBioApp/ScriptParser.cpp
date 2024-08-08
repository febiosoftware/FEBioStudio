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

size_t ScriptParser::AbstractValue::m_total_allocs = 0;

ScriptParser::ScriptParser(FEBioAppDocument* doc) : m_doc(doc)
{
	m_scriptLength = 0;
	m_index = 0;

	Object global;
	global.m_functions["alert"] = [=](const QList<Object>& args) {
		const Object& o = args.at(0);
		QMessageBox::information(m_doc->GetMainWindow(), "FEBio Studio", o.toString());
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
			UIElement w = uiWidget->GetElementByID(o.toString());

			Object ob;
			ob.m_functions["setText"] = [=](const QList<Object>& args) {
				const Object& o = args.at(0);
				QString txt = o.toString();
				w.setText(txt);
				return Object();
				};

			return ob;
			};
		m_vars["ui"] = ui;

		// add the print function if we have a UI
		m_vars[""].m_functions["print"] = [=](const QList<Object>& args) {
			bool first = true;
			QString t;
			for (auto& o : args)
			{
				if (!first) t += " ";
				else first = false;
				t += o.toString();
			}
			uiWidget->print(t);
			return Object();
			};
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
		parseStatement();

		if (m_token.type == SCRIPT_END)
		{
			break;
		}
		else if (m_token.type != STATEMENT_END)
		{
			return setError("Invalid token");
		}
	} while (true);

	return true;
}

void ScriptParser::parseStatement()
{
	nextToken();
	if (m_token.type == IDENTIFIER)
	{
		QString id = m_token.stringValue;
		Object tmp;
		parseIdentifier(id, tmp);
	}
	else if (m_token.type == VAR)
	{
		parseVar();
	}
	else if (m_token.type == IF)
	{
		parseIfStatement();
	}
}

void ScriptParser::parseVar()
{
	nextToken(IDENTIFIER);

	// insert a new (null) object
	Object o;
	QString varName = m_token.stringValue;
	m_vars[varName] = o;

	nextToken();
	if (m_token.type == ASSIGNMENT)
	{
		nextToken();
		if (m_token.type == NUMBER)
		{
			m_vars[varName].SetValue(new NumberValue(m_token.toNumber()));
		}
		else if (m_token.type == STRING_LITERAL)
		{
			m_vars[varName].SetValue(new StringValue(m_token.stringValue));
		}
		else if (m_token.type == IDENTIFIER)
		{
			QString idName = m_token.stringValue;
			return parseIdentifier(idName, m_vars[varName]);
		}
		else throw QString("Syntax error at position %d").arg(m_index);

		nextToken();
	}
}

void ScriptParser::parseIdentifier(const QString& id, Object& ref)
{
	nextToken();
	if (m_token.type == DOT)
	{
		nextToken(IDENTIFIER);
		QString member = m_token.stringValue;

		nextToken();
		if (m_token == LP)
		{
			parseFunction(m_vars[id], member, ref);
		}
		else throw QString("Invalid token");
	}
	else if (m_token.type == LP)
	{
		parseFunction(m_vars[""], id, ref);
	}
	else if (m_token.type == ASSIGNMENT)
	{
		if (m_vars.find(id) == m_vars.end()) throw QString("Unknown variable %s at position %d").arg(id).arg(m_index);
		Object& o = m_vars[id];
		nextToken();
		if (m_token.type == NUMBER)
		{
			o.SetValue(new NumberValue(m_token.toNumber()));
		}
		else if (m_token.type == STRING_LITERAL)
		{
			o.SetValue(new StringValue(m_token.stringValue));
		}
		nextToken();
	}
}

void ScriptParser::parseIfStatement()
{
	bool result = parseCondition();
	if (result)
	{
		parseBlock();
		if (m_token == ELSE) skipBlock();
	}
	else
	{
		skipBlock();
		if (m_token == ELSE) parseBlock();
	}
}

bool ScriptParser::parseCondition()
{
	nextToken(LP);

	nextToken();
	Object lvar;
	if (m_token == TokenType::NUMBER)
	{
		lvar = m_token.toNumber();
	}
	else if (m_token == BOOLEAN)
	{
		lvar = m_token.toBool();
	}
	else if (m_token == IDENTIFIER)
	{
		Object& o = m_vars[m_token.stringValue];
		assert(o.type() == ValType::Number);
		lvar = o;
	}

	nextToken();
	if ((m_token == GT) || (m_token == LT) || (m_token == EQUAL) || (m_token == GE) || (m_token == LE) || (m_token == NOT_EQUAL))
	{
		TokenType op = m_token.type;
		if (lvar.type() == ValType::Number)
		{
			nextToken();

			double rval = 0.0;
			if (m_token.type == NUMBER)
			{
				rval = m_token.toNumber();
			}
			else if (m_token.type == IDENTIFIER)
			{
				Object& o = m_vars[m_token.stringValue];
				if (o.type() == ValType::Number)
				{
					rval = o.toNumber();
				}
				else throw QString("Invalid operation at position %1").arg(m_index);
			}
			else throw QString("Invalid operation at position %1").arg(m_index);

			nextToken(RP);

			double lval = lvar.toNumber();

			switch (op)
			{
			case GT   : return (lval > rval); break;
			case LT   : return (lval < rval); break;
			case GE   : return (lval >= rval); break;
			case LE   : return (lval <= rval); break;
			case EQUAL: return (lval == rval); break;
			case NOT_EQUAL: return (lval != rval); break;
			}
		}
		else throw QString("Invalid operation at position %1").arg(m_index);
	}
	else throw QString("Error processing condition at position %1").arg(m_index);

	return false;
}

void ScriptParser::parseBlock()
{
	nextToken(LC);

	do {
		parseStatement();
		if (m_token == RC)
			break;
		else if (m_token.type != STATEMENT_END)
			throw QString("Unexpected token at position %1").arg(m_index);
	} while (true);

	return nextToken();
}

void ScriptParser::skipBlock()
{
	// eat whitespace
	while (m_script[m_index].isSpace())
	{
		m_index++;
		if (m_index >= m_scriptLength)
		{
			m_token.type = SCRIPT_END;
			return;
		}
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

	nextToken();
}

void ScriptParser::parseFunction(ScriptParser::Object& ob, const QString& funcName, Object& returnVal)
{
	nextToken();

	QList<Object> args;

	while (true)
	{
		if (m_token.type == NUMBER)
		{
			Object o;
			o = m_token.toNumber();
			args << o;
		}
		else if (m_token.type == STRING_LITERAL)
		{
			Object o;
			o = m_token.stringValue;
			args << o;
		}
		else if (m_token.type == IDENTIFIER)
		{
			QString varName = m_token.stringValue;
			args << m_vars[varName];
		}
		else if (m_token.type == RP)
		{
			break;
		}
		else if (m_token.type == COMMA)
		{
			// all good, move on
		}
		else throw QString("syntax error at position %1").arg(m_index);
		nextToken();
	}

	returnVal = callMethod(ob, funcName, args);

	nextToken();

	if (m_token == DOT)
	{
		nextToken(IDENTIFIER);
		QString member = m_token.stringValue;

		nextToken();
		if (m_token == LP)
		{
			parseFunction(returnVal, member, returnVal);
		}
		else throw QString("Invalid token");
	}
}

void ScriptParser::nextToken(TokenType expectedType)
{
	if (m_index >= m_scriptLength)
	{
		m_token.type = SCRIPT_END;
		return;
	}

	// eat whitespace
	while (m_script[m_index].isSpace())
	{
		m_index++;
		if (m_index >= m_scriptLength)
		{
			m_token.type = SCRIPT_END;
			return;
		}
	}

	m_token.type = TokenType::UNKNOWN;
	m_token.stringValue.clear();
	QChar c = m_script[m_index];
	if      (c == '.') { m_token.type = DOT; m_index++; }
	else if (c == ',') { m_token.type = COMMA; m_index++; }
	else if (c == '(') { m_token.type = LP; m_index++; }
	else if (c == ')') { m_token.type = RP; m_index++; }
	else if (c == '{') { m_token.type = LC; m_index++; }
	else if (c == '}') { m_token.type = RC; m_index++; }
	else if (c == ';') { m_token.type = STATEMENT_END; m_index++; }
	else if (c == '=') 
	{ 
		m_token.type = ASSIGNMENT;
		m_index++;
		if (m_script[m_index] == '=')
		{
			m_token.type = EQUAL;
			m_index++;
		}
	}
	else if (c == '<') { 
		m_token.type = LT;
		m_index++; 
		if (m_script[m_index] == '=')
		{
			m_token.type = LE;
			m_index++;
		}
	}
	else if (c == '>') 
	{ 
		m_token.type = GT; m_index++; 
		if (m_script[m_index] == '=')
		{
			m_token.type = GE;
			m_index++;
		}
	}
	else if (c == '!')
	{
		m_index++;
		if (m_script[m_index] == '=')
		{
			m_token.type = NOT_EQUAL;
			m_index++;
		}
		else throw QString("syntax error at position %1").arg(m_index);
	}
	else if (c == '\'')
	{
		for (m_index++; m_index < m_scriptLength; ++m_index)
		{
			c = m_script[m_index];
			if (c != '\'')
				m_token.stringValue.push_back(c);
			else break;
		}
		if (c != '\'') throw QString("Missing ' at position %1").arg(m_index);
		m_token.type = STRING_LITERAL;
		m_index++;
	}
	else
	{
		// see if we are reading a number or a string
		QChar c = m_script[m_index];
		if ((c == '-') || (c == '+') || c.isNumber())
		{
			m_token.type = NUMBER;

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

			m_token.stringValue = numVal;
		}
		else
		{
			for (; m_index < m_scriptLength; ++m_index)
			{
				QChar c = m_script[m_index];
				if (c.isLetterOrNumber() || (c == '_')) m_token.stringValue.push_back(c);
				else
				{
					m_token.type = IDENTIFIER;
					break;
				}
			}

			// check for keywords
			if (m_token.stringValue == "if")
			{
				m_token.type = IF;
			}
			else if (m_token.stringValue == "else")
			{
				m_token.type = ELSE;
			}
			else if ((m_token.stringValue == "true") ||
				(m_token.stringValue == "false"))
			{
				m_token.type = BOOLEAN;
			}
			else if (m_token.stringValue == "var")
			{
				m_token.type = VAR;
			}
		}
	}

	if ((expectedType != UNKNOWN) &&
		(expectedType != m_token.type))
	{
		throw QString("Invalid token at position %1").arg(m_index);
	}
}

QString ScriptParser::errorString() const { return m_error; }

bool ScriptParser::setError(const QString& err) { m_error = err; return false; }

ScriptParser::Object ScriptParser::callMethod(ScriptParser::Object& var, const QString& func, const QList<Object>& args)
{
	auto it = var.m_functions.find(func);
	if (it == var.m_functions.end()) throw QString("Unknown member function %1").arg(func);
	return it->second(args);
}
