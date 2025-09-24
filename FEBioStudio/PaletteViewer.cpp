/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2025 University of Utah, The Trustees of Columbia University in
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
#include "PaletteViewer.h"
#include "CColorButton.h"
#include <QGridLayout>
#include <GLWLib/convert.h>

CPaletteViewer::CPaletteViewer(QWidget* parent) : QWidget(parent)
{
	QGridLayout* l = new QGridLayout();
	for (int i=0; i<5; ++i)
		for (int j = 0; j < 5; ++j)
		{
			CColorButton* b = new CColorButton;
			m_col.push_back(b);
			b->setColor(QColor(0,0,0));
			b->setEnabled(false);
			l->addWidget(b, i, j);
		}

	setLayout(l);
}

void CPaletteViewer::setPalette(const CPalette& pal)
{
	int ncol = pal.Colors();
	if (ncol > m_col.size()) ncol = (int)m_col.size();
	for (int i = 0; i < ncol; ++i)
	{
		CColorButton* b = m_col[i];
		b->setColor(toQColor(pal.Color(i)));
	}

	for (int i = ncol; i < m_col.size(); ++i)
	{
		CColorButton* b = m_col[i];
		b->setColor(QColor(0, 0, 0));
	}
}
