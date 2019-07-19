#include "stdafx.h"
#include "DlgModelInfo.h"
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QPlainTextEdit>
#include <QLabel>

class Ui::CDlgModelInfo
{
public:
	QPlainTextEdit*		edit;

public:
	void setup(QDialog* dlg)
	{
		edit = new QPlainTextEdit;

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Close);

		QVBoxLayout* l = new QVBoxLayout;
		l->addWidget(new QLabel("Notes:"));
		l->addWidget(edit);
		l->addWidget(bb);

		dlg->setLayout(l);

		QObject::connect(bb, SIGNAL(rejected()), dlg, SLOT(reject()));
	}
};


CDlgModelInfo::CDlgModelInfo(QWidget* parent) : QDialog(parent), ui(new Ui::CDlgModelInfo)
{
	ui->setup(this);
}

QString CDlgModelInfo::text()
{
	return ui->edit->toPlainText();
}

void CDlgModelInfo::setText(const QString& s)
{
	ui->edit->setPlainText(s);
}
