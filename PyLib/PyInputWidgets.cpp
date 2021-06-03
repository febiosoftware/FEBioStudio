/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in 
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

#include "PyInputWidgets.h"

#ifdef HAS_PYTHON
#include "PythonInputHandler.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QSpinBox>
#include <FEBioStudio/SelectionBox.h>

//////////////////////////////////////////////////////////////////////
// PyInputWidget
//////////////////////////////////////////////////////////////////////

PyInputWidget::PyInputWidget(QString lblText, QWidget* parent) 
    : numWgts(0), QWidget(parent)
{
    layout = new QVBoxLayout;
    layout->setAlignment(Qt::AlignCenter);

    if(!lblText.isEmpty())
    {
        layout->addWidget(new QLabel(lblText));
        numWgts++;
    }

    QDialogButtonBox* box = new QDialogButtonBox(QDialogButtonBox::Ok);
    QObject::connect(box, &QDialogButtonBox::accepted, this, &PyInputWidget::done);
    layout->addWidget(box);

    setLayout(layout);
}

void PyInputWidget::addWidget(QWidget* wgt)
{
    layout->insertWidget(numWgts, wgt);
}

//////////////////////////////////////////////////////////////////////
// PyInputStringWidget
//////////////////////////////////////////////////////////////////////

PyInputStringWidget::PyInputStringWidget(QString lblText, QWidget* parent) 
    : PyInputWidget(lblText, parent)
{
    addWidget(edit = new QLineEdit);
}

std::string PyInputStringWidget::getVal()
{
    return edit->text().toStdString();
}

//////////////////////////////////////////////////////////////////////
// PyInputStringWidget
//////////////////////////////////////////////////////////////////////

PyInputIntWidget::PyInputIntWidget(QString lblText, QWidget* parent)
    : PyInputWidget(lblText, parent)
{
    addWidget(spinBox = new QSpinBox);
} 

int PyInputIntWidget::getVal()
{
    return spinBox->value();
}

//////////////////////////////////////////////////////////////////////
// PyInputSelectionWidget
//////////////////////////////////////////////////////////////////////

PyInputSelectionWidget::PyInputSelectionWidget(QString lblText, QWidget* parent)
    : PyInputWidget(lblText, parent)
{
    addWidget(box = new CSelectionBox);
} 

int PyInputSelectionWidget::getVal()
{
    return 0;
}

#else

PyInputWidget::PyInputWidget(QString lblText, QWidget* parent) {}
void PyInputWidget::addWidget(QWidget* wgt) {}

PyInputStringWidget::PyInputStringWidget(QString lblText, QWidget* parent) {}
std::string PyInputStringWidget::getVal() {return "";}

PyInputIntWidget::PyInputIntWidget(QString lblText, QWidget* parent) {}
int PyInputIntWidget::getVal() {return 0;}

PyInputSelectionWidget::PyInputSelectionWidget(QString lblText, QWidget* parent) {}
int PyInputSelectionWidget::getVal() {return 0;}

#endif