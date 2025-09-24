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

#include "stdafx.h"
#include "FESplitModifier.h"
#include <MeshLib/FSFaceEdgeList.h>
#include <MeshLib/FSNodeElementList.h>
using namespace std;

//-----------------------------------------------------------------------------
// Lookup-table for splitting tets into smaller tets
// Note that some combos result in pyramids and wedge elements
// These elements need to be split further by first splitting
// the quad faces. Note that for wedges not all combos can be split
// without introducing a new node.
const int NLT[64][8][6] = {
	{{ 0, 1, 2, 3,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1}},	// 0 - x1
	{{ 0, 4, 2, 3,-1,-1},{ 1, 2, 4, 3,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1}},	// 1 - x1
	{{ 0, 1, 5, 3,-1,-1},{ 2, 0, 5, 3,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1}},	// 2 - x1
	{{ 1, 5, 4, 3,-1,-1},{ 0, 4, 5, 2, 3,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1}},	// 3 - x2
	{{ 0, 1, 6, 3,-1,-1},{ 1, 2, 6, 3,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1}},	// 4 - x1
	{{ 0, 4, 6, 3,-1,-1},{ 1, 2, 6, 4, 3,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1}},	// 5 - x2
	{{ 2, 6, 5, 3,-1,-1},{ 0, 1, 5, 6, 3,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1}},	// 6 - x2
	{{ 0, 4, 6, 3,-1,-1},{ 1, 5, 4, 3,-1,-1},{ 2, 6, 5, 3,-1,-1},{ 4, 5, 6, 3,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1}},	// 7 - x1
	{{ 0, 1, 2, 7,-1,-1},{ 1, 2, 7, 3,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1}},	// 8 - x1
	{{ 0, 7, 4, 2,-1,-1},{ 1, 4, 7, 3, 2,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1}},	// 9 - x2
	{{ 0, 1, 5, 7,-1,-1},{ 1, 5, 7, 3,-1,-1},{ 0, 5, 2, 7,-1,-1},{ 2, 7, 5, 3,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1}},	// 10 -x1
	{{ 2, 7, 5, 3,-1,-1},{ 0, 4, 5, 2, 7,-1},{ 1, 4, 7, 3, 5,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1}},	// 11 - 1T, 2P 
	{{ 0, 1, 6, 7,-1,-1},{ 2, 3, 7, 6, 1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1}},	// 12 - x2
	{{ 0, 4, 6, 7,-1,-1},{ 1, 3, 2, 4, 7, 6},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1}},	// 13 - x8
	{{ 1, 5, 7, 3,-1,-1},{ 0, 1, 5, 6, 7,-1},{ 2, 3, 7, 6, 5,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1}},	// 14 -
	{{ 0, 4, 6, 7,-1,-1},{ 4, 5, 6, 7,-1,-1},{ 1, 4, 7, 3, 5,-1},{ 2, 3, 7, 6, 5,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1}},	// 15 -
	{{ 0, 8, 2, 3,-1,-1},{ 0, 1, 2, 8,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1}},	// 16 - x1
	{{ 1, 2, 4, 8,-1,-1},{ 0, 3, 8, 4, 2,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1}},	// 17 -
	{{ 0, 1, 5, 8,-1,-1},{ 2, 5, 8, 3, 0,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1}},	// 18 -
	{{ 1, 5, 4, 8,-1,-1},{ 0, 2, 3, 4, 5, 8},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1}},	// 19 x8
	{{ 0, 1, 6, 8,-1,-1},{ 0, 8, 6, 3,-1,-1},{ 1, 2, 6, 8,-1,-1},{ 2, 6, 8, 3,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1}},	// 20 *
	{{ 2, 6, 8, 3,-1,-1},{ 0, 3, 8, 4, 6,-1},{ 1, 2, 6, 4, 8,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1}},	// 21 -
	{{ 0, 8, 6, 3,-1,-1},{ 0, 1, 5, 6, 8,-1},{ 2, 5, 8, 3, 6,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1}},	// 22 -
	{{ 1, 5, 4, 8,-1,-1},{ 4, 5, 6, 8,-1,-1},{ 0, 3, 8, 4, 6,-1},{ 2, 5, 8, 3, 6,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1}},	// 23 -
	{{ 2, 7, 8, 3,-1,-1},{ 0, 7, 8, 1, 2,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1}},	// 24 -
	{{ 0, 7, 4, 2,-1,-1},{ 4, 7, 8, 2,-1,-1},{ 3, 8, 7, 2,-1,-1},{ 1, 4, 8, 2,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1}},	// 25 *
	{{ 0, 5, 2, 7,-1,-1},{ 0, 7, 8, 1, 5,-1},{ 2, 5, 8, 3, 7,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1}},	// 26 -
	{{ 1, 5, 4, 8,-1,-1},{ 4, 5, 7, 8,-1,-1},{ 4, 5, 2, 0, 7,-1},{ 2, 5, 8, 3, 7,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1}},	// 27 -
	{{ 1, 2, 6, 8,-1,-1},{ 0, 7, 8, 1, 6,-1},{ 2, 3, 7, 6, 8,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1}},	// 28 -
	{{ 0, 4, 6, 7,-1,-1},{ 4, 6, 7, 8,-1,-1},{ 1, 2, 6, 4, 8,-1},{ 2, 3, 7, 6, 8,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1-1,-1,},{-1,-1,-1,-1-1,-1,}},	// 29 -
	{{ 0, 6, 7, 1, 5, 8},{ 2, 6, 5, 3, 7, 8},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1}},	// 30 -
	{{ 0, 4, 6, 7,-1,-1},{ 1, 5, 4, 8,-1,-1},{ 5, 8, 7, 6, 4,-1},{ 2, 6, 5, 3, 7, 8},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1}},	// 31 -
	{{ 0, 1, 2, 9,-1,-1},{ 0, 1, 9, 3,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1}},	// 32 - x1
	{{ 1, 2, 4, 9,-1,-1},{ 0, 4, 2, 9,-1,-1},{ 1, 9, 4, 3,-1,-1},{ 0, 4, 9, 3,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1}},	// 33 *
	{{ 0, 5, 2, 9,-1,-1},{ 1, 3, 9, 5, 0,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1}},	// 34 -
	{{ 0, 4, 9, 3,-1,-1},{ 0, 4, 5, 2, 9,-1},{ 1, 3, 9, 5, 4,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1}},	// 35 -
	{{ 1, 2, 6, 9,-1,-1},{ 0, 6, 9, 3, 1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1}},	// 36 -
	{{ 1, 9, 4, 3,-1,-1},{ 1, 2, 6, 4, 9,-1},{ 0, 6, 9, 3, 4,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1}},	// 37 -
	{{ 2, 6, 5, 9,-1,-1},{ 0, 3, 1, 6, 9, 5},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1}},	// 38 -
	{{ 2, 6, 5, 9,-1,-1},{ 4, 5, 6, 9,-1,-1},{ 1, 3, 9, 5, 4,-1},{ 0, 6, 9, 3, 4,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1}},	// 39 -
	{{ 1, 9, 7, 3,-1,-1},{ 0, 2, 9, 7, 1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1}},	// 40 -
	{{ 1, 2, 4, 9,-1,-1},{ 1, 4, 7, 3, 9,-1},{ 0, 2, 9, 7, 4,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1}},	// 41 -
	{{ 0, 1, 5, 7,-1,-1},{ 1, 3, 9, 5, 7,-1},{ 0, 2, 9, 7, 5,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1}},	// 42 -
	{{ 1, 5, 4, 3, 9, 7},{ 0, 7, 4, 2, 9, 5},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1}},	// 43 -
	{{ 0, 1, 6, 7,-1,-1},{ 1, 6, 7, 9,-1,-1},{ 1, 9, 7, 3,-1,-1},{ 1, 2, 6, 9,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1}},	// 44 *
	{{ 0, 4, 6, 7,-1,-1},{ 4, 6, 7, 9,-1,-1},{ 1, 2, 6, 4, 9,-1},{ 1, 4, 7, 3, 9,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1}},	// 45 -
	{{ 2, 6, 5, 9,-1,-1},{ 5, 6, 7, 9,-1,-1},{ 0, 1, 5, 6, 7,-1},{ 1, 3, 9, 5, 7,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1}},	// 46 -
	{{ 0, 4, 6, 7,-1,-1},{ 2, 6, 5, 9,-1,-1},{ 4, 7, 9, 5, 6,-1},{ 1, 5, 4, 3, 9, 7},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1}},	// 47 -
	{{ 0, 8, 9, 3,-1,-1},{ 1, 8, 9, 2, 0,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1}},	// 48 -
	{{ 0, 4, 2, 9,-1,-1},{ 1, 8, 9, 2, 4,-1},{ 0, 3, 8, 4, 9,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1}},	// 49 -
	{{ 0, 1, 5, 8,-1,-1},{ 0, 5, 2, 9,-1,-1},{ 0, 8, 9, 3,-1,-1},{ 0, 8, 5, 9,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1}},	// 50 *
	{{ 1, 5, 4, 8,-1,-1},{ 4, 5, 9, 8,-1,-1},{ 0, 4, 5, 2, 9,-1},{ 0, 3, 8, 4, 9,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1}},	// 51 -
	{{ 0, 1, 6, 8,-1,-1},{ 1, 8, 9, 2, 6,-1},{ 0, 6, 9, 3, 8,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1}},	// 52 -
	{{ 1, 4, 8, 2, 6, 9},{ 0, 4, 6, 3, 8, 9},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1}},	// 53 -
	{{ 2, 6, 5, 9,-1,-1},{ 5, 6, 8, 9,-1,-1},{ 0, 1, 5, 6, 8,-1},{ 0, 6, 9, 3, 8,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1}},	// 54 -
	{{ 1, 5, 4, 8,-1,-1},{ 2, 6, 5, 9,-1,-1},{ 4, 6, 9, 8, 5,-1},{ 0, 4, 6, 3, 8, 9},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1}},	// 55 -
	{{ 7, 8, 9, 3,-1,-1},{ 0, 1, 2, 7, 8, 9},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1}},	// 56 -
	{{ 7, 8, 9, 3,-1,-1},{ 4, 7, 8, 9,-1,-1},{ 1, 8, 9, 2, 4,-1},{ 0, 2, 9, 7, 4,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1}},	// 57 -
	{{ 7, 8, 9, 3,-1,-1},{ 5, 7, 8, 9,-1,-1},{ 0, 2, 9, 7, 5,-1},{ 0, 7, 8, 1, 5,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1}},	// 58 -
	{{ 7, 8, 9, 3,-1,-1},{ 1, 5, 4, 8,-1,-1},{ 4, 5, 9, 7, 8,-1},{ 0, 7, 4, 2, 9, 5},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1}},	// 59 -
	{{ 7, 8, 9, 3,-1,-1},{ 6, 7, 8, 9,-1,-1},{ 0, 7, 8, 1, 6,-1},{ 1, 8, 9, 2, 6,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1}},	// 60 -
	{{ 7, 8, 9, 3,-1,-1},{ 0, 4, 6, 7,-1,-1},{ 4, 8, 9, 6, 7,-1},{ 1, 4, 8, 2, 6, 9},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1}},	// 61 -
	{{ 7, 8, 9, 3,-1,-1},{ 2, 6, 5, 9,-1,-1},{ 5, 6, 7, 8, 9,-1},{ 0, 6, 7, 1, 5, 8},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1,-1}},	// 62 -
	{{ 0, 4, 6, 7,-1,-1},{ 4, 1, 5, 8,-1,-1},{ 2, 6, 5, 9,-1,-1},{ 7, 8, 9, 3,-1,-1},{ 4, 8, 6, 7,-1,-1},{ 4, 8, 5, 6,-1,-1},{ 5, 6, 8, 9,-1,-1},{ 6, 7, 8, 9,-1,-1}},	// 63 *
};

//-----------------------------------------------------------------------------
// Helper function for deciding how to split a quad face.
// The facet is split according to the shortest edge or, if the edges are 
// (approximately) the same length, the edge that has the highest node index
// The em[4] contains the nodes numbers, ordered as a loop around the facet
// This function returns 0 for split [0,2], or 1 for split [1,3].
int split_face(FSMesh* pm, int em[4], double tol)
{
	assert(em[0] >= 0);
	assert(em[1] >= 0);
	assert(em[2] >= 0);
	assert(em[3] >= 0);
	assert(em[0] != em[2]);
	assert(em[1] != em[3]);

	int nsplit = -1;

	// get the node coordinates
	vec3d r0 = pm->Node(em[2]).r - pm->Node(em[0]).r;
	vec3d r1 = pm->Node(em[3]).r - pm->Node(em[1]).r;

	// calculate the lengths (squared)
	double l0 = r0*r0; assert(l0 > 0.0);
	double l1 = r1*r1; assert(l1 > 0.0);
	double r = (l1 - l0)/(l0 + l1);
	if (fabs(r) > tol)
	{
		// split along the shortest edge
		if (r > 0) nsplit = 0; else nsplit = 1;
	}
	else
	{
		// diagonals are approximately the same
		// so split according to edge with largest index
		int m0 = (em[0]>em[2]?em[0]:em[2]);
		int m1 = (em[1]>em[3]?em[1]:em[3]);
		assert(m0 != m1);
		if (m0 > m1) nsplit = 0; else nsplit = 1;
	}
	return nsplit;
}

//-----------------------------------------------------------------------------
// evaluate the volume of a tet.
// (used for finding inverted elements)
double tet_volume(FSMesh* pm, int en[4])
{
	vec3d r0 = pm->Node(en[0]).r;
	vec3d r1 = pm->Node(en[1]).r;
	vec3d r2 = pm->Node(en[2]).r;
	vec3d r3 = pm->Node(en[3]).r;

	vec3d a = r1 - r0;
	vec3d b = r2 - r0;
	vec3d c = r3 - r0;
	return ((a^b)*c)/6.0;
}

//-----------------------------------------------------------------------------
// constructor
FETetSplitModifier::FETetSplitModifier() : FEModifier("Split")
{
	m_tol = 1e-12;
}

//-----------------------------------------------------------------------------
FSMesh* FETetSplitModifier::Apply(FSMesh* pm)
{
	// The number of nodes of the original mesh
	int NN0 = pm->Nodes();

	// The number of elements of the original mesh
	int NT0 = pm->Elements();

	// build the edge table of the mesh
	EdgeList ET(*pm);
	int NE = (int) ET.size();

	// build the element-edge table
	FSElementEdgeList EET(*pm, ET);

	// tag all selected elements
	pm->TagAllElements(0);
	int nsel = 0;
	for (int i = 0; i < NT0; i++)
	{
		FSElement& el = pm->Element(i);
		if (el.IsSelected()) { el.m_ntag = 1; nsel++; }
	}
	if (nsel == 0) pm->TagAllElements(1);

	// mark all edges that will be split
	// this will make all selected elements case 63
	vector<int> EM; EM.assign(NE, -1);
	for (int i=0; i<NT0; i++)
	{
		FSElement& el = pm->Element(i);
		if (el.m_ntag == 1)
		{
			vector<int>& EETi = EET[i];
			EM[ EETi[0] ] =  1;
			EM[ EETi[1] ] =  1;
			EM[ EETi[2] ] =  1;
			EM[ EETi[3] ] =  1;
			EM[ EETi[4] ] =  1;
			EM[ EETi[5] ] =  1;
		}
	}

	const int nmethod = 1;
	if (nmethod == 1)
	{
		// this makes sure that all nodes on edges that are split will split of the corner node
		for (int i=0; i<NN0; ++i) pm->Node(i).m_ntag = 0;
		for (int i=0; i<NT0; ++i)
		{
			FSElement& el = pm->Element(i);
			if (el.m_ntag == 1)
			{
				pm->Node(el.m_node[0]).m_ntag = 1;
				pm->Node(el.m_node[1]).m_ntag = 1;
				pm->Node(el.m_node[2]).m_ntag = 1;
				pm->Node(el.m_node[3]).m_ntag = 1;
			}
		}
		for (int i=0; i<NT0; ++i)
		{
			FSElement& el = pm->Element(i);
			if (el.m_ntag != 1)
			{
				vector<int>& EETi = EET[i];
				for (int j=0; j<6; ++j)
				{
					pair<int,int>& ej = ET[ EETi[j] ];
					if ((pm->Node(ej.first ).m_ntag == 1)||
						(pm->Node(ej.second).m_ntag == 1)) EM[ EETi[j] ] = 1;
				}
			}
		}
	}

	// count the number of marked edges
	// Each edge will add a node
	int NN1 = NN0;
	for (int i=0; i<NE; ++i) if (EM[i] == 1) NN1++;

	// index the edges. Each marked edge will create one node
	// and the index will correspond to the node number
	int ni = NN0;
	for (int i=0; i<NE; ++i)
	{
		pair<int, int>& edge = ET[i];
		if (EM[i] == 1) EM[i] = ni++;
	}

	// create a new mesh (just nodes for now)
	FSMesh* pnew = new FSMesh;
	pnew->Create(NN1, 0);

	// copy old nodes
	ni = 0;
	for (int i=0; i<NN0; ++i, ++ni) pnew->Node(i) = pm->Node(i);

	// create edge nodes
	for (int i=0; i<NE; ++i)
	{
		pair<int, int>& edge = ET[i];
		if (EM[i] != -1)
		{
			FSNode& n0 = pm->Node(edge.first);
			FSNode& n1 = pm->Node(edge.second);

			// put the node halfway between the edge nodes
			vec3d r = (n0.r+n1.r)*0.5;
			pnew->Node(ni).r = r;

			ni++;
		}
	}

	// determine the major case numbers
	vector<vector<int> > EC(NT0); // case numbers ([0] = major case, [1..8] = subcase for each primitive)
	for (int i=0; i<NT0; ++i)
	{
		FSElement& el = pm->Element(i);
		vector<int>& EETi = EET[i];

		// determine the major case number
		int ncase = 0;
		if (EM[ EETi[0] ] != -1) ncase |=  1;
		if (EM[ EETi[1] ] != -1) ncase |=  2;
		if (EM[ EETi[2] ] != -1) ncase |=  4;
		if (EM[ EETi[3] ] != -1) ncase |=  8;
		if (EM[ EETi[4] ] != -1) ncase |= 16;
		if (EM[ EETi[5] ] != -1) ncase |= 32;
		EC[i].resize(9);
		EC[i][0] = ncase;
	}

	// list of center nodes for splitting special cases of wedges
	vector<vec3d> CN;

	// count the number of elements that will be created
	int NL[10]; // global node numbers of split tets
	int NT1 = 0;
	for (int i=0; i<NT0; ++i)
	{
		FSElement& el = pm->Element(i);
		vector<int>& EETi = EET[i];

		// get the (global) node numbers
		NL[0] = el.m_node[0];
		NL[1] = el.m_node[1];
		NL[2] = el.m_node[2];
		NL[3] = el.m_node[3];
		NL[4] = EM[ EETi[0] ];
		NL[5] = EM[ EETi[1] ];
		NL[6] = EM[ EETi[2] ];
		NL[7] = EM[ EETi[3] ];
		NL[8] = EM[ EETi[4] ];
		NL[9] = EM[ EETi[5] ];

		// get the splits
		int ncase = EC[i][0];
		const int (*nlt)[6] = NLT[ncase];
		int ne = 0;
		for (int j=0; j<8; ++j)
		{
			const int* nj = nlt[j];
			if (nj[0] == -1) break;

			// check whether this primitive is a tet, pyramid or wedge
			if ((nj[5] == -1)&&(nj[4] == -1))
			{
				// this is a tet so add one element
				ne++;
				EC[i][j+1] = 0;
			}
			else if (nj[5] == -1)
			{
				// this is a pyramid so add two tets
				ne += 2;

				// now determine the subcase by looking at the splits
				int em[4] = {NL[nj[0]], NL[nj[1]], NL[nj[2]], NL[nj[3]]};
				EC[i][j+1] = split_face(pnew, em, m_tol);
			}
			else
			{
				const int FNL[3][4] = {{0,1,4,3},{1,2,5,4},{2,0,3,5}};
				// this is a wedge 
				// we need to determine the subcase to figure out how many test we add
				// Note that for subcase 0 and 7, we also need to add a center node
				int nsubcase = 0;
				for (int k=0; k<3; ++k)
				{
					int em[4] = {NL[nj[FNL[k][0]]], NL[nj[FNL[k][1]]], NL[nj[FNL[k][2]]], NL[nj[FNL[k][3]]]};
					int s = split_face(pnew, em, m_tol);
					if (s) nsubcase |= (1 << k);
				}
				EC[i][j+1] = nsubcase;

				if ((nsubcase == 0)||(nsubcase == 7))
				{
					// for these cases we'll add 8 tets
					ne += 8;

					// and a center node
					vec3d rc(0,0,0);
					for (int k=0; k<6; ++k) rc += pnew->Node(NL[nj[k]]).r;
					rc /= 6.0;
					CN.push_back(rc);
				}
				else
				{
					// for all other case we'll add three tets
					ne += 3;
				}
			}
		}
		assert(ne > 0);
		NT1 += ne;
	}

	// add the center nodes
	NN1 += (int) CN.size();

	// create a new mesh
	pnew->Create(NN1, NT1);

	// add the center nodes (if any)
	for (int i=0; i<(int)CN.size(); ++i) pnew->Node(ni++).r = CN[i];

	// create new elements
	int elem = 0; // element counter
	int cn = NN1 - (int) CN.size(); // index for center nodes of wedges (case 0, 7)
	for (int i=0; i<NT0; ++i)
	{
		FSElement& el = pm->Element(i);
		vector<int>& EETi = EET[i];
		int ncase    = EC[i][0];
		bool bsel = (el.m_ntag == 1);

		// get the global node numbers
		NL[0] = el.m_node[0];
		NL[1] = el.m_node[1];
		NL[2] = el.m_node[2];
		NL[3] = el.m_node[3];
		NL[4] = EM[ EETi[0] ];
		NL[5] = EM[ EETi[1] ];
		NL[6] = EM[ EETi[2] ];
		NL[7] = EM[ EETi[3] ];
		NL[8] = EM[ EETi[4] ];
		NL[9] = EM[ EETi[5] ];

		// create all the tets
		const int (*nlt)[6] = NLT[ncase];
		for (int j=0; j<8; ++j)
		{
			const int* nj = nlt[j];
			if (nj[0] == -1) break;

			int nsubcase = EC[i][j+1];

			if ((nj[5]==-1)&&(nj[4]==-1))
			{
				// this is a tet
				FSElement& ej = pnew->Element(elem++);
				ej.SetType(FE_TET4);
				ej.m_gid = el.m_gid;

				ej.m_node[0] = NL[ nj[0] ]; assert(ej.m_node[0] != -1);
				ej.m_node[1] = NL[ nj[1] ]; assert(ej.m_node[1] != -1);
				ej.m_node[2] = NL[ nj[2] ]; assert(ej.m_node[2] != -1);
				ej.m_node[3] = NL[ nj[3] ]; assert(ej.m_node[3] != -1);

				if (bsel) ej.Select();
			}
			else if (nj[5] == -1)
			{
				assert((nsubcase == 0)||(nsubcase == 1));
				// this is a pyramid so create two tets
				const int IL[2][2][4] = {{{0,1,2,4},{2,3,0,4}},{{0,1,3,4},{2,3,1,4}}};
				const int (*il)[4] = IL[nsubcase];

				int ML[5];
				ML[0] = NL[nj[0]];
				ML[1] = NL[nj[1]];
				ML[2] = NL[nj[2]];
				ML[3] = NL[nj[3]];
				ML[4] = NL[nj[4]];

				for (int k=0; k<2; ++k)
				{
					const int* nk = il[k];
					FSElement& ej = pnew->Element(elem++);
					ej.SetType(FE_TET4);
					ej.m_gid = el.m_gid;
					ej.m_node[0] = ML[ nk[0] ]; assert(ej.m_node[0] != -1);
					ej.m_node[1] = ML[ nk[1] ]; assert(ej.m_node[1] != -1);
					ej.m_node[2] = ML[ nk[2] ]; assert(ej.m_node[2] != -1);
					ej.m_node[3] = ML[ nk[3] ]; assert(ej.m_node[3] != -1);

					if (bsel) ej.Select();
				}
			}
			else 
			{
				// this is a wedge
				const int IL[8][8][4] = {
					{{ 0, 1, 2, 6},{ 3, 5, 4, 6},{ 1, 4, 5, 6},{ 1, 5, 2, 6},{ 0, 4, 1, 6},{ 0, 3, 4, 6},{ 2, 5, 3, 6},{ 0, 2, 3, 6}},
					{{ 0, 1, 2, 3},{ 1, 4, 5, 3},{ 1, 5, 2, 3},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1}},
					{{ 0, 1, 2, 4},{ 0, 3, 4, 2},{ 3, 5, 4, 2},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1}},
					{{ 0, 1, 2, 3},{ 1, 4, 2, 3},{ 3, 5, 4, 2},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1}},
					{{ 0, 1, 2, 5},{ 0, 4, 5, 3},{ 0, 1, 5, 4},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1}},
					{{ 0, 1, 2, 5},{ 0, 1, 5, 3},{ 3, 5, 4, 1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1}},
					{{ 0, 1, 2, 4},{ 0, 4, 5, 3},{ 2, 4, 5, 0},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1},{-1,-1,-1,-1}},
					{{ 0, 1, 2, 6},{ 3, 5, 4, 6},{ 1, 4, 2, 6},{ 2, 4, 5, 6},{ 0, 3, 1, 6},{ 1, 3, 4, 6},{ 0, 5, 3, 6},{ 0, 2, 5, 6}}
				};

				int ML[7];
				ML[0] = NL[nj[0]];
				ML[1] = NL[nj[1]];
				ML[2] = NL[nj[2]];
				ML[3] = NL[nj[3]];
				ML[4] = NL[nj[4]];
				ML[5] = NL[nj[5]];
				ML[6] = -1;
				if ((nsubcase==0)||(nsubcase == 7)) ML[6] = cn++;

				const int (*il)[4] = IL[nsubcase];
				for (int k=0; k<8; ++k)
				{
					const int* nk = il[k];
					if (nk[0] == -1) break;

					FSElement& ej = pnew->Element(elem++);
					ej.SetType(FE_TET4);
					ej.m_gid = el.m_gid;

					ej.m_node[0] = ML[ nk[0] ]; assert(ej.m_node[0] != -1);
					ej.m_node[1] = ML[ nk[1] ]; assert(ej.m_node[1] != -1);
					ej.m_node[2] = ML[ nk[2] ]; assert(ej.m_node[2] != -1);
					ej.m_node[3] = ML[ nk[3] ]; assert(ej.m_node[3] != -1);

					if (bsel) ej.Select();
				}
			}
		}
	}
	assert(elem == pnew->Elements());
	pnew->RebuildMesh();

	return pnew;
}
