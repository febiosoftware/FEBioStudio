#pragma once
#include <QDialog>

#define UNSELECTED_HELP "unselected_help"

class FEProject;
class QLayout;

namespace Ui {
	class CHelpDialog;
};

class CHelpDialog : public QDialog
{
	Q_OBJECT

public:
	CHelpDialog(FEProject& prj, QWidget* parent);
	virtual ~CHelpDialog();

public slots:
	void LoadPage();

protected slots:
	void on_help_clicked();

protected:
	void SetLeftSideLayout(QLayout* layout);

	virtual void SetURL() = 0;

protected:
	int		m_module;
	QString m_url;
	QString m_unselectedHelp;
	QSize m_withoutHelp;
	QSize m_withHelp;
private:
	Ui::CHelpDialog* ui;
};
