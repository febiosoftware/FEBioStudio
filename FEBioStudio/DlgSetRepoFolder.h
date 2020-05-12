#pragma once
#include <QDialog>

namespace Ui {
	class CDlgSetRepoFolder;
}

class CDlgSetRepoFolder : public QDialog
{
	Q_OBJECT

public:
	CDlgSetRepoFolder(QString defaultPath, QWidget* parent);

	QString GetRepoFolder();

	void accept() override;

public slots:
	void on_openFileDialog_triggered();


private:
	Ui::CDlgSetRepoFolder* ui;

};
