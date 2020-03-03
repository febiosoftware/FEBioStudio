#pragma once
#include <QDialog>
#include <vector>
#include "LaunchConfig.h"

namespace Ui {
	class CDlgAddPublication;
}

class QStringList;
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
	void publicationChosen(CPublicationWidget* pub);
	void manualButtonClicked();
	void backButtonClicked();
	void connFinished(QNetworkReply* r);

private:
	Ui::CDlgAddPublication*	ui;
	QNetworkAccessManager* restclient;
};
