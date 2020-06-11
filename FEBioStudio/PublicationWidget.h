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

//#include <QPushButton>
#pragma once
#include <QFrame>
#include <QString>
#include <QStringList>
#include <QVariant>

namespace Ui{
class CPublicationWidget;
}

//enum pubWidgetType {EXPANDABLE, SELECTABLE, CHECKABLE};

class CPublicationWidget : public QFrame
{
	Q_OBJECT

public:

	enum Selection {NONE, CHECKBOX, BUTTON};

	CPublicationWidget(Selection selection = NONE, bool expandable = true);
	CPublicationWidget(QVariantMap& data, Selection selection = NONE, bool expandable = true);
	CPublicationWidget(QString title, QString year, QString journal, QString volume,
			QString issue, QString pages, QString DOI, QStringList authorGiven,
			QStringList authorFamily, Selection selection = NONE, bool expandable = true);

	CPublicationWidget(const CPublicationWidget& obj);

	void init();

	QString ShortText();
	QString FullText();

	const QStringList& getAuthorFamily() const;
	void setAuthorFamily(const QStringList &authorFamily);
	const QStringList& getAuthorGiven() const;
	void setAuthorGiven(const QStringList &authorGiven);
	const QString& getDOI() const;
	void setDOI(const QString &doi);
	const QString& getIssue() const;
	void setIssue(const QString &issue);
	const QString& getJournal() const;
	void setJournal(const QString &journal);
	const QString& getPages() const;
	void setPages(const QString &pages);
	const QString& getTitle() const;
	void setTitle(const QString &title);
	const QString& getVolume() const;
	void setVolume(const QString &volume);
	const QString& getYear() const;
	void setYear(const QString &year);
	bool isChecked() const;
	int getSelection() const;



public slots:
	void on_expand_triggered();
	void on_shrink_triggered();
	bool isExpandable() const;

	void on_select_triggered();
	void checkBox_stateChanged(int state);

signals:
	void chosen_publication(CPublicationWidget* pub);


private:
	Ui::CPublicationWidget* ui;


	QString title;
	QString year;
	QString journal;
	QString volume;
	QString issue;
	QString pages;
	QString DOI;

	QStringList authorGiven;
	QStringList authorFamily;

	bool expandable;
	Selection selection;
};
