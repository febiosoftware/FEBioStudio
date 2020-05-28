#pragma once

#include <vector>
#include <HelpDialog.h>

using namespace std;

namespace Ui {
	class CMaterialEditor;
}

class QTreeWidgetItem;
class FEProject;
class FEMaterial;
class GMaterial;

class CMaterialEditor : public CHelpDialog
{
	Q_OBJECT

public:
	CMaterialEditor(FEProject& prj, QWidget* parent);

	void SetInitMaterial(GMaterial* mat);

	FEMaterial* GetMaterial();

	QString GetMaterialName() const;

protected:
	void showEvent(QShowEvent* ev);
	void SetMaterial(FEMaterial* mat);
	void SetURL();

private slots:
	void on_tree_currentItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);
	void on_matClass_currentIndexChanged(int n);
	void materialChanged(int);
	void accept();

private:
	Ui::CMaterialEditor*	ui;
};
