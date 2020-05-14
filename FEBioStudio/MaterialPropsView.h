#pragma once
#include <QTreeView>

class GMaterial;

class CMaterialPropsModel;

class CMaterialPropsView : public QTreeView
{
	Q_OBJECT

public:
	CMaterialPropsView(QWidget* parent = nullptr);

	void SetMaterial(GMaterial* mat);

protected:
	void drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const override;

private:
	CMaterialPropsModel*	m_model;
};
