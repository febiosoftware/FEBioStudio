#pragma once
#include <QDialog>

class FEProject;

namespace Ui {
	class CDlgEditProject;
}

class CDlgEditProject : public QDialog
{
	Q_OBJECT

public:
	CDlgEditProject(FEProject& prj, QWidget* parent);

	void accept();

private slots:
	void onAllClicked();
	void onNoneClicked();

private:
	Ui::CDlgEditProject*	ui;
	FEProject&				m_prj;
};
