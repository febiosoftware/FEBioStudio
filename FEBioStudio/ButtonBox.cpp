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
#include "ButtonBox.h"
#include <QGridLayout>
#include <QButtonGroup>
#include <QPushButton>

class UIButtonBox
{
public:
	QGridLayout*  m_grid;
	QButtonGroup* m_group;
	int m_count = 0;

	void setup(CButtonBox* w, const QString& name)
	{
		m_grid = new QGridLayout;
		w->setLayout(m_grid);
		m_grid->setSpacing(2);

		m_group = new QButtonGroup(w);
		m_group->setObjectName(name);
	}

	void AddButton(const QString& txt, int id)
	{
		QPushButton* but = new QPushButton(txt);
		but->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
		but->setCheckable(true);

		m_grid->addWidget(but, m_count / 2, m_count % 2);
		m_group->addButton(but, id);
		m_count++;
	}
};

CButtonBox::CButtonBox(const QString& name, QWidget* parent) : QWidget(parent), ui(new UIButtonBox)
{
	ui->setup(this, name);
}

void CButtonBox::AddButton(const QString& txt, int id)
{
	ui->AddButton(txt, id);
}
