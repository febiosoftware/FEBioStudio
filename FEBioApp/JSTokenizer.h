#pragma once
#include <string>
#include <stdexcept>
#include <string>
#include <sstream>
#include "JSObject.h"

class SyntaxError : public std::runtime_error {
public: SyntaxError() : std::runtime_error("Syntax error") {}
};

class UnexpectedEnd : public std::runtime_error {
public: UnexpectedEnd() : std::runtime_error("Unexpected end") {}
};

class MissingStatementEnd : public std::runtime_error {
public: MissingStatementEnd() : std::runtime_error("Missing ;") {}
};

class UnknownToken : public std::runtime_error {
public: UnknownToken() : std::runtime_error("Unrecognized token") {}
};

enum TokenKind {
	UNKNOWN,
	NUMBER,
	BOOLEAN,
	STRING_LITERAL,
	IDENTIFIER,
	ASSIGNMENT, ASSIGN_ADD, ASSIGN_SUB, ASSIGN_MUL, ASSIGN_DIV,
	PLUS, MINUS,
	MULTIPLY, DIV, EXP, MOD,
	INC, DEC,
	VAR, CONST,
	IF,
	ELSE,
	WHILE,
	DO,
	FUNCTION,
	RETURN,
	DOT, COMMA, COLON,
	LP, RP, LC, RC, LB, RB,
	GT, LT, GE, LE, EQUAL, NOT_EQUAL, NOT,
	AND, OR,
	STATEMENT_END,
	SCRIPT_END
};

struct Token
{
	TokenKind kind;
	JSString  stringValue;
	size_t    pos;

	Token(TokenKind tokenKind) { kind = tokenKind; pos = 0; }
	Token() { kind = UNKNOWN; pos = 0; }

	bool operator == (TokenKind tokenKind) const { return kind == tokenKind; }
	bool operator != (TokenKind tokenKind) const { return kind != tokenKind; }

	double toNumber() const { return stringValue.toDouble(); }

	bool toBool() const { return (stringValue == "true"); }
};

class Tokenizer
{
public:
	Tokenizer(const JSString& string);

	void nextToken(Token& token);

	size_t currentLine() const { return m_line; }
	size_t currentPosition() const { return m_pos; }

private:
	// get the next character
	bool nextChar();

	// check if next char equals c and advance if true
	bool peekChar(char c);
	bool peekSpace();
	bool peekDigit();
	bool peekAlnum();

	void parseNumber(Token& token);
	void parseString(Token& token);

	void parseToken(Token& token, TokenKind kind) { token.kind = kind; }
	void parseLineComment(Token& token);
	void parseMultiLineComment(Token& token);
	void parseStringLiteral(Token& token, char c);
	void parseSpace(Token& token);

	bool check(char c) { return (m_char == c); }
	bool check(const char* sz);
	bool checkNumber();
	bool checkSpace();
	bool checkString();

private:
	char	m_char;
	size_t m_scriptLength;
	size_t	m_line, m_pos, m_index;
	const JSString& m_string;
};

