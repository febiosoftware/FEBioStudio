#pragma once
#include <functional>
#include <list>
#include <map>
#include <vector>
#include <stdexcept>
#include <assert.h>
#include <sstream>
#include <iomanip>
#include <math.h>

class NullDereference : public std::runtime_error {
public: NullDereference() : std::runtime_error("Dereferencing null object") {}
};

class UnknownVariable : public std::runtime_error {
public: UnknownVariable() : std::runtime_error("Unknown variable") {}
};

class UnknownFunction : public std::runtime_error {
public: UnknownFunction() : std::runtime_error("Unknown function") {}
};

class UnknownMember : public std::runtime_error {
public: UnknownMember() : std::runtime_error("Unknown member") {}
};

class JSInvalidOperation : public std::runtime_error {
public: JSInvalidOperation() : std::runtime_error("invalid operation") {}
};

class InvalidArrayIndex : public std::runtime_error {
public: InvalidArrayIndex() : std::runtime_error("Invalid array index") {}
};

class JSDivisionByZero : public std::runtime_error {
public: JSDivisionByZero() : std::runtime_error("Division by zero") {}
};

class JSString : public std::string
{
public:
	JSString() {}
	JSString(const std::string& s) : std::string(s) {}
	JSString(const char* sz) : std::string(sz) {}

	double toDouble() const { return atof(c_str()); }

	static JSString number(double a)
	{
		std::stringstream ss;
		ss << std::setprecision(16) << a;
		return JSString(ss.str());
	}
};

enum ValType {
	Invalid,
	Bool,
	Number,
	String,
	Array,
	Object
};

class AbstractValue;

class JSObject
{
public:
	typedef std::function<void(const std::list<JSObject>& args, JSObject& ret)> Function;

	JSObject();

	JSObject(AbstractValue* val);

	explicit JSObject(bool v);
	explicit JSObject(double v);
	explicit JSObject(const JSString& v);

	~JSObject();

	JSObject(const JSObject& o);

	void operator = (const JSObject& o);

	void SetValue(AbstractValue* val);

	AbstractValue* CopyValue() const;

	void operator = (bool v);
	void operator = (double v);
	void operator = (const JSString& v);

	bool isNull() const { return (m_val == nullptr); }

	JSObject get(const JSString& name);

public:

	JSObject operator + (JSObject& r);
	JSObject operator - (JSObject& r);
	JSObject operator * (JSObject& r);
	JSObject operator / (JSObject& r);

	void operator += (JSObject& r);
	void operator -= (JSObject& r);
	void operator *= (JSObject& r);
	void operator /= (JSObject& r);

	JSObject operator - ();
	JSObject operator ! ();

	void operator ++ ();
	void operator -- ();

public:
	bool isArray() const;
	size_t arraySize() const;
	JSObject& arrayElement(size_t);

	bool isObject() const;
	JSObject& ObjectProperty(const std::string& name);

public:
	JSString toString() const;

	double toNumber() const;

	int toInt(bool& ok) const;

	bool toBool() const;

	ValType type() const;

	bool isType(ValType t) const { return (type() == t); }

	bool isNumber() const { return isType(ValType::Number); }
	bool isString() const { return isType(ValType::String); }
	bool isBool() const { return isType(ValType::Bool); }

public:
	Function& getFunction(const JSString& fncName);

public:
	std::map<JSString, Function> m_functions;

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
	virtual JSString toString() const = 0;

	virtual JSObject get(const JSString& prop) { throw UnknownVariable(); }

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
	double toNumber() const override { return (m_val ? 1.0 : 0.0); }
	JSString toString() const override { return (m_val ? "true" : "false"); }

private:
	bool m_val;
};

class NumberValue : public AbstractValue
{
public:
	NumberValue(double val) : AbstractValue(ValType::Number), m_val(val) {}

	bool toBool() const override { return (m_val != 0.0); }
	double toNumber() const override { return m_val; }
	JSString toString() const override { return JSString::number(m_val); }

	void setNumber(double v) { m_val = v; }

private:
	double m_val;
};

class StringValue : public AbstractValue
{
public:
	StringValue(const JSString& val) : AbstractValue(ValType::String), m_val(val) {}

	bool toBool() const override { throw JSInvalidOperation(); return false; }
	double toNumber() const override { return m_val.toDouble(); }
	JSString toString() const override { return m_val; }

	void setString(const JSString& s) { m_val = s; }

private:
	JSString m_val;
};

class ArrayValue : public AbstractValue
{
public:
	ArrayValue() : AbstractValue(ValType::Array) {}
	ArrayValue(size_t n) : AbstractValue(ValType::Array) { m_val.resize(n); }

	bool toBool() const override { throw JSInvalidOperation(); return false; }
	double toNumber() const override { throw JSInvalidOperation(); return 0.0; }
	JSString toString() const override
	{
		JSString out;
		bool bfirst = true;
		for (const JSObject& o : m_val)
		{
			if (!bfirst) out += ",";
			else bfirst = false;
			out += o.toString();
		}
		return out;
	}

	void push_back(JSObject& o) { m_val.push_back(o); }

	size_t size() const { return m_val.size(); }

	void resize(size_t n) { m_val.resize(n); }

	JSObject& operator [] (size_t n) { return m_val[n]; }
	const JSObject& operator [] (size_t n) const { return m_val[n]; }

	JSObject& at(size_t n) { return m_val[n]; }

	JSObject get(const JSString& prop) override
	{
		if (prop == "length") return JSObject((double)m_val.size());
		else throw UnknownVariable();
	}

private:
	std::vector<JSObject> m_val;
};

class ObjectValue : public AbstractValue
{
public:
	ObjectValue() : AbstractValue(ValType::Object) {}

	bool toBool() const override { throw JSInvalidOperation(); return false; }
	double toNumber() const override { throw JSInvalidOperation(); return 0.0; }
	JSString toString() const override
	{
		JSString out;
		bool bfirst = true;
		out = "{";
		for (auto it : m_val)
		{
			if (!bfirst) out += ", ";
			else bfirst = false;
			out += it.first;
			out += ": ";
			out += it.second.toString();
		}
		out += "}";
		return out;
	}

	void addProperty(const std::string& name, const JSObject& val) { m_val[name] = val; }

	JSObject& operator[](const std::string& name)
	{
		return m_val[name];
	}

	JSObject get(const JSString& propName) override { 
		auto it = m_val.find(propName);
		if (it != m_val.end()) return it->second;
		throw UnknownMember();
	}

	std::map<std::string, JSObject>::iterator begin() { return m_val.begin(); }
	std::map<std::string, JSObject>::iterator end() { return m_val.end(); }

private:
	std::map<std::string, JSObject> m_val;
};

typedef std::list<JSObject> JSObjectList;

inline JSObject::JSObject() : m_val(nullptr) {}
inline JSObject::JSObject(AbstractValue* val) : m_val(nullptr) { SetValue(val); }

inline void JSObject::operator = (bool v) { SetValue(new BooleanValue(v)); }
inline void JSObject::operator = (double v) { SetValue(new NumberValue(v)); }
inline void JSObject::operator = (const JSString& v) { SetValue(new StringValue(v)); }

inline JSObject::JSObject(bool v) { m_val = nullptr; SetValue(new BooleanValue(v)); }
inline JSObject::JSObject(double v) { m_val = nullptr; SetValue(new NumberValue(v)); }
inline JSObject::JSObject(const JSString& v) { m_val = nullptr; SetValue(new StringValue(v)); }

inline JSObject::~JSObject()
{
	SetValue(nullptr);
}

inline JSObject::JSObject(const JSObject& o)
{
	m_val = o.m_val;
	if (m_val) m_val->inc();
	m_functions = o.m_functions;
}

inline void JSObject::operator = (const JSObject& o)
{
	SetValue(o.m_val);
	m_functions = o.m_functions;
}

inline void JSObject::SetValue(AbstractValue* val)
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

inline AbstractValue* JSObject::CopyValue() const
{
	switch (type())
	{
	case ValType::Number: return new NumberValue(toNumber()); break;
	case ValType::Bool  : return new BooleanValue(toBool()); break;
	case ValType::String: return new StringValue(toString()); break;
	case ValType::Array:
	{
		ArrayValue* src = dynamic_cast<ArrayValue*>(m_val);
		ArrayValue* dst = new ArrayValue;
		for (size_t i = 0; i < src->size(); ++i)
		{
			JSObject& src_i = src->at(i);
			JSObject tmp;
			tmp.SetValue(src_i.CopyValue());
			dst->push_back(tmp);
		}
		return dst;
	}
	break;
	case ValType::Object:
	{
		ObjectValue* src = dynamic_cast<ObjectValue*>(m_val);
		ObjectValue* dst = new ObjectValue;
		auto it = src->begin();
		for (; it != src->end(); ++it)
		{
			JSObject& src_i = it->second;
			JSObject tmp;
			tmp.SetValue(src_i.CopyValue());
			dst->addProperty(it->first, tmp);
		}
		return dst;
	}
	break;
	default:
		assert(false);
	}
	return nullptr;
}

inline JSString JSObject::toString() const
{
	if (m_val) return m_val->toString();
	else return "undefined";
}

inline double JSObject::toNumber() const
{
	if (m_val) return m_val->toNumber();
	else throw NullDereference();
}

inline int JSObject::toInt(bool& ok) const
{
	if (isNumber())
	{
		double v = toNumber();
		if (v == floor(v))
		{
			ok = true;
			return (int)v;
		}
	}
	ok = false;
	return 0;
}

inline bool JSObject::toBool() const
{
	if (m_val) return m_val->toBool();
	else throw NullDereference();
}

inline ValType JSObject::type() const
{
	if (m_val) return m_val->type();
	else return ValType::Invalid;
}

inline bool JSObject::isArray() const 
{ 
	return (m_val ? (m_val->type() == ValType::Array) : false); 
}

inline size_t JSObject::arraySize() const
{
	assert(isArray());
	ArrayValue* var = dynamic_cast<ArrayValue*>(m_val);
	if (var) return var->size();
	else return 0;
}

inline JSObject& JSObject::arrayElement(size_t n)
{
	assert(isArray());
	ArrayValue& var = dynamic_cast<ArrayValue&>(*m_val);
	if (n < 0) throw InvalidArrayIndex();
	if (n >= var.size()) var.resize(n + 1);
	return var[n];
}

inline bool JSObject::isObject() const
{
	return (m_val ? (m_val->type() == ValType::Object) : false);
}

inline JSObject& JSObject::ObjectProperty(const std::string& name)
{
	assert(isObject());
	ObjectValue& val = dynamic_cast<ObjectValue&>(*m_val);
	return val[name];
}

inline JSObject JSObject::get(const JSString& p)
{
	if (m_val) return m_val->get(p);
	else throw UnknownVariable();
}

inline JSObject JSObject::operator + (JSObject& r)
{
	if (isNumber() && r.isNumber())
	{
		return JSObject(toNumber() + r.toNumber());
	}
	else if (isString() && r.isString())
	{
		return JSObject(toString() + r.toString());
	}
	else throw JSInvalidOperation();
}

inline JSObject JSObject::operator - (JSObject& r)
{
	if (isNumber() && r.isNumber())
	{
		return JSObject(toNumber() - r.toNumber());
	}
	else throw JSInvalidOperation();
}

inline JSObject JSObject::operator * (JSObject& r)
{
	if (isNumber() && r.isNumber())
	{
		return JSObject(toNumber() * r.toNumber());
	}
	else throw JSInvalidOperation();
}

inline JSObject JSObject::operator / (JSObject& r)
{
	if (isNumber() && r.isNumber())
	{
		double rv = r.toNumber();
		if (rv == 0) throw JSDivisionByZero();
		return JSObject(toNumber() / r.toNumber());
	}
	else throw JSInvalidOperation();
}

inline void JSObject::operator += (JSObject& r)
{
	if (isNull() || r.isNull()) throw NullDereference();
	if (isNumber() && r.isNumber())
	{
		double sum = toNumber() + r.toNumber();
		SetValue(new NumberValue(sum));
	}
	else if (isString() && r.isString())
	{
		JSString s = toString() + r.toString();
		SetValue(new StringValue(s));
	}
	else throw JSInvalidOperation();
}

inline void JSObject::operator -= (JSObject& r)
{
	if (isNull() || r.isNull()) throw NullDereference();
	if (isNumber() && r.isNumber())
	{
		double dif = toNumber() - r.toNumber();
		SetValue(new NumberValue(dif));
	}
	else throw JSInvalidOperation();
}

inline void JSObject::operator *= (JSObject& r)
{
	if (isNull() || r.isNull()) throw NullDereference();
	if (isNumber() && r.isNumber())
	{
		double m = toNumber() * r.toNumber();
		SetValue(new NumberValue(m));
	}
	else throw JSInvalidOperation();
}

inline void JSObject::operator /= (JSObject& r)
{
	if (isNull() || r.isNull()) throw NullDereference();
	if (isNumber() && r.isNumber())
	{
		double m = toNumber() / r.toNumber();
		SetValue(new NumberValue(m));
	}
	else throw JSInvalidOperation();
}

inline JSObject::Function& JSObject::getFunction(const JSString& fncName)
{
	auto it = m_functions.find(fncName);
	if (it == m_functions.end()) throw UnknownFunction();
	return it->second;
}

inline JSObject JSObject::operator - ()
{
	if (isNumber())
	{
		double v = toNumber();
		return JSObject(-v);
	}
	else throw JSInvalidOperation();
}

inline JSObject JSObject::operator ! ()
{
	if (isBool())
	{
		bool v = toBool();
		return JSObject(!v);
	}
	else throw JSInvalidOperation();
}

inline void JSObject::operator ++ ()
{
	if (isNumber())
	{
		NumberValue* val = dynamic_cast<NumberValue*>(m_val);
		double v = val->toNumber();
		val->setNumber(v + 1);
	}
	else throw JSInvalidOperation();
}

inline void JSObject::operator -- ()
{
	if (isNumber())
	{
		NumberValue* val = dynamic_cast<NumberValue*>(m_val);
		double v = val->toNumber();
		val->setNumber(v - 1);
	}
	else throw JSInvalidOperation();
}
