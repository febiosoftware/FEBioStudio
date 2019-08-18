#pragma once
#include <QDialog>
#include <vector>
#include <MeshTools/ParamBlock.h>

class CDlgAddMeshDataUI;
class CObject;

class CDlgAddMeshData : public QDialog
{
	Q_OBJECT

public:
	CDlgAddMeshData(CObject* po, QWidget* parent);

public slots:
	void onCustom();
	void setItem(int i);

	std::string GetMapName();
	std::string GetParamName();
	Param_Type GetParamType();

private:
	CDlgAddMeshDataUI*	ui;
};
