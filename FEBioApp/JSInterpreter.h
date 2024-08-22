#pragma once
#include "JSAST.h"
#include "JSObject.h"
#include <map>
#include <stdexcept>
#include <stack>

class VarAlreadyDefined : public std::runtime_error {
public: VarAlreadyDefined() : std::runtime_error("Variable already defined") {}
};

class LValueExpected : public std::runtime_error {
public: LValueExpected() : std::runtime_error("l-value expected") {}
};

class ArrayExpected : public std::runtime_error {
public: ArrayExpected() : std::runtime_error("Array expected") {}
};

class InvalidMember : public std::runtime_error {
public: InvalidMember() : std::runtime_error("Invalid member") {}
};

class NullExpression : public std::runtime_error {
public: NullExpression() : std::runtime_error("Null expression") {}
};

typedef std::map<JSString, JSObject> JSScope;

class JSInterpreter
{
public:
	JSInterpreter();
	~JSInterpreter();

	void init();

	void run(JSAST& program);

	JSObject& addVar(const JSString& name);

private:

	void clearAllScope();

	JSScope& currentScope();

	JSObject& findVar(const JSString& name);
	JSObject& findGlobalVar(const JSString& name);

	void push_scope();
	void pop_scope();

private:
	void execStatement       (JSStatement& stmt);
	void execBlockStatement  (JSBlockStatement& stmt, JSObject& ret);
	void execVarDeclStatement(JSVarDeclarationStatement& stmt);
	void execExpressionStmt  (JSExpressionStatement& stmt);
	void execIfStatement     (JSIfStatement& stmt);
	void execWhileStatement  (JSWhileStatement& stmt);
	void execDoWhileStatement(JSDoWhileStatement& stmt);
	void execFunctionDeclaration(JSFunctionDeclaration& stmt);

	void execExpression(JSExpression* expr, JSObject& ret);
	void execSymbolExpression(JSSymbolExpr& var, JSObject& ret);
	void execFuncExpression  (JSFunctionExpr& fnc, JSObject& ret);
	void execUnaryExpression (JSUnaryExpr& fnc, JSObject& ret);
	void execBinaryExpression(JSBinaryExpr& fnc, JSObject& ret);
	void execAssignExpression(JSAssignmentExpr& fnc, JSObject& ret);
	void execArrayExpression (JSArrayLiteral& expr, JSObject& ret);
	void execObjectExpression(JSObjectLiteral& expr, JSObject& ret);
	void execArrayItemExpr   (JSArrayItemExpr& expr, JSObject& ret);
	void execMemberExpr      (JSMemberExpr& expr, JSObject& ret);

	JSObject* evalLValue(JSExpression* expr);

	void execMethod(JSObject& owner, JSObject::Function& f, JSExpressionList& args, JSObject& ret);
	
	void execJSFunction(JSFunctionDeclaration& fnc, JSObjectList& args, JSObject& ret);

private:
	std::vector<JSScope*> m_scope;
	std::map<JSString, JSFunctionDeclaration*> m_fnc;
};
