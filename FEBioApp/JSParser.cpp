#include "JSParser.h"
#include <iostream>
#include <memory>
using namespace std;

size_t AbstractValue::m_total_allocs = 0;

void printToken(Token& token)
{
	switch (token.kind)
	{
	case UNKNOWN        : cout << "UNKNOWN\n"; break;
	case IDENTIFIER     : cout << "IDENTIFIER (" << token.stringValue << ")\n"; break;
	case NUMBER         : cout << "NUMBER (" << token.stringValue << ")\n"; break;
	case BOOLEAN        : cout << "BOOLEAN (" << token.stringValue << ")\n"; break;
	case STRING_LITERAL : cout << "STRING_LITERAL (" << token.stringValue << ")\n"; break;
	case FUNCTION       : cout << "FUNCTION (" << token.stringValue << ")\n"; break;
	case VAR            : cout << "VAR\n"; break;
	case CONST          : cout << "CONST\n"; break;
	case ASSIGNMENT     : cout << "ASSIGNMENT\n"; break;
	case PLUS           : cout << "PLUS\n"; break;
	case MINUS          : cout << "MINUS\n"; break;
	case MULTIPLY       : cout << "MULTIPLY\n"; break;
	case DIV            : cout << "DIV\n"; break;
	case EXP            : cout << "EXP\n"; break;
	case IF             : cout << "IF\n"; break;
	case ELSE           : cout << "ELSE\n"; break;
	case DOT            : cout << "DOT\n"; break;
	case COMMA          : cout << "COMMA\n"; break;
	case LP             : cout << "LP\n";break;
	case RP             : cout << "RP\n"; break;
	case LC             : cout << "LC\n"; break;
	case RC             : cout << "RC\n"; break;
	case LB             : cout << "LB\n"; break;
	case RB             : cout << "RB\n"; break;
	case GT             : cout << "GT\n"; break;
	case LT             : cout << "LT\n"; break;
	case GE             : cout << "GE\n"; break;
	case LE             : cout << "LE\n"; break;
	case EQUAL          : cout << "EQUAL\n"; break;
	case NOT_EQUAL      : cout << "NOT_EQUAL\n"; break;
	case NOT            : cout << "NOT\n"; break;
	case AND            : cout << "AND\n"; break;
	case OR             : cout << "OR\n"; break;
	case STATEMENT_END  : cout << "STATEMENT_END\n"; break;
	case SCRIPT_END     : cout << "SCRIPT_END\n"; break;
	default:
		assert(false);
	}
}

JSParser::JSParser()
{
	m_tokenizer = nullptr;
}

JSParser::~JSParser()
{

}

void JSParser::nextToken()
{
	m_tokenizer->nextToken(m_token);
//	printToken(m_token);
}

void JSParser::nextToken(TokenKind expectedToken)
{
	nextToken();
	expectToken(expectedToken);
}

void JSParser::eatToken(TokenKind expectedToken)
{
	expectToken(expectedToken);
	nextToken();
}

void JSParser::expectToken(TokenKind expectedToken)
{
	if (m_token != expectedToken)
	{
		if (expectedToken == STATEMENT_END) throw MissingStatementEnd();
		else if (m_token == SCRIPT_END) throw UnexpectedEnd();
		else throw InvalidToken();
	}
}

JSString JSParser::errorString() const { return m_error; }

bool JSParser::setError(const JSString& err) { m_error = err; return false; }

bool JSParser::parse(const JSString& script)
{
	m_error.clear();
	m_ast.clear();

	if (script.empty()) return true;

	m_tokenizer = new Tokenizer(script);

	bool returnVal = true;
	try {
		parse();
	}
	catch (runtime_error e)
	{
		stringstream ss;
		ss << e.what() << " at line " << m_tokenizer->currentLine() << " position " << m_tokenizer->currentPosition() - 1;
		m_error = ss.str();
		returnVal = false;
	}
	delete m_tokenizer;

	return returnVal;
}

void JSParser::parse()
{
	JSBlockStatement* root = m_ast.root();
	nextToken();
	while (m_token != SCRIPT_END)
	{
		switch (m_token.kind)
		{
		case TokenKind::VAR     : root->AddStatement(parseVarDeclarationStatement()); break;
		case TokenKind::CONST   : root->AddStatement(parseConstDeclarationStatement()); break;
		case TokenKind::LC      : root->AddStatement(parseBlockStatement()); break;
		case TokenKind::IF      : root->AddStatement(parseIfStatement()); break;
		case TokenKind::WHILE   : root->AddStatement(parseWhileStatement()); break;
		case TokenKind::DO      : root->AddStatement(parseDoWhileStatement()); break;
		case TokenKind::FUNCTION: root->AddStatement(parseFunctionDeclaration()); break;
		case TokenKind::RETURN  : root->AddStatement(parseReturnStatement()); break;
		default:
			root->AddStatement(parseExpressionStatement());
		}
	}
}

JSExpressionStatement* JSParser::parseExpressionStatement()
{
	unique_ptr<JSExpression> expr(parseExpression());
	if ((m_token != STATEMENT_END) && (m_token != SCRIPT_END))
		throw MissingStatementEnd();
	nextToken();
	JSExpressionStatement* exprstmt = new JSExpressionStatement(expr.release());
	return exprstmt;
}

JSVarDeclarationStatement* JSParser::parseVarDeclarationStatement()
{
	std::unique_ptr<JSVarDeclarationStatement> stmt(new JSVarDeclarationStatement());
	nextToken(TokenKind::IDENTIFIER);
	while (true)
	{
		string name = m_token.stringValue;
		nextToken();
		JSExpression* expr = nullptr;
		if (m_token == ASSIGNMENT)
		{
			nextToken();
			expr = parseExpression();
		}
		stmt->addVar(name, expr, false);

		if (m_token == STATEMENT_END) break;
		else if (m_token == COMMA) nextToken(TokenKind::IDENTIFIER);
		else throw SyntaxError();
	}
	nextToken();
	return stmt.release();
}

JSVarDeclarationStatement* JSParser::parseConstDeclarationStatement()
{
	nextToken(TokenKind::IDENTIFIER);
	string name = m_token.stringValue;
	nextToken(ASSIGNMENT);
	nextToken();
	std::unique_ptr<JSExpression> expr(parseExpression());
	eatToken(TokenKind::STATEMENT_END);
	JSVarDeclarationStatement* declstmt = new JSVarDeclarationStatement();
	declstmt->addVar(name, expr.release(), true);
	return declstmt;
}

JSIfStatement* JSParser::parseIfStatement()
{
	eatToken(TokenKind::IF);
	eatToken(TokenKind::LP);
	unique_ptr<JSExpression> expr(parseExpression());
	eatToken(TokenKind::RP);
	expectToken(TokenKind::LC);
	unique_ptr<JSBlockStatement> onTrue(parseBlockStatement());
	JSBlockStatement* onFalse = nullptr;
	if (m_token == TokenKind::ELSE)
	{
		nextToken(TokenKind::LC);
		onFalse = parseBlockStatement();
	}
	return new JSIfStatement(expr.release(), onTrue.release(), onFalse);
}

JSWhileStatement* JSParser::parseWhileStatement()
{
	eatToken(TokenKind::WHILE);
	eatToken(TokenKind::LP);
	unique_ptr<JSExpression> expr(parseExpression());
	eatToken(TokenKind::RP);
	expectToken(TokenKind::LC);
	unique_ptr<JSBlockStatement> stmt(parseBlockStatement());
	return new JSWhileStatement(expr.release(), stmt.release());
}

JSDoWhileStatement* JSParser::parseDoWhileStatement()
{
	eatToken(TokenKind::DO);
	expectToken(TokenKind::LC);
	unique_ptr<JSBlockStatement> stmt(parseBlockStatement());
	eatToken(WHILE);
	eatToken(LP);
	unique_ptr<JSExpression> expr(parseExpression());
	eatToken(TokenKind::RP);
	eatToken(TokenKind::STATEMENT_END);
	return new JSDoWhileStatement(stmt.release(), expr.release());
}

JSBlockStatement* JSParser::parseBlockStatement()
{
	JSBlockStatement* block = new JSBlockStatement;
	eatToken(LC);
	while (m_token != RC)
	{
		switch (m_token.kind)
		{
		case TokenKind::VAR    : block->AddStatement(parseVarDeclarationStatement()); break;
		case TokenKind::CONST  : block->AddStatement(parseConstDeclarationStatement()); break;
		case TokenKind::IF     : block->AddStatement(parseIfStatement()); break;
		case TokenKind::LC     : block->AddStatement(parseBlockStatement()); break;
		case TokenKind::RETURN : block->AddStatement(parseReturnStatement()); break;
		default:
			block->AddStatement(parseExpressionStatement());
		}
	}
	nextToken();
	return block;
}

JSFunctionDeclaration* JSParser::parseFunctionDeclaration()
{
	eatToken(TokenKind::FUNCTION);
	expectToken(TokenKind::IDENTIFIER);
	string name = m_token.stringValue;
	nextToken(LP);
	vector<string> params;
	while (m_token != RP)
	{
		nextToken();
		if (m_token != IDENTIFIER) throw IdentifierExpected();
		params.push_back(m_token.stringValue);
		nextToken();
		if ((m_token != RP) && (m_token != COMMA)) throw SyntaxError();
	}
	eatToken(RP);
	expectToken(LC);
	JSBlockStatement* body = parseBlockStatement();
	return new JSFunctionDeclaration(name, params, body);
}

JSReturnStatement* JSParser::parseReturnStatement()
{
	eatToken(TokenKind::RETURN);
	JSExpression* e = nullptr;
	if (m_token != TokenKind::STATEMENT_END)
	{
		e = parseExpression();
	}
	eatToken(TokenKind::STATEMENT_END);
	return new JSReturnStatement(e);
}

JSExpression* JSParser::parseExpression()
{
	JSExpression* left = parseAssignment();
	switch (m_token.kind)
	{
	case TokenKind::ASSIGNMENT: nextToken(); return new JSAssignmentExpr(left, parseExpression(), JSAssignmentExpr::EQUAL); break;
	case TokenKind::ASSIGN_ADD: nextToken(); return new JSAssignmentExpr(left, parseExpression(), JSAssignmentExpr::EQUAL_ADD); break;
	case TokenKind::ASSIGN_SUB: nextToken(); return new JSAssignmentExpr(left, parseExpression(), JSAssignmentExpr::EQUAL_SUB); break;
	case TokenKind::ASSIGN_MUL: nextToken(); return new JSAssignmentExpr(left, parseExpression(), JSAssignmentExpr::EQUAL_MUL); break;
	case TokenKind::ASSIGN_DIV: nextToken(); return new JSAssignmentExpr(left, parseExpression(), JSAssignmentExpr::EQUAL_DIV); break;
	}
	return left;
}

JSExpression* JSParser::parseAssignment()
{
	JSExpression* left = parseLogicExpr();
	switch (m_token.kind)
	{
	case TokenKind::EQUAL: nextToken(); return new JSBinaryExpr(left, parseAssignment(), JSBinaryExpr::EQUAL); break;
	case TokenKind::GT   : nextToken(); return new JSBinaryExpr(left, parseAssignment(), JSBinaryExpr::GT   ); break;
	case TokenKind::LT   : nextToken(); return new JSBinaryExpr(left, parseAssignment(), JSBinaryExpr::LT   ); break;
	case TokenKind::GE   : nextToken(); return new JSBinaryExpr(left, parseAssignment(), JSBinaryExpr::GE   ); break;
	case TokenKind::LE   : nextToken(); return new JSBinaryExpr(left, parseAssignment(), JSBinaryExpr::LE   ); break;
	case TokenKind::AND  : nextToken(); return new JSBinaryExpr(left, parseAssignment(), JSBinaryExpr::AND  ); break;
	case TokenKind::OR   : nextToken(); return new JSBinaryExpr(left, parseAssignment(), JSBinaryExpr::OR   ); break;
	}
	return left;
}

JSExpression* JSParser::parseLogicExpr()
{
	JSExpression* left = parseTermExpr();
	switch (m_token.kind)
	{
	case TokenKind::PLUS : nextToken(); return new JSBinaryExpr(left, parseLogicExpr(), JSBinaryExpr::ADD); break;
	case TokenKind::MINUS: nextToken(); return new JSBinaryExpr(left, parseLogicExpr(), JSBinaryExpr::SUB); break;
	}
	return left;
}

JSExpression* JSParser::parseTermExpr()
{
	JSExpression* left = parseFactorExpr();
	switch (m_token.kind)
	{
	case TokenKind::MULTIPLY: nextToken(); return new JSBinaryExpr(left, parseTermExpr(), JSBinaryExpr::MUL); break;
	case TokenKind::DIV     : nextToken(); return new JSBinaryExpr(left, parseTermExpr(), JSBinaryExpr::DIV); break;
	case TokenKind::MOD     : nextToken(); return new JSBinaryExpr(left, parseTermExpr(), JSBinaryExpr::MOD); break;
	}
	return left;
}

JSExpression* JSParser::parseFactorExpr()
{
	JSExpression* left = parseMemberExpr();
	switch (m_token.kind)
	{
	case TokenKind::EXP: nextToken(); return new JSBinaryExpr(left, parseFactorExpr(), JSBinaryExpr::EXP); break;
	}
	return left;
}

JSExpression* JSParser::parseMemberExpr()
{
	unique_ptr<JSExpression> left(parseBracketExpr());
	while (m_token == DOT)
	{
		eatToken(DOT);
		JSExpression* rv = parseBracketExpr();
		JSExpression* m = new JSMemberExpr(left.release(), rv);
		left.reset(m);
	}
	return left.release();
}

JSExpression* JSParser::parseBracketExpr()
{
	unique_ptr<JSExpression> left(parsePostfixExpr());
	while ((m_token == LB) || (m_token == LP))
	{
		if (m_token == LB)
		{
			JSExpression* e = parseArrayItemExpr(left.release());
			left.reset(e);
		}
		else if (m_token == LP)
		{
			JSExpression* args = parseFunctionExpr(left.release());
			left.reset(args);
		}
	}
	return left.release();
}

JSExpression* JSParser::parsePostfixExpr()
{
	JSExpression* expr = parsePrefixExpr();
	switch (m_token.kind)
	{
	case TokenKind::INC:
	{
		nextToken();
		return new JSUnaryExpr(expr, JSUnaryExpr::POST_INC);
	}
	break;
	case TokenKind::DEC:
	{
		nextToken();
		return new JSUnaryExpr(expr, JSUnaryExpr::POST_DEC);
	}
	break;
	}
	return expr;
}

JSExpression* JSParser::parsePrefixExpr()
{
	JSExpression* expr = nullptr;
	switch (m_token.kind)
	{
	case TokenKind::MINUS:
	{
		nextToken();
		expr = new JSUnaryExpr(parsePrefixExpr(), JSUnaryExpr::NEG);
	}
	break;
	case TokenKind::NOT:
	{
		nextToken();
		expr = new JSUnaryExpr(parsePrefixExpr(), JSUnaryExpr::NOT);
	}
	break;
	case TokenKind::INC:
	{
		nextToken();
		expr = new JSUnaryExpr(parsePrefixExpr(), JSUnaryExpr::INC);
	}
	break;
	case TokenKind::DEC:
	{
		nextToken();
		expr = new JSUnaryExpr(parsePrefixExpr(), JSUnaryExpr::DEC);
	}
	break;
	default:
		expr = parsePrimExpr();
	}
	return expr;
}

JSExpression* JSParser::parsePrimExpr()
{
	JSExpression* expr = nullptr;
	switch (m_token.kind)
	{
	case TokenKind::NUMBER:
	{
		double v = m_token.toNumber();
		nextToken();
		expr = new JSNumberLiteral(v);
	}
	break;
	case TokenKind::BOOLEAN:
	{
		bool b = m_token.toBool();
		nextToken();
		expr = new JSBooleanLiteral(b);
	}
	break;
	case TokenKind::STRING_LITERAL:
	{
		string v = m_token.stringValue;
		nextToken();
		expr = new JSStringLiteral(v);
	}
	break;
	case TokenKind::IDENTIFIER:
	{
		JSString name = m_token.stringValue;
		nextToken();
		expr = new JSSymbolExpr(name);
	}
	break;
	case TokenKind::LP:
	{
		eatToken(TokenKind::LP);
		std::unique_ptr<JSExpression> expr_ptr(parseExpression());
		eatToken(TokenKind::RP);
		expr = expr_ptr.release();
	}
	break;
	case TokenKind::LB: expr = parseArrayLiteral(); break;
	case TokenKind::LC: expr = parseObjectLiteral(); break;
	}
	return expr;
}

JSArrayLiteral* JSParser::parseArrayLiteral()
{
	eatToken(TokenKind::LB);
	unique_ptr<JSArrayLiteral> a(new JSArrayLiteral);
	while (m_token != TokenKind::RB)
	{
		a->add(parseExpression());
		if ((m_token != COMMA) && (m_token != RB)) throw SyntaxError();
		if (m_token == COMMA) nextToken();
	}
	eatToken(RB);
	return a.release();
}

JSObjectLiteral* JSParser::parseObjectLiteral()
{
	eatToken(TokenKind::LC);
	unique_ptr<JSObjectLiteral> o(new JSObjectLiteral);
	while (m_token != TokenKind::RC)
	{
		expectToken(IDENTIFIER);
		string name = m_token.stringValue;
		nextToken(COLON);
		nextToken();
		o->AddProperty(name, parseExpression());
		if ((m_token != COMMA) && (m_token != RC)) throw SyntaxError();
		if (m_token == COMMA) nextToken();
	}
	eatToken(RC);
	return o.release();
}

JSArrayItemExpr* JSParser::parseArrayItemExpr(JSExpression* lv)
{
	eatToken(LB);
	unique_ptr<JSExpression> right(parseExpression());
	eatToken(RB);
	return new JSArrayItemExpr(lv, right.release());
}

JSFunctionExpr* JSParser::parseFunctionExpr(JSExpression* fnc)
{
	eatToken(LP);
	unique_ptr<JSFunctionExpr> func(new JSFunctionExpr(fnc));
	while (m_token != RP)
	{
		JSExpression* expr = parseExpression();
		func->AddArgument(expr);
		if ((m_token != COMMA) && (m_token != RP)) throw SyntaxError();
		if (m_token == COMMA) nextToken();
	}
	eatToken(RP);
	return func.release();
}
