#include "stdafx.h"
#include "DlgPlaneCut.h"
#include "MainWindow.h"
#include "GLView.h"
#include <QBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QValidator>
#include "DragBox.h"

class UIDlgPlaneCut
{
public:
	CGLView*	m_view;
	CDragBox*	w[4];

public:
	void setup(QDialog* dlg)
	{
		QVBoxLayout* l = new QVBoxLayout;

		QFormLayout* f = new QFormLayout;
		f->addRow("X-normal:", w[0] = new CDragBox); w[0]->setRange(-1.0, 1.0); w[0]->SetSingleStep(0.01);
		f->addRow("Y-normal:", w[1] = new CDragBox); w[1]->setRange(-1.0, 1.0); w[1]->SetSingleStep(0.01);
		f->addRow("Z-normal:", w[2] = new CDragBox); w[2]->setRange(-1.0, 1.0); w[2]->SetSingleStep(0.01);
		f->addRow("offset:"  , w[3] = new CDragBox); w[3]->setRange(-1.0, 1.0); w[3]->SetSingleStep(0.01);

		w[0]->setValue(1.0);
		w[1]->setValue(0.0);
		w[2]->setValue(0.0);
		w[3]->setValue(0.0);

		l->addLayout(f);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Close);
		l->addWidget(bb);

		dlg->setLayout(l);

		QObject::connect(bb, SIGNAL(rejected()), dlg, SLOT(reject()));
		QObject::connect(w[0], SIGNAL(valueChanged(double)), dlg, SLOT(onDataChanged()));
		QObject::connect(w[1], SIGNAL(valueChanged(double)), dlg, SLOT(onDataChanged()));
		QObject::connect(w[2], SIGNAL(valueChanged(double)), dlg, SLOT(onDataChanged()));
		QObject::connect(w[3], SIGNAL(valueChanged(double)), dlg, SLOT(onDataChanged()));
	}
};

CDlgPlaneCut::CDlgPlaneCut(CMainWindow* wnd) : QDialog(wnd), ui(new UIDlgPlaneCut)
{
	setWindowTitle("Plane cut");
	ui->m_view = wnd->GetGLView();
	ui->setup(this);
}

CDlgPlaneCut::~CDlgPlaneCut()
{

}

void CDlgPlaneCut::Update()
{

}

void CDlgPlaneCut::showEvent(QShowEvent* ev)
{

	ui->m_view->ShowPlaneCut(true);
}

void CDlgPlaneCut::closeEvent(QCloseEvent* ev)
{
	ui->m_view->ShowPlaneCut(false);
}

void CDlgPlaneCut::reject()
{
	ui->m_view->ShowPlaneCut(false);
	QDialog::reject();
}

void CDlgPlaneCut::onDataChanged()
{
	double a[4] = { 0 };
	a[0] = ui->w[0]->value();
	a[1] = ui->w[1]->value();
	a[2] = ui->w[2]->value();
	a[3] = ui->w[3]->value();

	double L = a[0] * a[0] + a[1] * a[1] + a[2] * a[2];
	if (L != 0)
	{
		a[0] /= L;
		a[1] /= L;
		a[2] /= L;
	}
	else a[0] = 1.0;

	ui->m_view->SetPlaneCut(a);
}
