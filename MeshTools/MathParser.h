// MathParser.h: interface for the CMathParser class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MATHPARSER_H__20979FC0_89B2_42C8_B21A_1BD7999D8E6A__INCLUDED_)
#define AFX_MATHPARSER_H__20979FC0_89B2_42C8_B21A_1BD7999D8E6A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <string>
#include <map>

class CMathParser  
{
protected:
	enum Token_value {
		NAME,	NUMBER, END,
		PLUS='+', MINUS='-', MUL='*', DIV='/', POW='^',
		LP='(',	RP=')', PRINT
	};

public:
	CMathParser();
	virtual ~CMathParser();

	void set_variable(const char* szname, const double val);

	double eval(const char* szexpr, int& ierr);

	const char* error_str() { return m_szerr; }

protected:
	double expr();	// add and subtract
	double term();	// multiply and divide
	double prim();	// handle primaries
	double power();	// power
	Token_value get_token();
	double error(const char* str);

	double get_number();
	void get_name(char* str);

	Token_value	curr_tok;

	const char* m_szexpr;

	std::map<std::string, double>	m_table;	// table that stores variables and constants

	double	number_value;
	char	string_value[256];

	char	m_szerr[256];

	int		m_nerrs;
};

#endif // !defined(AFX_MATHPARSER_H__20979FC0_89B2_42C8_B21A_1BD7999D8E6A__INCLUDED_)
