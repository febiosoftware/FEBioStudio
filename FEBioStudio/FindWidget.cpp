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

#include <QMouseEvent>
#include <QApplication>
#include <QCursor>
#include <QPainter>
#include <QStyle>
#include <QStyleOption>
#include <QLabel>
#include <QLineEdit>
#include <QToolBar>
#include <QAction>
#include <QBoxLayout>
#include "FindWidget.h"


MoveHandle::MoveHandle(QWidget* parent) : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Ignored);
    setMinimumWidth(14);
}

void MoveHandle::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    QStyle *style = this->style();
    QStyleOption opt;
    opt.state = opt.state | QStyle::State_Horizontal;

    int handleWidth = style->pixelMetric(QStyle::PM_ToolBarHandleExtent, &opt, this);
    opt.rect = QRect((width() - handleWidth)/2, 0, handleWidth, height());
    style->drawPrimitive(QStyle::PE_IndicatorToolBarHandle, &opt, &p, this);
}

void MoveHandle::mouseMoveEvent(QMouseEvent *event)
{
    QWidget* p = static_cast<QWidget*>(parent());
    QWidget* gp = static_cast<QWidget*>(p->parent());
    
    QPoint newPos = p->mapToParent(mapToParent(event->pos()));

    newPos -= mapToParent(QPoint(width()/2.0, height()/2.0));

    if(newPos.x() < 0) newPos.setX(0);
    if(newPos.x() > gp->width() - p->width()) newPos.setX(gp->width() - p->width());
    if(newPos.y() < 0) newPos.setY(0);
    if(newPos.y() > gp->height() - p->height()) newPos.setY(gp->height() - p->height());

    p->move(newPos);
}

void MoveHandle::enterEvent(QEnterEvent *event)
{
    QApplication::setOverrideCursor(Qt::DragMoveCursor);

}

void MoveHandle::leaveEvent(QEvent *event)
{
    QApplication::restoreOverrideCursor();
}

class Ui::CFindWidget
{
public:
    QToolBar* findToolbar;
    QLabel* findLabel;
    QLineEdit* findEdit;
    QLabel* numLabel;
    QAction* actionNext;
    QAction* actionPrevious;
    QAction* actionCaseSensative;
    QAction* actionClose;

    QToolBar* replaceToolbar;
    QLabel* replaceLabel;
    QLineEdit* replaceEdit;
    QAction* actionReplace;
    QAction* actionReplaceAll;


public:
    void setupUI(::CFindWidget* parent)
    {
        QStyle* style = qApp->style();

        QHBoxLayout* outerLayout = new QHBoxLayout;
        outerLayout->setContentsMargins(0,0,0,0);
        outerLayout->setSpacing(0);

        outerLayout->addWidget(new MoveHandle);
        
        QVBoxLayout* layout = new QVBoxLayout;
        layout->setContentsMargins(0,0,0,0);
        layout->setSpacing(0);
        
        findToolbar = new QToolBar;
        findToolbar->addWidget(findLabel = new QLabel("Find: "));
        findToolbar->addWidget(findEdit = new QLineEdit);
        findToolbar->addWidget(numLabel = new QLabel);
        findToolbar->addSeparator();
        findToolbar->addAction(actionNext = new QAction(style->standardIcon(QStyle::SP_ArrowDown), "Next"));
        findToolbar->addAction(actionPrevious = new QAction(style->standardIcon(QStyle::SP_ArrowUp), "Previous"));
        findToolbar->addAction(actionCaseSensative = new QAction("Aa"));
        actionCaseSensative->setCheckable(true);
        findToolbar->addAction(actionClose = new QAction(style->standardIcon(QStyle::SP_TitleBarCloseButton), "Close"));
        layout->addWidget(findToolbar);

        replaceToolbar = new QToolBar;
        replaceToolbar->addWidget(replaceLabel = new QLabel("Replace: "));
        replaceToolbar->addWidget(replaceEdit = new QLineEdit);
        replaceToolbar->addSeparator();
        replaceToolbar->addAction(actionReplace = new QAction("Cur"));
        replaceToolbar->addAction(actionReplaceAll = new QAction("All"));
        layout->addWidget(replaceToolbar);

        replaceToolbar->hide();

        outerLayout->addLayout(layout);

        parent->setLayout(outerLayout);
        parent->setContentsMargins(0,0,0,0);

        parent->setAutoFillBackground(true);

        posInit = false;
        current = 0;
        total = 0;
    }

    void updateNumLabel()
    {
        numLabel->setText(QString("%1/%2").arg(current).arg(total));
    }

public:
    bool posInit;
    int current, total;

};


CFindWidget::CFindWidget(QWidget* parent) 
    : QWidget(parent), ui(new Ui::CFindWidget)
{
    ui->setupUI(this);

    connect(ui->actionNext, &QAction::triggered, this, &CFindWidget::next);
    connect(ui->actionPrevious, &QAction::triggered, this, &CFindWidget::prev);
    connect(ui->actionReplace, &QAction::triggered, this, &CFindWidget::replace);
    connect(ui->actionReplaceAll, &QAction::triggered, this, &CFindWidget::replaceAll);
    connect(ui->actionClose, &QAction::triggered, [=]{hide();});

}

CFindWidget::~CFindWidget()
{
    delete ui;
}

QString CFindWidget::GetFindText()
{
    return ui->findEdit->text();
}

QString CFindWidget::GetReplaceText()
{
    return ui->replaceEdit->text();
}

bool CFindWidget::GetCaseSensative()
{
    return ui->actionCaseSensative->isChecked();
}

void CFindWidget::SetCurrent(int current)
{
    ui->current = current;
    ui->updateNumLabel();
}

void CFindWidget::SetTotal(int total)
{
    ui->total = total;
    ui->updateNumLabel();
}

void CFindWidget::ShowFind()
{
    hide();
    ui->replaceToolbar->hide();
    show();

    if(!ui->posInit)
    {
        move(static_cast<QWidget*>(parent())->width() - width(), 0);
        ui->posInit = true;
    }
    
}

void CFindWidget::ShowReplace()
{
    hide();
    ui->replaceToolbar->show();
    show();

    if(!ui->posInit)
    {
        move(static_cast<QWidget*>(parent())->width() - width(), 0);
        ui->posInit = true;
    }
}
