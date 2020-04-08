#pragma once
#include <QDialog>

namespace Ui {
	class CDlgExportFEBio;
}

class CDlgExportFEBio : public QDialog
{
	Q_OBJECT

private:
	enum { MAX_SECTIONS = 15 };

public:
	CDlgExportFEBio(QWidget* parent);

	void accept();

public:
	static int		m_nversion;
	bool	m_nsection[MAX_SECTIONS];
	bool	m_bexportSelections;
	bool	m_compress;
	bool	m_writeNotes;

private slots:
	void OnAllClicked();
	void OnNoneClicked();

private:
	Ui::CDlgExportFEBio*	ui;
};
