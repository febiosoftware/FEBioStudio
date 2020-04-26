#pragma once
#include "CommandPanel.h"
#include <QDialog>
#include <string>
#include <vector>

class CMainWindow;
class QModelIndex;
class CPostDoc;

namespace Post {
	class FEDataField;
}

namespace Ui {
	class CPostDataPanel;
	class CDlgAddDataFile;
	class CDlgFilter;
}

class CPostDataPanel : public CCommandPanel
{
	Q_OBJECT

public:
	CPostDataPanel(CMainWindow* pwnd, QWidget* parent = 0);

	void Update(bool breset) override;

	private slots:
	void on_AddStandard_triggered();
	void on_AddFromFile_triggered();
	void on_AddEquation_triggered();
	void on_AddFilter_triggered();
	void on_CopyButton_clicked();
	void on_DeleteButton_clicked();
	void on_ExportButton_clicked();
	void on_dataList_clicked(const QModelIndex&);
	void on_fieldName_editingFinished();
	void on_props_dataChanged(bool b);

private:
	CPostDoc* GetActiveDocument();

private:
	Ui::CPostDataPanel* ui;
};

class CDlgAddDataFile : public QDialog
{
	Q_OBJECT

public:
	CDlgAddDataFile(QWidget* parent);

	void accept();

	private slots:
	void onBrowse();

public:
	int	m_nclass;
	int	m_ntype;
	std::string	m_file;
	std::string m_name;

private:
	Ui::CDlgAddDataFile* ui;
};

class CDlgFilter : public QDialog
{
public:
	CDlgFilter(QWidget* parent);

	void setDataOperands(const std::vector<QString>& opNames);
	void setDataField(Post::FEDataField* pdf);

	int getArrayComponent();

	void accept();

	void setDefaultName(const QString& name);

	QString getNewName();

public:
	int	m_nflt;

	double	m_scale;

	double	m_theta;
	int		m_iters;

	int		m_nop;
	int		m_ndata;

private:
	Ui::CDlgFilter* ui;
};
