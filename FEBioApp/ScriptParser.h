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
#include <map>
#include <assert.h>
#include <stdexcept>

class FEBioAppDocument;

class ScriptParser
{
public:
	class NullDereference : public std::runtime_error {
	public: NullDereference() : std::runtime_error("Dereferencing null object") {}
	};

	class InvalidOperation : public std::runtime_error {
	public: InvalidOperation() : std::runtime_error("invalid operation") {}
	};

	class OutOfBounds : public std::runtime_error {
	public: OutOfBounds() : std::runtime_error("Out of bounds") {}
	};

	class InvalidIndex : public std::runtime_error {
	public: InvalidIndex() : std::runtime_error("Invalid index") {}
	};

	class InvalidToken : public std::runtime_error {
	public: InvalidToken() : std::runtime_error("Invalid token") {}
	};

	class SyntaxError : public std::runtime_error {
	public: SyntaxError() : std::runtime_error("Syntax error") {}
	};

	class UnknownVariable : public std::runtime_error {
	public: UnknownVariable() : std::runtime_error("Unknown variable") {}
	};

	class UnknownMemberFunction : public std::runtime_error {
	public: UnknownMemberFunction() : std::runtime_error("Unknown member function") {}
	};

	class UnexpectedEnd: public std::runtime_error {
	public: UnexpectedEnd() : std::runtime_error("Unexpected end") {}
	};

	class ExpressionExpected : public std::runtime_error {
	public: ExpressionExpected() : std::runtime_error("Expression expected") {}
	};

	class InvalidNrOfArgs : public std::runtime_error {
	public: InvalidNrOfArgs() : std::runtime_error("Invalid number of arguments") {}
	};

	enum TokenType {
		UNKNOWN,
		IDENTIFIER,
		NUMBER,
		BOOLEAN,
		ASSIGNMENT,
		PLUS, MINUS,
		MULTIPLY, DIV, EXP,
		VAR,
		IF,
		ELSE,
		FUNCTION,
		STRING_LITERAL,
		DOT,
		COMMA,
		LP, RP, LC, RC, LB, RB,
		GT, LT, GE, LE, EQUAL, NOT_EQUAL,
		STATEMENT_END,
		SCRIPT_END
	};

	struct Token
	{
		TokenType type;
		QString   stringValue;
		size_t    pos;

		Token(TokenType tokenType) { type = tokenType; pos = 0; }
		Token() { type = UNKNOWN; pos = 0; }

		bool operator == (TokenType tokenType) const { return type == tokenType; }
		bool operator != (TokenType tokenType) const { return type != tokenType; }

		double toNumber() const { return stringValue.toDouble(); }

		bool toBool() const { return (stringValue == "true"); }
	};

	enum ValType {
		Invalid,
		Bool,
		Number,
		String,
		Array
	};

	class AbstractValue;

	class Object
	{
	public:
		typedef std::function<Object (const QList<Object>& args)> Function;

		Object();

		explicit Object(bool v);
		explicit Object(double v);
		explicit Object(const QString& v);

		~Object();

		Object(const Object& o);

		void operator = (const Object& o);

		void SetValue(AbstractValue* val);

		void operator = (bool v);
		void operator = (double v);
		void operator = (const QString& v);

		bool isNull() const { return (m_val == nullptr); }

		Object get(const QString& name);

	public:
		void operator += (Object& r);
		void operator -= (Object& r);
		void operator *= (Object& r);
		void operator /= (Object& r);

	public:
		bool isArray() const { return (m_val ? (m_val->type() == ValType::Array) : false); }
		size_t arraySize() const;
		Object& arrayElement(size_t);

	public:
		QString toString() const
		{
			if (m_val) return m_val->toString();
			else throw NullDereference();
		}

		double toNumber() const
		{
			if (m_val) return m_val->toNumber();
			else throw NullDereference();
		}

		bool toBool() const
		{
			if (m_val) return m_val->toBool();
			else throw NullDereference();
		}

		ValType type() const
		{
			if (m_val) return m_val->type();
			else return ValType::Invalid;
		}

		bool isType(ValType t) const { return (type() == t); }

		bool isNumber() const { return isType(ValType::Number); }
		bool isString() const { return isType(ValType::String); }
		bool isBool  () const { return isType(ValType::Bool  ); }

	public:
		std::map<QString, Function> m_functions;

	private:
		AbstractValue* m_val;
	};

	class AbstractValue
	{
	public:
		AbstractValue(ValType type) : m_type(type), m_ref(0) { m_total_allocs++; }
		~AbstractValue() { m_total_allocs--; }

		ValType type() const { return m_type; }

		void inc() { m_ref++; }
		void dec() { m_ref--; }
		size_t refs() const { return m_ref; }

		virtual bool toBool() const = 0;
		virtual double toNumber() const = 0;
		virtual QString toString() const = 0;

		virtual Object get(const QString& prop) { throw UnknownVariable(); }

	private:
		size_t m_ref;
		ValType m_type;

		static size_t m_total_allocs;
	};

	class BooleanValue : public AbstractValue
	{
	public:
		BooleanValue(bool val) : AbstractValue(ValType::Bool), m_val(val) {}

		bool toBool() const override { return m_val; }
		double toNumber() const override { return (m_val?1.0:0.0); }
		QString toString() const override { return (m_val?"true":"false"); }

	private:
		bool m_val;
	};

	class NumberValue : public AbstractValue
	{
	public:
		NumberValue(double val) : AbstractValue(ValType::Number), m_val(val) {}

		bool toBool() const override { return (m_val != 0.0); }
		double toNumber() const override { return m_val; }
		QString toString() const override { return QString::number(m_val); }

		void setNumber(double v) { m_val = v; }

	private:
		double m_val;
	};

	class StringValue : public AbstractValue
	{
	public:
		StringValue(const QString& val) : AbstractValue(ValType::String), m_val(val) {}

		bool toBool() const override { throw InvalidOperation(); return false; }
		double toNumber() const override { return m_val.toDouble(); }
		QString toString() const override { return m_val; }

		void setString(const QString& s) { m_val = s; }

	private:
		QString m_val;
	};

	class ArrayValue : public AbstractValue
	{
	public:
		ArrayValue() : AbstractValue(ValType::Array) {}

		bool toBool() const override { throw InvalidOperation(); return false; }
		double toNumber() const override { throw InvalidOperation(); return 0.0; }
		QString toString() const override 
		{ 
			QString out;
			bool bfirst = true;
			for (const Object& o : m_val)
			{
				if (!bfirst) out += ",";
				else bfirst = false;
				out += o.toString();
			}
			return out; 
		}

		void push_back(Object& o) { m_val.push_back(o); }

		size_t size() const { return m_val.size(); }

		Object& operator [] (size_t n) { return m_val[n]; }
		const Object& operator [] (size_t n) const { return m_val[n]; }

		Object get(const QString& prop) override 
		{
			if (prop == "length") return Object((double)m_val.size());
			else throw UnknownVariable(); 
		}

	private:
		std::vector<Object> m_val;
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
	void parseIdentifier(const QString& id, Object& ret);
	void parseIdentifier(Object& o, Object& ret);
	void parseFunction(Object& ob, const QString& funcName, Object& returnVal);
	void parseArray(Object& o);

	void parseExpression(Object& o);
	void parseArithmetic(Object& o);
	void parseTerm(Object& o);
	void parseFactor(Object& o);
	void parsePower(Object& o);

	Object& parseVariable();

	void nextToken();
	void nextToken(TokenType expectedType);
	void parseNumber();
	QString parseString();

	bool setError(const QString& err);

	Object callMethod(Object& var, const QString& func, const QList<Object>& args);

private:
	// get the next character
	// will throw exception if try to read past end
	void nextChar();

	// check if next char equals c and advance if true
	bool peekChar(QChar c);

	// return next char without advancing
	// will throw exception if try to read past end
	QChar peekChar();

private:
	FEBioAppDocument* m_doc;
	QString m_error;
	QString m_script;
	size_t m_scriptLength;
	size_t	m_line, m_pos, m_index;
	Token	m_token;
	QChar	m_char;

	std::map<QString, Object> m_vars;
};

inline ScriptParser::Object::Object() : m_val(nullptr) {}

inline void ScriptParser::Object::operator = (bool v) { SetValue(new BooleanValue(v)); }
inline void ScriptParser::Object::operator = (double v) { SetValue(new NumberValue(v)); }
inline void ScriptParser::Object::operator = (const QString& v) { SetValue(new StringValue(v)); }

inline ScriptParser::Object::Object(bool v) { m_val = nullptr; SetValue(new BooleanValue(v)); }
inline ScriptParser::Object::Object(double v) { m_val = nullptr; SetValue(new NumberValue(v)); }
inline ScriptParser::Object::Object(const QString& v) { m_val = nullptr; SetValue(new StringValue(v)); }

inline ScriptParser::Object::~Object()
{
	SetValue(nullptr);
}

inline ScriptParser::Object::Object(const Object& o)
{
	m_val = o.m_val;
	if (m_val) m_val->inc();
	m_functions = o.m_functions;
}

inline void ScriptParser::Object::operator = (const Object& o)
{
	SetValue(o.m_val);
	m_functions = o.m_functions;
}

inline void ScriptParser::Object::SetValue(AbstractValue* val)
{
	if (val != m_val)
	{
		if (m_val)
		{
			m_val->dec();
			if (m_val->refs() == 0) delete m_val;
			m_val = nullptr;
		}
		m_val = val;
		if (m_val) m_val->inc();
	}
}

inline size_t ScriptParser::Object::arraySize() const
{
	assert(isArray());
	ArrayValue* var = dynamic_cast<ArrayValue*>(m_val);
	if (var) return var->size();
	else return 0;
}

inline ScriptParser::Object& ScriptParser::Object::arrayElement(size_t n)
{
	assert(isArray());
	ArrayValue& var = dynamic_cast<ArrayValue&>(*m_val);
	return var[n];
}

inline ScriptParser::Object ScriptParser::Object::get(const QString& p)
{
	if (m_val) return m_val->get(p);
	else throw UnknownVariable();
}

inline void ScriptParser::Object::operator += (ScriptParser::Object& r)
{
	if (isNull() || r.isNull()) throw NullDereference();
	if (isNumber() && r.isNumber())
	{
		double sum = toNumber() + r.toNumber();
		(dynamic_cast<NumberValue*>(m_val))->setNumber(sum);
	}
	else if (isString() && r.isString())
	{
		(dynamic_cast<StringValue*>(m_val))->setString(toString() + r.toString());
	}
	else throw InvalidOperation();
}

inline void ScriptParser::Object::operator -= (ScriptParser::Object& r)
{
	if (isNull() || r.isNull()) throw NullDereference();
	if (isNumber() && r.isNumber())
	{
		double dif = toNumber() - r.toNumber();
		(dynamic_cast<NumberValue*>(m_val))->setNumber(dif);
	}
	else throw InvalidOperation();
}

inline void ScriptParser::Object::operator *= (ScriptParser::Object& r)
{
	if (isNull() || r.isNull()) throw NullDereference();
	if (isNumber() && r.isNumber())
	{
		double m = toNumber() * r.toNumber();
		(dynamic_cast<NumberValue*>(m_val))->setNumber(m);
	}
	else throw InvalidOperation();
}

inline void ScriptParser::Object::operator /= (ScriptParser::Object& r)
{
	if (isNull() || r.isNull()) throw NullDereference();
	if (isNumber() && r.isNumber())
	{
		double m = toNumber() / r.toNumber();
		(dynamic_cast<NumberValue*>(m_val))->setNumber(m);
	}
	else throw InvalidOperation();
}
