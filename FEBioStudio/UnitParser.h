#pragma once
#include <vector>
#include <QString>
using namespace std;

struct SYMBOL
{
	const char*		s;	// symbol string
	int				p;	// power
};

class Unit
{
public:
	Unit() {}
	Unit(const char* symbol, int p = 1) { SYMBOL u{ symbol,p }; m_f.push_back(u); }
	Unit(const Unit& u) { m_f = u.m_f; }
	void operator = (const Unit& u) { m_f = u.m_f; }

	void operator *= (const Unit& u)
	{
		m_f.insert(m_f.end(), u.m_f.begin(), u.m_f.end());
	}

	void operator /= (const Unit& u)
	{
		for (vector<SYMBOL>::const_iterator it = u.m_f.begin(); it != u.m_f.end(); ++it)
		{
			SYMBOL negu = *it;
			negu.p = -negu.p;
			m_f.push_back(negu);
		}
	}

	void raise(int p)
	{
		for (int i = 0; i < m_f.size(); ++i) m_f[i].p *= p;
	}

	QString toString();

public:
	vector<SYMBOL>	m_f; // all factors making up unit
};

class UnitParser
{
	enum Token
	{
		END,
		MUL,
		DIV,
		POW,
		SYMBOL
	};

public:
	UnitParser(int unit_system) : m_unit_system(unit_system) {}

	Unit parseUnitString(const char* szunit);

private:
	Unit expr();
	Unit term();
	Unit prim();
	int number();
	Unit symbol();
	Token get_token();

private:
	int	m_unit_system;
	Token curr_tok;
	const char*	sz;
};
