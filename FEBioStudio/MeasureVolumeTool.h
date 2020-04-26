#include "Tool.h"

//-----------------------------------------------------------------------------
class CDocument;

//-----------------------------------------------------------------------------
class CMeasureVolumeTool : public CBasicTool
{
public:
	// constructor
	CMeasureVolumeTool(CMainWindow* wnd);

	// Apply button
	bool OnApply() override;

	QVariant GetPropertyValue(int i);
	void SetPropertyValue(int i, const QVariant& v);

private:
	double	m_vol;		// volume of selection
	int		m_nformula;	// choose formula

	friend class Props;
};
