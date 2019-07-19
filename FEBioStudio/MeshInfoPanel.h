#pragma once
#include <QWidget>

class QLabel;
class GObject;
class FESurfaceMesh;

class CMeshInfoPanel : public QWidget
{
private:
	QLabel*	m_nodes;
	QLabel*	m_faces;
	QLabel*	m_elems;

	QLabel*	m_Dx;
	QLabel*	m_Dy;
	QLabel*	m_Dz;

public:
	CMeshInfoPanel(QWidget* parent = 0);

	void setInfo(GObject* pm);
	void setMeshInfo(int nodes, int faces, int elems);
	void setDimensions(double dx, double dy, double dz);
};

class CSurfaceMeshInfoPanel : public QWidget
{
private:
	QLabel*	m_nodes;
	QLabel*	m_edges;
	QLabel*	m_faces;

public:
	CSurfaceMeshInfoPanel(QWidget* parent = 0);

	void setInfo(const FESurfaceMesh* pm);
	void setInfo(int nodes, int edges, int faces);
};
