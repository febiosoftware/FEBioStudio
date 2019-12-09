#include "Tool.h"

//-----------------------------------------------------------------------------
class CDocument;

//-----------------------------------------------------------------------------
class CMeasureVolumeTool : public CBasicTool
{
public:
	// constructor
	CMeasureVolumeTool();

	// Apply button
	bool OnApply() override;

	QVariant GetPropertyValue(int i);
	void SetPropertyValue(int i, const QVariant& v);

private:
	int		m_nsel;		// selected faces
	double	m_vol;		// volume of selection
	int		m_nformula;	// choose formula

	friend class Props;
};
