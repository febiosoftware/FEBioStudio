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

#include <QWidget>

class QVBoxLayout;
class QDialogButtonBox;
class QLabel;
class QLineEdit;
class QSpinBox;
class CSelectionBox;

class PyInputWidget : public QWidget
{
    Q_OBJECT

public:

    PyInputWidget(QString lblText = "", QWidget* parent = nullptr);

protected:
    void addWidget(QWidget* wgt);

protected:
    QVBoxLayout* layout;
    int numWgts;

signals:
    void done();

};

class PyInputStringWidget : public PyInputWidget
{
public:
    PyInputStringWidget(QString lblText = "", QWidget* parent = nullptr); 

    std::string getVal();

private:
    QLineEdit* edit;
};

class PyInputIntWidget : public PyInputWidget
{
public:
    PyInputIntWidget(QString lblText = "", QWidget* parent = nullptr); 

    int getVal();

private:
    QSpinBox* spinBox;
};

class PyInputSelectionWidget : public PyInputWidget
{
public:
    PyInputSelectionWidget(QString lblText = "", QWidget* parent = nullptr); 

    int getVal();

private:
    CSelectionBox* box;
};