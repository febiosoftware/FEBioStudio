#include "stdafx.h"
#include "DlgCheck.h"
#include <QBoxLayout>
#include <QListWidget>
#include <QDialogButtonBox>
#include <QLabel>

class Ui::CDlgCheck
{
public:
	QListWidget* errList;

public:
	void setup(QDialog* dlg)
	{
		QVBoxLayout* l = new QVBoxLayout;

		errList = new QListWidget;

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Yes | QDialogButtonBox::No);

		l->addWidget(new QLabel("There are issues with this model that may prevent it from runnnig correctly in FEBio:"));
		l->addWidget(errList);
		l->addWidget(new QLabel("Do you wish to continue?"));
		l->addWidget(bb);
			
		dlg->setLayout(l);

		QObject::connect(bb, SIGNAL(accepted()), dlg, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), dlg, SLOT(reject()));
	}
};

CDlgCheck::CDlgCheck(QWidget* parent): QDialog(parent), ui(new Ui::CDlgCheck)
{
	ui->setup(this);
}

void CDlgCheck::SetWarnings(const std::vector<MODEL_ERROR>& errList)
{
	ui->errList->clear();

	for (size_t i = 0; i < errList.size(); ++i)
	{
		QListWidgetItem* item = new QListWidgetItem;
		item->setText(QString::fromStdString(errList[i].second));
		if (errList[i].first == CRITICAL)
			item->setIcon(QIcon(":/icons/error.png"));
		else
			item->setIcon(QIcon(":/icons/warning.png"));

		ui->errList->addItem(item);
	}
}
