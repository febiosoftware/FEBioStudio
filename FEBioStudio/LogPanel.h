#pragma once
#include <QWidget>

namespace Ui {
	class CLogPanel;
};

class CLogPanel : public QWidget
{
	Q_OBJECT

public:
	CLogPanel(QWidget* parent = 0);

	void ClearLog();

	void ClearOutput();

	void ShowOutput();

	void AddText(const QString& txt, int n = 0);

private slots:
	void on_logSave_clicked(bool b);
	void on_logClear_clicked(bool b);
	void on_combo_currentIndexChanged(int i);

private:
	Ui::CLogPanel*	ui;
};
