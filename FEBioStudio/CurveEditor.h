#pragma once
#include <QMainWindow>
#include "CommandManager.h"
#include "Command.h"
#include "PlotWidget.h"
#include <FSCore/LoadCurve.h>
#include <FSCore/ParamBlock.h>

class CMainWindow;
class FELoadCurve;
class FEMaterial;
class QTreeWidgetItem;
class CCurveEditorItem;
class FSObject;

namespace Ui {
	class CCurveEdior;
}

class CCurveEditor : public QMainWindow
{
	Q_OBJECT

	enum { FLT_ALL, FLT_GEO, FLT_MAT, FLT_BC, FLT_LOAD, FLT_CONTACT, FLT_CONSTRAINT, FLT_CONNECTOR, FLT_DISCRETE, FLT_STEP, FLT_LOAD_CURVES };

public:
	CCurveEditor(CMainWindow* wnd);

	void Update();

	void closeEvent(QCloseEvent* ev);

	static QRect preferredSize();
	static void setPreferredSize(const QRect& rt);

private:
	bool Filter(int n) { if ((m_nflt == FLT_ALL) || (m_nflt == n)) return true; return false; }
	void AddMaterial(FEMaterial* pm, QTreeWidgetItem* tp);
	void AddMultiMaterial(FEMaterial* pm, QTreeWidgetItem* tp);
	void SetLoadCurve(FELoadCurve* plc);
	void UpdateLoadCurve();

private:
	void BuildLoadCurves();
	void BuildModelTree();
	void BuildLoadCurves(QTreeWidgetItem* t1, FSObject* po);
	void BuildMaterialCurves(QTreeWidgetItem* t1, FEMaterial* mat, const std::string& name);
	void UpdateSelection();

private slots:
	void on_tree_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* prev);
	void on_filter_currentIndexChanged(int n);
	void on_plot_pointClicked(QPointF p, bool shift);
	void on_plot_pointSelected(int n);
	void on_plot_draggingStart(QPoint p);
	void on_plot_pointDragged(QPoint p);
	void on_plot_draggingEnd(QPoint p);
	void on_plot_backgroundImageChanged();
	void on_plot_doneZoomToRect();
	void on_plot_regionSelected(QRect);
	void on_plot_doneSelectingRect(QRect);
	void on_open_triggered();
	void on_save_triggered();
	void on_clip_triggered();
	void on_copy_triggered();
	void on_paste_triggered();
	void on_delete_triggered();
	void on_xval_textEdited();
	void on_yval_textEdited();
	void on_deletePoint_clicked();
	void on_zoomToFit_clicked();
	void on_zoomX_clicked();
	void on_zoomY_clicked();
	void on_map_clicked();
	void on_undo_triggered();
	void on_redo_triggered();
	void on_math_triggered();
	void on_lineType_currentIndexChanged(int n);
	void on_extendMode_currentIndexChanged(int n);

private:
	Ui::CCurveEdior*	ui;
	CCurveEditorItem*	m_currentItem;
	CMainWindow*		m_wnd;
	FELoadCurve*		m_plc_copy;
	int					m_nflt;

	// undo stack
	CBasicCmdManager	m_cmd;

	static QRect	m_preferredSize;
};

class CCmdAddPoint : public CCommand
{
public:
	CCmdAddPoint(FELoadCurve* plc, LOADPOINT& p);

	void Execute() override;
	void UnExecute() override;

	int Index() { return m_index; }

private:
	FELoadCurve*	m_lc;
	LOADPOINT		m_pt;
	int				m_index;
};

class CCmdRemovePoint : public CCommand
{
public:
	CCmdRemovePoint(FELoadCurve* plc, const vector<int>& index);

	void Execute() override;
	void UnExecute() override;

private:
	FELoadCurve*	m_lc;
	FELoadCurve		m_copy;
	vector<int>		m_index;
};

class CCmdMovePoint: public CCommand
{
public:
	CCmdMovePoint(FELoadCurve* plc, int index, LOADPOINT to);

	void Execute() override;
	void UnExecute() override;

private:
	FELoadCurve*	m_lc;
	LOADPOINT		m_p;
	int				m_index;
};

class CCmdDeleteCurve : public CCommand
{
public:
	CCmdDeleteCurve(Param* pp);
	~CCmdDeleteCurve();
	void Execute() override;
	void UnExecute() override;
private:
	FELoadCurve*	m_plc;
	Param*			m_pp;
};

class CLoadCurveData : public CPlotData
{
public:
	CLoadCurveData(FELoadCurve* lc) : m_lc(lc) {}

	void draw(QPainter& painter, CPlotWidget& plt);

private:
	FELoadCurve*	m_lc;
};
