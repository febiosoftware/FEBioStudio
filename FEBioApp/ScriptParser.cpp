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

static std::map<QString, ScriptParser::TokenType> keymap;

void init_keymap()
{
	keymap["if"   ] = ScriptParser::TokenType::IF;
	keymap["else" ] = ScriptParser::TokenType::ELSE;
	keymap["var"  ] = ScriptParser::TokenType::VAR;
	keymap["true" ] = ScriptParser::TokenType::BOOLEAN;
	keymap["false"] = ScriptParser::TokenType::BOOLEAN;
}

ScriptParser::ScriptParser(FEBioAppDocument* doc) : m_doc(doc)
{
	if (keymap.empty()) init_keymap();

	m_scriptLength = 0;
	m_index = 0;
	m_line = 0;
	m_pos = 0;

	Object global;
	global.m_functions["alert"] = [=](const QList<Object>& args) {
		if (args.size() != 1) throw InvalidNrOfArgs();
		const Object& o = args.at(0);
		QMessageBox::information(m_doc->GetMainWindow(), "FEBio Studio", o.toString());
		return Object();
		};
	m_vars[""] = global;

	Object Math;
	Math.m_functions["sqrt"] = [=](const QList<Object>& args) {
		if (args.size() != 1) throw InvalidNrOfArgs();
		const Object& o = args.at(0);
		double v = sqrt(o.toNumber());
		return Object(v);
		};
	Math.m_functions["min"] = [=](const QList<Object>& args) {
		if (args.size() < 1) throw InvalidNrOfArgs();
		const Object& o = args.at(0);
		double v = o.toNumber();
		for (auto& oi : args)
		{
			double vi = oi.toNumber();
			if (vi < v) v = vi;
		}
		return Object(v);
		};
	m_vars["Math"] = Math;

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
	if (script.isEmpty()) return true;

	m_script = script;
	m_index = 0;
	m_line = 0;
	m_pos = 0;
	m_scriptLength = script.length();
	m_char = script[0];

	try {
		if (!parseScript())
			return false;
	}
	catch (std::runtime_error e)
	{
		m_error = e.what();
		m_error += QString(" at line %1 position %2").arg(m_line).arg(m_token.pos);
		return false;
	}

	return true;
}

bool ScriptParser::parseScript()
{
	do
	{
		parseStatement();

		if (m_token == SCRIPT_END)
		{
			break;
		}
		else if (m_token != STATEMENT_END)
		{
			return setError(QString("Invalid token at line %1 position %2").arg(m_line).arg(m_pos));
		}
	} while (true);

	return true;
}

void ScriptParser::parseStatement()
{
	nextToken();
	if (m_token == IDENTIFIER)
	{
		QString id = m_token.stringValue;

		// is this a variable
		if (m_vars.find(id) != m_vars.end())
		{
			Object& o = parseVariable();
			if (m_token == ASSIGNMENT)
			{
				nextToken();
				parseExpression(o);
			}
			else if (m_token == DOT)
			{
				Object tmp;
				parseIdentifier(o, tmp);
			}
		}
		else
		{
			// not a variable. Maybe a global function?
			Object tmp;
			nextToken();
			if (m_token == LP)
			{
				parseFunction(m_vars[""], id, tmp);
			}
			else throw UnknownVariable();
		}
	}
	else if (m_token == VAR)
	{
		parseVar();
	}
	else if (m_token == IF)
	{
		parseIfStatement();
	}
}

ScriptParser::Object& ScriptParser::parseVariable()
{
	QString id = m_token.stringValue;
	if (m_vars.find(id) == m_vars.end()) throw UnknownVariable();

	Object& o = m_vars[id];
	nextToken();
	if (m_token == LB)
	{
		if (!o.isArray()) throw SyntaxError();
		nextToken();
		Object tmp;
		parseExpression(tmp);
		if (!tmp.isNumber()) throw InvalidIndex();
		double v = tmp.toNumber();
		if (v != floor(v)) throw InvalidIndex();
		int n = (int)v;
		if ((n < 0) || (n >= o.arraySize())) throw OutOfBounds();
		if (m_token != RB) throw SyntaxError();
		nextToken();
		return o.arrayElement(n);
	}
	else return o;
}

void ScriptParser::parseVar()
{
	nextToken(IDENTIFIER);

	// insert a new (null) object
	Object o;
	QString varName = m_token.stringValue;
	m_vars[varName] = o;

	nextToken();
	if (m_token == ASSIGNMENT)
	{
		nextToken();
		parseExpression(m_vars[varName]);
	}
}

void ScriptParser::parseExpression(Object& o)
{
	// TODO: need to implement short-circuit rules
	parseArithmetic(o);
	while (true)
	{
		if ((m_token == GT) || (m_token == LT) || (m_token == EQUAL) || (m_token == GE) || (m_token == LE) || (m_token == NOT_EQUAL))
		{
			TokenType op = m_token.type;

			nextToken();
			Object r;
			parseArithmetic(r);

			bool b = false;

			if (o.isNumber() && r.isNumber())
			{
				double lval = o.toNumber();
				double rval = r.toNumber();
				switch (op)
				{
				case GT       : b = (lval > rval); break;
				case LT       : b = (lval < rval); break;
				case GE       : b = (lval >= rval); break;
				case LE       : b = (lval <= rval); break;
				case EQUAL    : b = (lval == rval); break;
				case NOT_EQUAL: b = (lval != rval); break;
				}
			}
			else if (o.isBool() && r.isBool())
			{
				bool lval = o.toBool();
				bool rval = r.toBool();
				switch (op)
				{
				case EQUAL    : b = (lval == rval); break;
				case NOT_EQUAL: b = (lval != rval); break;
				default:
					throw InvalidOperation();
				}
			}
			else throw InvalidOperation();

			o = b;
		}
		else break;
	}
}

void ScriptParser::parseArithmetic(Object& o)
{
	parseTerm(o);
	while (true)
	{
		if (m_token == PLUS)
		{
			nextToken();
			Object r;
			parseTerm(r);
			o += r;
		}
		else if (m_token == MINUS)
		{
			nextToken();
			Object r;
			parseTerm(r);
			o -= r;
		}
		else break;
	}
}

void ScriptParser::parseTerm(Object& o)
{
	parseFactor(o);
	while (true)
	{
		if (m_token == MULTIPLY)
		{
			nextToken();
			Object r;
			parseFactor(r);
			o *= r;
		}
		else if (m_token == DIV)
		{
			nextToken();
			Object r;
			parseFactor(r);
			o /= r;
		}
		else break;
	}
}

void ScriptParser::parseFactor(Object& o)
{
	parsePower(o);
	while (true)
	{
		if (m_token == EXP)
		{
			nextToken();
			Object r;
			parsePower(r);
			if (o.isNumber() && r.isNumber())
			{
				double x = o.toNumber();
				double y = r.toNumber();
				o = pow(x, y);
			}
			else throw InvalidOperation();
		}
		else break;
	}
}

void ScriptParser::parsePower(Object& o)
{
	if (m_token == NUMBER)
	{
		o = m_token.toNumber();
	}
	else if (m_token == BOOLEAN)
	{
		o = m_token.toBool();
	}
	else if (m_token == STRING_LITERAL)
	{
		o = m_token.stringValue;
	}
	else if (m_token == IDENTIFIER)
	{
		QString idName = m_token.stringValue;
		parseIdentifier(idName, o);
		return;
	}
	else if (m_token == LB)
	{
		parseArray(o);
	}
	else if (m_token == LP)
	{
		nextToken();
		parseExpression(o);
		if (m_token != RP) throw SyntaxError();
	}
	else if (m_token == MINUS)
	{
		nextToken();
		Object tmp;
		parsePower(tmp);
		if (!tmp.isNumber()) throw InvalidOperation();
		double v = -(tmp.toNumber());
		o = v;
		return;
	}
	else if (m_token == PLUS)
	{
		nextToken();
		parsePower(o);
		if (!o.isNumber()) throw InvalidOperation();
		return;
	}
	else throw ExpressionExpected();

	nextToken();
}

void ScriptParser::parseArray(Object& o)
{
	ArrayValue* var = new ArrayValue();
	do {
		nextToken();
		Object tmp;
		parseExpression(tmp);
		var->push_back(tmp);
		if ((m_token != COMMA) && (m_token != RB)) throw SyntaxError();

	} while (m_token != RB);

	o.SetValue(var);
}

void ScriptParser::parseIdentifier(const QString& id, Object& ret)
{
	if (m_vars.find(id) == m_vars.end())
	{
		// not a variable, but could be a global function
		nextToken();
		if (m_token == LP)
		{
			parseFunction(m_vars[""], id, ret);
		}
		else throw SyntaxError();
	}
	else
	{
		// get the variable
		nextToken();
		parseIdentifier(m_vars[id], ret);
	}
}

void ScriptParser::parseIdentifier(Object& o, Object& ret)
{
	if (m_token == DOT)
	{
		nextToken(IDENTIFIER);
		QString member = m_token.stringValue;

		nextToken();
		if (m_token == LP)
		{
			parseFunction(o, member, ret);
		}
		else ret = o.get(member);
	}
	else if (m_token == LB)
	{
		if (!o.isArray()) throw SyntaxError();
		nextToken();
		Object tmp;
		parseExpression(tmp);
		if (!tmp.isNumber()) throw SyntaxError();
		double v = tmp.toNumber();
		if (v != floor(v)) throw InvalidIndex();
		int n = (int)v;
		if ((n < 0) || (n >= o.arraySize())) throw OutOfBounds();
		if (m_token != RB) throw SyntaxError();
		ret = o.arrayElement(n);
		nextToken();
	}
	else ret = o;
}

void ScriptParser::parseIfStatement()
{
	nextToken(LP);
	Object tmp;
	nextToken();
	parseExpression(tmp);
	if (m_token != RP) throw SyntaxError();

	bool result = tmp.toBool();
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

void ScriptParser::parseBlock()
{
	nextToken(LC);

	do {
		parseStatement();
		if (m_token == RC)
			break;
		else if (m_token != STATEMENT_END)
			throw SyntaxError();
	} while (true);

	nextToken();
}

void ScriptParser::skipBlock()
{
	nextToken(LC);

	int l = 1;
	while (l > 0)
	{
		nextChar();
		if (m_char == '{') l++;
		else if (m_char == '}') l--;
	}

	nextToken();
}

void ScriptParser::parseFunction(ScriptParser::Object& ob, const QString& funcName, Object& returnVal)
{
	nextToken();

	QList<Object> args;

	if (m_token != RP)
	{
		while (true)
		{
			Object tmp;
			parseExpression(tmp);
			args.push_back(tmp);

			if (m_token == RP)
			{
				break;
			}
			else if (m_token == COMMA)
			{
				// all good, move on
				nextToken();
			}
			else throw SyntaxError();
		}
	}

	returnVal = callMethod(ob, funcName, args);

	nextToken();

	if ((m_token == DOT) || (m_token == LB))
	{
		Object tmp;
		parseIdentifier(returnVal, tmp);
		returnVal = tmp;
	}
}

void ScriptParser::nextChar()
{
	if (m_index < m_scriptLength)
	{
		m_char = m_script[m_index++];
		if (m_char == '\n')
		{
			m_line++;
			m_pos = 0;
		}
		else m_pos++;
	}
	else
	{
		m_index = m_scriptLength;
		m_token.pos = m_pos;
		throw UnexpectedEnd();
	}
}

bool ScriptParser::peekChar(QChar c)
{
	if (m_index < m_scriptLength)
	{
		if (m_script[m_index] == c)
		{
			nextChar();
			return true;
		}
	}
	return false;
}

QChar ScriptParser::peekChar()
{
	QChar c;
	if (m_index < m_scriptLength)
	{
		c = m_script[m_index];
	}
	else throw UnexpectedEnd();
	return c;
}

void ScriptParser::nextToken(TokenType expectedType)
{
	nextToken();
	if (m_token != expectedType)
	{
		if (m_token == SCRIPT_END) throw UnexpectedEnd();
		else throw InvalidToken();
	}
}

void ScriptParser::nextToken()
{
	m_token.type = TokenType::UNKNOWN;
	m_token.stringValue.clear();
	m_token.pos = m_pos;

	// read next char but skip white space
	do {
		if (m_index < m_scriptLength)
			nextChar();
		else
		{
			m_token.type = SCRIPT_END;
			m_token.pos = m_pos;
			return;
		}
	} while (m_char.isSpace());

	// skip comments
	if (m_char == '/')
	{
		if (peekChar('/'))
		{
			while (m_char != '\n')
			{
				if (m_index < m_scriptLength)
					nextChar();
				else
				{
					m_token.type = SCRIPT_END;
					m_token.pos = m_pos;
					return;
				}
			}
		}
		else if (peekChar('*'))
		{
			do {
				if (m_index < m_scriptLength)
					nextChar();
				else
				{
					m_token.type = SCRIPT_END;
					m_token.pos = m_pos;
					return;
				}

				if (m_char == '*')
				{
					if (peekChar('/'))
					{
						nextChar();
						break;
					}
				}

			} while (true);
		}

		// skip white space
		while (m_char.isSpace())
		{
			if (m_index < m_scriptLength)
				nextChar();
			else
			{
				m_token.type = SCRIPT_END;
				m_token.pos = m_pos;
				return;
			}
		}
	}

	m_token.pos = m_pos - 1;

	QChar c = m_char;
	if      (c == ',') m_token.type = COMMA;
	else if (c == '(') m_token.type = LP;
	else if (c == ')') m_token.type = RP;
	else if (c == '{') m_token.type = LC;
	else if (c == '}') m_token.type = RC;
	else if (c == '[') m_token.type = LB;
	else if (c == ']') m_token.type = RB;
	else if (c == ';') m_token.type = STATEMENT_END;
	else if (c == '+') m_token.type = PLUS;
	else if (c == '-') m_token.type = MINUS;
	else if (c == '/') m_token.type = DIV;
	else if (c == '*')
	{
		m_token.type = MULTIPLY;
		if (peekChar('*')) m_token.type = EXP;
	}
	else if (c == '=') 
	{ 
		m_token.type = ASSIGNMENT;
		if (peekChar('=')) m_token.type = EQUAL;
	}
	else if (c == '<') { 
		m_token.type = LT;
		if (peekChar('=')) m_token.type = LE;
	}
	else if (c == '>') 
	{ 
		m_token.type = GT;
		if (peekChar('=')) m_token.type = GE;
	}
	else if (c == '!')
	{
		nextChar();
		if (m_char == '=') m_token.type = NOT_EQUAL;
		else throw SyntaxError();
	}
	else if (c == '\'')
	{
		do {
			nextChar();
			if (m_char != '\'') m_token.stringValue += m_char;
		} while (m_char != '\'');
		m_token.type = STRING_LITERAL;
	}
	else if (c == '.')
	{
		QChar cn = peekChar();
		if (cn.isNumber()) parseNumber();
		else m_token.type = DOT;
	}
	else
	{
		// see if we are reading a number or a string
		if (c.isNumber())
		{
			parseNumber();
		}
		else
		{
			QString val = parseString();

			m_token.type = IDENTIFIER;
			m_token.stringValue = val;

			// check for keywords
			auto it = keymap.find(val);
			if (it != keymap.end())
			{
				m_token.type = it->second;
			}
		}
	}
}

void ScriptParser::parseNumber()
{
	// the first digit was already read in
	QString numVal;
	while (true)
	{
		numVal.push_back(m_char);
		QChar c = peekChar();
		if (c.isNumber()) nextChar();
		else if ((c == '.') || (c == 'e') || (c == 'E')) { nextChar(); break; }
		else break;
	}

	if (m_char == '.')
	{
		while (true)
		{
			numVal.push_back(m_char);
			QChar c = peekChar();
			if (c.isNumber()) nextChar();
			else if ((c == 'e') || (c == 'E')) { nextChar(); break; }
			else break;
		}
	}

	if ((m_char == 'e') || (m_char == 'E'))
	{
		numVal.push_back(m_char);
		nextChar();
		if ((m_char == '+') || (m_char == '-'))
		{
			numVal.push_back(m_char);
			nextChar();
		}

		while (true)
		{
			if (m_char.isNumber()) numVal.push_back(m_char);
			QChar c = peekChar();
			if (c.isNumber()) nextChar();
			else break;
		}
	}

	m_token.type = NUMBER;
	m_token.stringValue = numVal;
}

QString ScriptParser::parseString()
{
	QString val;
	val += m_char;
	while (true)
	{
		QChar c = peekChar();
		if (c.isLetterOrNumber() || (c == '_'))
		{
			val.push_back(c);
			nextChar();
		}
		else
		{
			break;
		}
	}
	return val;
}

QString ScriptParser::errorString() const { return m_error; }

bool ScriptParser::setError(const QString& err) { m_error = err; return false; }

ScriptParser::Object ScriptParser::callMethod(ScriptParser::Object& var, const QString& func, const QList<Object>& args)
{
	auto it = var.m_functions.find(func);
	if (it == var.m_functions.end()) throw UnknownMemberFunction();
	return it->second(args);
}
