#include "stdafx.h"
#include "DragBox.h"
#include <QMouseEvent>
#include <QLineEdit>

class MyLineEdit : public QLineEdit
{
public:
	MyLineEdit(QWidget* parent) : QLineEdit(parent)
	{
		m_step = 1.0;
	}

	void setSingleStep(double v) { m_step = v; }

	void mousePressEvent(QMouseEvent* ev)
	{
		m_p0 = ev->pos();
		QLineEdit::mousePressEvent(ev);
	}

	void mouseMoveEvent(QMouseEvent* ev) 
	{
		ev->accept();
		double val = text().toDouble();

		if (ev->buttons() & Qt::LeftButton)
		{
			const QValidator* v = validator();
			if (v == 0) return;

			QPoint p1 = ev->pos();
			if (p1.x() > m_p0.x())
				val += m_step;
			else if (p1.x() < m_p0.x())
				val -= m_step;
			else return;

			QString newText = QString("%1").arg(val);
			int npos = 0;
			if (v->validate(newText, npos) != QValidator::Invalid)
				setText(newText);
			m_p0 = p1;
		}
	}

private:
	QPoint	m_p0;
	double	m_step;
};

CDragBox::CDragBox(QWidget* parent) : QDoubleSpinBox(parent)
{
	setLineEdit(new MyLineEdit(this));
	setDecimals(4);
}

void CDragBox::SetSingleStep(double v)
{
	setSingleStep(v);
	MyLineEdit* edit = dynamic_cast<MyLineEdit*>(lineEdit());
	if (edit)
	{
		edit->setSingleStep(v);
	}
}
