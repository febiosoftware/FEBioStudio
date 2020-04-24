#include "stdafx.h"
#include "DlgMeasure.h"
#include "MainWindow.h"
#include <QLabel>
#include <QBoxLayout>
#include <QGroupBox>
#include <QListWidget>
#include <QLineEdit>
#include <GLLib/GDecoration.h>
#include "Document.h"

typedef QVariant(*MEASURE_FUNC)(FESelection*, bool&, GDecoration** deco);

QVariant node_position(FESelection*, bool&, GDecoration**);
QVariant two_point_distance(FESelection*, bool&, GDecoration**);
QVariant three_point_angle(FESelection*, bool&, GDecoration**);
QVariant four_point_angle(FESelection*, bool&, GDecoration**);
QVariant face_area(FESelection*, bool&, GDecoration**);
QVariant elem_volume(FESelection*, bool&, GDecoration**);

class Ui::CDlgMeasure
{
public:
	::CMainWindow*	m_wnd;

	QLabel*		m_sel;
	QListWidget*			m_list;
	vector<MEASURE_FUNC>	m_measures;
	QLineEdit*	m_result;

	QRect	m_rt;

	GDecoration*	m_deco;

	int		m_lastType;
	int		m_lastSize;

public:
	void setup(QDialog* dlg)
	{
		QGroupBox* pg = nullptr;
		QVBoxLayout* l = new QVBoxLayout;

		m_deco = nullptr;

		// Selection pane
		l->addWidget(pg = new QGroupBox("Selection"));
		m_sel = new QLabel;
		QVBoxLayout* l1 = new QVBoxLayout;
		l1->addWidget(m_sel);
		pg->setLayout(l1);

		// Measures pane
		l->addWidget(pg = new QGroupBox("Measures"));
		m_list = new QListWidget;
		QVBoxLayout* l2 = new QVBoxLayout;
		l2->addWidget(m_list);
		pg->setLayout(l2);

		// Results pane
		l->addWidget(pg = new QGroupBox("Result"));
		m_result = new QLineEdit;
		m_result->setReadOnly(true);
		QVBoxLayout* l3 = new QVBoxLayout;
		l3->addWidget(m_result);
		pg->setLayout(l3);

		dlg->setLayout(l);

		QObject::connect(m_list, SIGNAL(currentRowChanged(int)), dlg, SLOT(onMeasureChanged(int)));
	}

	void setDecoration(GDecoration* deco)
	{
		CGLView* view = m_wnd->GetGLView();
		if (m_deco) 
		{
			view->RemoveDecoration(m_deco); 
			delete m_deco;
		}
		m_deco = deco;
		if (m_deco) view->AddDecoration(m_deco);
	}

	void setResult(const QString& t)
	{
		m_result->setText(t);
	}

	void setResult(double a)
	{
		m_result->setText(QString::number(a));
	}

	void clear()
	{
		m_lastType = -1;
		m_lastSize = -1;
		m_list->clear();
		m_sel->setText("(none)");
		m_result->setText("");
		m_measures.clear();
		setDecoration(nullptr);
	}

	void setSelectionText(const QString& t, int count)
	{
		m_sel->setText(t + QString(" (%1)").arg(count));
	}

	void setFENodeSelection(FENodeSelection* sel)
	{
		clear();
		int N = sel->Size();
		setSelectionText("Nodes", N);

		m_list->addItem((N == 1 ? "Position" : "Average position")); m_measures.push_back(node_position);

		if (N == 2)
		{
			m_list->addItem("Distance"); m_measures.push_back(two_point_distance);
		}

		if (N == 3)
		{
			m_list->addItem("Three point angle (degrees)"); m_measures.push_back(three_point_angle);
		}

		if (N == 4)
		{
			m_list->addItem("Four point angle (degrees)"); m_measures.push_back(four_point_angle);
		}
	}

	void setFEEdgeSelection(FEEdgeSelection* sel)
	{
		setSelectionText("Edges", sel->Size());
	}

	void setFEFaceSelection(FEFaceSelection* sel)
	{
		clear();
		int N = sel->Size();
		setSelectionText("Faces", sel->Size());

		m_list->addItem("Area"); m_measures.push_back(face_area);
	}

	void setFEElementSelection(FEElementSelection* sel)
	{
		setSelectionText("Elements", sel->Size());

		m_list->addItem("Volume"); m_measures.push_back(elem_volume);
	}
};

CDlgMeasure::CDlgMeasure(CMainWindow* wnd) : QDialog(wnd, Qt::Tool), ui(new Ui::CDlgMeasure)
{
	setWindowTitle("Measure");
	ui->m_wnd = wnd;
	ui->setup(this);
}

void CDlgMeasure::showEvent(QShowEvent* ev)
{
	Update();

	if (ui->m_rt.isValid()) setGeometry(ui->m_rt);
}

void CDlgMeasure::closeEvent(QCloseEvent* ev)
{
	ui->setDecoration(nullptr);
	ui->m_rt = geometry();
}

void CDlgMeasure::Update()
{
	CDocument* doc = ui->m_wnd->GetDocument();
	FESelection* sel = doc->GetCurrentSelection();
	if ((sel == nullptr) || (sel->Size() == 0))
	{
		ui->clear();
		return;
	}

	// see if we really need to update
	if ((sel->Type() != ui->m_lastType) || (sel->Size() != ui->m_lastSize))
	{
		switch (sel->Type())
		{
		case SELECT_FE_NODES: ui->setFENodeSelection(dynamic_cast<FENodeSelection   *>(sel)); break;
		case SELECT_FE_EDGES: ui->setFEEdgeSelection(dynamic_cast<FEEdgeSelection   *>(sel)); break;
		case SELECT_FE_FACES: ui->setFEFaceSelection(dynamic_cast<FEFaceSelection   *>(sel)); break;
		case SELECT_FE_ELEMENTS: ui->setFEElementSelection(dynamic_cast<FEElementSelection*>(sel)); break;
		default:
			ui->clear();
			return;
		}

		ui->m_lastSize = sel->Size();
		ui->m_lastType = sel->Type();
	}
	else
	{
		// just update the result
		onMeasureChanged(ui->m_list->currentRow());
	}
}

void CDlgMeasure::onMeasureChanged(int n)
{
	if ((n < 0) || (n >= ui->m_measures.size()))
	{
		ui->setResult("");
		return;
	}

	CDocument* doc = ui->m_wnd->GetDocument();
	FESelection* sel = doc->GetCurrentSelection();
	if ((sel == nullptr) || (sel->Size() == 0))
	{
		ui->setResult("");
		return;
	}

	ui->setDecoration(nullptr);

	MEASURE_FUNC measure = ui->m_measures[n];
	bool berr = false;
	GDecoration* deco = nullptr;
	QVariant value = measure(sel, berr, &deco);

	if (berr)
	{
		ui->setResult("(ERROR)");
		if (deco) delete deco;
	}
	else
	{
		ui->setResult(value.toString());
		ui->setDecoration(deco);
	}
}

//====================================================================================

QVariant node_position(FESelection* sel, bool& berr, GDecoration** deco)
{
	berr = false;

	FENodeSelection* nsel = dynamic_cast<FENodeSelection*>(sel);
	if (nsel == nullptr) { berr = true; return 0.0; }

	FELineMesh* mesh = nsel->GetMesh();

	vec3d r(0, 0, 0);
	int N = nsel->Size();
	FENodeSelection::Iterator it = nsel->First();
	for (int i = 0; i < N; ++i, ++it)
	{
		r += mesh->LocalToGlobal(it->pos());
	}
	if (N != 0) r /= (double)N;

	*deco = new GPointDecoration(to_vec3f(r));

	return QString("%1,%2,%3").arg(r.x).arg(r.y).arg(r.z);
}

QVariant two_point_distance(FESelection* sel, bool& berr, GDecoration** deco)
{
	berr = false;

	FENodeSelection* nsel = dynamic_cast<FENodeSelection*>(sel);
	if (nsel == nullptr) { berr = true; return 0.0; }

	int N = nsel->Size();
	if (N != 2) { berr = true; return 0.0; }

	FENodeSelection::Iterator it = nsel->First();
	vec3d r0 = it->pos(); ++it;
	vec3d r1 = it->pos();

	*deco = new GLineDecoration(to_vec3f(r0), to_vec3f(r1));

	return (r1 - r0).Length();
}

QVariant three_point_angle(FESelection* sel, bool& berr, GDecoration** deco)
{
	berr = false;

	FENodeSelection* nsel = dynamic_cast<FENodeSelection*>(sel);
	if (nsel == nullptr) { berr = true; return 0.0; }

	int N = nsel->Size();
	if (N != 3) { berr = true; return 0.0; }

	FENodeSelection::Iterator it = nsel->First();

	vec3d a = it->pos(); ++it;
	vec3d b = it->pos(); ++it;
	vec3d c = it->pos();

	vec3d e1 = a - b; e1.Normalize();
	vec3d e2 = c - b; e2.Normalize();

	double angle = 180.0*acos(e1*e2) / PI;

	GCompositeDecoration* cd = new GCompositeDecoration;
	cd->AddDecoration(new GLineDecoration(to_vec3f(b), to_vec3f(a)));
	cd->AddDecoration(new GLineDecoration(to_vec3f(b), to_vec3f(c)));
	cd->AddDecoration(new GArcDecoration(b, a, c));
	*deco = cd;

	return angle;
}

QVariant four_point_angle(FESelection* sel, bool& berr, GDecoration** deco)
{
	berr = false;

	FENodeSelection* nsel = dynamic_cast<FENodeSelection*>(sel);
	if (nsel == nullptr) { berr = true; return 0.0; }

	int N = nsel->Size();
	if (N != 4) { berr = true; return 0.0; }

	FENodeSelection::Iterator it = nsel->First();

	vec3d a = it->pos(); ++it;
	vec3d b = it->pos(); ++it;
	vec3d c = it->pos(); ++it;
	vec3d d = it->pos();

	vec3d e1 = b - a; e1.Normalize();
	vec3d e2 = d - c; e2.Normalize();

	double angle = 180.0*acos(e1*e2) / PI;

	GCompositeDecoration* cd = new GCompositeDecoration;
	cd->AddDecoration(new GLineDecoration(to_vec3f(a), to_vec3f(b)));
	cd->AddDecoration(new GLineDecoration(to_vec3f(c), to_vec3f(d)));
	*deco = cd;

	return angle;
}

QVariant face_area(FESelection* sel, bool& berr, GDecoration** deco)
{
	berr = false;

	FEFaceSelection* fsel = dynamic_cast<FEFaceSelection*>(sel);
	if (fsel == nullptr) { berr = true; return 0.0; }

	int N = fsel->Size();

	FEMeshBase* mesh = fsel->GetMesh();
	FEFaceSelection::Iterator it(mesh);

	double area = 0.0;
	vector<vec3d> rt;
	for (int i = 0; i<N; ++i, ++it)
	{
		FEFace& f = *it;
		area += mesh->FaceArea(f);
	}

	return area;
}

QVariant elem_volume(FESelection* sel, bool& berr, GDecoration** deco)
{
	berr = false;

	FEElementSelection* esel = dynamic_cast<FEElementSelection*>(sel);
	if (esel == nullptr) { berr = true; return 0.0; }

	int N = esel->Size();

	FEMesh* mesh = esel->GetMesh();
	FEElementSelection::Iterator it(mesh);

	double vol = 0.0;
	vector<vec3d> rt;
	for (int i = 0; i<N; ++i, ++it)
	{
		FEElement_& el = *it;
		vol += mesh->ElementVolume(el);
	}

	return vol;
}
