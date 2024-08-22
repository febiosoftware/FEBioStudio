#pragma once
#include <map>
#include <list>
#include <vector>
#include <assert.h>
#include <stdexcept>
#include <functional>
#include <sstream>
#include "JSTokenizer.h"
#include "JSAST.h"
#include "JSObject.h"

class OutOfBounds : public std::runtime_error {
public: OutOfBounds() : std::runtime_error("Out of bounds") {}
};

class InvalidIndex : public std::runtime_error {
public: InvalidIndex() : std::runtime_error("Invalid index") {}
};

class InvalidToken : public std::runtime_error {
public: InvalidToken() : std::runtime_error("Invalid token") {}
};

class UnknownMemberFunction : public std::runtime_error {
public: UnknownMemberFunction() : std::runtime_error("Unknown member function") {}
};

class ExpressionExpected : public std::runtime_error {
public: ExpressionExpected() : std::runtime_error("Expression expected") {}
};

class IdentifierExpected : public std::runtime_error {
public: IdentifierExpected() : std::runtime_error("Identifier expected") {}
};

class JSParser
{
public:
	JSParser();

	~JSParser();

	bool parse(const JSString& script);

	JSAST& GetAST() { return m_ast; }

	JSString errorString() const;

private:
	bool setError(const JSString& err);

	void nextToken();
	void nextToken(TokenKind expectedToken); // next, then check
	void eatToken(TokenKind expectedToken); // check, then next
	void expectToken(TokenKind expectedToken);

private:
	void parse();
	JSExpressionStatement*     parseExpressionStatement();
	JSVarDeclarationStatement* parseVarDeclarationStatement();
	JSVarDeclarationStatement* parseConstDeclarationStatement();
	JSIfStatement*             parseIfStatement();
	JSWhileStatement*          parseWhileStatement();
	JSDoWhileStatement*        parseDoWhileStatement();
	JSBlockStatement*          parseBlockStatement();
	JSFunctionDeclaration*     parseFunctionDeclaration();
	JSReturnStatement*         parseReturnStatement();

	JSExpression* parseExpression();
	JSExpression* parseAssignment();
	JSExpression* parseLogicExpr();
	JSExpression* parseTermExpr();
	JSExpression* parseFactorExpr();
	JSExpression* parseBracketExpr();
	JSExpression* parseMemberExpr();
	JSExpression* parsePrimExpr();
	JSExpression* parsePrefixExpr();
	JSExpression* parsePostfixExpr();

	JSArrayLiteral* parseArrayLiteral();
	JSObjectLiteral* parseObjectLiteral();
	JSArrayItemExpr* parseArrayItemExpr(JSExpression* lv);
	JSFunctionExpr* parseFunctionExpr(JSExpression* fnc);

private:
	JSString m_error;
	Token	m_token;
	Tokenizer* m_tokenizer;
	JSAST m_ast;
};
