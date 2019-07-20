#pragma once
#include <QWidget>
#include <vector>
using namespace std;

class CMainWindow;
class FEObject;
class FEItemListBuilder;
class GMaterial;
class CPropertyList;
class QLineEdit;
class QLabel;
class CColorButton;
class QComboBox;

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

	void showColor(bool b);

	void setNameReadOnly(bool b);

protected slots:
	void on_name_textEdited(const QString&);
	void on_col_colorChanged(QColor c);

signals:
	void nameChanged(const QString& newName);
	void colorChanged(const QColor& c);

private:
	QLineEdit*		m_name;
	QLabel*			m_type;
	CColorButton*	m_col;
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

protected slots:
	void on_name_textEdited(const QString&);
	void on_list_currentIndexChanged(int n);

signals:
	void nameChanged(const QString& newName);
	void stepChanged(int n);

private:
	QLineEdit*		m_name;
	QLabel*			m_type;
	QComboBox*		m_list;
};

class CModelPropsPanel : public QWidget
{
	Q_OBJECT

public:
	CModelPropsPanel(CMainWindow* wnd, QWidget* parent = 0);

	void SetObjectProps(FEObject* po, CPropertyList* props, int flags);

	FEObject* GetCurrentObject() { return m_currentObject; }

	void Update();

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
	void on_props_dataChanged();
	void on_form_dataChanged(bool itemModified);
	void on_bcobject_stepChanged(int n);
	void on_post_dataChanged();

private:
	void SetSelection(int n, FEItemListBuilder* it);
	void SetSelection(GMaterial* pmat);

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
	FEObject*			m_currentObject;
	bool				m_isUpdating;
};
