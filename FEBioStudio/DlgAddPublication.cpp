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

#include "stdafx.h"
#include <QWidget>
#include <QStringList>
#include <QStackedLayout>
#include <QAction>
#include <QLineEdit>
#include <QTreeWidget>
#include <QFormLayout>
#include <QBoxLayout>
#include <QToolButton>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QVariantMap>
#include <QUrl>
#include "DlgAddPublication.h"
#include "PublicationWidgetView.h"
#include "PublicationWidget.h"


#include <iostream>

class AuthorTreeWidget : public QTreeWidget
{
public:
	AuthorTreeWidget() : QTreeWidget()
	{
		setAcceptDrops(true);
		setDragEnabled(true);
		setDropIndicatorShown(true);
		setDragDropMode(QAbstractItemView::InternalMove);

		setColumnCount(3);

		QStringList headers;
		headers.append("Order");
		headers.append("Given Name");
		headers.append("Family Name");
		setHeaderLabels(headers);
	}

	QStringList getAuthorGiven()
	{
		QStringList authorGiven;
		for(int item = 0; item < topLevelItemCount(); item++)
		{
			authorGiven.push_back(topLevelItem(item)->text(1));
		}

		return authorGiven;
	}

	QStringList getAuthorFamily()
		{
			QStringList authorFamily;
			for(int item = 0; item < topLevelItemCount(); item++)
			{
				authorFamily.push_back(topLevelItem(item)->text(2));
			}

			return authorFamily;
		}

protected:
	void dropEvent(QDropEvent *event)
	{
		QTreeWidget::dropEvent(event);

		for(int item = 0; item < topLevelItemCount(); item++)
		{
			topLevelItem(item)->setText(0, QString::number(item + 1));
		}
	}




};


class Ui::CDlgAddPublication
{
public:
	QStackedLayout* stack;

	QWidget* queryPage;
	QLineEdit* DOI;
	QAction* DOILookup;
	QLineEdit* query;
	QAction* queryLookup;
	::CPublicationWidgetView* pubs;

	QWidget* infoPage;
	QLineEdit* title;
	QLineEdit* year;
	QLineEdit* journal;
	QLineEdit* volume;
	QLineEdit* issue;
	QLineEdit* pages;
	QLineEdit* DOI2;
	AuthorTreeWidget* authors;

public:
	void setup(QDialog* dlg)
	{
//		QVBoxLayout* layout = new QVBoxLayout;
		stack = new QStackedLayout;

		// DOI Page
		queryPage = new QWidget;
		QVBoxLayout* queryPageLayout = new QVBoxLayout;
		QFormLayout* queryForm = new QFormLayout;
		QHBoxLayout* DOILayout = new QHBoxLayout;

		DOI = new QLineEdit;
		DOILayout->addWidget(DOI);

		DOILookup = new QAction(dlg);
		DOILookup->setIcon(QIcon(":/icons/search.png"));
		DOILookup->setObjectName("DOILookup");
		QToolButton* DOILookupBtn = new QToolButton;
		DOILookupBtn->setDefaultAction(DOILookup);
		DOILayout->addWidget(DOILookupBtn);

		queryForm->addRow("DOI: ", DOILayout);


		QHBoxLayout* queryLayout= new QHBoxLayout;

		query = new QLineEdit;
		queryLayout->addWidget(query);

		queryLookup = new QAction(dlg);
		queryLookup->setIcon(QIcon(":/icons/search.png"));
		queryLookup->setObjectName("queryLookup");
		QToolButton* queryLookupBtn = new QToolButton;
		queryLookupBtn->setDefaultAction(queryLookup);

		queryLayout->addWidget(queryLookupBtn);

		queryForm->addRow("Search: ", queryLayout);

		queryPageLayout->addLayout(queryForm);

		pubs = new ::CPublicationWidgetView(::CPublicationWidgetView::SELECTABLE);
		queryPageLayout->addWidget(pubs);

		QDialogButtonBox* querybb = new QDialogButtonBox;
		QPushButton* manualButton = querybb->addButton("Manual Input", QDialogButtonBox::ActionRole);
		querybb->addButton(QDialogButtonBox::Cancel);

		QObject::connect(querybb, SIGNAL(rejected()), dlg, SLOT(reject()));
		QObject::connect(manualButton, SIGNAL(clicked()), dlg, SLOT(manualButtonClicked()));

		queryPageLayout->addWidget(querybb);

		queryPage->setLayout(queryPageLayout);
		stack->addWidget(queryPage);

		// Info Page
		infoPage = new QWidget;
		QVBoxLayout* infoPageLayout = new QVBoxLayout;

		QFormLayout* infoForm = new QFormLayout;
		infoForm->addRow("Title: ", title = new QLineEdit);
		infoForm->addRow("Year: ", year = new QLineEdit);
		infoForm->addRow("Journal: ", journal = new QLineEdit);
		infoForm->addRow("Volume: ", volume = new QLineEdit);
		infoForm->addRow("Issue: ", issue = new QLineEdit);
		infoForm->addRow("Pages: ", pages = new QLineEdit);
		infoForm->addRow("DOI: ", DOI2 = new QLineEdit);

		infoPageLayout->addLayout(infoForm);

		authors = new AuthorTreeWidget;

		infoPageLayout->addWidget(authors);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		QPushButton* backButton = bb->addButton("Back", QDialogButtonBox::ActionRole);
		QObject::connect(bb, SIGNAL(accepted()), dlg, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), dlg, SLOT(reject()));
		QObject::connect(backButton, SIGNAL(clicked()), dlg, SLOT(backButtonClicked()));

		infoPageLayout->addWidget(bb);

		infoPage->setLayout(infoPageLayout);
		stack->addWidget(infoPage);

//		layout->addLayout(stack);



		dlg->setLayout(stack);



	}

	void SetPublicationInfo(::CPublicationWidget* pub)
	{
		title->setText(pub->getTitle());
		title->setCursorPosition(0);

		journal->setText(pub->getJournal());
		journal->setCursorPosition(0);

		year->setText(pub->getYear());
		volume->setText(pub->getVolume());
		issue->setText(pub->getIssue());
		pages->setText(pub->getPages());
		DOI2->setText(pub->getDOI());
		DOI2->setCursorPosition(0);

		authors->clear();
		for(int author = 0; author < pub->getAuthorFamily().size(); author++)
		{
			QStringList info;
			info.push_back(QString::number(author));
			info.push_back(pub->getAuthorGiven()[author]);
			info.push_back(pub->getAuthorFamily()[author]);

			QTreeWidgetItem* item = new QTreeWidgetItem(info);
			authors->addTopLevelItem(item);
		}

		stack->setCurrentIndex(1);
	}

	void addPublication(QJsonObject& publication)
	{
		QString title = publication.value("title").toArray().at(0).toString();
		QString journal = publication.value("container-title").toArray().at(0).toString();
		QString volume = publication.value("volume").toString();
		QString issue = publication.value("issue").toString();
		QString pages = publication.value("page").toString();
		QString DOI = publication.value("DOI").toString();

		QString year;

		if(publication.contains("published-print"))
		{
			year = QString::number(publication.value("published-print").toObject().value("date-parts").toArray().at(0).toArray().at(0).toInt());
		}
		else if(publication.contains("created"))
		{
			year = QString::number(publication.value("created").toObject().value("date-parts").toArray().at(0).toArray().at(0).toInt());
		}

		QStringList authorGiven;
		QStringList authorFamily;

		for(QJsonValueRef author : publication.value("author").toArray())
		{
			QJsonObject authorObject = author.toObject();
			QStringList info;

			if(authorObject.value("sequence").toString().compare("first") == 0)
			{
				authorGiven.push_front(authorObject.value("given").toString());
				authorFamily.push_front(authorObject.value("family").toString());
			}
			else
			{
				authorGiven.push_back(authorObject.value("given").toString());
				authorFamily.push_back(authorObject.value("family").toString());
			}

		}

		pubs->addPublication(title, year, journal, volume, issue, pages, DOI, authorGiven, authorFamily);

	}

};

CDlgAddPublication::CDlgAddPublication(QWidget* parent) : QDialog(parent), ui(new Ui::CDlgAddPublication)
{
	ui->setup(this);
	setWindowTitle("Add Publication");

	restclient = new QNetworkAccessManager;
	connect(restclient, SIGNAL(finished(QNetworkReply*)), this, SLOT(connFinished(QNetworkReply*)));
	connect(ui->pubs, &CPublicationWidgetView::chosen_publication, this, &CDlgAddPublication::publicationChosen);

	QMetaObject::connectSlotsByName(this);
}

void CDlgAddPublication::on_DOILookup_triggered()
{
	QString path = QString("/works/") + ui->DOI->text();

	QUrl myurl;
	myurl.setScheme("https");
	myurl.setHost("api.crossref.org");
	myurl.setPath(path);


	QNetworkRequest request(myurl);
	restclient->get(request);

}

void CDlgAddPublication::on_queryLookup_triggered()
{
	QStringList terms = ui->query->text().split(" ");
	QString path = QString("/works?query.bibliographic=");

	for(auto term : terms)
	{
		path += term + "+";
	}

	// Remove the last +
	path.chop(1);

	path += "&rows=10&filter=type:journal-article";

	QUrl myurl;
	myurl.setScheme("https");
	myurl.setHost("api.crossref.org");
	myurl.setPath(path);

	// Necessary becuase it was not parsing the ? properly
	myurl.setUrl(QUrl::fromPercentEncoding(myurl.toEncoded()));

	std::cout << myurl.toString().toStdString() << std::endl;

	QNetworkRequest request(myurl);
	restclient->get(request);

}

void CDlgAddPublication::connFinished(QNetworkReply* r)
{
	ui->pubs->clear();

	QJsonDocument jsonDoc = QJsonDocument::fromJson(r->readAll());

	QJsonObject message = jsonDoc.object().value("message").toObject();

	if(message.contains("items"))
	{
		for(auto publication : message.value("items").toArray())
		{
			QJsonObject pub = publication.toObject();

			ui->addPublication(pub);
		}
	}
	else
	{
		ui->addPublication(message);
	}

}

void CDlgAddPublication::publicationChosen(CPublicationWidget* pub)
{
	ui->SetPublicationInfo(pub);
}

void CDlgAddPublication::manualButtonClicked()
{
	ui->stack->setCurrentIndex(1);
}

void CDlgAddPublication::backButtonClicked()
{
	ui->stack->setCurrentIndex(0);
}

QString CDlgAddPublication::getTitle()
{
	return ui->title->text();
}

QString CDlgAddPublication::getYear()
{
	return ui->year->text();
}

QString CDlgAddPublication::getJournal()
{
	return ui->journal->text();
}

QString CDlgAddPublication::getVolume()
{
	return ui->volume->text();
}

QString CDlgAddPublication::getIssue()
{
	return ui->issue->text();
}

QString CDlgAddPublication::getPages()
{
	return ui->pages->text();
}

QString CDlgAddPublication::getDOI()
{
	return ui->DOI2->text();
}

QStringList CDlgAddPublication::getAuthorGiven()
{
	return ui->authors->getAuthorGiven();
}

QStringList CDlgAddPublication::getAuthorFamily()
{
	return ui->authors->getAuthorFamily();
}









