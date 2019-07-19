#include "stdafx.h"
#include "DlgBatchConvert.h"
#include <QComboBox>
#include <QFormLayout>
#include <QBoxLayout>
#include <QDialogButtonBox>

class Ui::CDlgBatchConvert
{
public:
	QComboBox*	formats;
	int		format;

public:
	void setup(QDialog* dlg)
	{
		format = 0;

		formats = new QComboBox;
		formats->addItem("VTK files (.vtk)");
		formats->addItem("PLY files (.ply)");
		formats->addItem("LSDYNA keyword (*.k)");
		formats->addItem("HyperSurface files (*.surf)");
		formats->addItem("BYU files (*.byu)");
		formats->addItem("STL-ASCII files (*.stl)");
		formats->addItem("ViewPoint files (*.vp)");
		formats->addItem("Mesh files (*.mesh)");
		formats->addItem("TetGen files (*.ele)");

		QFormLayout* form = new QFormLayout;
		form->addRow("Select output format:", formats);


		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

		QVBoxLayout* l = new QVBoxLayout;
		l->addLayout(form);
		l->addWidget(bb);

		dlg->setLayout(l);

		QObject::connect(bb, SIGNAL(accepted()), dlg, SLOT(accept()));
		QObject::connect(bb, SIGNAL(rejected()), dlg, SLOT(reject()));
	}
};

CDlgBatchConvert::CDlgBatchConvert(QWidget* parent) : QDialog(parent), ui(new Ui::CDlgBatchConvert)
{
	ui->setup(this);
}

void CDlgBatchConvert::accept()
{
	ui->format = ui->formats->currentIndex();
	QDialog::accept();
}

int CDlgBatchConvert::GetFileFormat()
{
	return ui->format;
}
