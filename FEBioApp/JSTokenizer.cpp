#include "JSTokenizer.h"
#include <map>

static std::map<JSString, TokenKind> keymap;

void init_keymap()
{
	keymap["var"     ] = TokenKind::VAR;
	keymap["const"   ] = TokenKind::CONST;
	keymap["true"    ] = TokenKind::BOOLEAN;
	keymap["false"   ] = TokenKind::BOOLEAN;
	keymap["if"      ] = TokenKind::IF;
	keymap["else"    ] = TokenKind::ELSE;
	keymap["while"   ] = TokenKind::WHILE;
	keymap["do"      ] = TokenKind::DO;
	keymap["function"] = TokenKind::FUNCTION;
	keymap["return"  ] = TokenKind::RETURN;
}

Tokenizer::Tokenizer(const JSString& string) : m_string(string)
{
	m_scriptLength = string.length();
	m_char = 0;
	m_index = 0;
	m_line = 0;
	m_pos = 0;
	if (keymap.empty()) init_keymap();
}

bool Tokenizer::nextChar()
{
	if (m_index < m_scriptLength)
	{
		m_char = m_string[m_index++];
		if (m_char == '\n')
		{
			m_line++;
			m_pos = 0;
		}
		else m_pos++;

		return true;
	}
	else
	{
		m_index = m_scriptLength;
		return false;
	}
}

bool Tokenizer::peekChar(char c)
{
	if (m_index < m_scriptLength)
	{
		if (m_string[m_index] == c)
		{
			return true;
		}
	}
	return false;
}

bool Tokenizer::peekDigit()
{
	if (m_index < m_scriptLength)
	{
		if (isdigit(m_string[m_index]))
		{
			return true;
		}
	}
	return false;
}

bool Tokenizer::peekSpace()
{
	if (m_index < m_scriptLength)
	{
		if (isspace(m_string[m_index]))
		{
			return true;
		}
	}
	return false;
}

bool Tokenizer::peekAlnum()
{
	if (m_index < m_scriptLength)
	{
		char c = m_string[m_index];
		if (isalnum(c) || (c == '_'))
		{
			return true;
		}
	}
	return false;
}

// NOTE: only works for two character tokens!
bool Tokenizer::check(const char* sz)
{
	if (m_char != sz[0]) return false;
	if (!peekChar(sz[1])) return false;
	nextChar();
	return true;
}

bool Tokenizer::checkNumber()
{
	if (isdigit(m_char)) return true;
	if ((m_char == '.') && peekDigit()) return true;
	return false;
}

bool Tokenizer::checkSpace()
{
	return isspace(m_char);
}

bool Tokenizer::checkString()
{
	return (isalnum(m_char) || (m_char == '_'));
}

void Tokenizer::nextToken(Token& token)
{
	token.kind = TokenKind::UNKNOWN;
	token.stringValue.clear();
	token.pos = m_pos;

	if (!nextChar())
	{
		token.kind = SCRIPT_END;
		return;
	}

	if      (check('(' )) parseToken(token, LP);
	else if (check(')' )) parseToken(token, RP);
	else if (check('{' )) parseToken(token, LC);
	else if (check('}' )) parseToken(token, RC);
	else if (check('[' )) parseToken(token, LB);
	else if (check(']' )) parseToken(token, RB);
	else if (check(';' )) parseToken(token, STATEMENT_END);
	else if (check("+=")) parseToken(token, ASSIGN_ADD);
	else if (check("++")) parseToken(token, INC);
	else if (check('+' )) parseToken(token, PLUS);
	else if (check("-=")) parseToken(token, ASSIGN_SUB);
	else if (check("--")) parseToken(token, DEC);
	else if (check('-' )) parseToken(token, MINUS);
	else if (check('%' )) parseToken(token, MOD);
	else if (check("//")) parseLineComment(token);
	else if (check("/*")) parseMultiLineComment(token);
	else if (check("/=")) parseToken(token, ASSIGN_DIV);
	else if (check('/' )) parseToken(token, DIV);
	else if (check("**")) parseToken(token, EXP);
	else if (check("*=")) parseToken(token, ASSIGN_MUL);
	else if (check('*' )) parseToken(token, MULTIPLY);
	else if (check("==")) parseToken(token, EQUAL);
	else if (check('=' )) parseToken(token, ASSIGNMENT);
	else if (check("<=")) parseToken(token, LE);
	else if (check('<' )) parseToken(token, LT);
	else if (check(">=")) parseToken(token, GE);
	else if (check('>' )) parseToken(token, GT);
	else if (check("!=")) parseToken(token, NOT_EQUAL);
	else if (check('!' )) parseToken(token, NOT);
	else if (check('\'')) parseStringLiteral(token, '\'');
	else if (check('\"')) parseStringLiteral(token, '\"');
	else if (check("&&")) parseToken(token, AND);
	else if (check("||")) parseToken(token, OR);
	else if (checkNumber()) parseNumber(token); // do this before checking '.'
	else if (check('.' )) parseToken(token, DOT);
	else if (check(',' )) parseToken(token, COMMA);
	else if (check(':' )) parseToken(token, COLON);
	else if (checkSpace()) parseSpace(token);
	else if (checkString()) parseString(token);
	else throw UnknownToken();
}

void Tokenizer::parseLineComment(Token& token)
{
	while (true) {
		if (!nextChar()) break;
		if (m_char == '\n') break;
	}
	nextToken(token);
}

void Tokenizer::parseMultiLineComment(Token& token)
{
	while (true) {
		if (!nextChar()) break;
		if (m_char == '*')
		{
			if (peekChar('/'))
			{
				nextChar(); // eat '/'
				break;
			}
		}
	}
	nextToken(token);
}

void Tokenizer::parseStringLiteral(Token& token, char c)
{
	do {
		if (!nextChar()) { throw UnexpectedEnd(); }
		if ((m_char == '\n') || (m_char == '\r')) throw SyntaxError();
		if (m_char != c) token.stringValue += m_char;
	} while (m_char != c);
	token.kind = STRING_LITERAL;
}

void Tokenizer::parseSpace(Token& token)
{
	nextToken(token);
}

void Tokenizer::parseNumber(Token& token)
{
	JSString numVal;

	// the first digit was already read in
	numVal.push_back(m_char);
	while (true)
	{
		if (peekDigit())
		{
			nextChar(); // eat digit
			numVal.push_back(m_char);
		}
		else break;
	}

	if (peekChar('.'))
	{
		nextChar();
		numVal.push_back(m_char);
		while (true)
		{
			if (peekDigit())
			{
				nextChar();
				numVal.push_back(m_char);
			}
			else break;
		}
	}

	if (peekChar('e') || peekChar('E'))
	{
		nextChar();
		numVal.push_back(m_char);
		if (peekChar('+') || peekChar('-'))
		{
			nextChar();
			numVal.push_back(m_char);
		}
		while (true)
		{
			if (peekDigit())
			{
				nextChar();
				numVal.push_back(m_char);
			}
			else break;
		}
	}

	token.kind = NUMBER;
	token.stringValue = numVal;
}

void Tokenizer::parseString(Token& token)
{
	JSString val;
	val += m_char;
	while (true)
	{
		if (peekAlnum())
		{
			nextChar();
			val.push_back(m_char);
		}
		else break;
	}

	// check for keywords
	auto it = keymap.find(val);
	if (it != keymap.end())
	{
		token.kind = it->second;
		token.stringValue = val;
	}
	else
	{
		token.kind = IDENTIFIER;
		token.stringValue = val;
	}
}
