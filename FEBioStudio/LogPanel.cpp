#include "stdafx.h"
#include "ui_logpanel.h"
#include <QFileDialog>
#include <QMessageBox>

CLogPanel::CLogPanel(QWidget* parent) : QWidget(parent), ui(new Ui::CLogPanel)
{
	ui->setupUi(this);
}

void CLogPanel::ClearLog()
{
	ui->clearLog(0);
}

void CLogPanel::ClearOutput()
{
	ui->clearLog(1);
}

void CLogPanel::AddText(const QString& txt, int n)
{
	ui->txt[n]->moveCursor(QTextCursor::End);
	ui->txt[n]->insertPlainText(txt);
	ui->txt[n]->moveCursor(QTextCursor::End);
}

void CLogPanel::on_logSave_clicked(bool b)
{
	QString txt = ui->currentTxt()->toPlainText();

	QString fileName = QFileDialog::getSaveFileName(this, "Save", "", "Text Files (*.txt)");
	if (fileName.isEmpty() == false)
	{
		// convert to const char*
		std::string sfile = fileName.toStdString();
		const char* szfile = sfile.c_str();

		// open the file
		FILE* fp = fopen(szfile, "wb");
		if (fp == 0)
		{
			QMessageBox::critical(this, "FEBio Studio", "Failed saving log");
			return;
		}

		// convert data to string
		std::string s = txt.toStdString();
		size_t len = s.length();
		size_t nwritten = fwrite(s.c_str(), sizeof(char), len, fp);

		// close the file
		fclose(fp);
	}
}

void CLogPanel::on_logClear_clicked(bool b)
{
	ui->currentTxt()->clear();
}

void CLogPanel::on_combo_currentIndexChanged(int i)
{
	ui->setOutput(i);
}

void CLogPanel::ShowOutput()
{
	ui->showTxt(1);
}
