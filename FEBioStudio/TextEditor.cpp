#include "stdafx.h"
#include "TextEditor.h"
#include <QPainter>
//#include <QSyntaxHighLighter>
// I had to change this include statement for it to find the file on linux
#include <qsyntaxhighlighter.h>
#include <QDockWidget>
#include <QApplication>
#include <QTextCursor>
#include <QTextBlock>
#include <QTextLayout>
#include <QTextOption>
#include <QRegularExpression>
#include <QBoxLayout>
#include "TextDocument.h"
#include "XMLDocument.h"
#include "MainWindow.h"
#include "QMessageBox"

class HighlightRule
{
public:
	QRegularExpression pattern;
	QTextCharFormat format;
	
	HighlightRule() {}
	HighlightRule(const QString& regex) : pattern(regex) {}

	void setPattern(const QString& regex) { pattern.setPattern(regex); }
	void setStyle(const QBrush& fg, int weight = QFont::Medium)
	{
		format.setForeground(fg);
		format.setFontWeight(weight);
	}
};

class CSyntaxHighlighter : public QSyntaxHighlighter
{
public:
	CSyntaxHighlighter(QTextDocument* doc) : QSyntaxHighlighter(doc) {}

	void AddRule(const HighlightRule& rule) { m_rules.append(rule); }

	void AddRule(const QString& regex, const QBrush& fg, int weight = QFont::Medium)
	{
		HighlightRule rule(regex);
		rule.setStyle(fg, weight);
		m_rules.append(rule);
	}

	void SetMultilineComment(const QString& start, const QString& end, const QBrush& fg, int weight = QFont::Medium)
	{
		m_startExpression = start;
		m_endExpression = end;
		m_commmentFormat.setForeground(fg);
		m_commmentFormat.setFontWeight(weight);
	}

	void highlightBlock(const QString& text) override
	{
		ApplyRules(text);
		CheckMultilineComment(text);
	}

protected:

	void ApplyRules(const QString& text)
	{
		for (const auto& rule : m_rules)
		{
			ApplyRule(text, rule);
		}
	}

	bool ApplyRule(const QString& text, const HighlightRule& rule, int offset = 0)
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
		if (m_startExpression.isEmpty() || m_endExpression.isEmpty()) return;

		QRegularExpression startExpression(m_startExpression);
		QRegularExpression endExpression(m_endExpression);

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
			setFormat(startIndex, commentLength, m_commmentFormat);
			startIndex = text.indexOf(startExpression,
				startIndex + commentLength);
		}
	}

protected:
	QList<HighlightRule>	m_rules;

	// multiline comments
	QString m_startExpression;
	QString m_endExpression;
	QTextCharFormat m_commmentFormat;
};

class PlainTextHighlighter : public CSyntaxHighlighter
{
public:
	PlainTextHighlighter(QTextDocument* doc) : CSyntaxHighlighter(doc) {}
	void highlightBlock(const QString& text) {}
};

class XMLHighlighter : public CSyntaxHighlighter
{
public:
	XMLHighlighter(QTextDocument* doc, int theme) : CSyntaxHighlighter(doc)
	{
		QString tag("(?<=<|<\\?|<\\/)[\\w]+");
		QString value("(?<=>)[^<]+");
		QString stringValue("(?<=>)[a-zA-Z][\\w]*");
		QString attName("\\b[a-zA-Z0-9_]+(?=\\=)");
		QString attVal("\"[^\"]*\"");

		if (theme == 0)
		{
			AddRule(tag    , Qt::blue);
			AddRule(value  , Qt::darkGray);
			AddRule(attName, Qt::red);
			AddRule(attVal , Qt::blue);
		}
		else
		{
			AddRule(tag        , QColor("cornflowerblue"));
			AddRule(value      , QColor("gainsboro"));
			AddRule(stringValue, QColor("khaki"));
			AddRule(attName    , QColor::fromRgb(102, 204, 255));
			AddRule(attVal     , QColor::fromRgb(255, 150, 50));
		}

		SetMultilineComment("<!--", "-->", QColor("forestgreen"));
	}
};

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

class CppHighlighter : public CSyntaxHighlighter
{
public:
	CppHighlighter(QTextDocument* doc, int theme) : CSyntaxHighlighter(doc)
	{
		QString keywords = toString(szcppkeywords, sizeof(szcppkeywords) / sizeof(const char*));
		QString controls = toString(szcppcontrols, sizeof(szcppcontrols) / sizeof(const char*));

		QString cppKeyWords(keywords);
		QString cppControls(controls);
		QString directives("#.*");
		QString lineComment("//.*");
		QString stringLiterals("([\"'])(.*?)\\1");
		QString numbers("(?<!\\w)[-+]?\\d*\\.?\\d+([eE][-+]?\\d+)?");
		QString allcaps("\\b[A-Z][A-Z_\\d]*\\b");
		QString braces("[\\(\\[\\{\\)\\]\\}]");

		if (theme == 0) // light theme
		{
			AddRule(cppKeyWords   , Qt::blue);
			AddRule(cppControls   , Qt::darkMagenta);
			AddRule(numbers       , Qt::darkCyan);
			AddRule(braces        , QColor("gold"));
			AddRule(allcaps       , Qt::green);
			AddRule(stringLiterals, QColor("orangered"));
			AddRule(directives    , Qt::darkRed);
			AddRule(lineComment   , Qt::darkGreen);
		}
		else // dark theme
		{
			AddRule(cppKeyWords   , QColor("dodgerblue"));
			AddRule(cppControls   , QColor("plum"));
			AddRule(numbers       , Qt::cyan);
			AddRule(braces        , QColor("gold"));
			AddRule(allcaps       , QColor("lightskyblue"));
			AddRule(stringLiterals, QColor("orange"));
			AddRule(directives    , QColor("darkorange"));
			AddRule(lineComment   , QColor("forestgreen"));
		}

		SetMultilineComment("\\/\\*", "\\*\\/", QColor("forestgreen"));
	}
};

const char* szcmakecmd[] = {
	"add_compile_options",
	"add_definitions",
	"add_library",
	"cmake_minimum_required", 
	"find_library", 
	"include", 
	"include_directories", 
	"link_directories", 
	"list",
	"mark_as_advanced",
	"message", 
	"option", 
	"project", 
	"set", 
	"set_property", 
	"target_link_libraries", 
	"unset"
};

const char* szcmakectrl[] = {
	"else",  
	"elseif", 
	"endforeach",
	"endif", 
	"foreach", 
	"function", 
	"if", 
	"macro",
	"while",
};

class CMakeHighlighter : public CSyntaxHighlighter
{
public:
	CMakeHighlighter(QTextDocument* doc, int theme) : CSyntaxHighlighter(doc)
	{
		QString cmdList  = toString(szcmakecmd, sizeof(szcmakecmd) / sizeof(const char*));
		QString ctrlList = toString(szcmakectrl, sizeof(szcmakectrl) / sizeof(const char*));

		QString commands(cmdList);
		QString control(ctrlList);
		QString allcaps("\\b[A-Z][A-Z_\\d]*\\b");
		QString braces("[\\(\\[\\{\\)\\]\\}]");
		QString stringLiterals("([\"'])(.*?)\\1");
		QString lineComment("#.*");

		if (theme == 0) // light theme
		{
			AddRule(commands      , Qt::darkBlue);
			AddRule(control       , Qt::blue);
			AddRule(braces        , QColor("gold"));
			AddRule(allcaps       , Qt::green);
			AddRule(lineComment   , Qt::darkGreen);
			AddRule(stringLiterals, QColor("orangered"));
		}
		else // dark theme
		{
			AddRule(commands      , QColor("khaki"));
			AddRule(control       , QColor("cornflowerblue"));
			AddRule(braces        , QColor("gold"));
			AddRule(allcaps       , QColor("lightskyblue"));
			AddRule(lineComment   , QColor("olivedrab"));
			AddRule(stringLiterals, QColor("coral"));
		}
	}
};

const char* szpythonkeys[] = {
	"and", "as", "assert",
	"async", "await",
	"break",
	"class", "continue",
	"def", "del",
	"elif", "else", "except",
	"False", "finally",
	"for", "from",
	"global",
	"if", "import", "in", "is", 
	"lambda",
	"None", "nonlocal", "not",
	"or",
	"pass",
	"raise", "return",
	"True", "try",
	"while", "with",
	"yield"
};

const char* szpythonfncs[] = {
	"__import__",
	"abs", "all", "any", "ascii",
	"bin", "bool", "bytearray", "bytes",
	"callable", "chr", "classmethod", "compile", "complex",
	"delattr", "dict", "dir", "divmod",
	"enumerate", "eval", "exec",
	"filter", "float", "format", "frozenset",
	"getattr", "globals",
	"hasattr", "hash", "help", "hex",
	"id", "input", "int", "isinstance", "issubclass", "iter",
	"len", "list", "locals",
	"map", "max", "memoryview", "min",
	"next",
	"object", "oct", "open", "ord",
	"pow", "print", "property", "range", "repr", "reversed", "round",
	"set", "setattr", "slice", "sorted", "staticmethod", "str", "sum", "super",
	"tuple", "type",
	"vars",
	"zip",
	"aiter", "anext", "breakpoint",
};

class CPythonHighlighter : public CSyntaxHighlighter
{
public:
	CPythonHighlighter(QTextDocument* doc, int theme) : CSyntaxHighlighter(doc)
	{
		m_stringFormat.setForeground(theme == 0 ? QColor("orangered") : QColor("orange"));

		QString keywords = toString(szpythonkeys, sizeof(szpythonkeys) / sizeof(const char*));
		QString funcs    = toString(szpythonfncs, sizeof(szpythonfncs) / sizeof(const char*));

		QString stringLiterals("(?i)(?:\\b(?:fr|rf|br|rb|r|u|b|f))?(\"(?:\\\\.|[^\"\\\\])*\"|'(?:\\\\.|[^'\\\\])*')");
		QString numbers("\\b(?:0[xX][0-9a-fA-F_]+|0[bB][01_]+|0[oO][0-7_]+|(?:\\d[\\d_]*\\.?[\\d_]*|\\.\\d[\\d_]*)(?:[eE][+-]?\\d[\\d_]*)?[jJ]?)\\b");
		QString allcaps("\\b[A-Z][A-Z_\\d]*\\b");
		QString braces("[\\(\\[\\{\\)\\]\\}]");
		QString decorators("@[A-Za-z_]\\w*(?:\\.[A-Za-z_]\\w*)*");
		QString classNames("(?<=\\bclass\\s)\\w+");
		QString functionNames("(?<=\\bdef\\s)\\w+");
		QString magicNames("\\b__\\w+__\\b");
		QString selfNames("\\b(?:self|cls)\\b");
		QString exceptions("\\b(?:BaseException|Exception|ArithmeticError|AssertionError|AttributeError|EOFError|ImportError|IndexError|KeyError|KeyboardInterrupt|LookupError|NameError|NotImplementedError|OSError|OverflowError|RuntimeError|StopIteration|SyntaxError|SystemExit|TypeError|ValueError|ZeroDivisionError)\\b");

		if (theme == 0) // light theme
		{
			AddRule(decorators     , Qt::darkYellow);
			AddRule(keywords       , Qt::darkMagenta, QFont::Bold);
			AddRule(funcs          , QColor("saddlebrown"));
			AddRule(exceptions     , QColor("saddlebrown"));
			AddRule(selfNames      , QColor("steelblue"));
			AddRule(numbers        , Qt::darkCyan);
			AddRule(braces         , QColor("gold"));
			AddRule(allcaps        , Qt::green);
			AddRule(magicNames     , QColor("royalblue"));
			AddRule(classNames     , QColor("darkBlue"), QFont::Bold);
			AddRule(functionNames  , QColor("darkBlue"));
			AddRule(stringLiterals , QColor("orangered"));
			m_commentFormat.setForeground(Qt::darkGreen);
		}
		else // dark theme
		{
			AddRule(decorators     , QColor("goldenrod"));
			AddRule(keywords       , QColor("plum"), QFont::Bold);
			AddRule(funcs          , QColor("khaki"));
			AddRule(exceptions     , QColor("khaki"));
			AddRule(selfNames      , QColor("lightskyblue"));
			AddRule(numbers        , Qt::cyan);
			AddRule(braces         , QColor("gold"));
			AddRule(allcaps        , QColor("lightskyblue"));
			AddRule(magicNames     , QColor("deepskyblue"));
			AddRule(classNames     , QColor("dodgerblue"), QFont::Bold);
			AddRule(functionNames  , QColor("dodgerblue"));
			AddRule(stringLiterals , QColor("orange"));
			m_commentFormat.setForeground(QColor("forestgreen"));
		}

	}

	void highlightBlock(const QString& text) override
	{
		ApplyRules(text);
		HighlightLineComment(text);
		HighlightMultilineStrings(text);
	}

private:
	void HighlightLineComment(const QString& text)
	{
		int index = FindLineComment(text);
		if (index >= 0) setFormat(index, text.length() - index, m_commentFormat);
	}

	int FindLineComment(const QString& text) const
	{
		for (int i = 0; i < text.length(); ++i)
		{
			QChar ch = text[i];
			if (ch == '#') return i;

			if ((ch == '"') || (ch == '\''))
			{
				if ((i + 2 < text.length()) && (text[i + 1] == ch) && (text[i + 2] == ch))
				{
					int end = text.indexOf(QString(3, ch), i + 3);
					if (end == -1) return -1;
					i = end + 2;
				}
				else
				{
					i = FindStringEnd(text, i);
					if (i == -1) return -1;
				}
			}
		}

		return -1;
	}

	int FindStringEnd(const QString& text, int start) const
	{
		QChar quote = text[start];
		for (int i = start + 1; i < text.length(); ++i)
		{
			if (text[i] == '\\')
			{
				i++;
				continue;
			}

			if (text[i] == quote) return i;
		}

		return -1;
	}

	void HighlightMultilineStrings(const QString& text)
	{
		setCurrentBlockState(0);

		int startIndex = 0;
		QString delimiter;
		if (previousBlockState() == 1)
		{
			delimiter = "\"\"\"";
		}
		else if (previousBlockState() == 2)
		{
			delimiter = "'''";
		}
		else
		{
			startIndex = FindTripleQuote(text, 0, delimiter);
		}

		while (startIndex >= 0)
		{
			int delimiterLength = delimiter.length();
			int endIndex = text.indexOf(delimiter, startIndex + delimiterLength);
			int stringLength = 0;

			if (endIndex == -1)
			{
				setCurrentBlockState(delimiter == "\"\"\"" ? 1 : 2);
				stringLength = text.length() - startIndex;
				setFormat(startIndex, stringLength, m_stringFormat);
				return;
			}

			stringLength = endIndex - startIndex + delimiterLength;
			setFormat(startIndex, stringLength, m_stringFormat);
			startIndex = FindTripleQuote(text, startIndex + stringLength, delimiter);
		}
	}

	int FindTripleQuote(const QString& text, int offset, QString& delimiter) const
	{
		int singleQuote = text.indexOf("'''", offset);
		int doubleQuote = text.indexOf("\"\"\"", offset);

		if (singleQuote == -1)
		{
			delimiter = "\"\"\"";
			return doubleQuote;
		}
		if (doubleQuote == -1)
		{
			delimiter = "'''";
			return singleQuote;
		}

		if (singleQuote < doubleQuote)
		{
			delimiter = "'''";
			return singleQuote;
		}

		delimiter = "\"\"\"";
		return doubleQuote;
	}

private:
	QTextCharFormat m_stringFormat;
	QTextCharFormat m_commentFormat;
};

CTextEditor::CTextEditor(QWidget* parent) : QPlainTextEdit(parent)
{
	m_countCache.first = -1;
	m_countCache.second = -1;
	m_useDarkTheme = false;
	m_showWhitespace = false;

	lineNumberArea = new LineNumberArea(this);

	connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
	connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLinearNumberArea(QRect,int)));
	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));

	updateLineNumberAreaWidth(0);
//	setCenterOnScroll(true);
//	highlightCurrentLine();
}

void CTextEditor::useDarkTheme(bool b)
{
	m_useDarkTheme = b;

	QPalette p = palette();
	p.setColor(QPalette::Text, (b ? QColor(225, 225, 225) : Qt::black));// QColor::fromRgb(51, 153, 255)));
	p.setColor(QPalette::Base, (b ? QColor(24, 26, 28) : Qt::white));
	setPalette(p);
}

void CTextEditor::setShowWhitespace(bool b)
{
	m_showWhitespace = b;

	QTextDocument* doc = document();
	if (doc)
	{
		QTextOption ops = doc->defaultTextOption();
		QTextOption::Flags flags = ops.flags();
		flags &= ~(QTextOption::ShowTabsAndSpaces | QTextOption::ShowLineAndParagraphSeparators);
		ops.setFlags(flags);
		doc->setDefaultTextOption(ops);
	}

	viewport()->update();
}

bool CTextEditor::showWhitespace() const
{
	return m_showWhitespace;
}

void CTextEditor::SetHighlighter(QTextDocument* doc, TextFormat fmt)
{
	CSyntaxHighlighter* hl = nullptr;
	switch (fmt)
	{
	case TextFormat::PLAIN : hl = new PlainTextHighlighter(doc); break;
	case TextFormat::XML   : hl = new XMLHighlighter      (doc, (m_useDarkTheme ? 1 : 0)); break;
	case TextFormat::CODE  : hl = new CppHighlighter      (doc, (m_useDarkTheme ? 1 : 0)); break;
	case TextFormat::CMAKE : hl = new CMakeHighlighter    (doc, (m_useDarkTheme ? 1 : 0)); break;
	case TextFormat::PYTHON: hl = new CPythonHighlighter  (doc, (m_useDarkTheme ? 1 : 0)); break;
	default:
		assert(false);
	}
}

void CTextEditor::SetDocument(QTextDocument* doc, TextFormat fmt)
{
	QPalette p = palette();
	p.setColor(QPalette::WindowText, Qt::blue); // Foreground was deprecated. Was told to replace with this.
	setPalette(p);

	setDocument(doc);

	if (doc)
	{
		setDisabled(false);
		SetHighlighter(doc, fmt);
	}
	else 
	{
		setDisabled(true);
		updateLineNumberAreaWidth(0);
	}

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

	if (m_showWhitespace && document())
	{
		QPainter painter(viewport());
		paintWhitespace(painter);
	}
	
	if (isEnabled()==false)
	{
		QPainter painter(viewport());
		painter.fillRect(rect(), Qt::gray);
	}
}

void CTextEditor::paintWhitespace(QPainter& painter)
{
	QColor markColor = m_useDarkTheme ? QColor(95, 95, 95) : QColor(95, 95, 95);
	painter.setPen(QPen(markColor, 1));
	painter.setBrush(markColor);
	painter.setRenderHint(QPainter::Antialiasing, true);

	QTextBlock block = firstVisibleBlock();
	int top = (int)blockBoundingGeometry(block).translated(contentOffset()).top();
	int bottom = top + (int)blockBoundingRect(block).height();

	while (block.isValid() && (top <= viewport()->rect().bottom()))
	{
		if (block.isVisible() && (bottom >= viewport()->rect().top()))
		{
			QTextLayout* layout = block.layout();
			QString text = block.text();
			QPointF blockPos = blockBoundingGeometry(block).translated(contentOffset()).topLeft();

			for (int lineIndex = 0; lineIndex < layout->lineCount(); ++lineIndex)
			{
				QTextLine line = layout->lineAt(lineIndex);
				int lineStart = line.textStart();
				int lineEnd = lineStart + line.textLength();
				qreal centerY = blockPos.y() + line.y() + line.height() / 2.0;

				for (int i = lineStart; i < lineEnd && i < text.length(); ++i)
				{
					if (text[i] == ' ')
					{
						qreal x1 = line.cursorToX(i);
						qreal x2 = line.cursorToX(i + 1);
						qreal centerX = blockPos.x() + (x1 + x2) / 2.0;
						painter.drawEllipse(QPointF(centerX, centerY), 1.35, 1.35);
					}
					else if (text[i] == '\t')
					{
						qreal x1 = blockPos.x() + line.cursorToX(i) + 2.0;
						qreal x2 = blockPos.x() + line.cursorToX(i + 1) - 2.0;
						if (x2 > x1)
						{
							painter.drawLine(QPointF(x1, centerY), QPointF(x2, centerY));
							painter.drawLine(QPointF(x2, centerY), QPointF(x2 - 4.0, centerY - 3.0));
							painter.drawLine(QPointF(x2, centerY), QPointF(x2 - 4.0, centerY + 3.0));
						}
					}
				}
			}
		}

		block = block.next();
		top = bottom;
		bottom = top + (int)blockBoundingRect(block).height();
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
	else
	{
		QPlainTextEdit::wheelEvent(ev);
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

	QBrush bg;
	if (m_useDarkTheme)
		bg = QColor::fromRgb(34, 48, 64);
	else
		bg = QColor::fromRgb(240, 240, 255);

	if (!isReadOnly() && isEnabled())
	{
		QTextEdit::ExtraSelection selection;
		selection.format.setBackground(bg);
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

	int theme = (m_useDarkTheme ? 1 : 0);
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

CTextEditView::CTextEditView(CMainWindow* wnd) : CDocumentView(wnd)
{
	QVBoxLayout* l = new QVBoxLayout;
	l->addWidget(m_edit = new CTextEditor(wnd));
	m_edit->useDarkTheme(wnd->usingDarkTheme());
	m_edit->setObjectName("txtedit");
	setLayout(l);
}

QTextDocument* CTextEditView::textDocument()
{
	return m_edit->document();
}

bool CTextEditView::find(const QString& txt)
{
	bool b = m_edit->find(txt);
	if (b)
	{
		m_lastFindText = txt;
		centerCursor();
	}
	else
		QMessageBox::information(this, "FEBio Studio", QString("Cannot find: %1").arg(txt));

	return b;
}

bool CTextEditView::findAgain()
{
	if (m_lastFindText.isEmpty()) return true;
	return find(m_lastFindText);
}

void CTextEditView::centerCursor()
{
	m_edit->centerCursor();
}

void CTextEditView::toggleLineComment()
{
	m_edit->toggleLineComment();
}

void CTextEditView::duplicateLine()
{
	m_edit->duplicateLine();
}

void CTextEditView::deleteLine()
{
	m_edit->deleteLine();
}

void CTextEditView::setDocument(CDocument* doc)
{
	CTextDocument* txtDoc = dynamic_cast<CTextDocument*>(doc);
	if (txtDoc)
	{
		QString title = QString::fromStdString(txtDoc->GetDocTitle());
		CTextEditor::TextFormat fmt = CTextEditor::PLAIN;
		if (title.indexOf(QRegularExpression("\\.(hpp|cpp|cxx|h)")) != -1) fmt = CTextEditor::CODE;
		if (title.indexOf(QRegularExpression("\\.(py)")) != -1) fmt = CTextEditor::PYTHON;
		if (title.indexOf("CMakeLists.txt") != -1) fmt = CTextEditor::CMAKE;

		m_edit->blockSignals(true);
		m_edit->SetDocument(txtDoc->GetText(), fmt);
		m_edit->blockSignals(false);
		return;
	}

	CXMLDocument* xmlDoc = dynamic_cast<CXMLDocument*>(doc);
	if (xmlDoc)
	{
		m_edit->SetDocument(xmlDoc->GetTextDocument(), CTextEditor::XML);
		return;
	}

	m_edit->SetDocument(nullptr, CTextEditor::PLAIN);
//	m_edit->clear();
}
