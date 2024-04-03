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

// Data type enum
// NOTE: this is serialized. Don't change order!
enum DATA_TYPE {
	DATA_SCALAR,		// scalar (1 comp)
	DATA_VEC3,			// 3D vector (3 comp)
	DATA_MAT3,			// 3D Matrix (9 comp)

	// from post namespace
	DATA_MAT3S,			// 3D symmetric Matrix (6 comp)
	DATA_MAT3SD,		// 3D diagonal Matrix (3 comp)
	DATA_TENS4S,		// 6D symmetric Matrix (21 comp)
	DATA_ARRAY,			// variable array (see ModelDataField::GetArraySize())
	DATA_ARRAY_VEC3		// variable array of 3D vectors (comp =  3*ModelDataField::GetArraySize())
};

// Data format enum
// NOTE: this is serialized. Don't change order!
enum DATA_FORMAT {
	DATA_ITEM,	// one value per mesh item
	DATA_NODE,	// one value for each node of selection
	DATA_MULT,	// multiple values for each mesh item: one value for each node of that item

	// from post namespace
	DATA_REGION // one value for all the items of the region
};

// Data class 
enum DATA_CLASS {
	NODE_DATA,
	FACE_DATA,
	ELEM_DATA,
	PART_DATA,

	EDGE_DATA,

	// from post namespace
	OBJECT_DATA
};
