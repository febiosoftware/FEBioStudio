#pragma once
#include <QDialog>
#include <vector>
#include <PreViewLib/FEObject.h>

class CDlgAddMeshDataUI;
class FEObject;

class CDlgAddMeshData : public QDialog
{
	Q_OBJECT

public:
	CDlgAddMeshData(FEObject* po, QWidget* parent);

public slots:
	void onCustom();
	void setItem(int i);

	std::string GetMapName();
	std::string GetParamName();
	Param_Type GetParamType();

private:
	CDlgAddMeshDataUI*	ui;
};
