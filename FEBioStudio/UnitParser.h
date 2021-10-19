/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2021 University of Utah, The Trustees of Columbia University in
the City of New York, and others.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

#pragma once
#include <vector>
#include <QString>
//using namespace std;

using std::vector;

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
