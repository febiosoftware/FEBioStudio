#pragma once
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
