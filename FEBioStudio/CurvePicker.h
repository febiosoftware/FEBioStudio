#pragma once
#include <QWidget>

class GItem;

namespace Ui {
	class CCurvePicker;
}

//-----------------------------------------------------------------------------
class CCurvePicker : public QWidget
{
	Q_OBJECT

public:
	CCurvePicker(QWidget* parent = 0);

	void setCurve(const QString& curveName);

	QString curveName() const;

	private slots:
	void nameChanged();
	void buttonToggled(bool bchecked);
	void itemPicked(GItem* item);

signals:
	void curveChanged();

private:
	Ui::CCurvePicker*	ui;
};

//-----------------------------------------------------------------------------

namespace Ui {
	class CCurveListPicker;
}

class CCurveListPicker : public QWidget
{
	Q_OBJECT

public:
	CCurveListPicker(QWidget* parent = 0);
	~CCurveListPicker();

	void setCurves(const QStringList& curves);

	QStringList curveNames() const;

private slots:
	void onAddButtonClicked();
	void onSubButtonClicked();
	void onDelButtonClicked();
	void onSelButtonClicked();

signals:
	void curveChanged();

private:
	Ui::CCurveListPicker*	ui;
};
