#pragma once
#include <QWidget>
#include <vector>
using namespace std;

class CMainWindow;
class FSObject;
class FEItemListBuilder;
class GMaterial;
class CPropertyList;
class QLineEdit;
class QLabel;
class CColorButton;
class QComboBox;
class QCheckBox;
class GDiscreteElementSet;

namespace Ui {
	class CModelPropsPanel;
}

class CObjectPropsPanel : public QWidget
{
	Q_OBJECT

public:
	CObjectPropsPanel(QWidget* parent = 0);

	void setName(const QString& name);

	void setType(const QString& name);

	void setColor(const QColor& col);

	void setStatus(bool b);

	void showColor(bool b);

	void showStatus(bool b);

	void setNameReadOnly(bool b);

protected slots:
	void on_name_textEdited(const QString&);
	void on_col_colorChanged(QColor c);
	void on_status_clicked(bool b);

signals:
	void nameChanged(const QString& newName);
	void colorChanged(const QColor& c);
	void statusChanged(bool b);

private:
	QLineEdit*		m_name;
	QLabel*			m_type;
	CColorButton*	m_col;
	QCheckBox*		m_status;
};

class CBCObjectPropsPanel : public QWidget
{
	Q_OBJECT

public:
	CBCObjectPropsPanel(QWidget* parent = 0);

	void setName(const QString& name);

	void setType(const QString& name);

	void setStepValues(const vector<pair<QString, int> >& l);

	void setStepID(int n);

	int currentStepID();

	void showActiveState(bool b);

	void setActiveState(bool b);

protected slots:
	void on_name_textEdited(const QString&);
	void on_list_currentIndexChanged(int n);
	void on_state_toggled(bool b);

signals:
	void nameChanged(const QString& newName);
	void stepChanged(int n);
	void stateChanged(bool isActive);

private:
	QLineEdit*		m_name;
	QLabel*			m_type;
	QComboBox*		m_list;
	QCheckBox*		m_state;
};

class CModelPropsPanel : public QWidget
{
	Q_OBJECT

public:
	CModelPropsPanel(CMainWindow* wnd, QWidget* parent = 0);

	void SetObjectProps(FSObject* po, CPropertyList* props, int flags);

	FSObject* GetCurrentObject() { return m_currentObject; }

	void Update();

	void Refresh();

private slots:
	void on_select1_addButtonClicked();
	void on_select1_subButtonClicked();
	void on_select1_delButtonClicked();
	void on_select1_selButtonClicked();
	void on_select2_addButtonClicked();
	void on_select2_subButtonClicked();
	void on_select2_delButtonClicked();
	void on_select2_selButtonClicked();
	void on_select1_nameChanged(const QString& t);
	void on_select2_nameChanged(const QString& t);
	void on_object_nameChanged(const QString&);
	void on_bcobject_nameChanged(const QString&);
	void on_object_colorChanged(const QColor& col);
	void on_props_dataChanged(int n);
	void on_form_dataChanged(bool itemModified);
	void on_bcobject_stepChanged(int n);
	void on_bcobject_stateChanged(bool isActive);
	void on_object_statusChanged(bool b);

private:
	void SetSelection(int n, FEItemListBuilder* it);
	void SetSelection(GMaterial* pmat);
	void SetSelection(GDiscreteElementSet* set);

	void addSelection(int n);
	void subSelection(int n);
	void delSelection(int n);
	void selSelection(int n);

signals:
	void nameChanged(const QString& txt);
	void selectionChanged();
	void dataChanged(bool b);

private:
	Ui::CModelPropsPanel* ui;
	CMainWindow*		m_wnd;
	FSObject*			m_currentObject;
	bool				m_isUpdating;
};
