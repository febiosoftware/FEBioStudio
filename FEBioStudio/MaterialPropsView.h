#pragma once
#include <QTreeView>
#include <QStyledItemDelegate>

class GMaterial;

class CMaterialPropsModel;

class CMaterialPropsDelegate : public QStyledItemDelegate
{
	Q_OBJECT

public:
	explicit CMaterialPropsDelegate(QObject* parent = nullptr);

	QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;

	void setEditorData(QWidget* editor, const QModelIndex& index) const override;

	void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;

private slots:
	void OnEditorSignal();
};

class CMaterialPropsView : public QTreeView
{
	Q_OBJECT

public:
	CMaterialPropsView(QWidget* parent = nullptr);

	void SetMaterial(GMaterial* mat);

protected:
	void drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const override;

private slots:
	void onModelDataChanged();

private:
	CMaterialPropsModel*	m_model;
};
