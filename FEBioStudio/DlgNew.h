#pragma once
#include <QDialog>

namespace Ui {
	class CDlgNew;
}

class CMainWindow;

class CDlgNew : public QDialog
{
public:
	CDlgNew(CMainWindow* parent);

	void accept();

	int getTemplate();

	bool createNew();

	QString getRecentFileName();

public:
	Ui::CDlgNew*	ui;
};
