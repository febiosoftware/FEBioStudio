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

#include <QApplication>
#include <QPalette>
#include <QTextDocument>
#include <QResizeEvent>
#include "MultiLineLabel.h"

MultiLineLabel::MultiLineLabel(QWidget *parent)
	: QTextBrowser(parent)
{
	init();
}

MultiLineLabel::MultiLineLabel(const QString &text, QWidget *parent)
	: QTextBrowser(parent)
{
	init();

	setText(text);
}

void MultiLineLabel::setText(const QString &text)
{
	QTextBrowser::setHtml(text);

    adjustSize();
}

void MultiLineLabel::resizeEvent(QResizeEvent *event)
{
	document()->setTextWidth(event->size().width());

	setMinimumHeight(document()->size().height());
	setMaximumHeight(document()->size().height());
}

void MultiLineLabel::init()
{
	// Make this look like a QLabel
	setFrameStyle(QFrame::Plain|QFrame::NoFrame);
	QPalette qpalette = palette();
	qpalette.setColor(QPalette::Base, qApp->palette().color(QPalette::Window));
	setPalette(qpalette);

	// Make this only take up the necessary size
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

	// Allow hyperlinks to open in a browser
	setOpenExternalLinks(true);
	setTextInteractionFlags(Qt::LinksAccessibleByMouse);
}
