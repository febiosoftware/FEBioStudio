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

#ifdef HAS_PYTHON

#ifdef PY_EXTERNAL
#define PY_MODULE_TYPE PYBIND11_MODULE
#else
#define PY_MODULE_TYPE PYBIND11_EMBEDDED_MODULE
#endif

#include <pybind11/pybind11.h>
#include <pybind11/embed.h>

#include "PyFBSCore.h"
#include "PyFBSModel.h"

#ifndef PY_EXTERNAL
#include "PyFBSUI.h"
#endif

PY_MODULE_TYPE(fbs, m)
{
    init_FBSCore(m);

#ifndef PY_EXTERNAL
    init_FBSUI(m);
    init_FBSModel(m);
#endif
}

#endif