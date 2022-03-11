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

#include <QWidget>

class MoveHandle : public QWidget
{
public:
    MoveHandle(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent *event) override;
};

namespace Ui
{
    class CFindWidget;
}

class CFindWidget : public QWidget
{
    Q_OBJECT
public:
    CFindWidget(QWidget* parent = nullptr);
    ~CFindWidget();

    QString GetFindText();
    QString GetReplaceText();
    bool GetCaseSensative();

    void SetCurrent(int current);
    void SetTotal(int total);

public slots:
    void ShowFind();
    void ShowReplace();

signals:
    void next();
    void prev();
    void replace();
    void replaceAll();

private:
    Ui::CFindWidget* ui;
};