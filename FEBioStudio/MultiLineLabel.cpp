#include <QApplication>
#include <QPalette>
#include <QTextDocument>
#include <QResizeEvent>
#include "MultiLineLabel.h"

MultiLineLabel::MultiLineLabel(QWidget *parent)
	: QTextBrowser(parent)
{
	init();
}

MultiLineLabel::MultiLineLabel(const QString &text, QWidget *parent)
	: QTextBrowser(parent)
{
	init();

	setText(text);
}

void MultiLineLabel::setText(const QString &text)
{
	QTextBrowser::setText(text);

	QResizeEvent event(size(), size());
	resizeEvent(&event);
}

void MultiLineLabel::resizeEvent(QResizeEvent *event)
{
	document()->setTextWidth(event->size().width());

	setMinimumHeight(document()->size().height());
	setMaximumHeight(document()->size().height());
}

void MultiLineLabel::init()
{
	// Make this look like a QLabel
	setFrameStyle(QFrame::Plain|QFrame::NoFrame);
	QPalette qpalette = palette();
	qpalette.setColor(QPalette::Base, qApp->palette().color(QPalette::Window));
	setPalette(qpalette);

	// Make this only take up the necessary size
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

	// Allow hyperlinks to open in a browser
	setOpenExternalLinks(true);
	setTextInteractionFlags(Qt::LinksAccessibleByMouse);
}
