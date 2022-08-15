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

#include "stdafx.h"
#include "UnitParser.h"
#include "units.h"

// prefixes
#define GIGA  "G"	// 10^9
#define MEGA  "M"	// 10^6
#define KILO  "k"	// 10^3
#define HECTO "h"	// 10^2
#define DECA  "da"	// 10^1
#define DECI  "d"	// 10^-1
#define CENTI "c"	// 10^-2
#define MILLI "m"	// 10^-3
#define MICRO "mu"	// 10^-6
#define NANO  "n"	// 10^-9
#define PICO  "p"   // 10^-12
#define FEMTO "f"   // 10^-15
#define ATTO  "a"   // 10^-18

// turn this off to use non-unicode representation of symbols
#define USE_UNICODE

#ifdef USE_UNICODE
#define POW_2 QString(QChar(0x00B2))
#define POW_3 QString(QChar(0x00B3))
#define POW_4 QString(QChar(0x2074))
#define POW_5 QString(QChar(0x2075))
#else
#define POW_2 QString("^2")
#define POW_3 QString("^3")
#define POW_4 QString("^4")
#define POW_5 QString("^5")
#endif

#define DEG "_deg"
#define RAD "rad"

Unit UnitParser::parseUnitString(const char* szunit)
{
	sz = szunit;
	return expr();
}

Unit UnitParser::expr()
{
	Unit left = term();

	for (;;)
		switch (curr_tok)
		{
		case MUL:
			left *= term();
			break;
		case DIV:
			left /= expr();
			break;
		case END:
			return left;
			break;
		default:
			assert(false);
			return left;
		}
}

Unit UnitParser::term()
{
	Unit left = prim();

	for (;;)
		switch (curr_tok)
		{
		case POW:
			left.raise(number());
			break;
		default:
			return left;
		}
}

Unit UnitParser::prim()
{
	get_token();

	Unit u;
	switch (curr_tok)
	{
	case SYMBOL:
	{
		u = symbol();
	}
	break;
	case END: return u; break;
	default:
		assert(false);
	}

	return u;
}

int UnitParser::number()
{
	int num = atoi(sz);
	while (isdigit(*sz)) sz++;
	get_token();
	return num;
}

Unit UnitParser::symbol()
{
	assert(curr_tok == SYMBOL);

	char ch = *sz++;

	const char* s = nullptr;
	switch (ch)
	{
	case 'L': s = Units::GetUnitSymbol(m_unit_system, Units::LENGTH); break;
	case 'M': s = Units::GetUnitSymbol(m_unit_system, Units::MASS); break;
	case 't': s = Units::GetUnitSymbol(m_unit_system, Units::TIME); break;
	case 'T': s = Units::GetUnitSymbol(m_unit_system, Units::TEMPERATURE); break;
	case 'I': s = Units::GetUnitSymbol(m_unit_system, Units::CURRENT); break;
	case 'n': s = Units::GetUnitSymbol(m_unit_system, Units::SUBSTANCE); break;
	case 'F': s = Units::GetUnitSymbol(m_unit_system, Units::FORCE); break;
	case 'P': s = Units::GetUnitSymbol(m_unit_system, Units::PRESSURE); break;
	case 'E': s = Units::GetUnitSymbol(m_unit_system, Units::ENERGY); break;
	case 'W': s = Units::GetUnitSymbol(m_unit_system, Units::POWER); break;
	case 'V': s = Units::GetUnitSymbol(m_unit_system, Units::VOLTAGE); break;
	case 'c': s = Units::GetUnitSymbol(m_unit_system, Units::CONCENTRATION); break;
    case 'o': s = Units::GetUnitSymbol(m_unit_system, Units::ONE); break;
	case 'd': s = DEG; break;
	case 'r': s = RAD; break;
	}

	get_token();

	if (s[0] == '[')
	{
		UnitParser p(m_unit_system);
		return p.parseUnitString(s + 1);
	}
	else
	{
		return Unit(s);
	}
}

UnitParser::Token UnitParser::get_token()
{
	if (*sz == ']') sz++;

	char ch = *sz++;
	switch (ch)
	{
	case 0: return curr_tok = END; break;
	case '^': return curr_tok = POW; break;
	case '.': return curr_tok = MUL; break;
	case '/': return curr_tok = DIV; break;
	}
	sz--;
	return curr_tok = SYMBOL;
}


QString Unit::toString()
{
	QString s;
	int neg = 0;
	int m = 0;
	for (int i = 0; i < m_f.size(); ++i)
	{
		SYMBOL& f = m_f[i]; assert(f.p != 0);
		if (f.p > 0)
		{
			if (m != 0) s += ".";
			if (strcmp(f.s, DEG) == 0) s += QString(QChar(0x00B0));
			else s += f.s;
			if (f.p > 1)
			{
				switch (f.p)
				{
				case 2: s += POW_2; break;
				case 3: s += POW_3; break;
				case 4: s += POW_4; break;
                case 5: s += POW_5; break;
				default:
					s += QString("^%1").arg(f.p);
				}
			}
			m++;
		}
		if (f.p < 0) neg++;
	}

	if (neg > 0)
	{
		s += "/";
		m = 0;
		for (int i = 0; i < m_f.size(); ++i)
		{
			SYMBOL& f = m_f[i]; assert(f.p != 0);
			if (f.p < 0)
			{
				if (m != 0) s += ".";
				if (strcmp(f.s, DEG) == 0) s += QString(QChar(0x00B0));
				else s += f.s;
				int p = -f.p;
				if (p > 1)
				{
					switch (p)
					{
					case 2: s += POW_2; break;
					case 3: s += POW_3; break;
					case 4: s += POW_4; break;
                    case 5: s += POW_5; break;
					default:
						s += QString("^%1").arg(p);
					}
				}
				m++;
			}
		}
	}

	return s;
}
