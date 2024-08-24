#pragma once
#include <vector>
#include <string>
#include <map>
#include <stdexcept>
#include <assert.h>

class InvalidNrOfArgs : public std::runtime_error {
public: InvalidNrOfArgs() : std::runtime_error("Invalid number of arguments") {}
};

class JSStatement
{
public:
	enum JSStatementKind {
		ExpressionStatement,
		BlockStatement,
		VarDeclarationStatement,
		FunctionDeclaration,
		IfStatement,
		WhileStatement,
		DoWhileStatement,
		ReturnStatement
	};

public:
	JSStatement(JSStatementKind kind) : m_kind(kind) {}
	virtual ~JSStatement() {}

	JSStatementKind kind() const { return m_kind; }

private:
	JSStatementKind m_kind;
};

class JSExpression
{
public:
	enum ExprKind {
		Number,
		Boolean,
		String,
		Symbol,
		Array,
		Object,
		Unary,
		Binary,
		Assignment,
		Function,
		ArrayItem,
		Member
	};

public:
	JSExpression(ExprKind kind) : m_kind(kind) {}
	virtual ~JSExpression() {}

	ExprKind kind() const { return m_kind; }

private:
	ExprKind m_kind;
};

class JSExpressionList : public std::vector<JSExpression*>
{
public:
	void deleteItems()
	{
		for (size_t i = 0; i < size(); ++i) delete this->at(i);
		clear();
	}
};


// --- Expressions ---
class JSNumberLiteral : public JSExpression
{
public:
	JSNumberLiteral(double v) : JSExpression(JSExpression::Number), m_val(v) {}

	double value() const { return m_val; }

private:
	double m_val;
};

class JSBooleanLiteral : public JSExpression
{
public:
	JSBooleanLiteral(bool v) : JSExpression(JSExpression::Boolean), m_val(v) {}

	bool value() const { return m_val; }

private:
	bool m_val;
};

class JSStringLiteral : public JSExpression
{
public:
	JSStringLiteral(const std::string& s) : JSExpression(JSExpression::String), m_val(s) {}

	const std::string& value() const { return m_val; }

private:
	std::string m_val;
};

class JSSymbolExpr : public JSExpression
{
public:
	JSSymbolExpr(const std::string& s) : JSExpression(JSExpression::Symbol), m_name(s) {}

	const std::string& name() const { return m_name; }

private:
	std::string m_name;
};

class JSArrayLiteral : public JSExpression
{
public:
	JSArrayLiteral() : JSExpression(JSExpression::Array) {}
	~JSArrayLiteral() { m_el.deleteItems(); }

	void add(JSExpression* e) { m_el.push_back(e); }

	size_t size() const { return m_el.size(); }

	JSExpression* operator [] (size_t n) { return m_el[n]; }

private:
	JSExpressionList m_el;
};

class JSObjectLiteral : public JSExpression
{
public:
	JSObjectLiteral() : JSExpression(JSExpression::Object) {}
	~JSObjectLiteral()
	{
		for (auto it : m_props) delete it.second;
		m_props.clear();
	}

	size_t Properties() const { return m_props.size(); }

	std::map<std::string, JSExpression*>::iterator begin() { return m_props.begin(); }
	std::map<std::string, JSExpression*>::iterator end() { return m_props.end(); }
	
	void AddProperty(const std::string& name, JSExpression* e) { m_props[name] = e; }

private:
	std::map<std::string, JSExpression*> m_props;
};

class JSUnaryExpr : public JSExpression
{
public:
	enum UnaryOp {
		NEG,
		NOT,
		INC,
		DEC,
		POST_INC,
		POST_DEC
	};

public:
	JSUnaryExpr(JSExpression* operand, UnaryOp unaryOp) : JSExpression(JSExpression::Unary), expr(operand), op(unaryOp) {}
	~JSUnaryExpr() { delete expr; }

public:
	JSExpression* expr;
	UnaryOp op;
};

class JSBinaryExpr : public JSExpression
{
public:
	enum BinaryOp {
		ADD, SUB, MUL, DIV, MOD, EXP,
		EQUAL, GT, LT, GE, LE, 
		AND, OR
	};

public:
	JSBinaryExpr(JSExpression* l, JSExpression* r, BinaryOp binaryOp) : JSExpression(JSExpression::Binary), left(l), right(r), op(binaryOp) {}
	~JSBinaryExpr() { delete left; delete right; }

public:
	JSExpression* left;
	JSExpression* right;
	BinaryOp op;
};

class JSAssignmentExpr : public JSExpression
{
public:
	enum AssignOp {
		EQUAL, EQUAL_ADD, EQUAL_SUB, EQUAL_MUL, EQUAL_DIV
	};

public:
	JSAssignmentExpr(JSExpression* leftval, JSExpression* rightval, AssignOp assignOp) : JSExpression(JSExpression::Assignment), lv(leftval), rv(rightval), op(assignOp) {}
	~JSAssignmentExpr() { delete lv; delete rv; }

public:
	JSExpression* lv;
	JSExpression* rv;
	AssignOp op;
};

class JSArrayItemExpr : public JSExpression
{
public:
	JSArrayItemExpr(JSExpression* leftval, JSExpression* rightval) : JSExpression(JSExpression::ArrayItem), lv(leftval), rv(rightval) {}
	~JSArrayItemExpr() { delete lv; delete rv; }

public:
	JSExpression* lv;
	JSExpression* rv;
};

class JSFunctionExpr : public JSExpression
{
public:
	JSFunctionExpr(JSExpression* fnc) : JSExpression(JSExpression::Function), m_fnc(fnc) {}
	~JSFunctionExpr() { m_args.deleteItems(); }

	JSExpression* FunctionExpr() { return m_fnc; }

	void AddArgument(JSExpression* expr) { m_args.push_back(expr); }
	const JSExpressionList& GetExpressionList() const { return m_args; }

	size_t Arguments() const { return m_args.size(); }
	JSExpression* Argument(size_t n) { return m_args[n]; }

public:
	JSExpression*	m_fnc;
	JSExpressionList m_args;
};

// --- Statements ---
class JSExpressionStatement : public JSStatement
{
public:
	JSExpressionStatement(JSExpression* expr) : JSStatement(JSStatement::ExpressionStatement), m_expr(expr) {}
	~JSExpressionStatement() { delete m_expr; }

	JSExpression* expr() { return m_expr; }

private:
	JSExpression* m_expr;
};

class JSMemberExpr : public JSExpression
{
public:
	JSMemberExpr(JSExpression* leftval, JSExpression* rightval) : JSExpression(JSExpression::Member), lv(leftval), rv(rightval) {}
	~JSMemberExpr() { delete lv; delete rv; }

public:
	JSExpression* lv;
	JSExpression* rv;
};

class JSVarDeclarationStatement : public JSStatement
{
public:
	struct Var {
		std::string name;
		bool isConst;
		JSExpression* expr;
	};

public:
	JSVarDeclarationStatement() : JSStatement(JSStatement::VarDeclarationStatement) {}
	~JSVarDeclarationStatement() { 
		for (Var& v : m_vars) delete v.expr;
		m_vars.clear();
	}

	void addVar(const std::string& varName, JSExpression* expr, bool isConst)
	{
		m_vars.push_back({ varName, isConst, expr });
	}

	Var& var(size_t n) { return m_vars[n]; }
	size_t size() const { return m_vars.size(); }

private:
	std::vector<Var> m_vars;
};

class JSBlockStatement : public JSStatement
{
public:
	JSBlockStatement() : JSStatement(JSStatement::BlockStatement) {}
	~JSBlockStatement() {
		clear();
	}
	void clear()
	{
		for (JSStatement* s : m_stmt) delete s;
		m_stmt.clear();
	}

	void AddStatement(JSStatement* stmt) { m_stmt.push_back(stmt); }
	size_t Statements() const { return m_stmt.size(); }
	JSStatement* Statement(size_t n) { return m_stmt[n]; }

private:
	std::vector<JSStatement*> m_stmt;
};

class JSIfStatement : public JSStatement
{
public:
	JSIfStatement(JSExpression* cond, JSBlockStatement* onTrue, JSBlockStatement* onFalse) : JSStatement(JSStatement::IfStatement), 
		m_cond(cond), m_onTrue(onTrue), m_onFalse(onFalse) {}

	~JSIfStatement() { delete m_cond; delete m_onTrue; delete m_onFalse; }

	JSExpression* Condition() { return m_cond; }
	JSBlockStatement* OnTrue() { return m_onTrue; }
	JSBlockStatement* OnFalse() { return m_onFalse; }

private:
	JSExpression* m_cond;
	JSBlockStatement* m_onTrue;
	JSBlockStatement* m_onFalse;
};

class JSWhileStatement : public JSStatement
{
public:
	JSWhileStatement(JSExpression* cond, JSBlockStatement* stmt) : JSStatement(JSStatement::WhileStatement),
		m_cond(cond), m_stmt(stmt) {}

	~JSWhileStatement() { delete m_cond; delete m_stmt; }

	JSExpression* Condition() { return m_cond; }
	JSBlockStatement* Statement() { return m_stmt; }

private:
	JSExpression* m_cond;
	JSBlockStatement* m_stmt;
};

class JSDoWhileStatement : public JSStatement
{
public:
	JSDoWhileStatement(JSBlockStatement* stmt, JSExpression* cond) : JSStatement(JSStatement::DoWhileStatement),
		m_cond(cond), m_stmt(stmt) {}

	~JSDoWhileStatement() { delete m_cond; delete m_stmt; }

	JSExpression* Condition() { return m_cond; }
	JSBlockStatement* Statement() { return m_stmt; }

private:
	JSExpression* m_cond;
	JSBlockStatement* m_stmt;
};

class JSFunctionDeclaration : public JSStatement
{
public:
	JSFunctionDeclaration(const std::string& fncName, const std::vector<std::string>& params, JSBlockStatement* body) : JSStatement(JSStatement::FunctionDeclaration), m_name(fncName), m_params(params), m_body(body) {}
	~JSFunctionDeclaration() { delete m_body; }

	const std::string& name() const { return m_name; }
	JSBlockStatement* body() { return m_body; }

	const std::vector<std::string>& paramList() const { return m_params; }

private:
	std::string  m_name;
	std::vector<std::string> m_params;
	JSBlockStatement* m_body;
};

class JSReturnStatement : public JSStatement
{
public:
	JSReturnStatement(JSExpression* expr) : JSStatement(JSStatement::ReturnStatement), m_expr(expr) {}
	~JSReturnStatement() { delete m_expr; }

	JSExpression* expr() { return m_expr; }

private:
	JSExpression* m_expr;
};

class JSAST
{
public:
	JSAST() {}

	void clear() { m_ast.clear(); }

	JSBlockStatement* root() { return &m_ast; }

private:
	JSBlockStatement m_ast;
};

void printAST(JSAST& ast);
