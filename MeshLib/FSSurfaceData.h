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
#include "FSMeshData.h"
#include <GeomLib/FSGroup.h>
#include <vector>

class FSSurfaceData : public FSMeshData
{
public:
	FSSurfaceData(FSMesh* mesh = nullptr);
	FSSurfaceData(FSMesh* mesh, DATA_TYPE dataType, DATA_FORMAT dataFormat);
	~FSSurfaceData();

	void Create(FSMesh* mesh, FSSurface* surface, DATA_TYPE dataType, DATA_FORMAT dataFormat);

	FSSurface* GetSurface() {return dynamic_cast<FSSurface*>(GetItemList());}

	void SetItemList(FSItemListBuilder* surf, int n = 0) override;

public:
	void Save(OArchive& ar);
	void Load(IArchive& ar);

private:
	FSSurfaceData(const FSSurfaceData& data);
	void operator = (const FSSurfaceData& data);

	void AllocateData();

private:
	int	m_maxNodesPerFacet;
};
