#pragma once
#include <QPlainTextEdit>
#include "DocumentView.h"

class CTextEditor : public QPlainTextEdit
{
	Q_OBJECT

public:
	enum TextFormat {
		PLAIN,
		XML,
		CODE,
		CMAKE,
		PYTHON
	};

public:
	CTextEditor(QWidget* parent);

	void lineNumberAreaPaintEvent(QPaintEvent* event);
	int lineNumberAreaWidth();

	void SetDocument(QTextDocument* doc, TextFormat fmt = TextFormat::PLAIN);

	void toggleLineComment();
	void duplicateLine();
	void deleteLine();

	void useDarkTheme(bool b);

protected:
	void resizeEvent(QResizeEvent* event) override;
	void paintEvent(QPaintEvent* event) override;
	void wheelEvent(QWheelEvent* ev) override;
	void SetHighlighter(QTextDocument* doc, TextFormat fmt);

private slots:
	void updateLineNumberAreaWidth(int newBlockCount);
	void highlightCurrentLine();
	void updateLinearNumberArea(const QRect&, int);

private:
	QWidget*			lineNumberArea;
	QPair<int, int>		m_countCache;
	bool				m_useDarkTheme;
};

class LineNumberArea : public QWidget
{
public:
	LineNumberArea(CTextEditor* editor) : QWidget(editor){ m_txtEditor = editor; }

	QSize sizeHint() const 
	{
		return QSize(m_txtEditor->lineNumberAreaWidth(), 0);
	}

protected:
	void paintEvent(QPaintEvent* event)
	{
		m_txtEditor->lineNumberAreaPaintEvent(event);
	}

private:
	CTextEditor*	m_txtEditor;
};

class CTextEditView : public CDocumentView
{
public:
	CTextEditView(CMainWindow* wnd);

	void setDocument(CDocument* doc) override;

public:
	QTextDocument* textDocument();

	bool find(const QString& txt);
	bool findAgain();
	void centerCursor();
	void toggleLineComment();
	void duplicateLine();
	void deleteLine();

private:
	CTextEditor* m_edit;
	QString m_lastFindText;
};
