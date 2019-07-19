#pragma once
#include <QDialog>
#include <vector>
using namespace std;

namespace Ui {
	class CMaterialEditor;
}

class QTreeWidgetItem;
class FEMaterial;
class GMaterial;

class CMaterialEditor : public QDialog
{
	Q_OBJECT

public:
	CMaterialEditor(QWidget* parent);

	void SetInitMaterial(GMaterial* mat);

	FEMaterial* GetMaterial();

	QString GetMaterialName() const;

	void SetModules(int module);

protected:
	void showEvent(QShowEvent* ev);
	void SetMaterial(FEMaterial* mat);

private slots:
	void on_tree_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);
	void on_matClass_currentIndexChanged(int n);
	void materialChanged(int);
	void accept();

private:
	Ui::CMaterialEditor*	ui;
};
