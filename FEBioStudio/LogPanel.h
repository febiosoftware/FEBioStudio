#pragma once
#include <QWidget>
#include <QTextCharFormat>

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

	// Changed to allow for parsing of ANSI escape codes that might be
	// returned from an ssh session
	// Code taken from https://stackoverflow.com/questions/26500429/qtextedit-and-colored-bash-like-output-emulation
	void AddText(const QString& txt, int n = 0);

private slots:
	void on_logSave_clicked(bool b);
	void on_logClear_clicked(bool b);
	void on_combo_currentIndexChanged(int i);

private:
	// Code taken from https://stackoverflow.com/questions/26500429/qtextedit-and-colored-bash-like-output-emulation
	void parseEscapeSequence(int attribute, QListIterator< QString > & i, QTextCharFormat & textCharFormat, QTextCharFormat const & defaultTextCharFormat);

	Ui::CLogPanel*	ui;
};
