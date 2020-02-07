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


class CDlgAddPublication : public QDialog
{
	Q_OBJECT

public:
	CDlgAddPublication(QWidget* parent);

public slots:
	void on_DOILookup_triggered();
	void on_actionBack_triggered();
	void connFinished(QNetworkReply* r);

private:
	Ui::CDlgAddPublication*	ui;
	QNetworkAccessManager* restclient;
};
