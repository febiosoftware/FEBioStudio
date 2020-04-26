#pragma once
#include "CommandPanel.h"
#include <vector>
#include <QtCore/QThread>
#include <QDialog>
using namespace std;

class CMainWindow;
class QToolButton;
class QGridLayout;
class QButtonGroup;
class QLineEdit;
class QPushButton;
class FEModifier;
class QAction;
class FEMesh;
class CCommand;
class GObject;
class QProgressBar;
class FEMesher;
class QLabel;

namespace Ui {
	class CMeshPanel;
};

class CMeshPanel : public CCommandPanel
{
	Q_OBJECT

public:
	CMeshPanel(CMainWindow* wnd, QWidget* parent = 0);

	// update mesh panel
	void Update(bool breset = true) override;

	void Apply() override;

private slots:
	void on_buttons_buttonSelected(int n);
	void on_buttons2_buttonSelected(int n);
	void on_apply_clicked(bool b);
	void on_apply2_clicked(bool b);
	void on_menu_triggered(QAction* pa);

private:
	int					m_nid;	// current button selected
	FEModifier*			m_mod;	// temporary modifier
	Ui::CMeshPanel*		ui;
};

class MeshingThread : public QThread
{
	Q_OBJECT

	void run() Q_DECL_OVERRIDE;

public:
	MeshingThread(GObject* po);

	double progress();

	const char* currentTask();

	void stop();

signals:
	void resultReady();

private:
	GObject*	m_po;
	FEMesher*	m_mesher;
};

class CDlgStartThread : public QDialog
{
	Q_OBJECT

public:
	CDlgStartThread(QWidget* parent, MeshingThread* thread);

	void accept();

private slots:
	void threadFinished();
	void checkProgress();
	void cancel();

private:
	MeshingThread*	m_thread;
	bool			m_bdone;

	QLabel*			m_task;
	QProgressBar*	m_progress;
	QPushButton*	m_stop;

	const char*		m_szcurrentTask;
};
