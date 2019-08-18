#pragma once
#include "CreatePanel.h"

//=============================================================================
// CAD Point-2-point line

class CCreateP2PLinePane : public CCreatePane
{
	Q_OBJECT

public:
	CCreateP2PLinePane(CCreatePanel* parent);

	bool IsCurveCapped() const;

	void Activate() override;
	void Deactivate() override;

	bool Clear() override;

	bool Undo() override;

	CObject* Create() override;

	// set the input event for this pane
	void setInput(const vec3d& r) override;

	void hideEvent(QHideEvent* ev) override;

protected slots:
	void on_getNode_clicked();
	void on_newCurve_clicked();

private:
	void AddPoint(const vec3d& r);

private:
	QLineEdit*	m_in[3];
	QCheckBox*	m_cap;
	QPushButton*	m_newCurve;
	GCurveMeshObject*	m_tmp;	// temp object
	int		m_lastNode; // last node that was created
};
