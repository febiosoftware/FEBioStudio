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

#include <QEvent>
#include <QAction>
#include <QScrollArea>
#include <QScrollBar>
#include <QLayout>
#include <QBoxLayout>
#include <QToolButton>
#include <QList>
#include <QVariantMap>
#include <QVariant>
#include "PublicationWidgetView.h"
#include "PublicationWidget.h"
#include "DlgAddPublication.h"


class VerticalScrollArea : public QScrollArea
{
public:
	VerticalScrollArea(QWidget *parent = 0) : QScrollArea(parent)
	{
		setWidgetResizable(true);
		setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	}

	bool eventFilter(QObject *o, QEvent *e){
		// This works because QScrollArea::setWidget installs an eventFilter on the widget
		if(o && o == widget() && e->type() == QEvent::Resize)
		{
			setMinimumWidth(widget()->minimumSizeHint().width() + verticalScrollBar()->width());

			return true;
		}

		return QScrollArea::eventFilter(o, e);
	}

};


class Ui::CPublicationWidgetView
{
public:
	QWidget* pubs;
	QVBoxLayout* pubsLayout;

	QAction* addPub;
	QAction* delPub;

public:
	void setup(::CPublicationWidgetView* parent, bool scroll, bool frame)
	{
		QHBoxLayout* layout = new QHBoxLayout;
        layout->setContentsMargins(0,0,0,0);

		pubs = new QWidget;
		pubs->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
		pubsLayout = new QVBoxLayout;
		pubsLayout->setContentsMargins(0, 0, 0, 0);
		pubs->setLayout(pubsLayout);

		if(scroll)
		{
			VerticalScrollArea* scrollArea = new VerticalScrollArea;

			if(!frame) scrollArea->setFrameStyle(QFrame::NoFrame);

			scrollArea->setWidget(pubs);

			layout->addWidget(scrollArea);
		}
		else
		{
			layout->addWidget(pubs);
		}

		if(parent->getType() == ::CPublicationWidgetView::EDITABLE)
		{
			QVBoxLayout* buttonLayout = new QVBoxLayout;
			buttonLayout->setAlignment(Qt::AlignTop);

			addPub = new QAction(parent);
			addPub->setIcon(QIcon(":/icons/selectAdd.png"));
			QObject::connect(addPub, &QAction::triggered, parent, &::CPublicationWidgetView::on_addPub_triggered);
			QToolButton* addPubBtn = new QToolButton;
			addPubBtn->setDefaultAction(addPub);
			buttonLayout->addWidget(addPubBtn);

			delPub = new QAction(parent);
			delPub->setIcon(QIcon(":/icons/selectSub.png"));
			QObject::connect(delPub, &QAction::triggered, parent, &::CPublicationWidgetView::on_delPub_triggered);
			QToolButton* delPubBtn = new QToolButton;
			delPubBtn->setDefaultAction(delPub);
			buttonLayout->addWidget(delPubBtn);

			layout->addLayout(buttonLayout);
		}

		parent->setLayout(layout);
	}

	void addPublication(::CPublicationWidget* pub)
	{
		pubsLayout->addWidget(pub);
	}

};



CPublicationWidgetView::CPublicationWidgetView(Type type, bool scroll, bool frame)
	: type(type), ui(new Ui::CPublicationWidgetView)
{
	ui->setup(this, scroll, frame);
}

int CPublicationWidgetView::getType() const {
	return type;
}

CPublicationWidget* CPublicationWidgetView::addPublication(CPublicationWidget* pub)
{
	pubs.push_back(pub);
	ui->addPublication(pub);

	QObject::connect(pub, &CPublicationWidget::chosen_publication, this, &CPublicationWidgetView::chosen_publication);

	return pub;
}

CPublicationWidget* CPublicationWidgetView::addPublication(QVariantMap &data)
{
	CPublicationWidget* pub;

	switch(type)
	{
	case LIST:
		pub = new CPublicationWidget(data);
		break;
	case EDITABLE:
		pub = new CPublicationWidget(data, CPublicationWidget::CHECKBOX, false);
		break;
	case SELECTABLE:
		pub = new CPublicationWidget(data, CPublicationWidget::BUTTON, false);
		break;
	}

	pubs.push_back(pub);
	ui->addPublication(pub);

	QObject::connect(pub, &CPublicationWidget::chosen_publication, this, &CPublicationWidgetView::chosen_publication);

	return pub;
}

CPublicationWidget* CPublicationWidgetView::addPublication(QString title, QString year,
		QString journal, QString volume, QString issue, QString pages,
		QString DOI, QStringList authorGiven, QStringList authorFamily)
{

	CPublicationWidget* pub;

	switch(type)
	{
	case LIST:
		pub = new CPublicationWidget(title, year, journal, volume, issue, pages,
				DOI, authorGiven, authorFamily);
		break;
	case EDITABLE:
		pub = new CPublicationWidget(title, year, journal, volume, issue, pages,
				DOI, authorGiven, authorFamily, CPublicationWidget::CHECKBOX, false);
		break;
	case SELECTABLE:
		pub = new CPublicationWidget(title, year, journal, volume, issue, pages,
				DOI, authorGiven, authorFamily, CPublicationWidget::BUTTON, false);
		break;
	}

	pubs.push_back(pub);
	ui->addPublication(pub);

	QObject::connect(pub, &CPublicationWidget::chosen_publication, this, &CPublicationWidgetView::chosen_publication);

	return pub;
}

CPublicationWidget* CPublicationWidgetView::addPublicationCopy(CPublicationWidget& oldPub)
{
	CPublicationWidget* pub = new CPublicationWidget(oldPub);

	pubs.push_back(pub);
	ui->addPublication(pub);

	QObject::connect(pub, &CPublicationWidget::chosen_publication, this, &CPublicationWidgetView::chosen_publication);

	return pub;
}

void CPublicationWidgetView::on_addPub_triggered()
{
	CDlgAddPublication dlg(this);

	if(dlg.exec())
	{
		QString title = dlg.getTitle();
		QString year = dlg.getYear();
		QString journal = dlg.getJournal();
		QString volume = dlg.getVolume();
		QString issue = dlg.getIssue();
		QString pages = dlg.getPages();
		QString DOI = dlg.getDOI();
		QStringList authorGiven = dlg.getAuthorGiven();
		QStringList authorFamily= dlg.getAuthorFamily();

		addPublication(title, year, journal, volume, issue, pages,
					DOI, authorGiven, authorFamily);
	}
}

void CPublicationWidgetView::on_delPub_triggered()
{
	auto it = pubs.begin();

	while (it != pubs.end())
	{
		// If the publication is checked, remove it from the ui, and
		// then remove it from the vector
	    if ((*it)->isChecked())
	    {
	    	delete (*it);
	    	it = pubs.erase(it);
	    }
	    else
	    {
	        it++;
	    }
	}

}

void CPublicationWidgetView::clear()
{
	for(auto pub : pubs)
	{
		delete pub;
	}

	pubs.clear();
}

int CPublicationWidgetView::count()
{
	return pubs.size();
}

QList<QVariant> CPublicationWidgetView::getPublicationInfo()
{
	QList<QVariant> pubInfo;

	for(auto pub : pubs)
	{
		QVariantMap publication;

		publication["title"] = pub->getTitle();
		publication["year"] = pub->getYear();
		publication["journal"] = pub->getJournal();
		publication["volume"] = pub->getVolume();
		publication["issue"] = pub->getIssue();
		publication["pages"] = pub->getPages();
		publication["DOI"] = pub->getDOI();

		QList<QVariant> authors;

		int size = pub->getAuthorGiven().size();
		for(int name = 0; name < size; name++)
		{
			QVariantMap names;
			names["given"] = pub->getAuthorGiven()[name];
			names["family"] = pub->getAuthorFamily()[name];

			authors.push_back(names);
		}

		publication["authors"] = authors;

		pubInfo.push_back(publication);

	}

	return pubInfo;
}

const std::vector<CPublicationWidget*>& CPublicationWidgetView::getPublications()
{
	return pubs;
}

