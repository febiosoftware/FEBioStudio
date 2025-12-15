/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2021 University of Utah, The Trustees of Columbia University in
the City of New York, and others.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/
#include "DlgMeshDiagnostics.h"
#include <QBoxLayout>
#include <QPushButton>
#include <QFormLayout>
#include <QLabel>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <GeomLib/GObject.h>
#include <MeshLib/FSMesh.h>
using namespace std;

class CDlgMeshDiagnosticsUI
{
public:
	QLabel*			objName;
	QPlainTextEdit*	out;
	QComboBox*		cb;

public:
	GObject*	obj;
	int			testCount;
	int			errorCount;
	int			warningCount;
	int			outLevel;

public:
	void setup(QDialog* dlg)
	{
		outLevel = 0;

		QVBoxLayout* l = new QVBoxLayout;

		QFormLayout* f = new QFormLayout;
		f->addRow("Object:", objName = new QLabel);

		l->addLayout(f);

		QHBoxLayout* h = new QHBoxLayout;
		QPushButton* b = new QPushButton("Run Diagnostics");
		cb = new QComboBox;
		cb->addItems(QStringList() << "Default" << "Detailed");
		h->setContentsMargins(0,0,0,0);
		h->addWidget(b);
		h->addWidget(new QLabel("Output level:"));
		h->addWidget(cb);
		h->addStretch();

		l->addLayout(h);

		out = new QPlainTextEdit;
		out->setReadOnly(true);
		out->setFont(QFont("Courier", 11));
		out->setWordWrapMode(QTextOption::NoWrap);

		l->addWidget(out);

		QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok);
		l->addWidget(bb);

		dlg->setLayout(l);

		b->setFocus();

		QObject::connect(bb, SIGNAL(accepted()), dlg, SLOT(accept()));
		QObject::connect(b, SIGNAL(clicked()), dlg, SLOT(runDiagnostics()));
	}

	void setObjectName(const QString& name)
	{
		objName->setText(name);
	}

	void clearOutput()
	{
		out->clear();
	}

	void log(const QString& msg)
	{
		out->appendPlainText(msg);
	}

	void logWarning(const QString& msg)
	{
		QTextCharFormat cf = out->currentCharFormat();
		QBrush fg = cf.foreground();
		cf.setForeground(QColor::fromRgb(255, 128, 0));
		out->setCurrentCharFormat(cf);
		warningCount++;
		out->appendPlainText(QString("WARNING: ") + msg);
		cf.setForeground(fg);
		out->setCurrentCharFormat(cf);
	}

	void logError(const QString& msg)
	{
		QTextCharFormat cf = out->currentCharFormat();
		QBrush fg = cf.foreground();
		cf.setForeground(Qt::red);
		out->setCurrentCharFormat(cf);
		errorCount++;
		out->appendPlainText(QString("ERROR: ") + msg);
		cf.setForeground(fg);
		out->setCurrentCharFormat(cf);
	}

	void diagnose()
	{
		outLevel = cb->currentIndex();
		if (outLevel < 0) outLevel = 0;

		testCount = 0;
		errorCount = 0;
		warningCount = 0;

		checkMeshStats(); testCount++;
		checkElementConnectivity(); testCount++;
		checkFaceConnectivity(); testCount++;
		checkEdgeConnectivity(); testCount++;
		checkIsolatedVertices(); testCount++;
		checkDuplicateEdges(); testCount++;
		checkDuplicateFaces(); testCount++;
		checkDuplicateElements(); testCount++;
		checkDegenerateEdges(); testCount++;
		checkElementNeighbors(); testCount++;
		checkFaceNeighbors(); testCount++;
		checkEdgeNeighbors(); testCount++;
		checkElementFaceTable(); testCount++;
		checkElementPartitioning(); testCount++;
		checkFacePartitioning(); testCount++;
		checkEdgePartitioning(); testCount++;
		checkNodePartitioning(); testCount++;
		checkSlivers(); testCount++;
	}

	void checkMeshStats();
	void checkElementConnectivity();
	void checkFaceConnectivity();
	void checkEdgeConnectivity();
	void checkIsolatedVertices();
	void checkDuplicateEdges();
	void checkDuplicateFaces();
	void checkDuplicateElements();
	void checkDegenerateEdges();
	void checkElementNeighbors();
	void checkFaceNeighbors();
	void checkEdgeNeighbors();
	void checkElementFaceTable();
	void checkElementPartitioning();
	void checkFacePartitioning();
	void checkEdgePartitioning();
	void checkNodePartitioning();
	void checkSlivers();
};

CDlgMeshDiagnostics::CDlgMeshDiagnostics(QWidget* parent) : QDialog(parent), ui(new CDlgMeshDiagnosticsUI)
{
	ui->obj = nullptr;
	setMinimumSize(800, 600);
	ui->setup(this);
	setWindowTitle("Mesh Diagnostics Tool");
}

void CDlgMeshDiagnostics::SetObject(GObject* po)
{
	ui->obj = po;
	if (po) ui->setObjectName(QString::fromStdString(po->GetName()));
	else ui->setObjectName("---");
	ui->clearOutput();
}

void CDlgMeshDiagnostics::runDiagnostics()
{
	ui->clearOutput();

	// make sure we have an object
	GObject* po = ui->obj;
	if (po == nullptr)
	{
		ui->log("No object selected to diagnose.");
		return;
	}

	// see if this object has a mesh
	FSMesh* pm = po->GetFEMesh();
	if (pm == nullptr)
	{
		ui->log("This object does not have a mesh to diagnose.");
		return;
	}
	FSMesh& mesh = *pm;

	try {
		ui->diagnose();
	}
	catch (...)
	{
		ui->log("\nCatastropic error during diagnostic. Aborting!");
	}

	ui->log("\nDiagnostics summary:");
	ui->log(QString("  Tests completed    : %1").arg(ui->testCount));
	ui->log(QString("  Warnings generated : %1").arg(ui->warningCount));
	ui->log(QString("  Errors generated   : %1").arg(ui->errorCount));
	ui->log("\n\nDiagnostics completed!");
}

void CDlgMeshDiagnosticsUI::checkMeshStats()
{
	FSMesh& mesh = *obj->GetFEMesh();

	// collect some statistics
	int nodes = mesh.Nodes();    log(QString(" Nodes    = %1").arg(nodes));
	int edges = mesh.Edges();    log(QString(" Edges    = %1").arg(edges));
	int faces = mesh.Faces();    log(QString(" Faces    = %1").arg(faces));
	int elems = mesh.Elements(); log(QString(" Elements = %1").arg(elems));

	// break down elements
	int solidElems = 0;
	int shellElems = 0;
	int beamElems  = 0;
	const int MAX_ELEM_TYPES = 22;
	int elemCount[MAX_ELEM_TYPES] = { 0 };
	for (int i = 0; i < elems; ++i)
	{
		FSElement& el = mesh.Element(i);
		int elemType = el.Type();
		if ((elemType >= 0) && (elemType < MAX_ELEM_TYPES)) elemCount[elemType]++; else elemCount[FE_INVALID_ELEMENT_TYPE]++;
		if      (el.IsSolid()) solidElems++;
		else if (el.IsShell()) shellElems++;
		else if (el.IsBeam ()) beamElems++;
	}

	log(QString("Solid elements = %1").arg(solidElems));
	log(QString("Shell elements = %1").arg(shellElems));
	log(QString("Beam elements = %1").arg(beamElems));
	log("Element breakdown:");
	const char* szelem[] = { "invalid","HEX8","TET4","PENTA6","QUAD4","TRI3", "BEAM2", "HEX20", "QUAD8", "BEAM3", "TET10", "TRI6", "TET15", "HEX27", "TRI7", "QUAD9", "PENTA15", "PYRA5", "TET20", "TRI10", "TET5", "PYRA13" };
	for (int i = 1; i < MAX_ELEM_TYPES; ++i)
	{
		int ni = elemCount[i];
		if (ni > 0)
		{
			QString s = QString(" %1 = %2").arg(szelem[i], 9).arg(ni);
			log(s);
		}
	}
	if (elemCount[FE_INVALID_ELEMENT_TYPE] > 0)
	{
		logError(QString("%d invalid elements found.").arg(elemCount[FE_INVALID_ELEMENT_TYPE]));
	}

	// break down faces
	const int MAX_FACE_TYPES = 8;
	int faceCount[MAX_FACE_TYPES] = { 0 };
	for (int i = 0; i < faces; ++i)
	{
		FSFace& face = mesh.Face(i);
		int faceType = face.Type();
		if ((faceType >= 0) && (faceType < MAX_FACE_TYPES)) faceCount[faceType]++;
		else faceCount[FE_FACE_INVALID_TYPE]++;
	}

	log("Face breakdown:");
	const char* szface[] = { "invalid","TRI3","QUAD4","TRI6","TRI7","QUAD8", "QUAD9", "TRI10" };
	for (int i = 1; i < MAX_FACE_TYPES; ++i)
	{
		int ni = faceCount[i];
		if (ni > 0)
		{
			QString s = QString(" %1 = %2").arg(szface[i], 9).arg(ni);
			log(s);
		}
	}
	if (faceCount[FE_FACE_INVALID_TYPE] > 0)
	{
		logError(QString("%1 invalid faces found.").arg(faceCount[FE_FACE_INVALID_TYPE]));
	}

	// break down edges
	const int MAX_EDGE_TYPES = 4;
	int edgeCount[MAX_EDGE_TYPES] = { 0 };
	for (int i = 0; i < edges; ++i)
	{
		FSEdge& edge = mesh.Edge(i);
		int edgeType = edge.Type();
		if ((edgeType >= 0) && (edgeType < MAX_EDGE_TYPES)) edgeCount[edgeType]++; else edgeCount[FE_EDGE_INVALID]++;
	}

	log("Edge breakdown:");
	const char* szline[] = { "LINE2","LINE3","LINE4","invalid"};
	for (int i = 0; i < MAX_EDGE_TYPES - 1; ++i)
	{
		int ni = edgeCount[i];
		if (ni > 0)
		{
			QString s = QString(" %1 = %2").arg(szline[i], 9).arg(ni);
			log(s);
		}
	}
	if (edgeCount[FE_EDGE_INVALID] > 0)
	{
		logError(QString("%1 invalid edges found.").arg(edgeCount[FE_EDGE_INVALID]));
	}

	// count solid-shell and solid-shell-solid interface.
	int nss = 0;
	int nsss = 0;
	for (int i = 0; i < mesh.Faces(); ++i)
	{
		FSFace& face = mesh.Face(i);
		FSElement_* pe = mesh.ElementPtr(face.m_elem[0].eid);
		if (pe && pe->IsShell())
		{
			if (face.m_elem[2].eid >= 0) nsss++;
			else if (face.m_elem[1].eid >= 0) nss++;
		}
	}
	log(QString("Solid-shell interface elements = %1").arg(nss));
	log(QString("Solid-shell-solid interface elements = %1").arg(nsss));
}

void CDlgMeshDiagnosticsUI::checkElementConnectivity()
{
	FSMesh& mesh = *obj->GetFEMesh();

	int errors = 0;
	int NN = mesh.Nodes();
	int NE = mesh.Elements();
	for (int i = 0; i < NE; ++i)
	{
		FSElement& el = mesh.Element(i);
		for (int j = 0; j < el.Nodes(); ++j)
		{
			int nj = el.m_node[j];
			if ((nj < 0) || (nj >= NN)) errors++;
		}
	}

	if (errors == 0) log("Element connectivity looks good.");
	else logError(QString("%1 invalid node reference(s) found in element connectivity.").arg(errors));
}

void CDlgMeshDiagnosticsUI::checkFaceConnectivity()
{
	FSMesh& mesh = *obj->GetFEMesh();

	int errors = 0;
	int NN = mesh.Nodes();
	int NF = mesh.Faces();
	for (int i = 0; i < NF; ++i)
	{
		FSFace& face = mesh.Face(i);
		int nf = face.Nodes();
		for (int j = 0; j < nf; ++j)
		{
			int nj = face.n[j];
			if ((nj < 0) || (nj >= NN)) errors++;

			for (int k = 0; k < nf; ++k)
			{
				int nk = face.n[k];
				if ((k != j) && (nk == nj)) errors++;
			}
		}
	}

	if (errors == 0) log("Face connectivity looks good.");
	else logError(QString("%1 invalid node reference(s) found in face connectivity.").arg(errors));
}

void CDlgMeshDiagnosticsUI::checkEdgeConnectivity()
{
	FSMesh& mesh = *obj->GetFEMesh();

	int errors = 0;
	int NN = mesh.Nodes();
	int NE = mesh.Edges();
	for (int i = 0; i < NE; ++i)
	{
		FSEdge& edge = mesh.Edge(i);
		for (int j = 0; j < edge.Nodes(); ++j)
		{
			int nj = edge.n[j];
			if ((nj < 0) || (nj >= NN)) errors++;
		}
	}

	if (errors == 0) log("Edge connectivity looks good.");
	else logError(QString("%1 invalid node reference(s) found in edge connectivity.").arg(errors));
}

void CDlgMeshDiagnosticsUI::checkIsolatedVertices()
{
	FSMesh& mesh = *obj->GetFEMesh();
	mesh.TagAllNodes(0);
	int NN = mesh.Nodes();
	int NE = mesh.Elements();
	for (int i = 0; i < NE; ++i)
	{
		FSElement& el = mesh.Element(i);
		for (int j = 0; j < el.Nodes(); ++j)
		{
			int nj = el.m_node[j];
			if ((nj >= 0) && (nj < NN))
			{
				FSNode& node = mesh.Node(nj);
				node.m_ntag = 1;
			}
		}
	}

	int nerr = 0;
	for (int i = 0; i < NN; ++i)
	{
		if (mesh.Node(i).m_ntag == 0) nerr++;
	}

	if (nerr == 0) log("No isolated nodes found.");
	else logWarning(QString("%1 isolated nodes found.").arg(nerr));

	// Count required nodes
	int nreq = 0;
	for (int i = 0; i < NN; ++i)
	{
		if (mesh.Node(i).IsRequired()) nreq++;
	}

	log(QString("%1 required nodes found.").arg(nreq));
}

void CDlgMeshDiagnosticsUI::checkDuplicateEdges()
{
	FSMesh& mesh = *obj->GetFEMesh();

	int NE = mesh.Edges();
	int NN = mesh.Nodes();

	vector<vector<int> > NET(NN);
	for (int i = 0; i < mesh.Edges(); ++i)
	{
		FSEdge& ei = mesh.Edge(i);
		if ((ei.n[0] >= 0) && (ei.n[0] < NN)) NET[ei.n[0]].push_back(i);
		if ((ei.n[1] >= 0) && (ei.n[1] < NN)) NET[ei.n[1]].push_back(i);
	}

	int duplicates = 0;
	mesh.TagAllEdges(-1);
	for (int i = 0; i < NE; ++i)
	{
		FSEdge& ei = mesh.Edge(i);
		if (ei.m_ntag == -1)
		{
			if ((ei.n[0] >= 0) && (ei.n[0] < NN))
			{
				vector<int>& net = NET[ei.n[0]];
				for (int j = 0; j < net.size(); ++j)
				{
					int nej = net[j];
					if (nej > i)
					{
						FSEdge& ej = mesh.Edge(nej);
						if ((ej.m_ntag == -1) && (ej == ei))
						{
							ej.m_ntag = i;
							duplicates++;
						}
					}
				}
			}
		}
	}

	if (duplicates == 0) log("No duplicate edges found.");
	else logError(QString("%1 duplicate edges found.").arg(duplicates));
}

void CDlgMeshDiagnosticsUI::checkDuplicateFaces()
{
	FSMesh& mesh = *obj->GetFEMesh();

	int NF = mesh.Faces();
	int NN = mesh.Nodes();

	vector<vector<int> > NFT(NN);
	for (int i = 0; i < mesh.Faces(); ++i)
	{
		FSFace& face = mesh.Face(i);
		int nf = 0;
		if (face.Shape() == FE_FACE_QUAD) nf = 4;
		if (face.Shape() == FE_FACE_TRI ) nf = 3;
		for (int j = 0; j < nf; ++j)
		{
			int nj = face.n[j];
			if ((nj >= 0) && (nj < NN))
			{
				NFT[nj].push_back(i);
			}
		}
	}

	int duplicates = 0;
	mesh.TagAllFaces(-1);
	for (int i = 0; i < NF; ++i)
	{
		FSFace& facei = mesh.Face(i);
		if (facei.m_ntag == -1)
		{
			int n0 = facei.n[0];
			if ((n0 >= 0) && (n0 < NN))
			{
				vector<int>& nft = NFT[n0];
				for (int j = 0; j < nft.size(); ++j)
				{
					int nfj = nft[j];
					if (nfj > i)
					{
						FSFace& facej = mesh.Face(nfj);
						if ((facej.m_ntag == -1) && (facej == facei))
						{
							facej.m_ntag = i;
							duplicates++;
						}
					}
				}
			}
		}
	}

	if (duplicates == 0) log("No duplicate faces found.");
	else logError(QString("%1 duplicate faces found.").arg(duplicates));
}

void CDlgMeshDiagnosticsUI::checkDuplicateElements()
{
	FSMesh& mesh = *obj->GetFEMesh();

	int NE = mesh.Elements();
	int NN = mesh.Nodes();

	vector<vector<int> > NET(NN);
	for (int i = 0; i < mesh.Elements(); ++i)
	{
		FSElement& el = mesh.Element(i);
		int ne = 0;
		switch (el.Shape())
		{
		case ELEM_LINE : ne = 2; break;
		case ELEM_TRI  : ne = 3; break;
		case ELEM_QUAD : ne = 4; break;
		case ELEM_TET  : ne = 4; break;
		case ELEM_PENTA: ne = 6; break;
		case ELEM_HEX  : ne = 8; break;
		case ELEM_PYRA : ne = 5; break;
		default:
			assert(false);
		}

		for (int j = 0; j < ne; ++j)
		{
			int nj = el.m_node[j];
			if ((nj >= 0) && (nj < NN))
			{
				NET[nj].push_back(i);
			}
		}
	}

	int duplicates = 0;
	mesh.TagAllElements(-1);
	for (int i = 0; i < NE; ++i)
	{
		FSElement& eli = mesh.Element(i);
		if (eli.m_ntag == -1)
		{
			int n0 = eli.m_node[0];
			if ((n0 >= 0) && (n0 < NN))
			{
				vector<int>& net = NET[n0];
				for (int j = 0; j < net.size(); ++j)
				{
					int nej = net[j];
					if (nej > i)
					{
						FSElement& elj = mesh.Element(nej);
						if ((elj.m_ntag == -1) && elj.is_equal(eli))
						{
							elj.m_ntag = i;
							duplicates++;
						}
					}
				}
			}
		}
	}

	if (duplicates == 0) log("No duplicate elements found.");
	else logError(QString("%1 duplicate elements found.").arg(duplicates));
}

void CDlgMeshDiagnosticsUI::checkDegenerateEdges()
{
	FSMesh& mesh = *obj->GetFEMesh();
	int nerr = 0;
	const int NE = mesh.Edges();
	for (int i=0; i<NE; ++i)
	{
		FSEdge& edge = mesh.Edge(i);
		if (edge.n[0] == edge.n[1])
		{
			nerr++;
		}
	}
	if (nerr == 0) log("No degenerate edges found.");
	else logError(QString("%1 degenerate edges found.").arg(nerr));
}

void CDlgMeshDiagnosticsUI::checkElementNeighbors()
{
	FSMesh& mesh = *obj->GetFEMesh();
	int nerr = 0;
	int NE = mesh.Elements();
	for (int i = 0; i < NE; ++i)
	{
		FSElement& el = mesh.Element(i);
		if (el.IsSolid())
		{
			int nf = el.Faces();
			for (int j = 0; j < nf; ++j)
			{
				int nebr = el.m_nbr[j];
				if ((nebr >= 0) && (nebr >= NE))
				{
					nerr++;
				}
			}
		}
		else if (el.IsShell())
		{
			int ne = el.Edges();
			for (int j = 0; j < ne; ++j)
			{
				int nebr = el.m_nbr[j];
				if ((nebr >= 0) && (nebr >= NE))
				{
					nerr++;
				}
			}
		}
	}

	if (nerr == 0) log("Element neighbors look good.");
	else logError(QString("%1 elements have invalid neighbors.").arg(nerr));
}

void CDlgMeshDiagnosticsUI::checkFaceNeighbors()
{
	FSMesh& mesh = *obj->GetFEMesh();
	int nerr = 0;
	int NF = mesh.Faces();
	for (int i = 0; i < NF; ++i)
	{
		FSFace& face = mesh.Face(i);
		int nf = face.Edges();
		for (int j = 0; j < nf; ++j)
		{
			int nebr = face.m_nbr[j];
			if ((nebr >= 0) && (nebr >= NF))
			{
				nerr++;
			}
		}
	}

	if (nerr == 0) log("Face neighbors look good.");
	else logError(QString("%1 faces have invalid neighbors.").arg(nerr));
}

void CDlgMeshDiagnosticsUI::checkEdgeNeighbors()
{
	FSMesh& mesh = *obj->GetFEMesh();
	vector<int> err;
	int NE = mesh.Edges();
	for (int i = 0; i < NE; ++i)
	{
		FSEdge& edge = mesh.Edge(i);
		for (int j = 0; j < 2; ++j)
		{
			int nebr = edge.m_nbr[j];
			if ((nebr >= 0) && (nebr >= NE))
			{
				err.push_back(i);
			}
			else if (nebr >= 0)
			{
				// an edge neighbor must have the same GID
				FSEdge* pen = mesh.EdgePtr(nebr);
				if (pen == nullptr) err.push_back(i);
				else if (pen->m_gid != edge.m_gid) err.push_back(i);
			}
			else if ((nebr < 0) && (edge.m_gid >= 0))
			{
				// if an edge has no neighbor,
				// then it must have a GNode
				int nj = edge.n[j];
				if ((nj >= 0) && (nj < mesh.Nodes()))
				{
					FSNode& node = mesh.Node(nj);
					if (node.m_gid < 0) err.push_back(i);
				}
				else err.push_back(i);
			}
		}
	}

	if (err.empty()) log("Edge neighbors look good.");
	else
	{
		logError(QString("%1 edges have invalid neighbors.").arg(err.size()));
		if (outLevel > 0)
		{
			for (int i = 0; i < err.size(); ++i)
			{
				FSEdge& edge = mesh.Edge(err[i]);
				logError(QString("Edge %1, nodes =%2, %3, gid = %4, nbr = %5, %6").arg(err[i]+1).arg(edge.n[0]+1).arg(edge.n[1]+1).arg(edge.m_gid).arg(edge.m_nbr[0]+1).arg(edge.m_nbr[1]+1));
			}
		}
	}
}

void CDlgMeshDiagnosticsUI::checkElementFaceTable()
{
	int invalidSolidFaces = 0;
	int invalidShellFaces = 0;
	int invalidSolidShellFaces = 0;
	int elemFaces = 0; 
	FSMesh& mesh = *obj->GetFEMesh();
	int NE = mesh.Elements();
	int NF = mesh.Faces();
	for (int i = 0; i < NE; ++i)
	{
		FSElement& el = mesh.Element(i);
		if (el.IsSolid())
		{
			int nf = el.Faces();
			for (int j = 0; j < nf; ++j)
			{
				int nfj = el.m_face[j];
				FSElement_* pej = mesh.ElementPtr(el.m_nbr[j]);

				// Make sure the face is valid
				if (nfj >= NF) invalidSolidFaces++;

				// An element can only have a face if it does not have a neighbor
				// or if the neighbor belongs to a different part.
				if ((nfj >= 0) && (nfj < NF))
				{
					if (pej && (pej->m_gid == el.m_gid)) invalidSolidFaces++;
					else
					{
						// make sure that this face's element is this element
						FSFace& f = mesh.Face(nfj);
						if (pej == nullptr)
						{
							// If there is no neighbor, then the face only should have elem[0] set
							if (f.m_elem[0].eid != i) invalidSolidFaces++;
							if (f.m_elem[0].lid != j) invalidSolidFaces++;
							if (f.m_elem[1].eid != -1) invalidSolidFaces++;
						}
						else if (pej->IsSolid())
						{
							// If the neighbor is a solid, then the face can be either elem.
							if (f.m_elem[0].eid == i)
							{
								if (f.m_elem[0].lid != j) invalidSolidFaces++;
							}
							else if (f.m_elem[1].eid == i)
							{
								if (f.m_elem[1].lid != j) invalidSolidFaces++;
							}
							else invalidSolidFaces++;
						}
						else if (pej->IsShell())
						{
							// The face should be elem[1] or elem[2] (since elem[0] is the shell)
							if (f.m_elem[0].eid == i) invalidSolidShellFaces++;
							else if (f.m_elem[1].eid == i)
							{
								if (f.m_elem[1].lid != j) invalidSolidShellFaces++;
							}
							else if (f.m_elem[2].eid == i)
							{
								if (f.m_elem[2].lid != j) invalidSolidShellFaces++;
							}
							else invalidSolidShellFaces++;
						}
					}
				}
				else if (nfj < 0)
				{
					if ((pej == nullptr) || (pej->m_gid != el.m_gid)) invalidSolidFaces++;
				}

				if ((pej == nullptr) || (pej->IsSolid() && (pej->m_gid > el.m_gid))) elemFaces++;
			}
		}
		else if (el.IsShell())
		{
			// a shell should always have a face
			int nf0 = el.m_face[0];
			if ((nf0 < 0) || (nf0 >= NF)) invalidShellFaces++;
			else
			{
				// make sure that the face's element is this element
				FSFace& f = mesh.Face(nf0);
				if (f.m_elem[0].eid != i) invalidShellFaces++;
			}

			elemFaces++;
		}
	}

	int nerrs = 0;
	nerrs += invalidSolidFaces;
	nerrs += invalidShellFaces;
	nerrs += invalidSolidShellFaces;

	if ((nerrs == 0) && (elemFaces == mesh.Faces())) log("Element-face table looks good.");
	if (invalidSolidFaces > 0) logError(QString("%1 invalid solid faces found when checking element-face table.").arg(invalidSolidFaces));
	if (invalidShellFaces > 0) logError(QString("%1 invalid shell faces found when checking element-face table.").arg(invalidShellFaces));
	if (invalidSolidShellFaces > 0) logError(QString("%1 invalid solid-shell faces found when checking element-face table.").arg(invalidSolidShellFaces));
	if (elemFaces != mesh.Faces()) logError(QString("Element faces (%1) does not match actual faces (%2)").arg(elemFaces).arg(mesh.Faces()));
}

void CDlgMeshDiagnosticsUI::checkElementPartitioning()
{
	FSMesh& mesh = *obj->GetFEMesh();
	int ng = mesh.CountElementPartitions();
	vector<int> lut(ng, 0);
	int invalidGID = 0;
	for (int i = 0; i < mesh.Elements(); ++i)
	{
		FSElement& el = mesh.Element(i);
		int gid = el.m_gid;
		if ((gid >= 0) && (gid < ng)) lut[gid]++;
		else invalidGID++;
	}

	bool berr = false;
	if (invalidGID > 0) { berr = true; logError(QString("%1 elements have an invalid partition ID.").arg(invalidGID)); }
	for (int i = 0; i < ng; ++i)
	{
		if (lut[i] == 0) {
			berr = true;
			logWarning(QString("element partition %1 has zero elements.").arg(i));
		}
	}
	if (ng != obj->Parts()) {
		berr = true;
		logError(QString("Number of element partitions (%1) does not match number of parts (%2).").arg(ng).arg(obj->Parts()));
	}
	if (berr == false) log("Element partitions look good.");
}

void CDlgMeshDiagnosticsUI::checkFacePartitioning()
{
	FSMesh& mesh = *obj->GetFEMesh();
	int ng = mesh.CountFacePartitions();
	vector<int> lut(ng, 0);
	int invalidGID = 0;
	for (int i = 0; i < mesh.Faces(); ++i)
	{
		FSFace& face = mesh.Face(i);
		int gid = face.m_gid;
		if ((gid >= 0) && (gid < ng)) lut[gid]++;
		else invalidGID++;
	}

	bool berr = false;
	if (invalidGID > 0) { berr = true; logError(QString("%1 faces have an invalid partition ID.").arg(invalidGID)); }
	for (int i = 0; i < ng; ++i)
	{
		if (lut[i] == 0) {
			berr = true;
			logWarning(QString("face partition %1 has zero faces.").arg(i));
		}
	}
	if (ng != obj->Faces()) {
		berr = true;
		logError(QString("Number of face partitions (%1) does not match number of surfaces (%2).").arg(ng).arg(obj->Faces()));
	}

	// make sure that each surface has a valid part ID
	int invalidPIDs = 0;
	for (int i = 0; i < obj->Faces(); ++i)
	{
		GFace* pg = obj->Face(i);
		if (pg->m_nPID[0] < 0) invalidPIDs++;
	}
	if (invalidPIDs > 0) {
		berr = true;
		logError(QString("%1 surfaces with invalid part ID 0").arg(invalidPIDs));
	}

	// see if the face-elem data is consistent with the surface-part data
	int invalidSurfaces = 0;
	for (int i = 0; i < mesh.Faces(); ++i)
	{
		FSFace& face = mesh.Face(i);
		int fid = face.m_gid;
		if ((fid >= 0) && (fid < obj->Faces()))
		{
			FSElement_* pe0 = mesh.ElementPtr(face.m_elem[0].eid);
			FSElement_* pe1 = mesh.ElementPtr(face.m_elem[1].eid);
			FSElement_* pe2 = mesh.ElementPtr(face.m_elem[2].eid);

			int pid[3];
			pid[0] = (pe0 ? pe0->m_gid : -1);
			pid[1] = (pe1 ? pe1->m_gid : -1);
			pid[2] = (pe2 ? pe2->m_gid : -1);

			GFace* pg = obj->Face(fid);

			// the first part-ID should always match
			if (pg->m_nPID[0] != pid[0]) invalidSurfaces++;

			// the other ones, I'm not sure if they would appear in the same order 
			if ((pid[1] >= 0) && ((pid[1] != pg->m_nPID[1]) && (pid[1] != pg->m_nPID[2]))) invalidSurfaces++;
			if ((pid[2] >= 0) && ((pid[2] != pg->m_nPID[1]) && (pid[2] != pg->m_nPID[2]))) invalidSurfaces++;

			if ((pid[1] < 0) && (pg->m_nPID[1] >= 0)) invalidSurfaces++;
			if ((pid[2] < 0) && (pg->m_nPID[2] >= 0)) invalidSurfaces++;
		}
	}
	if (invalidSurfaces) {
		berr = true;
		logError(QString("%1 surfaces with inconsistent PIDs").arg(invalidSurfaces));
	}

	if (berr == false) log("Face partitions look good.");
}

void CDlgMeshDiagnosticsUI::checkEdgePartitioning()
{
	FSMesh& mesh = *obj->GetFEMesh();
	int ng = mesh.CountEdgePartitions();
	vector<int> lut(ng, 0);
	int invalidGID = 0;
	for (int i = 0; i < mesh.Edges(); ++i)
	{
		FSEdge& edge = mesh.Edge(i);
		int gid = edge.m_gid;
		if ((gid >= 0) && (gid < ng)) lut[gid]++;
		else if (gid >= ng) invalidGID++;
	}

	bool berr = false;
	if (invalidGID > 0) { berr = true; logError(QString("%1 edges have an invalid partition ID.").arg(invalidGID)); }
	for (int i = 0; i < ng; ++i)
	{
		if (lut[i] == 0) {
			berr = true;
			logWarning(QString("edge partition %1 has zero edges.").arg(i));
		}
	}
	if (ng != obj->Edges()) {
		berr = true;
		logError(QString("Number of edge partitions (%1) does not match number of curves (%2).").arg(ng).arg(obj->Edges()));
	}
	if (berr == false) log("Edge partitions look good.");
}

void CDlgMeshDiagnosticsUI::checkNodePartitioning()
{
	FSMesh& mesh = *obj->GetFEMesh();
	int ng = mesh.CountNodePartitions();
	vector<int> lut(ng, 0);
	int invalidGID = 0;
	for (int i = 0; i < mesh.Nodes(); ++i)
	{
		FSNode& node = mesh.Node(i);
		int gid = node.m_gid;
		if ((gid >= 0) && (gid < ng)) lut[gid]++;
		else if (gid >= ng) invalidGID++;
	}

	bool berr = false;
	if (invalidGID > 0) { berr = true; logError(QString("%1 nodes have an invalid partition ID.").arg(invalidGID)); }
	for (int i = 0; i < ng; ++i)
	{
		if (lut[i] == 0) {
			berr = true;
			logWarning(QString("node partition %1 has zero nodes.").arg(i));
		}
	}
	if (ng != obj->Nodes()) {
		berr = true;
		logError(QString("Number of node partitions (%1) does not match number of vertices (%2).").arg(ng).arg(obj->Nodes()));
	}

	if (berr == false) log("Node partitions look good.");
}

void CDlgMeshDiagnosticsUI::checkSlivers()
{
	const int T[4][4] = {
		{0,1,2,3},
		{0,3,1,2},
		{0,2,3,1},
		{1,3,2,0}
	};

	vector<int> slivers;
	FSMesh& mesh = *obj->GetFEMesh();
	for (int i = 0; i < mesh.Elements(); ++i)
	{
		FSElement& el = mesh.Element(i);
		if (el.Shape() == ELEM_TET)
		{
			// get the max edge length
			double Lmax = 0;
			for (int i=0; i<4; ++i)
				for (int j = 1; j < 4; ++j)
				{
					vec3d a = mesh.Node(el.m_node[i]).pos();
					vec3d b = mesh.Node(el.m_node[j]).pos();

					double lij = (b - a).SqrLength();
					if (lij > Lmax) Lmax = lij;
				}
			Lmax = sqrt(Lmax);

			// pick the plane with the largest area
			double Amax = 0.0;
			int imax = -1;
			for (int i = 0; i < 4; ++i)
			{
				vec3d r0 = mesh.Node(el.m_node[T[i][0]]).pos();
				vec3d r1 = mesh.Node(el.m_node[T[i][1]]).pos();
				vec3d r2 = mesh.Node(el.m_node[T[i][2]]).pos();

				double Ai = ((r1 - r0) ^ (r2 - r0)).SqrLength();
				if (Ai >= Amax) { Amax = Ai; imax = i; }
			}
			assert(imax != -1);

			// create a plane from the first 3 points
			vec3d r0 = mesh.Node(el.m_node[T[imax][0]]).pos();
			vec3d r1 = mesh.Node(el.m_node[T[imax][1]]).pos();
			vec3d r2 = mesh.Node(el.m_node[T[imax][2]]).pos();
			vec3d r3 = mesh.Node(el.m_node[T[imax][3]]).pos();

			vec3d N = (r1 - r0) ^ (r2 - r0); N.Normalize();

			// see if the fourth point is (approximately) on this plane. 
			double l = ((r3 - r0) * N)/Lmax;

			if (l < 1e-5) slivers.push_back(i);
		}
	}

	if (slivers.empty()) log("No tet slivers found.");
	else logWarning(QString("%1 tet slivers found!").arg(slivers.size()));
}
