#include "JSAST.h"
#include <assert.h>
#include <iostream>
#include <fstream>
using namespace std;

void indent(ostream& out, int n)
{
	if (n <= 0) return;
	for (int i = 0; i < n; ++i) out << "    ";
}

void printAST(ostream& out, JSExpression& expr, int l);

void printAST(ostream& out, JSExpression* expr, int l)
{
	if (expr == nullptr) out << "<null>";
	else printAST(out, *expr, l);
}

void printAST(ostream& out, JSNumberLiteral& expr, int l)
{
	out << "NumberLiteral {\n";
	indent(out, l + 1);
	out << "Value : " << expr.value() << "\n";
	indent(out, l);
	out << "}";
}

void printAST(ostream& out, JSBooleanLiteral& expr, int l)
{
	out << "BooleanLiteral {\n";
	indent(out, l + 1);
	out << "Value : " << (expr.value()? "true" : "false") << "\n";
	indent(out, l);
	out << "}";
}

void printAST(ostream& out, JSStringLiteral& expr, int l)
{
	out << "StringLiteral {\n";
	indent(out, l + 1);
	out << "Value : \'" << expr.value() << "\'\n";
	indent(out, l);
	out << "}";
}

void printAST(ostream& out, JSSymbolExpr& expr, int l)
{
	out << "SymbolExpr {\n";
	indent(out, l + 1);
	out << "Name : \'" << expr.name() << "\'\n";
	indent(out, l);
	out << "}";
}

void printAST(ostream& out, JSUnaryExpr& expr, int l)
{
	out << "UnaryExpr {\n";
	indent(out, l + 1); out << "Operator : ";
	switch (expr.op)
	{
	case JSUnaryExpr::NEG: out << "NEG"; break;
	case JSUnaryExpr::NOT: out << "NOT"; break;
	case JSUnaryExpr::INC: out << "INC"; break;
	case JSUnaryExpr::DEC: out << "DEC"; break;
	case JSUnaryExpr::POST_INC: out << "POST_INC"; break;
	case JSUnaryExpr::POST_DEC: out << "POST_DEC"; break;
	default:
		assert(false);
		break;
	}
	out << ",\n";
	indent(out, l + 1); out << "Expression : ";
	printAST(out, expr.expr, l+1);
	out << "\n"; indent(out, l); out << "}";
}

void printAST(ostream& out, JSBinaryExpr& expr, int l)
{
	out << "BinaryExpr {\n";
	indent(out, l + 1); out << "Operator : ";
	switch (expr.op)
	{
	case JSBinaryExpr::ADD  : out << "ADD"; break;
	case JSBinaryExpr::SUB  : out << "SUB"; break;
	case JSBinaryExpr::MUL  : out << "MUL"; break;
	case JSBinaryExpr::DIV  : out << "DIV"; break;
	case JSBinaryExpr::MOD  : out << "MOD"; break;
	case JSBinaryExpr::EQUAL: out << "EQUAL"; break;
	case JSBinaryExpr::GT   : out << "GT"; break;
	case JSBinaryExpr::GE   : out << "GE"; break;
	case JSBinaryExpr::LT   : out << "LT"; break;
	case JSBinaryExpr::LE   : out << "LE"; break;
	case JSBinaryExpr::AND  : out << "AND"; break;
	case JSBinaryExpr::OR   : out << "OR"; break;
	case JSBinaryExpr::EXP  : out << "EXP"; break;
	default:
		assert(false);
		break;
	}
	out << ",\n";
	indent(out, l + 1); out << "Left : ";
	printAST(out, expr.left, l + 1);
	out << ",\n";
	indent(out, l + 1); out << "Right : ";
	printAST(out, expr.right, l + 1);
	out << "\n";  indent(out, l); out << "}";
}

void printAST(ostream& out, JSArrayItemExpr& expr, int l)
{
	out << "ArrayItemExpr {\n";
	indent(out, l + 1); out << "Left : ";
	printAST(out, expr.lv, l + 1);
	out << ",\n";
	indent(out, l + 1); out << "Right : ";
	printAST(out, expr.rv, l + 1);
	out << "\n";  indent(out, l); out << "}";
}

void printAST(ostream& out, JSMemberExpr& expr, int l)
{
	out << "MemberExpr {\n";
	indent(out, l + 1); out << "Left : ";
	printAST(out, expr.lv, l + 1);
	out << ",\n";
	indent(out, l + 1); out << "Right : ";
	printAST(out, expr.rv, l + 1);
	out << "\n";  indent(out, l); out << "}";
}

void printAST(ostream& out, JSAssignmentExpr& expr, int l)
{
	out << "AssignmentExpr {\n";
	indent(out, l + 1); out << "Operator : ";
	switch (expr.op)
	{
	case JSAssignmentExpr::EQUAL    : out << "Equal"; break;
	case JSAssignmentExpr::EQUAL_ADD: out << "EqualAdd"; break;
	case JSAssignmentExpr::EQUAL_SUB: out << "EqualSub"; break;
	case JSAssignmentExpr::EQUAL_MUL: out << "EqualMul"; break;
	case JSAssignmentExpr::EQUAL_DIV: out << "EqualDiv"; break;
	default:
		assert(false);
		break;
	}
	out << ",\n";
	indent(out, l + 1); out << "Left : ";
	printAST(out, expr.lv, l + 1);
	out << ",\n";
	indent(out, l + 1); out << "Right : ";
	printAST(out, expr.rv, l + 1);
	out << "\n"; indent(out, l); out << "}";
}

void printAST(ostream& out, JSFunctionExpr& fnc, int l)
{
	out << "FunctionExpr {\n";
	indent(out, l + 1); out << "Function : ";
	printAST(out, fnc.FunctionExpr(), l + 1);
	out << "\n"; indent(out, l + 1); out << "Args : [\n";
	for (int i = 0; i < fnc.Arguments(); ++i)
	{
		JSExpression* arg = fnc.Argument(i);
		indent(out, l + 2);
		printAST(out, arg, l + 2);
		if (i != fnc.Arguments() - 1)
			out << ",\n";
		else
			out << "\n";
	}
	indent(out, l + 1); out << "]\n";
	indent(out, l); out << "}";
}

void printAST(ostream& out, JSArrayLiteral& ar, int l)
{
	out << "ArrayLiteral {\n";
	indent(out, l + 1); out << "Value : [\n";
	for (int i = 0; i < ar.size(); ++i)
	{
		JSExpression* arg = ar[i];
		indent(out, l + 2);
		printAST(out, arg, l + 2);
		if (i != ar.size() - 1)
			out << ",\n";
		else
			out << "\n";
	}
	indent(out, l + 1); out << "]\n";
	indent(out, l); out << "}";
}

void printAST(ostream& out, JSObjectLiteral& ar, int l)
{
	out << "ObjectLiteral {\n";
	indent(out, l + 1); out << "Value : {\n";
	auto it = ar.begin();
	size_t n = ar.Properties();
	for (size_t i=0; i<n; ++it, ++i)
	{
		std::string name = it->first;
		JSExpression* arg = it->second;
		indent(out, l + 2); out << "Name  : " << name << ",\n";
		indent(out, l + 2); out << "Value : ";
		printAST(out, arg, l + 2);
		if (i != n - 1)
			out << ",\n";
		else
			out << "\n";
	}
	indent(out, l + 1); out << "}\n";
	indent(out, l); out << "}";
}

void printAST(ostream& out, JSExpression& expr, int l)
{
	switch (expr.kind())
	{
	case JSExpression::Number    : printAST(out, dynamic_cast<JSNumberLiteral &>(expr), l); break;
	case JSExpression::Boolean   : printAST(out, dynamic_cast<JSBooleanLiteral&>(expr), l); break;
	case JSExpression::String    : printAST(out, dynamic_cast<JSStringLiteral &>(expr), l); break;
	case JSExpression::Symbol    : printAST(out, dynamic_cast<JSSymbolExpr    &>(expr), l); break;
	case JSExpression::Unary     : printAST(out, dynamic_cast<JSUnaryExpr     &>(expr), l); break;
	case JSExpression::Binary    : printAST(out, dynamic_cast<JSBinaryExpr    &>(expr), l); break;
	case JSExpression::Assignment: printAST(out, dynamic_cast<JSAssignmentExpr&>(expr), l); break;
	case JSExpression::Function  : printAST(out, dynamic_cast<JSFunctionExpr  &>(expr), l); break;
	case JSExpression::Array     : printAST(out, dynamic_cast<JSArrayLiteral  &>(expr), l); break;
	case JSExpression::ArrayItem : printAST(out, dynamic_cast<JSArrayItemExpr &>(expr), l); break;
	case JSExpression::Object    : printAST(out, dynamic_cast<JSObjectLiteral &>(expr), l); break;
	case JSExpression::Member    : printAST(out, dynamic_cast<JSMemberExpr    &>(expr), l); break;
	default:
		assert(false);
		break;
	}
}

void printASTStatement(ostream& out, JSStatement& stmt, int l);

void printAST(ostream& out, JSExpressionStatement& stmt, int l)
{
	out << "ExpressionStatement {\n"; 
	indent(out, l + 1);
	out << "Expression: ";
	JSExpression* expr = stmt.expr();
	printAST(out, *expr, l +  1);
	out << "\n";
	indent(out, l); out << "}";
}

void printAST(ostream& out, JSVarDeclarationStatement& stmt, int l)
{
	out << "VarDeclarationStatement {\n";
	if (stmt.size() == 1)
	{
		JSVarDeclarationStatement::Var& v = stmt.var(0);
		indent(out, l + 1); out << "Name: " << v.name << ",\n";
		indent(out, l + 1); out << "Expression : ";
		JSExpression* expr = v.expr;
		if (expr) printAST(out, *expr, l + 1); else out << "<null>";
	}
	else
	{
		indent(out, l + 1); out << "Variables: [\n";
		for (size_t i = 0; i < stmt.size(); ++i)
		{
			indent(out, l + 2); out << "{\n";
			JSVarDeclarationStatement::Var& v = stmt.var(i);
			indent(out, l + 3); out << "Name: " << v.name << ",\n";
			indent(out, l + 3); out << "Expression : ";
			JSExpression* expr = v.expr;
			if (expr) printAST(out, *expr, l + 3); else out << "<null>";
			out << "\n"; indent(out, l + 2);
			if (i != stmt.size() - 1) out << "},\n"; else out << "}\n";
		}
		indent(out, l + 1); out << "]";
	}
	out << "\n"; indent(out, l); out << "}";
}

void printAST(ostream& out, JSBlockStatement& stmt, int l)
{
	out << "BlockStatement {\n";
	indent(out, l + 1); out << "Statements : [\n";
	for (int i = 0; i < stmt.Statements(); ++i)
	{
		JSStatement& si = *stmt.Statement(i);
		indent(out, l + 2);
		printASTStatement(out, si, l+2);
		if (i != stmt.Statements()-1)
			out << ",\n";
		else
			out << "\n";
	}
	indent(out, l+1); out << "]\n";
	indent(out, l); out << "}";
}

void printAST(ostream& out, JSFunctionDeclaration& stmt, int l)
{
	out << "FunctionDeclaration {\n";
	indent(out, l + 1); out << "Name: " << stmt.name() << "\n";
	indent(out, l + 1); out << "Params: [";
	const auto& params = stmt.paramList();
	for (size_t i=0; i<params.size(); ++i)
	{
		const string& s = params[i];
		out << s;
		if (i != params.size() - 1) out << ", ";
		else out << "],\n";
	}
	indent(out, l + 1); out << "Body: ";
	printAST(out, *stmt.body(), l + 1);
	out << "\n"; indent(out, l); out << "}";
}

void printAST(ostream& out, JSIfStatement& stmt, int l)
{
	out << "IfStatement {\n";
	indent(out, l + 1); out << "Condition: ";
	JSExpression& cond = *stmt.Condition(); 
	printAST(out, cond, l + 1);
	out << ",\n"; 
	JSBlockStatement* onTrue = stmt.OnTrue();
	indent(out, l + 1); out << "OnTrue : ";
	printAST(out, *onTrue, l + 1);
	JSBlockStatement* onFalse = stmt.OnFalse();
	if (onFalse)
	{
		out << ",\n"; indent(out, l + 1); out << "OnFalse : ";
		printAST(out, *onFalse, l + 1);
	}
	out << "\n"; indent(out, l); out << "}";
}

void printAST(ostream& out, JSWhileStatement& stmt, int l)
{
	out << "WhileStatement {\n";
	indent(out, l + 1); out << "Condition: ";
	JSExpression& cond = *stmt.Condition();
	printAST(out, cond, l + 1);
	out << ",\n";
	JSBlockStatement* block = stmt.Statement();
	indent(out, l + 1); out << "Statement : ";
	printAST(out, *block, l + 1);
	out << "\n"; indent(out, l); out << "}";
}

void printAST(ostream& out, JSDoWhileStatement& stmt, int l)
{
	out << "DoWhileStatement {\n";
	JSBlockStatement* block = stmt.Statement();
	indent(out, l + 1); out << "Statement : ";
	printAST(out, *block, l + 1);
	out << ",\n";
	indent(out, l + 1); out << "Condition: ";
	JSExpression& cond = *stmt.Condition();
	printAST(out, cond, l + 1);
	out << "\n"; indent(out, l); out << "}";
}

void printAST(ostream& out, JSReturnStatement& stmt, int l)
{
	out << "ReturnStatement {\n";
	indent(out, l + 1); out << "Expression : ";
	JSExpression* e = stmt.expr();
	if (e) printAST(out, *e, l + 1);
	else out << "<null>";
	out << "\n"; indent(out, l); out << "}";
}

void printASTStatement(ostream& out, JSStatement& stmt, int l)
{
	switch (stmt.kind())
	{
	case JSStatement::ExpressionStatement    : printAST(out, dynamic_cast<JSExpressionStatement    &>(stmt), l); break;
	case JSStatement::VarDeclarationStatement: printAST(out, dynamic_cast<JSVarDeclarationStatement&>(stmt), l); break;
	case JSStatement::IfStatement            : printAST(out, dynamic_cast<JSIfStatement            &>(stmt), l); break;
	case JSStatement::DoWhileStatement       : printAST(out, dynamic_cast<JSDoWhileStatement       &>(stmt), l); break;
	case JSStatement::WhileStatement         : printAST(out, dynamic_cast<JSWhileStatement         &>(stmt), l); break;
	case JSStatement::BlockStatement         : printAST(out, dynamic_cast<JSBlockStatement         &>(stmt), l); break;
	case JSStatement::FunctionDeclaration    : printAST(out, dynamic_cast<JSFunctionDeclaration    &>(stmt), l); break;
	case JSStatement::ReturnStatement        : printAST(out, dynamic_cast<JSReturnStatement        &>(stmt), l); break;
	default:
		break;
	}
}

void printAST(JSAST& ast)
{
	ofstream f("ast.txt");
	printASTStatement(f, *ast.root(), 0);
	f.close();
}
