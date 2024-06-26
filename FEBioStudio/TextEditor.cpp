#include "stdafx.h"
#include "TextEditor.h"
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

struct HighlightingRule
{
	QRegularExpression pattern;
	QTextCharFormat format;
	int	offset = 0;

	void setPattern(const QString& regex) { pattern.setPattern(regex); }
	void setStyle(const QBrush& fg, int weight)
	{
		format.setForeground(fg);
		format.setFontWeight(weight);
	}
};

class PlainTextHighlighter : public QSyntaxHighlighter
{
public:
	PlainTextHighlighter(QTextDocument* doc) : QSyntaxHighlighter(doc) {}
	void highlightBlock(const QString& text) {}
};

class XMLHighlighter : public QSyntaxHighlighter
{
public:
	enum HighlightColors {
		XML_VALUE,
		XML_ATTRIBUTE_NAME,
		XML_ATTRIBUTE_VALUE,
		XML_COMMENT,
		XML_HIGHLIGHT
	};

public:
	XMLHighlighter(QTextDocument* doc, int theme) : QSyntaxHighlighter(doc)
	{
		HighlightingRule rule;

		if (theme == 0)
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
		foreach( const HighlightingRule &rule, highlightingRules) 
		{
			const QRegularExpression& expression = rule.pattern;
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
	QVector<HighlightingRule> highlightingRules;

	QTextCharFormat	commentFormat;
	QRegularExpression commentStartExpression;
	QRegularExpression commentEndExpression;

public:
	static QBrush	m_pal[5];
};

QBrush XMLHighlighter::m_pal[5];

const char* szcppkeywords[] = {
	"alignas", "alignof", "and", "and_eq", "asm", "auto",
	"bitand", "bitor", "bool",
	"char", "char8_t", "char16_t", "char32_t", "class", "compl", "concept", "const",
	"consteval", "constexpr", "constinit", "const_cast",
	"decltype", "delete", "double", "dynamic_cast",
	"enum", "explicit", "export", "extern",
	"false", "float", "friend",
	"inline", "int",
	"long",
	"mutable",
	"namespace", "new", "noexcept", "not", "not_eq", "nullptr",
	"operator", "or", "or_eq", "override",
	"private", "protected", "public",
	"reflexpr", "register", "reinterpret_cast", "requires",
	"short", "signed", "sizeof", "static", "static_assert", "static_cast", "struct",
	"template", "this", "thread_local", "true", "typedef", "typeid", "typename",
	"union", "unsigned", "using",
	"virtual", "void", "volatile",
	"wchar_t",
	"xor", "xor_eq"
};

const char* szcppcontrols[] = {
	"break", "case", "catch", 
	"continue", "co_await", "co_return", "co_yield",
	"default", "do",
	"else",
	"for",
	"goto",
	"if",
	"return",
	"switch",
	"throw", "try",
	"while"
};

QString toString(const char* sz[], size_t n)
{
	QString s;
	for (size_t i =0; i<n; ++i)
	{
		s += "\\b";
		s += sz[i];
		s += "\\b";
		if (i != n-1) s += "|";
	}
	return s;
}

class CppHighlighter : public QSyntaxHighlighter
{
public:
	CppHighlighter(QTextDocument* doc, int theme) : QSyntaxHighlighter(doc)
	{
		QString keywords = toString(szcppkeywords, sizeof(szcppkeywords) / sizeof(const char*));
		QString controls = toString(szcppcontrols, sizeof(szcppcontrols) / sizeof(const char*));

		m_cppKeyWords.setPattern(keywords);
		m_cppControls.setPattern(controls);
		m_directives.setPattern("#.*");
		m_lineComment.setPattern("//.*");
		m_stringLiterals.setPattern("\".*\"");
		// TODO: This doesn't work correctly in all cases yet. 
		//       I added the whitespace in front to avoid that variable names with integers would match,
		//       but this also causes issues. I think I may need to use conditionals, but not sure yet how. 
		m_numbers.setPattern("\\s[-+]?\\d*\\.?\\d+([eE][-+]?\\d+)?");

		if (theme == 0) // light theme
		{
			m_cppKeyWords.setStyle(Qt::blue, QFont::DemiBold);
			m_cppControls.setStyle(Qt::darkMagenta, QFont::DemiBold);
			m_directives.setStyle(Qt::darkRed, QFont::DemiBold);
			m_lineComment.setStyle(Qt::darkGreen, QFont::DemiBold);
			m_stringLiterals.setStyle(QColor("orangered"), QFont::DemiBold);
			m_numbers.setStyle(Qt::darkCyan, QFont::DemiBold);
		}
		else // dark theme
		{
			m_cppKeyWords.setStyle(QColor("dodgerblue"), QFont::DemiBold);
			m_cppControls.setStyle(QColor("plum"), QFont::DemiBold);
			m_directives.setStyle(QColor("orangered"), QFont::DemiBold);
			m_lineComment.setStyle(Qt::darkGreen, QFont::DemiBold);
			m_stringLiterals.setStyle(QColor("orange"), QFont::DemiBold);
			m_numbers.setStyle(Qt::cyan, QFont::DemiBold);
		}
	}

	void highlightBlock(const QString& text)
	{
		ApplyRule(text, m_cppKeyWords);
		ApplyRule(text, m_cppControls);
		ApplyRule(text, m_numbers);
		ApplyRule(text, m_stringLiterals);
		ApplyRule(text, m_directives);
		ApplyRule(text, m_lineComment);

		CheckMultilineComment(text);
	}

private:
	bool ApplyRule(const QString& text, const HighlightingRule& rule, int offset = 0)
	{
		int matches = 0;
		QRegularExpressionMatchIterator i = rule.pattern.globalMatch(text, offset);
		while (i.hasNext()) {
			QRegularExpressionMatch match = i.next();
			setFormat(match.capturedStart(), match.capturedLength(), rule.format);
			matches++;
		}
		return (matches != 0);
	}

	void CheckMultilineComment(const QString& text)
	{
		QTextCharFormat multiLineCommentFormat;
		multiLineCommentFormat.setForeground(Qt::darkGreen);
		multiLineCommentFormat.setFontWeight(QFont::DemiBold);
		QRegularExpression startExpression("/\\*");
		QRegularExpression endExpression("\\*/");

		setCurrentBlockState(0);

		int startIndex = 0;
		if (previousBlockState() != 1)
			startIndex = text.indexOf(startExpression);

		while (startIndex >= 0) {
			QRegularExpressionMatch endMatch;
			int endIndex = text.indexOf(endExpression, startIndex, &endMatch);
			int commentLength;
			if (endIndex == -1) {
				setCurrentBlockState(1);
				commentLength = text.length() - startIndex;
			}
			else {
				commentLength = endIndex - startIndex
					+ endMatch.capturedLength();
			}
			setFormat(startIndex, commentLength, multiLineCommentFormat);
			startIndex = text.indexOf(startExpression,
				startIndex + commentLength);
		}
	}

private:
	HighlightingRule m_cppKeyWords;
	HighlightingRule m_cppControls;
	HighlightingRule m_directives;
	HighlightingRule m_lineComment;
	HighlightingRule m_stringLiterals;
	HighlightingRule m_numbers;
};

CTextEditor::CTextEditor(CMainWindow* wnd) : QPlainTextEdit(wnd), m_wnd(wnd)
{
	m_countCache.first = -1;
	m_countCache.second = -1;

	QPalette p = palette();
	p.setColor(QPalette::Text, (wnd->usingDarkTheme() ? Qt::lightGray : Qt::black));// QColor::fromRgb(51, 153, 255)));
	setPalette(p);

	lineNumberArea = new LineNumberArea(this);

	connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
	connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLinearNumberArea(QRect,int)));
	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));

	updateLineNumberAreaWidth(0);
//	setCenterOnScroll(true);
//	highlightCurrentLine();
}

void CTextEditor::SetDocument(QTextDocument* doc, TextFormat fmt)
{
	QPalette p = palette();
	p.setColor(QPalette::WindowText, Qt::blue); // Foreground was deprecated. Was told to replace with this.
	setPalette(p);

	if (doc)
	{
		switch (fmt)
		{
		case TextFormat::PLAIN:
		{
			PlainTextHighlighter* highLighter = new PlainTextHighlighter(doc);
		}
		break;
		case TextFormat::XML:
		{
			XMLHighlighter* highLighter = new XMLHighlighter(doc, m_wnd->currentTheme());
		}
		break;
		case TextFormat::CODE:
		{
			CppHighlighter* highLighter = new CppHighlighter(doc, m_wnd->currentTheme());
		}
		break;
		}
	}
	else 
	{
		setDisabled(true);
		updateLineNumberAreaWidth(0);
	}

	setDocument(doc);

	setTabStopDistance(40);
}

int CTextEditor::lineNumberAreaWidth()
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

void CTextEditor::updateLineNumberAreaWidth(int newBlockCount)
{
	setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CTextEditor::updateLinearNumberArea(const QRect& rect, int dy)
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

void CTextEditor::paintEvent(QPaintEvent* event)
{
	QPlainTextEdit::paintEvent(event);
	
	if (isEnabled()==false)
	{
		QPainter painter(viewport());
		painter.fillRect(rect(), Qt::gray);
	}
}

void CTextEditor::wheelEvent(QWheelEvent* ev)
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

void CTextEditor::resizeEvent(QResizeEvent* e)
{
	QPlainTextEdit::resizeEvent(e);

	QRect cr = contentsRect();
	lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CTextEditor::highlightCurrentLine()
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

void CTextEditor::lineNumberAreaPaintEvent(QPaintEvent* e)
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

void CTextEditor::toggleLineComment()
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

void CTextEditor::duplicateLine()
{
	QTextCursor cursor = textCursor();
	QString txt = cursor.block().text();

	cursor.beginEditBlock();
	cursor.movePosition(QTextCursor::EndOfBlock);
	cursor.insertText("\n" + txt);
	cursor.endEditBlock();
}

void CTextEditor::deleteLine()
{
	QTextCursor cursor = textCursor();
	cursor.beginEditBlock();
	cursor.select(QTextCursor::BlockUnderCursor);
	cursor.removeSelectedText();
	cursor.endEditBlock();
}
