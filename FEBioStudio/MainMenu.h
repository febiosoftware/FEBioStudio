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
#pragma once
#include <QAction>
#include <QActionGroup>
#include <QString>
#include <QKeySequence>
#include <QMenu>

class CMainWindow;

class CMainMenu
{
public:
	CMainMenu(CMainWindow* wnd);

	QAction* createAction(const QString& title, const QString& name, const QString& iconFile = QString(), bool bcheckable = false, const QKeySequence& key = QKeySequence::UnknownKey);

public:
	QMenu* menuFile;
	QMenu* menuEdit;
	QMenu* menuEditPost;
	QMenu* menuEditTxt;
	QMenu* menuEditXml;
	QMenu* menuPhysics;
	QMenu* menuTools;
	QMenu* menuPost;
	QMenu* menuFEBio;
	QMenu* menuRecord;
	QMenu* menuView;
	QMenu* menuHelp;
	QMenu* menuRecentFiles;
	QMenu* menuRecentProjects;
	QMenu* menuRecentGeomFiles;
	QMenu* menuImportImage;
	QMenu* menuRecentImages;
	QMenu* menuWindows;
	QMenu* menuViews;

	// FILE menu
	QAction* actionNewModel;
	QAction* actionOpen;
	QAction* actionSave;
	QAction* actionExportFE;
	QAction* actionImportGeom;
	QAction* actionExportGeom;
	QAction* actionSnapShot;
	QAction* actionRayTrace;

	// EDIT menu
	QAction* actionUndo;
	QAction* actionRedo;

	// FEBIO menu
	QAction* actionFEBioRun;
	QAction* actionFEBioMonitor;
	QAction* actionFEBioMonitorSettings;
	QAction* actionFEBioContinue;
	QAction* actionFEBioStop;
	QAction* actionFEBioCheck;
	QAction* actionFEBioPause;
	QAction* actionFEBioNext;

	// PHYSICS menu
	QAction* actionAddRigidConnector;
	QAction* actionSoluteTable;
	QAction* actionSBMTable;
	QAction* actionAddReaction;
	QAction* actionAddMembraneReaction;

	// TOOLS menu
	QAction* actionCurveEditor;
	QAction* actionMeshInspector;
	QAction* actionMeshDiagnostic;
	QAction* actionMaterialTest;

	// VIEW menu
	QAction* actionUndoViewChange;
	QAction* actionRedoViewChange;
	QAction* actionShowGVContext;
	QAction* actionZoomSelect;
	QAction* actionZoomExtents;
	QAction* actionViewCapture;
	QAction* actionShowGrid;
	QAction* actionShowMeshLines;
	QAction* actionShowEdgeLines;
	QAction* actionBackfaceCulling;
	QAction* actionViewSmooth;
	QAction* actionShowNormals;
	QAction* actionOrtho;
	QAction* actionFront;
	QAction* actionBack;
	QAction* actionRight;
	QAction* actionLeft;
	QAction* actionTop;
	QAction* actionBottom;
	QAction* actionIsometric;
	QAction* actionOptions;
	QAction* actionRenderMode;
	QAction* actionShowFibers;
	QAction* actionShowMatAxes;
	QAction* actionShowDiscrete;
	QAction* actionShowRigidBodies;
	QAction* actionShowRigidJoints;
	QAction* actionShowRigidLabels;
	QAction* actionToggleLight;
	QAction* actionToggleTagInfo;

	// other actions
	QAction* actionSelectObjects;
	QAction* actionSelectParts;
	QAction* actionSelectSurfaces;
	QAction* actionSelectCurves;
	QAction* actionSelectNodes;
	QAction* actionSelectDiscrete;
	QAction* actionMeasureTool;
	QAction* actionPlaneCutTool;
	QAction* actionPickColor;
	QAction* actionExplodedView;
	QAction* actionSelect;
	QAction* actionRotate;
	QAction* actionTranslate;
	QAction* actionScale;
	QAction* selectRect;
	QAction* selectCircle;
	QAction* selectFree;

	QAction* actionMerge;
	QAction* actionDetach;
	QAction* actionExtract;
	QAction* actionClone;
	QAction* actionCloneGrid;
	QAction* actionCloneRevolve;
	QAction* actionPurge;

	QAction* actionAddProbe;
	QAction* actionAddCurveProbe;
	QAction* actionAddRuler;
	QAction* actionMusclePath;
	QAction* actionPlotGroup;
	QAction* actionGraph;
	QAction* actionScatter;
	QAction* actionSummary;
	QAction* actionStats;
	QAction* actionIntegrate;
	QAction* actionIntegrateSurface;
	QAction* actionImportPoints;
	QAction* actionImportLines;

	QAction* actionPlaneCut;
	QAction* actionMirrorPlane;
	QAction* actionVectorPlot;
	QAction* actionTensorPlot;
	QAction* actionIsosurfacePlot;
	QAction* actionSlicePlot;
	QAction* actionDisplacementMap;
	QAction* actionStreamLinePlot;
	QAction* actionParticleFlowPlot;
	QAction* actionVolumeFlowPlot;
	QAction* actionVectorGlyph;

	QAction* actionImageSlicer;
	QAction* actionVolumeRender;
	QAction* actionMarchingCubes;
	QAction* actionImageWarp;

	QAction* actionAddAttribute;
	QAction* actionAddElement;
	QAction* actionRemoveRow;

	QActionGroup* recentFilesActionGroup;
	QActionGroup* recentProjectsActionGroup;
	QActionGroup* recentFEFilesActionGroup;
	QActionGroup* recentGeomFilesActionGroup;
	QActionGroup* recentImageFilesActionGroup;

private:
	CMainWindow* m_wnd;
};
