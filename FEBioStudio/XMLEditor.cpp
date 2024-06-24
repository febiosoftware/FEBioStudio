#include "stdafx.h"
#include "XMLEditor.h"
#include <QPainter>
//#include <QSyntaxHighLighter>
// I had to change this include statement for it to find the file on linux
#include <qsyntaxhighlighter.h>
#include <QDockWidget>
#include <QApplication>
#include "MainWindow.h"
#include <QTextCursor>
#include <QTextBlock>
#include <QRegularExpression>

class XMLHighlighter : public QSyntaxHighlighter
{
public:
	enum HighlighColors {
		XML_VALUE,
		XML_ATTRIBUTE_NAME,
		XML_ATTRIBUTE_VALUE,
		XML_COMMENT,
		XML_HIGHLIGHT
	};

public:
	XMLHighlighter(QTextDocument* doc) : QSyntaxHighlighter(doc)
	{
		HighlightingRule rule;

		QPalette palette = qApp->palette();

		// XML values
//		rule.pattern = QRegExp("\\b[\\-0-9\\.+\\-e,]+\\b");
		rule.pattern = QRegularExpression(">[^<]*");
		rule.format.setForeground(m_pal[XML_VALUE]);
		rule.format.setFontWeight(QFont::Bold);
		rule.offset = 1;
		highlightingRules.append(rule);

		// xml attribute values
		rule.offset = 0;
		rule.pattern = QRegularExpression("\"(?:[^\"]|\\.)*\"");
		rule.format.setForeground(m_pal[XML_ATTRIBUTE_VALUE]);
		highlightingRules.append(rule);

		// xml attributes
		rule.offset = 0;
		rule.pattern = QRegularExpression("\\b[a-zA-Z0-9_]+(?=\\=)");
		rule.format.setForeground(m_pal[XML_ATTRIBUTE_NAME]);
		highlightingRules.append(rule);

		// comments
		commentFormat.setForeground(m_pal[XML_COMMENT]);
		commentStartExpression = QRegularExpression("<!--");
		commentEndExpression = QRegularExpression("-->");
	}

	static void setColor(const QBrush& b, int role)
	{
		m_pal[role] = b;
	}

	static const QBrush& color(int role) { return m_pal[role]; }

	void highlightBlock(const QString& text)
	{
		foreach(const HighlightingRule &rule, highlightingRules) 
		{
			QRegularExpression expression(rule.pattern);
			QRegularExpressionMatch match = expression.match(text);
			int index = match.capturedStart();
			while (index >= 0) {
				int length = match.capturedLength();
				setFormat(index + rule.offset, length - 1*rule.offset, rule.format);
				
				match = expression.match(text, index + length);
				index = match.capturedStart();
			}
		}


		setCurrentBlockState(0);

		int startIndex = 0;
		if (previousBlockState() != 1)
			startIndex = commentStartExpression.match(text).capturedStart();

		while (startIndex >= 0)
		{
			int endIndex = commentEndExpression.match(text, startIndex).capturedStart();
			int commentLength;
			if (endIndex == -1)
			{
				setCurrentBlockState(1);
				commentLength = text.length() - startIndex;
			}
			else
			{
				commentLength = endIndex - startIndex + commentEndExpression.match(text, startIndex).capturedLength();
			}
			setFormat(startIndex, commentLength, commentFormat);
			startIndex = commentStartExpression.match(text, startIndex + commentLength).capturedStart();
		}
	}

private:
	struct HighlightingRule
	{
		QRegularExpression pattern;
		QTextCharFormat format;
		int	offset;
	};
	QVector<HighlightingRule> highlightingRules;

	QTextCharFormat	commentFormat;
	QRegularExpression commentStartExpression;
	QRegularExpression commentEndExpression;

public:
	static QBrush	m_pal[5];
};

QBrush XMLHighlighter::m_pal[5];

XMLEditor::XMLEditor(CMainWindow* wnd) : QPlainTextEdit(wnd), m_wnd(wnd)
{
	m_countCache.first = -1;
	m_countCache.second = -1;

	QPalette p = palette();
	p.setColor(QPalette::Text, (!wnd->usingDarkTheme() ? Qt::darkBlue : QColor::fromRgb(51, 153, 255)));
	setPalette(p);

	if (!wnd->usingDarkTheme())
	{
		XMLHighlighter::setColor(Qt::black, XMLHighlighter::XML_VALUE);
		XMLHighlighter::setColor(Qt::red, XMLHighlighter::XML_ATTRIBUTE_NAME);
		XMLHighlighter::setColor(Qt::blue, XMLHighlighter::XML_ATTRIBUTE_VALUE);
		XMLHighlighter::setColor(Qt::darkGreen, XMLHighlighter::XML_COMMENT);
		XMLHighlighter::setColor(QColor::fromRgb(240, 240, 255), XMLHighlighter::XML_HIGHLIGHT);
	}
	else
	{
		XMLHighlighter::setColor(Qt::white, XMLHighlighter::XML_VALUE);
		XMLHighlighter::setColor(QColor::fromRgb(102, 204, 255), XMLHighlighter::XML_ATTRIBUTE_NAME);
		XMLHighlighter::setColor(QColor::fromRgb(255, 150, 50), XMLHighlighter::XML_ATTRIBUTE_VALUE);
		XMLHighlighter::setColor(Qt::darkGreen, XMLHighlighter::XML_COMMENT);
		XMLHighlighter::setColor(QColor::fromRgb(0, 51, 102), XMLHighlighter::XML_HIGHLIGHT);
	}

	lineNumberArea = new LineNumberArea(this);

	connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
	connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLinearNumberArea(QRect,int)));
	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));

	updateLineNumberAreaWidth(0);
//	setCenterOnScroll(true);
//	highlightCurrentLine();
}

void XMLEditor::SetDocument(QTextDocument* doc, const QString& title)
{
	QPalette p = palette();
	p.setColor(QPalette::WindowText, Qt::blue); // Foreground was deprecated. Was told to replace with this.
	setPalette(p);

	if (doc)
	{
		XMLHighlighter* highLighter = new XMLHighlighter(doc);
	}
	else 
	{
		setDisabled(true);
		updateLineNumberAreaWidth(0);
	}

	setDocument(doc);

    setTabStopDistance(40);
}

int XMLEditor::lineNumberAreaWidth()
{
	int digits = 1;
	int max = qMax(1, blockCount());
	while (max >= 10) { max /= 10; ++digits; }

	int w = 0;
	if (document())
	{
		QFontMetrics fm(document()->defaultFont());
		w = fm.horizontalAdvance(QLatin1Char('9'));
	}
	else
	{
		w = fontMetrics().horizontalAdvance(QLatin1Char('9'));
	}

	int space = 20 + w*digits + 7;

	return space;
}

void XMLEditor::updateLineNumberAreaWidth(int newBlockCount)
{
	setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void XMLEditor::updateLinearNumberArea(const QRect& rect, int dy)
{
	if (dy)
		lineNumberArea->scroll(0, dy);
	else if ((m_countCache.first != blockCount()) ||
			(m_countCache.second != textCursor().block().lineCount()))
		{
			lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());
			m_countCache.first = blockCount();
			m_countCache.second = textCursor().block().lineCount();
		}

	if (rect.contains(viewport()->rect()))
		updateLineNumberAreaWidth(0);
}

void XMLEditor::paintEvent(QPaintEvent* event)
{
	QPlainTextEdit::paintEvent(event);
	
	if (isEnabled()==false)
	{
		QPainter painter(viewport());
		painter.fillRect(rect(), Qt::gray);
	}
}

void XMLEditor::wheelEvent(QWheelEvent* ev)
{
	QTextDocument* doc = document();
	if (doc == nullptr) return;

	QFont font = doc->defaultFont();
	qreal fontsize = font.pointSizeF();

	Qt::KeyboardModifiers key = ev->modifiers();
	bool bctrl = (key & Qt::ControlModifier);
	if (bctrl)
	{
		int y = ev->angleDelta().y();
		if (y == 0) y = ev->angleDelta().x();
		if (y > 0) fontsize++;
		else if (y < 0) fontsize--;

		if (fontsize < 8) fontsize = 8;
		if (fontsize > 24) fontsize = 24;

		font.setPointSizeF(fontsize);
		doc->setDefaultFont(font);
		ev->accept();
		update();
	}
}

void XMLEditor::resizeEvent(QResizeEvent* e)
{
	QPlainTextEdit::resizeEvent(e);

	QRect cr = contentsRect();
	lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void XMLEditor::highlightCurrentLine()
{
	QList<QTextEdit::ExtraSelection> extraSelection;

	if (!isReadOnly() && isEnabled())
	{
		QTextEdit::ExtraSelection selection;
		selection.format.setBackground(XMLHighlighter::color(XMLHighlighter::XML_HIGHLIGHT));
		selection.format.setProperty(QTextFormat::FullWidthSelection, true);
		selection.cursor = textCursor();
		selection.cursor.clearSelection();
		extraSelection.append(selection);

		setExtraSelections(extraSelection);
	}
}

void XMLEditor::lineNumberAreaPaintEvent(QPaintEvent* e)
{
	QPainter painter(lineNumberArea);
	if (isEnabled() == false)
	{
		painter.fillRect(e->rect(), Qt::gray);
		return;
	}

	int theme = m_wnd->currentTheme();
	painter.fillRect(e->rect(), (theme == 0 ? Qt::darkGray : Qt::black));
	painter.setPen((theme == 0 ? Qt::white : Qt::darkGray));

	if (!isEnabled()) return;

	QFont font = document()->defaultFont();
	painter.setFont(font);

	QTextBlock block = firstVisibleBlock();
	int blockNumber = block.blockNumber();
	int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
	int bottom = top + (int) blockBoundingRect(block).height();

	QFontMetrics fm(font);
	int h = fm.height();
	int w = lineNumberArea->width();

	while (block.isValid() && (top <= e->rect().bottom()))
	{
		if (block.isVisible() && (bottom >= e->rect().top()))
		{
			QString number = QString::number(blockNumber + 1);
			painter.drawText(0, top, w - 7, h, Qt::AlignRight, number);
		}

		block = block.next();
		top = bottom;
		bottom = top + (int) blockBoundingRect(block).height();
		++blockNumber;
	}
}

void XMLEditor::toggleLineComment()
{
	QTextCursor cursor = textCursor();

	QString txt = cursor.block().text();

	// see if we can find a begin comment
	if (txt.contains("<!--"))
	{
		txt.replace("<!--", "");
		txt.replace("-->", "");
		cursor.beginEditBlock();
		cursor.movePosition(QTextCursor::StartOfBlock);
		cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
		cursor.removeSelectedText();
		cursor.insertText(txt);
		cursor.endEditBlock();
	}
	else
	{
		cursor.beginEditBlock();
		cursor.movePosition(QTextCursor::StartOfBlock);
		cursor.insertText("<!--");
		cursor.movePosition(QTextCursor::EndOfBlock);
		cursor.insertText("-->");
		cursor.endEditBlock();
	}
}

void XMLEditor::duplicateLine()
{
	QTextCursor cursor = textCursor();
	QString txt = cursor.block().text();

	cursor.beginEditBlock();
	cursor.movePosition(QTextCursor::EndOfBlock);
	cursor.insertText("\n" + txt);
	cursor.endEditBlock();
}

void XMLEditor::deleteLine()
{
	QTextCursor cursor = textCursor();
	cursor.beginEditBlock();
	cursor.select(QTextCursor::BlockUnderCursor);
	cursor.removeSelectedText();
	cursor.endEditBlock();
}
