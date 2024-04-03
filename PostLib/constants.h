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
// the following macros can be used to identifiy the type of the field
#define IS_NODE_FIELD(m) (((m) >> 16) == 0)
#define IS_FACE_FIELD(m) (((m) >> 16) == 1)
#define IS_ELEM_FIELD(m) (((m) >> 16) == 2)
#define IS_EDGE_FIELD(m) (((m) >> 16) == 4)

#define IS_VALID(m) ((IS_NODE_FIELD(m))||(IS_FACE_FIELD(m))||(IS_ELEM_FIELD(m))||(IS_EDGE_FIELD(m)))

#define FIELD_CODE(n) (((n) & 0xFF00) >> 8)
#define FIELD_COMP(n) ((n) & 0x00FF)
#define BUILD_FIELD(a,b,c) (((a)<<16)|((b)<<8)|(c))
