/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2020 University of Utah, The Trustees of Columbia University in 
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

#include <Python.h>
#include <FEBioStudio/FEBioStudio.h>
#include <FEBioStudio/MainWindow.h>
#include <FEBioStudio/FileThread.h>
#include <FEBioStudio/ModelDocument.h>
#include <MeshIO/FileReader.h>
#include <MeshTools/GModel.h>
#include <GeomLib/GMeshObject.h>
#include "PythonTool.h"
#include <FEBioStudio/PythonToolsPanel.h>


#include <iostream>

typedef struct {
    PyObject_HEAD
    CDocument* doc;
    /* Type-specific fields go here. */
} pyDocumentObject;

static PyTypeObject pyDocumentType = {
    PyVarObject_HEAD_INIT(NULL, 0)};

static void pyDocumentType_init()
{
    pyDocumentType.tp_name = "fbs.Document";
    pyDocumentType.tp_doc = "Document class";
    pyDocumentType.tp_basicsize = sizeof(pyDocumentObject);
    pyDocumentType.tp_itemsize = 0;
    pyDocumentType.tp_flags = Py_TPFLAGS_DEFAULT;
    pyDocumentType.tp_new = PyType_GenericNew;
}

typedef struct {
    PyObject_HEAD
    GDiscreteSpringSet* set;
    /* Type-specific fields go here. */
} pySpringSet;

static PyTypeObject pySpringSetType = {
    PyVarObject_HEAD_INIT(NULL, 0)};



static int pySprintSet_init(pySpringSet* self, PyObject *args)
{
    const char* name;

    if(!PyArg_ParseTuple(args, "s", &name)) return -1;


    auto wnd = PRV::getMainWindow();
    auto doc = dynamic_cast<CModelDocument*>(wnd->GetDocument());
    auto gmodel = doc->GetGModel();

    self->set = new GDiscreteSpringSet(gmodel);
    self->set->SetMaterial(new FELinearSpringMaterial);
    self->set->SetName(name);

    gmodel->AddDiscreteObject(self->set);

    return 0;
}

static PyObject * pySpringSet_addSpring(pySpringSet *self, PyObject *args)
{
    int n0, n1;

    if(!PyArg_ParseTuple(args, "ii", &n0, &n1)) return NULL;

    self->set->AddElement(n0, n1);

    Py_RETURN_NONE;
}

static PyMethodDef pySpringSet_methods[] = {
    {"addSpring", (PyCFunction) pySpringSet_addSpring, METH_VARARGS,
     "Add a spring to the spring group"
    },
    {NULL}  /* Sentinel */
};

static void pySpringSetType_init()
{
    pySpringSetType.tp_name = "fbs.SpringSet";
    pySpringSetType.tp_doc = "Discrete Element set class";
    pySpringSetType.tp_basicsize = sizeof(pySpringSet);
    pySpringSetType.tp_itemsize = 0;
    pySpringSetType.tp_flags = Py_TPFLAGS_DEFAULT;
    pySpringSetType.tp_new = PyType_GenericNew;
    pySpringSetType.tp_init = (initproc) pySprintSet_init;
    pySpringSetType.tp_methods = pySpringSet_methods;
}


static PyObject* pyFindOrMakeNode(PyObject *self, PyObject *args)
{
    double x, y, z, tol;

    if(!PyArg_ParseTuple(args, "dddd", &x, &y, &z, &tol)) return NULL;

    vec3d r(x,y,z);

    auto wnd = PRV::getMainWindow();
    auto doc = dynamic_cast<CModelDocument*>(wnd->GetDocument());
    // auto po = dynamic_cast<GMeshObject*>(doc->GetActiveObject());
    auto po = dynamic_cast<GMeshObject*>(doc->GetActiveObject());

	// find closest node
	int imin = -1;
	double l2min = 0.0;
	FEMesh* m = po->GetFEMesh();
	int N = m->Nodes();
	imin = -1;
	for (int i = 0; i < N; ++i)
	{
		FENode& ni = m->Node(i);
		if (ni.IsExterior())
		{
			vec3d ri = m->LocalToGlobal(ni.r);

			double l2 = (r - ri).SqrLength();
			if ((imin == -1) || (l2 < l2min))
			{
				imin = i;
				l2min = l2;
			}
		}
	}
	if ((imin!=-1) && (l2min < tol*tol))
	{
		return Py_BuildValue("i", po->MakeGNode(imin));
	}

    return Py_BuildValue("i", po->AddNode(r));
}

//// Python Tool
typedef struct {
    PyObject_HEAD
    CPythonTool* tool;
    /* Type-specific fields go here. */
} pyPythonTool;

static PyTypeObject pyPythonToolType = {
    PyVarObject_HEAD_INIT(NULL, 0)};


static int pyPythonTool_init(pyPythonTool* self, PyObject *args)
{
    const char* name;
    PyObject* func;

    if(!PyArg_ParseTuple(args, "sO", &name, &func)) return -1;

    auto wnd = PRV::getMainWindow();
    CPythonToolsPanel* pythonToolsPanel = wnd->GetPythonToolsPanel();

    self->tool = pythonToolsPanel->addTool(name, func);

    return 0;
}

static PyObject * pyPythonTool_addBoolProperty(pyPythonTool *self, PyObject *args)
{
    const char* name;
    bool bl;

    if(!PyArg_ParseTuple(args, "sp", &name, &bl)) return NULL;

    self->tool->addBoolProperty(bl, std::string(name));

    Py_RETURN_NONE;
}

static PyObject * pyPythonTool_addIntProperty(pyPythonTool *self, PyObject *args)
{
    const char* name;
    int integer;

    if(!PyArg_ParseTuple(args, "si", &name, &integer)) return NULL;

    self->tool->addIntProperty(integer, std::string(name));

    Py_RETURN_NONE;
}

static PyObject * pyPythonTool_addDoubleProperty(pyPythonTool *self, PyObject *args)
{
    const char* name;
    double dbl;

    if(!PyArg_ParseTuple(args, "sd", &name, &dbl)) return NULL;

    self->tool->addDoubleProperty(dbl, std::string(name));

    Py_RETURN_NONE;
}

static PyObject * pyPythonTool_addStringProperty(pyPythonTool *self, PyObject *args)
{
    const char* name;
    const char* str;

    if(!PyArg_ParseTuple(args, "ss", &name, &str)) return NULL;

    self->tool->addStringProperty(str, std::string(name));

    Py_RETURN_NONE;
}

static PyObject * pyPythonTool_addResourceProperty(pyPythonTool *self, PyObject *args)
{
    const char* name;
    const char* str;

    if(!PyArg_ParseTuple(args, "ss", &name, &str)) return NULL;

    self->tool->addResourceProperty(str, std::string(name));

    Py_RETURN_NONE;
}

static PyObject * pyPythonTool_finalize(pyPythonTool *self, PyObject *args)
{
    auto wnd = PRV::getMainWindow();
    CPythonToolsPanel* pythonToolsPanel = wnd->GetPythonToolsPanel();

    pythonToolsPanel->finalizeTool(self->tool);

    Py_RETURN_NONE;
}

static PyMethodDef pyPythonTool_methods[] = {
    {"addBoolProperty", (PyCFunction) pyPythonTool_addBoolProperty, METH_VARARGS,
     "Add a boolean property to the tool"
    },
    {"addIntProperty", (PyCFunction) pyPythonTool_addIntProperty, METH_VARARGS,
     "Add an integer property to the tool"
    },
    {"addDoubleProperty", (PyCFunction) pyPythonTool_addDoubleProperty, METH_VARARGS,
     "Add a double property to the tool"
    },
    {"addStringProperty", (PyCFunction) pyPythonTool_addStringProperty, METH_VARARGS,
     "Add a string property to the tool"
    },
    {"addResourceProperty", (PyCFunction) pyPythonTool_addResourceProperty, METH_VARARGS,
     "Add a resource property to the tool"
    },
    {"finalize", (PyCFunction) pyPythonTool_finalize, METH_VARARGS,
     "Finalize the tool and add it to the GUI"
    },
    {NULL}  /* Sentinel */
};

static void pyPythonToolType_init()
{
    pyPythonToolType.tp_name = "fbs.PythonTool";
    pyPythonToolType.tp_doc = "Represents a tool added to the GUI";
    pyPythonToolType.tp_basicsize = sizeof(pyPythonTool);
    pyPythonToolType.tp_itemsize = 0;
    pyPythonToolType.tp_flags = Py_TPFLAGS_DEFAULT;
    pyPythonToolType.tp_new = PyType_GenericNew;
    pyPythonToolType.tp_init = (initproc) pyPythonTool_init;
    pyPythonToolType.tp_methods = pyPythonTool_methods;
}



//// I/O

static PyObject*
pyOpenFile(PyObject *self, PyObject *args)
{
    const char *fileName;

    if(!PyArg_ParseTuple(args, "s", &fileName))
        return NULL;

    PRV::getMainWindow()->OpenFile(fileName);

    Py_RETURN_NONE;
}

static PyObject*
pyImportFile(PyObject *self, PyObject *args)
{
    const char *fileName;

    if(!PyArg_ParseTuple(args, "s", &fileName))
        return NULL;

    auto wnd = PRV::getMainWindow();
    auto doc = wnd->GetDocument();
    FileReader* fileReader = wnd->CreateFileReader(fileName);

    QueuedFile qfile(doc, fileName, fileReader, 0);

    wnd->ReadFile(qfile);

    Py_RETURN_NONE;
}

static PyObject*
pyNewDocument(PyObject *self, PyObject *args)
{
    const char *fileName;

    if(!PyArg_ParseTuple(args, "s", &fileName))
        fileName = "New Model";

    auto wnd = PRV::getMainWindow();
    auto doc = wnd->CreateNewDocument();
    doc->SetDocTitle(fileName);
    wnd->SetActiveDocument(doc);

    Py_RETURN_NONE;
}

static PyMethodDef fbsMethods[] = {
    {"openFile", pyOpenFile, METH_VARARGS, "Open the specified file in FEBio Studio."},
    {"importFile", pyImportFile, METH_VARARGS,"Import the specified file in FEBio Studio."},
    {"newDocument", pyNewDocument, METH_VARARGS,"Create a new document specified file in FEBio Studio."},
    {"findOrMakeNode", pyFindOrMakeNode, METH_VARARGS, "Find a note near the coordintes or make one"},
    {NULL, NULL, 0, NULL}
};

static PyModuleDef fbsModule = {
    PyModuleDef_HEAD_INIT,
    .m_name = "fbs",
    .m_doc = "Example module that creates an extension type.",
    .m_size = -1,
    .m_methods = fbsMethods,
};

PyMODINIT_FUNC
PyInit_fbs(void)
{
    PyObject *m;

    pyDocumentType_init();
    pySpringSetType_init();
    pyPythonToolType_init();


    if (PyType_Ready(&pyDocumentType) < 0)
        return NULL;

    if (PyType_Ready(&pySpringSetType) < 0)
        return NULL;

    if (PyType_Ready(&pyPythonToolType) < 0)
        return NULL;

    m = PyModule_Create(&fbsModule);
    if (m == NULL)
        return NULL;

    Py_INCREF(&pyDocumentType);
    if (PyModule_AddObject(m, "Document", (PyObject *) &pyDocumentType) < 0) {
        Py_DECREF(&pyDocumentType);
        Py_DECREF(m);
        return NULL;
    }

    Py_INCREF(&pySpringSetType);
    if (PyModule_AddObject(m, "SpringSet", (PyObject *) &pySpringSetType) < 0) {
        Py_DECREF(&pySpringSetType);
        Py_DECREF(m);
        return NULL;
    }

    Py_INCREF(&pyPythonToolType);
    if (PyModule_AddObject(m, "PythonTool", (PyObject *) &pyPythonToolType) < 0) {
        Py_DECREF(&pyPythonToolType);
        Py_DECREF(m);
        return NULL;
    }

    return m;
}