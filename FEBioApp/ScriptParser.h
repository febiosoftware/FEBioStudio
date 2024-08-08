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
#pragma once
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <map>

class FEBioAppDocument;

class ScriptParser
{
	enum TokenType {
		UNKNOWN,
		IDENTIFIER,
		NUMBER,
		BOOLEAN,
		VAR,
		ASSIGNMENT,
		IF,
		ELSE,
		FUNCTION,
		STRING_LITERAL,
		DOT,
		COMMA,
		LP, RP, LC, RC,
		GT, LT, GE, LE, EQUAL, NOT_EQUAL,
		STATEMENT_END,
		SCRIPT_END
	};

	struct Token
	{
		TokenType type;
		QString   stringValue;

		Token(TokenType tokenType) { type = tokenType; }
		Token() { type = UNKNOWN; }

		bool operator == (TokenType tokenType) const { return type == tokenType; }

		double toNumber() const { return stringValue.toDouble(); }

		bool toBool() const { return (stringValue == "true"); }
	};


	class Object
	{
	public:
		typedef std::function<Object (const QList<Object>& args)> Function;

	public:
		std::map<QString, Function> m_functions;

		QVariant m_val;
	};

public:
	ScriptParser(FEBioAppDocument* doc);

	~ScriptParser();

	bool execute(const QString& script);

	QString errorString() const;

private:
	bool parseScript();

	void parseBlock();
	void skipBlock();
	void parseStatement();
	void parseVar();
	void parseIfStatement();
	void parseIdentifier(const QString& id, Object& ref);
	void parseFunction(Object& ob, const QString& funcName, Object& returnVal);
	void nextToken(TokenType expectedType = TokenType::UNKNOWN);

	bool parseCondition();

	bool setError(const QString& err);

	Object callMethod(Object& var, const QString& func, const QList<Object>& args);

private:
	FEBioAppDocument* m_doc;
	QString m_error;
	QString m_script;
	size_t m_scriptLength;
	size_t	m_index;
	Token	m_token;

	std::map<QString, Object> m_vars;
};
