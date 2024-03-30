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
#include "CustomLineEdit.h"
#include <QKeyEvent>
#include <QHeaderView>
#include <QScrollBar>

CustomLineEdit::CustomLineEdit(QWidget* parent)  : QLineEdit(parent), c(nullptr)
{
	m_wrapQuotes = true;
	m_delim = " ";
}

void CustomLineEdit::setMultipleCompleter(QCompleter* completer)
{
    c = completer;
    c->setWidget(this);
    QObject::connect(c, SIGNAL(activated(QString)), this, SLOT(insertCompletion(QString)));
}

void CustomLineEdit::setWrapQuotes(bool b)
{
	m_wrapQuotes = b;
}

void CustomLineEdit::setDelimiter(QString d)
{
	m_delim = d;
}

void CustomLineEdit::keyPressEvent(QKeyEvent* e)
{
    switch (e->key())
    {
    case Qt::Key_Enter:
    case Qt::Key_Return:
        if (c && c->popup()->isVisible())
        {
            e->ignore();
            return; // let the completer do default behavior
        }
        else
        {
            QLineEdit::keyPressEvent(e);
            return;
        }
    default:
        break;
    }

    QLineEdit::keyPressEvent(e);

    if (!c) return;

    c->setCompletionPrefix(cursorWord(this->text()));

    QRect cr = cursorRect();
    cr.setWidth(c->popup()->sizeHintForColumn(0)
        + c->popup()->verticalScrollBar()->sizeHint().width());
    c->complete(cr);
}

void CustomLineEdit::focusInEvent(QFocusEvent* e)
{
    QLineEdit::focusInEvent(e);

    if (!c) return;

    c->setCompletionPrefix(cursorWord(this->text()));

    QRect cr = cursorRect();
    cr.setWidth(c->popup()->sizeHintForColumn(0)
        + c->popup()->verticalScrollBar()->sizeHint().width());
    c->complete(cr);
}

QString CustomLineEdit::cursorWord(const QString& sentence) const
{
    return sentence.mid(sentence.left(cursorPosition()).lastIndexOf(" ") + 1,
        cursorPosition() -
        sentence.left(cursorPosition()).lastIndexOf(" ") - 1);
}

void CustomLineEdit::insertCompletion(QString arg)
{
    // Surround the term in quotes if there's a space in it
    if (m_wrapQuotes && arg.contains(' '))
    {
        arg = "\"" + arg + "\"";
    }

    setText(text().replace(text().left(cursorPosition()).lastIndexOf(m_delim) + 1,
        cursorPosition() -
        text().left(cursorPosition()).lastIndexOf(m_delim) - 1,
        arg));
}
