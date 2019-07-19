#pragma once
#include <vector>
#include <list>
#include <QDialog>
using namespace std;

class CMainWindow;
class QListWidget;
class QListWidgetItem;

class FEMaterial;
class FEReactionMaterial;
class FEModel;
class GMaterial;
class QAbstractButton;

class QSelectBox : public QWidget
{
	Q_OBJECT

public:
	QSelectBox(QWidget* parent = 0);

	void clear();

	void addSourceItem(const QString& item, int data, int flag = 0);
	void addTargetItem(const QString& item, int data, int flag = 0);

	list<int> targetData();

	void targetData(vector<int>& sols, vector<int>& smbs);

private slots:
	void on_toTarget_clicked();
	void on_toSource_clicked();

private:
	QListWidget*	source;
	QListWidget*	target;
};

class CReactionList : public QWidget
{
	Q_OBJECT

public:
	CReactionList(QWidget* parent = 0);

	void Clear();

	void AddItem(const QString& name, const QVariant& data, bool sel = false);

	void SetCurrentText(const QString& t);

signals:
	void currentIndexChanged(int n);
	void onAdd();
	void onRemove();

private slots:
	void onCurrentIndexChanged(int n);
	void OnAdd();
	void OnRemove();

private:
	QListWidget*	m_list;
};

namespace Ui {
	class CDlgAddChemicalReaction;
};

class CDlgAddChemicalReaction : public QDialog
{
	Q_OBJECT

public:
	CDlgAddChemicalReaction(CMainWindow* wnd);

private:
	void InitDialog();
	void apply();
	void accept();
	void SetMaterial(GMaterial* mat, FEModel& fem);
	void SetReaction(FEReactionMaterial* mat);
	bool hasChanged();

protected slots:
	void onMaterialChanged(int n);
	void onReactionChanged(int n);
	void onReactionType(int n);
	void onAddReaction();
	void onRemoveReaction();
	void onNameChanged(const QString& t);
	void onClicked(QAbstractButton* button);

private:
	GMaterial*			m_pmp;		// parent multiphasic material
	FEReactionMaterial*	m_reaction;	// active reaction

	string				m_name;		// name of chemical reaction
	bool				m_bovrd;	// override calculated Vbar
	vector<int>			m_solR;		// solute reactants
	vector<int>			m_sbmR;		// solid-bound molecule reactants
	vector<int>			m_solP;		// solute products
	vector<int>			m_sbmP;		// solid-bound molecule products

	int			m_reactionMat;		// reaction material
	int			m_fwdMat;			// forward rate material
	int			m_revMat;			// backward rate material
	bool        m_brr;				// flag for specification of reverse rate

private:
	CMainWindow*	m_wnd;
	Ui::CDlgAddChemicalReaction*	ui;
	vector<pair<string, int> >		m_species;
};
