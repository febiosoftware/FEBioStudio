#include "JSInterpreter.h"
#include <iostream>
using namespace std;

JSInterpreter::JSInterpreter() {}
JSInterpreter::~JSInterpreter()
{
	clearAllScope();
}

void JSInterpreter::run(JSAST& program)
{
	JSStatement& root = *program.root();
	execStatement(root);
}

void JSInterpreter::clearAllScope()
{
	for (auto p : m_scope) delete p;
	m_scope.clear();
}

JSScope& JSInterpreter::currentScope()
{
	return *m_scope[m_scope.size() - 1];
}

JSObject& JSInterpreter::findVar(const JSString& name)
{
	int scopes = (int)m_scope.size();
	for (int i = scopes - 1; i >= 0; --i)
	{
		JSScope& scope = *m_scope[i];
		auto it = scope.find(name);
		if (it != scope.end()) return it->second;
	}
	throw UnknownVariable();
}

JSObject& JSInterpreter::findGlobalVar(const JSString& name)
{
	JSScope& scope = *m_scope[0];
	auto it = scope.find(name);
	if (it != scope.end()) return it->second;
	throw UnknownVariable();
}

JSObject& JSInterpreter::addVar(const JSString& name)
{
	JSScope& scope = currentScope();
	return scope[name];
}

void JSInterpreter::push_scope()
{
	m_scope.push_back(new JSScope);
}

void JSInterpreter::pop_scope()
{
	size_t n = m_scope.size();
	assert(n > 0);
	if (n > 0)
	{
		delete m_scope[n - 1];
		m_scope.erase(m_scope.begin() + n - 1);
	}
}

void JSInterpreter::init()
{
	clearAllScope();

	// create global scope
	push_scope();

	// add Math module
	JSObject& math = addVar("Math");
	math.m_functions["sin"] = [](const JSObjectList& args, JSObject& ret) {
		ret = sin(args.begin()->toNumber());
		};
	math.m_functions["cos"] = [](const JSObjectList& args, JSObject& ret) {
		ret = cos(args.begin()->toNumber());
		};
	math.m_functions["exp"] = [](const JSObjectList& args, JSObject& ret) {
		ret = exp(args.begin()->toNumber());
		};

	math.m_functions["pow"] = [](const JSObjectList& args, JSObject& ret) {
		auto it = args.begin();
		double x = it->toNumber(); ++it;
		double y = it->toNumber();
		ret = pow(x, y);
		};

	math.m_functions["max"] = [](const JSObjectList& args, JSObject& ret) {
		auto it = args.begin();
		double v = it->toNumber(); it++;
		for (; it != args.end(); ++it)
		{
			double x = it->toNumber();
			if (x > v) v = x;
		}
		ret = v;
		};

	math.m_functions["min"] = [](const JSObjectList& args, JSObject& ret) {
		auto it = args.begin();
		double v = it->toNumber(); it++;
		for (; it != args.end(); ++it)
		{
			double x = it->toNumber();
			if (x < v) v = x;
		}
		ret = v;
		};

}

void JSInterpreter::execStatement(JSStatement& stmt)
{
	switch (stmt.kind())
	{
	case JSStatement::BlockStatement:
	{
		JSObject tmp;
		execBlockStatement(dynamic_cast<JSBlockStatement&>(stmt), tmp);
	}
	break;
	case JSStatement::VarDeclarationStatement: execVarDeclStatement(dynamic_cast<JSVarDeclarationStatement&>(stmt)); break;
	case JSStatement::ExpressionStatement    : execExpressionStmt  (dynamic_cast<JSExpressionStatement&>(stmt)); break;
	case JSStatement::IfStatement            : execIfStatement     (dynamic_cast<JSIfStatement&>(stmt)); break;
	case JSStatement::WhileStatement         : execWhileStatement  (dynamic_cast<JSWhileStatement&>(stmt)); break;
	case JSStatement::DoWhileStatement       : execDoWhileStatement(dynamic_cast<JSDoWhileStatement&>(stmt)); break;
	case JSStatement::FunctionDeclaration    : execFunctionDeclaration(dynamic_cast<JSFunctionDeclaration&>(stmt)); break;
	default:
		assert(false);
	}
}

void JSInterpreter::execBlockStatement(JSBlockStatement& stmt, JSObject& ret)
{
	for (size_t i = 0; i < stmt.Statements(); ++i)
	{
		JSStatement& si = *stmt.Statement(i);
		if (si.kind() != JSStatement::ReturnStatement)
			execStatement(si);
		else
		{
			JSReturnStatement& returnStmt = dynamic_cast<JSReturnStatement&>(si);
			JSExpression* expr = returnStmt.expr();
			if (expr) execExpression(expr, ret);
			break;
		}
	}
}

void JSInterpreter::execVarDeclStatement(JSVarDeclarationStatement& stmt)
{
	for (size_t i = 0; i < stmt.size(); ++i)
	{
		JSVarDeclarationStatement::Var& v = stmt.var(i);
		JSObject& var = addVar(v.name);
		if (v.expr) execExpression(v.expr, var);
	}
}

void JSInterpreter::execIfStatement(JSIfStatement& stmt)
{
	JSObject cond;
	execExpression(stmt.Condition(), cond);
	bool b = cond.toBool();
	if (b)
	{
		execStatement(*stmt.OnTrue());
	}
	else
	{
		if (stmt.OnFalse()) execStatement(*stmt.OnFalse());
	}
}

void JSInterpreter::execWhileStatement(JSWhileStatement& stmt)
{
	JSObject cond;
	execExpression(stmt.Condition(), cond);
	while (cond.toBool()) {
		execStatement(*stmt.Statement());
		execExpression(stmt.Condition(), cond);
	}
}

void JSInterpreter::execDoWhileStatement(JSDoWhileStatement& stmt)
{
	JSObject cond;
	do {
		execStatement(*stmt.Statement());
		execExpression(stmt.Condition(), cond);
	} while (cond.toBool());
}

void JSInterpreter::execExpressionStmt(JSExpressionStatement& stmt)
{
	JSObject tmp;
	execExpression(stmt.expr(), tmp);
}

void JSInterpreter::execExpression(JSExpression* expr, JSObject& ret)
{
	if (expr == nullptr) throw NullExpression();
	switch (expr->kind())
	{
	case JSExpression::Number    : ret = dynamic_cast<JSNumberLiteral *>(expr)->value(); break;
	case JSExpression::String    : ret = dynamic_cast<JSStringLiteral *>(expr)->value(); break;
	case JSExpression::Boolean   : ret = dynamic_cast<JSBooleanLiteral*>(expr)->value(); break;
	case JSExpression::Symbol    : execSymbolExpression(dynamic_cast<JSSymbolExpr    &>(*expr), ret); break;
	case JSExpression::Function  : execFuncExpression  (dynamic_cast<JSFunctionExpr  &>(*expr), ret); break;
	case JSExpression::Unary     : execUnaryExpression (dynamic_cast<JSUnaryExpr     &>(*expr), ret); break;
	case JSExpression::Binary    : execBinaryExpression(dynamic_cast<JSBinaryExpr    &>(*expr), ret); break;
	case JSExpression::Assignment: execAssignExpression(dynamic_cast<JSAssignmentExpr&>(*expr), ret); break;
	case JSExpression::Array     : execArrayExpression (dynamic_cast<JSArrayLiteral  &>(*expr), ret); break;
	case JSExpression::ArrayItem : execArrayItemExpr   (dynamic_cast<JSArrayItemExpr &>(*expr), ret); break;
	case JSExpression::Member    : execMemberExpr      (dynamic_cast<JSMemberExpr    &>(*expr), ret); break;
	case JSExpression::Object    : execObjectExpression(dynamic_cast<JSObjectLiteral&>(*expr), ret); break;
	default:
		assert(false);
	}
}

void JSInterpreter::execSymbolExpression(JSSymbolExpr& var, JSObject& ret)
{
	ret = findVar(var.name());
}

void JSInterpreter::execFuncExpression(JSFunctionExpr& fnc, JSObject& ret)
{
	JSExpression& lv = *fnc.FunctionExpr();
	switch (lv.kind())
	{
	case JSExpression::Symbol:
	{
		JSSymbolExpr& sym = dynamic_cast<JSSymbolExpr&>(lv);
		auto it = m_fnc.find(sym.name());
		if (it != m_fnc.end())
		{
			JSObjectList args;
			for (size_t i = 0; i < fnc.Arguments(); ++i)
			{
				JSObject tmp;
				execExpression(fnc.Argument(i), tmp);
				args.push_back(tmp);
			}

			execJSFunction(*it->second, args, ret);
		}
		else
		{
			// check global variable
			JSObject& global = findGlobalVar("");

			auto it2 = global.m_functions.find(sym.name());
			if (it2 == global.m_functions.end())
				throw UnknownFunction();

			execMethod(global, it2->second, fnc.m_args, ret);
		}
	}
	break;
	default:
		throw UnknownFunction();
	}
}

void JSInterpreter::execUnaryExpression(JSUnaryExpr& b, JSObject& ret)
{
	switch (b.op)
	{
	case JSUnaryExpr::NEG: { JSObject tmp; execExpression(b.expr, tmp); ret = -tmp; } break;
	case JSUnaryExpr::NOT: { JSObject tmp; execExpression(b.expr, tmp); ret = !tmp; } break;
	case JSUnaryExpr::INC:
	{
		JSObject& lv = *evalLValue(b.expr);
		++lv;
		ret = lv;
	}
	break;
	case JSUnaryExpr::DEC:
	{
		JSObject& lv = *evalLValue(b.expr);
		--lv;
		ret = lv;
	}
	break;
	case JSUnaryExpr::POST_INC:
	{
		JSObject& lv = *evalLValue(b.expr);
		ret.SetValue(lv.CopyValue());
		++lv;
	}
	break;
	case JSUnaryExpr::POST_DEC:
	{
		JSObject& lv = *evalLValue(b.expr);
		ret.SetValue(lv.CopyValue());
		--lv;
	}
	break;
	default:
		assert(false);
	}
}

void JSInterpreter::execBinaryExpression(JSBinaryExpr& b, JSObject& ret)
{
	JSObject left;
	JSObject right;
	execExpression(b.left , left);
	execExpression(b.right, right);

	switch (b.op)
	{
	case JSBinaryExpr::ADD: ret = left + right; break;
	case JSBinaryExpr::SUB: ret = left - right; break;
	case JSBinaryExpr::MUL: ret = left * right; break;
	case JSBinaryExpr::DIV: ret = left / right; break;
	case JSBinaryExpr::EXP: ret = pow(left.toNumber(), right.toNumber()); break;
	case JSBinaryExpr::GT : ret = (left.toNumber() > right.toNumber()); break;
	case JSBinaryExpr::LT : ret = (left.toNumber() < right.toNumber()); break;
	case JSBinaryExpr::GE : ret = (left.toNumber() >= right.toNumber()); break;
	case JSBinaryExpr::LE : ret = (left.toNumber() <= right.toNumber()); break;
	case JSBinaryExpr::AND: ret = (left.toBool() && right.toBool()); break;
	case JSBinaryExpr::OR : ret = (left.toBool() || right.toBool()); break;
	break;
	default:
		assert(false);
	}
}

JSObject* JSInterpreter::evalLValue(JSExpression* expr)
{
	switch (expr->kind())
	{
	case JSExpression::Symbol:
	{
		JSSymbolExpr* var = dynamic_cast<JSSymbolExpr*>(expr);
		return &(findVar(var->name()));
	}
	break;
	case JSExpression::ArrayItem:
	{
		JSArrayItemExpr* var = dynamic_cast<JSArrayItemExpr*>(expr);
		JSObject* lv = evalLValue(var->lv);
		if ((lv == nullptr) || !lv->isArray()) throw ArrayExpected();

		JSObject tmp;
		execExpression(var->rv, tmp);
		bool ok = true;
		int n = tmp.toInt(ok);
		return &lv->arrayElement(n);
	}
	break;
	case JSExpression::Member:
	{
		JSMemberExpr* me = dynamic_cast<JSMemberExpr*>(expr);
		JSObject* lv = evalLValue(me->lv);
		if ((lv == nullptr) || !lv->isObject()) throw UnknownVariable();

		JSSymbolExpr* member = dynamic_cast<JSSymbolExpr*>(me->rv);
		if (member == nullptr) throw UnknownMember();

		return &lv->ObjectProperty(member->name());
	}
	break;
	}
	throw LValueExpected();
	return nullptr;
}

void JSInterpreter::execAssignExpression(JSAssignmentExpr& expr, JSObject& ret)
{
	JSObject& lv = *evalLValue(expr.lv);

	JSObject tmp;
	execExpression(expr.rv, tmp);
	switch (expr.op)
	{
	case JSAssignmentExpr::EQUAL    : lv.SetValue(tmp.CopyValue()); break;
	case JSAssignmentExpr::EQUAL_ADD: lv += tmp; break;
	case JSAssignmentExpr::EQUAL_SUB: lv -= tmp; break;
	case JSAssignmentExpr::EQUAL_MUL: lv *= tmp; break;
	case JSAssignmentExpr::EQUAL_DIV: lv /= tmp; break;
	default:
		assert(false);
	}
	ret = lv;
}

void JSInterpreter::execArrayExpression(JSArrayLiteral& expr, JSObject& ret)
{
	ArrayValue* ar = new ArrayValue(expr.size());
	ret.SetValue(ar);
	for (size_t i = 0; i < expr.size(); ++i)
	{
		JSExpression* e = expr[i];
		execExpression(e, ar->at(i));
	}
}

void JSInterpreter::execObjectExpression(JSObjectLiteral& expr, JSObject& ret)
{
	ObjectValue* o = new ObjectValue();
	ret.SetValue(o);
	auto it = expr.begin();
	for (; it != expr.end(); ++it)
	{
		JSObject tmp;
		execExpression(it->second, tmp);
		o->addProperty(it->first, tmp);
	}
}

void JSInterpreter::execArrayItemExpr(JSArrayItemExpr& expr, JSObject& ret)
{
	JSObject left;
	execExpression(expr.lv, left);
	if (!left.isArray()) throw ArrayExpected();

	JSObject right;
	execExpression(expr.rv, right);
	bool ok = true;
	int n = right.toInt(ok);
	if (!ok) throw InvalidArrayIndex();

	ret = left.arrayElement(n);
}

void JSInterpreter::execMemberExpr(JSMemberExpr& expr, JSObject& ret)
{
	JSObject tmp;
	execExpression(expr.lv, tmp);

	JSExpression& member = *expr.rv;
	switch (member.kind())
	{
	case JSExpression::Symbol: 
	{
		JSSymbolExpr& sym = dynamic_cast<JSSymbolExpr&>(member);
		ret = tmp.get(sym.name());
	}
	break;
	case JSExpression::ArrayItem:
	{
		JSArrayItemExpr& it = dynamic_cast<JSArrayItemExpr&>(member);
		if (it.lv->kind() != JSExpression::Symbol) throw InvalidMember();
		JSSymbolExpr& sym = dynamic_cast<JSSymbolExpr&>(*it.lv);
		JSObject o = tmp.get(sym.name());
		if (!o.isArray()) throw ArrayExpected();

		JSObject rv;
		execExpression(it.rv, rv);
		bool ok = true;
		int n = rv.toInt(ok);
		if (!ok) throw InvalidArrayIndex();
		ret = o.arrayElement(n);
	}
	break;
	case JSExpression::Function:
	{
		JSFunctionExpr& fnc = dynamic_cast<JSFunctionExpr&>(member);
		if (fnc.FunctionExpr()->kind() != JSExpression::Symbol) throw InvalidMember();
		JSSymbolExpr& sym = dynamic_cast<JSSymbolExpr&>(*fnc.FunctionExpr());
		JSObject::Function method = tmp.getFunction(sym.name());
		execMethod(tmp, method, fnc.m_args, ret);
	}
	break;
	default:
		throw InvalidMember();
	}
}

void JSInterpreter::execMethod(JSObject& owner, JSObject::Function& f, JSExpressionList& args, JSObject& ret)
{
	JSObjectList paramList;
	for (int i = 0; i < args.size(); ++i)
	{
		JSObject tmp;
		execExpression(args[i], tmp);
		paramList.push_back(tmp);
	}

	f(paramList, ret);
}

void JSInterpreter::execFunctionDeclaration(JSFunctionDeclaration& stmt)
{
	string name = stmt.name();
	m_fnc[name] = &stmt;
}

void JSInterpreter::execJSFunction(JSFunctionDeclaration& fnc, JSObjectList& args, JSObject& ret)
{
	const auto& paramList = fnc.paramList();
	if (paramList.size() != args.size()) throw InvalidNrOfArgs();
	push_scope();
	auto it = args.begin();
	for (size_t i = 0; i < paramList.size(); ++i, ++it)
	{
		addVar(paramList[i]) = *it;
	}
	execBlockStatement(*fnc.body(), ret);
	pop_scope();
}
