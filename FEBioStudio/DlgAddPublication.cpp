#include "stdafx.h"
#include <QWidget>
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
//#include "PublicationWidget.h"

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

	QWidget* DOIPage;
	QLineEdit* DOI;
	QAction* DOILookup;

	QWidget* infoPage;
	QLineEdit* title;
	QLineEdit* year;
	QLineEdit* journal;
	QLineEdit* volume;
	QLineEdit* issue;
	QLineEdit* pages;
	QLineEdit* DOI2;
	AuthorTreeWidget* authors;

	QAction* actionBack;

public:
	void setup(QDialog* dlg)
	{
		QVBoxLayout* layout = new QVBoxLayout;
		stack = new QStackedLayout;

		// DOI Page
		DOIPage = new QWidget;
		QVBoxLayout* DOIPageLayout = new QVBoxLayout;
		QHBoxLayout* hlayout = new QHBoxLayout;

		QFormLayout* DOIForm = new QFormLayout;
		DOIForm->addRow("DOI: ", DOI = new QLineEdit);

		hlayout->addLayout(DOIForm);

		DOILookup = new QAction(dlg);
		DOILookup->setIcon(QIcon(":/icons/search.png"));
		DOILookup->setObjectName("DOILookup");
		QToolButton* DOILookupBtn = new QToolButton;
		DOILookupBtn->setDefaultAction(DOILookup);

		hlayout->addWidget(DOILookupBtn);

		DOIPageLayout->addLayout(hlayout);

//		CPublicationWidget* test = new CPublicationWidget("This is a test", "This is a test to see how big this thing can get.\n\nAnd if it can handle new lines.\n\nOr is that's not supported.");
//		DOIPageLayout->addWidget(test);

		DOIPage->setLayout(DOIPageLayout);
		stack->addWidget(DOIPage);

		// Info Page
		infoPage = new QWidget;
		QVBoxLayout* infoPageLayout = new QVBoxLayout;

		actionBack = new QAction(dlg);
//		actionBack->setIcon(QIcon(":/icons/search.png"));
		actionBack->setText("<");
		actionBack->setObjectName("actionBack");
		QToolButton* actionBackBtn = new QToolButton;
		actionBackBtn->setToolButtonStyle(Qt::ToolButtonTextOnly);
		actionBackBtn->setDefaultAction(actionBack);

		infoPageLayout->addWidget(actionBackBtn);

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

		infoPage->setLayout(infoPageLayout);
		stack->addWidget(infoPage);

		layout->addLayout(stack);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
		layout->addWidget(bb);

		dlg->setLayout(layout);

		QObject::connect(bb, SIGNAL(accepted()), dlg, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), dlg, SLOT(reject()));

	}

	void SetInfoFromDOI(QJsonDocument& jsonDoc)
	{
		QJsonObject message = jsonDoc.object().value("message").toObject();

		title->setText(message.value("title").toArray().at(0).toString());
		title->setCursorPosition(0);

		if(message.contains("published-print"))
		{
			year->setText(QString::number(message.value("published-print").toObject().value("date-parts").toArray().at(0).toArray().at(0).toInt()));
		}
		else if(message.contains("created"))
		{
			year->setText(QString::number(message.value("created").toObject().value("date-parts").toArray().at(0).toArray().at(0).toInt()));
		}

		journal->setText(message.value("container-title").toArray().at(0).toString());
		journal->setCursorPosition(0);
		volume->setText(message.value("volume").toString());
		issue->setText(message.value("issue").toString());
		pages->setText(message.value("page").toString());
		DOI2->setText(message.value("DOI").toString());
		DOI2->setCursorPosition(0);

		authors->clear();
		int order = 2;
		for(QJsonValueRef author : message.value("author").toArray())
		{
			QJsonObject authorObject = author.toObject();
			QStringList info;

			if(authorObject.value("sequence").toString().compare("first") == 0)
			{
				info.push_back("1");
			}
			else
			{
				info.push_back(QString::number(order++));
			}

			info.push_back(authorObject.value("given").toString());
			info.push_back(authorObject.value("family").toString());

			QTreeWidgetItem* item = new QTreeWidgetItem(info);
			authors->addTopLevelItem(item);
		}

		stack->setCurrentIndex(1);
	}
};

CDlgAddPublication::CDlgAddPublication(QWidget* parent) : QDialog(parent), ui(new Ui::CDlgAddPublication)
{
	ui->setup(this);
	setWindowTitle("Add Publication");

	restclient = new QNetworkAccessManager;
	connect(restclient, SIGNAL(finished(QNetworkReply*)), this, SLOT(connFinished(QNetworkReply*)));

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

void CDlgAddPublication::on_actionBack_triggered()
{
	ui->stack->setCurrentIndex(0);
}

void CDlgAddPublication::connFinished(QNetworkReply* r)
{
	QJsonDocument jsonDoc = QJsonDocument::fromJson(r->readAll());

	ui->SetInfoFromDOI(jsonDoc);
}









