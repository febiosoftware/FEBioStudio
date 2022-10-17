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

#include "FEDOF.h"

//-----------------------------------------------------------------------------
FEDOF::FEDOF(const std::string& name, const std::string& symbol)
{
	m_name = name;
	m_symbol = symbol;
}

FEDOF::FEDOF(const FEDOF& dof)
{
	m_name = dof.m_name;
	m_symbol = dof.m_symbol;
}

void FEDOF::operator=(const FEDOF& dof)
{
	m_name = dof.m_name;
	m_symbol = dof.m_symbol;
}

FEDOFVariable::FEDOFVariable(const std::string& name)
{
	m_name = name;
}

FEDOFVariable::FEDOFVariable(const FEDOFVariable& var)
{
	m_name = var.m_name;
	m_dofs = var.m_dofs;
}

void FEDOFVariable::operator = (const FEDOFVariable& var)
{
	m_name = var.m_name;
	m_dofs = var.m_dofs;
}

void FEDOFVariable::AddDOF(const std::string& name, const std::string& symbol)
{
	FEDOF dof(name, symbol);
	m_dofs.push_back(dof);
}
