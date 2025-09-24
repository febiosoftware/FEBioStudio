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

#pragma once
#include <QDialog>
#include <QStringList>
#include <vector>

namespace Ui {
	class CDlgAddPublication;
}

//class QStringList;
class QTreeWidgetItem;
class QNetworkAccessManager;
class QNetworkReply;
class CPublicationWidget;


class CDlgAddPublication : public QDialog
{
	Q_OBJECT

public:
	CDlgAddPublication(QWidget* parent);

	QString getTitle();
	QString getYear();
	QString getJournal();
	QString getVolume();
	QString getIssue();
	QString getPages();
	QString getDOI();
	QStringList getAuthorGiven();
	QStringList getAuthorFamily();


public slots:
	void on_DOILookup_triggered();
	void on_queryLookup_triggered();
	void on_addAuthor_triggered();
	void on_removeAuthor_triggered();
	void on_authorTree_itemDoubleClicked(QTreeWidgetItem * item, int column);
	void publicationChosen(CPublicationWidget* pub);
	void manualButtonClicked();
	void backButtonClicked();
	void connFinished(QNetworkReply* r);

protected:
	void keyPressEvent(QKeyEvent* e) override;

private:
	Ui::CDlgAddPublication*	ui;
	QNetworkAccessManager* restclient;
};
