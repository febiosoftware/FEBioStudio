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
#include "TextDocument.h"
#include <QDir>
#include <QTextStream>
#include <QPlainTextDocumentLayout>

CTextDocument::CTextDocument(CMainWindow* wnd) : CDocument(wnd)
{
	SetIcon(":/icons/febio.png");
	m_txt.setDocumentLayout(new QPlainTextDocumentLayout(&m_txt));
}

QTextDocument* CTextDocument::GetText()
{
	return &m_txt;
}

bool CTextDocument::ReadFromFile(const QString& fileName)
{
	m_txt.clear();
	m_txt.setDefaultFont(QFont("Courier", 11));
	QTextOption ops = m_txt.defaultTextOption();
	QFontInfo fi(m_txt.defaultFont());
	ops.setTabStopDistance(2 * fi.pixelSize());
	m_txt.setDefaultTextOption(ops);

	m_bValid = false;

	QString normalizeFileName = QDir::fromNativeSeparators(fileName);

	QFile file(normalizeFileName);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return false;

	QString s;
	QTextStream in(&file);
	while (!in.atEnd()) {
		QString line = in.readLine();
		s += line + "\n";
	}

	file.close();

	m_txt.setPlainText(s);
	m_txt.setModified(false);

	m_bValid = true;
	return true;
}

bool CTextDocument::SaveDocument()
{
	QString fileName = QString::fromStdString(GetDocFilePath());
	QString normalizeFileName = QDir::fromNativeSeparators(fileName);

	QFile file(normalizeFileName);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
		return false;

	QTextStream out(&file);
	out << GetText()->toPlainText();
	file.close();

	SetModifiedFlag(false);
	m_txt.setModified(false);

	return true;
}


CHTMLDocument::CHTMLDocument(CMainWindow* wnd) : CDocument(wnd)
{
}

QTextDocument* CHTMLDocument::GetText()
{
	return &m_txt;
}
