#include "stdafx.h"
#include "CColorButton.h"

CColorButton::CColorButton(QWidget* parent) : QWidget(parent)
{
	m_col = QColor(Qt::black);
	setMinimumSize(sizeHint());
}

void CColorButton::paintEvent(QPaintEvent* ev)
{
	QPainter p(this);
	p.fillRect(rect(), m_col);
	p.setPen(Qt::black);
	QRect rt = rect();
	rt.adjust(0,0,-1,-1);
	p.drawRect(rt);
}

void CColorButton::setColor(const QColor& c)
{ 
	m_col = c;
	repaint();
}

void CColorButton::mouseReleaseEvent(QMouseEvent* ev)
{
	QColorDialog dlg(this);
	QColor col = dlg.getColor(m_col);
	if (col.isValid())
	{
		m_col = col;
		repaint();
		emit colorChanged(m_col);
	}
}
