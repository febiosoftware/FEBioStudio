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

//-----------------------------------------------------------------------------
// This number has to be the same as the SAVE_VERSION!
#define PVO_VERSION_NUMBER	0x00020000

//-----------------------------------------------------------------------------
#define PVO_VERSION			0x00000001
#define PVO_OBJECT			0x00010000
#define PVO_OBJECT_TYPE		0x00010001
#define PVO_OBJECT_DATA		0x00010002
#define PVO_DISCRETE_OBJECT	0x00010003

//-----------------------------------------------------------------------------
#define PVM_VERSION_NUMBER	0x00020000
#define PVM_VERSION			0x00000001
#define PVM_MATERIAL		0x00010000
#define PVM_MATERIAL_TYPE	0x00010001
#define PVM_MATERIAL_DATA	0x00010002
