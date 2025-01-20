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
#include "U3DFile.h"
#include <assert.h>
using namespace std;

//-----------------------------------------------------------------------------
U3DFile::BLOCK::BLOCK() 
{ 
}

U3DFile::BLOCK::~BLOCK()
{ 
}

//-----------------------------------------------------------------------------
U3DFile::U3DFile()
{
	m_priority = 0;
}

bool U3DFile::Load(FILE* fp)
{
	m_fp = fp;
	if (m_fp == 0) return false;

	// read the file header block
	FILE_HEADER fileHeader;
	if (readFileHeader(fileHeader) == false) { assert(false); return false; }

	// read the blocks
	while (!feof(m_fp) && !ferror(m_fp))
	{
		// read the next block
		BLOCK block;
		if (readBlock(block) == false) { assert(false); return false; }

		// process the file structure blocks
		switch (block.blockType)
		{
		case PriorityUpdate: if (readPriorityUpdateBlock(block) == false) { assert(false); return false; } break;
		case ModifierChain : if (readModifierChainBlock (block) == false) { assert(false); return false; } break;
		case CLODBaseMesh  : if (readCLODBaseMeshBlock  (block) == false) { assert(false); return false; } break;
		}
	}

	// all done
	return true;
}

bool U3DFile::readFileHeader(FILE_HEADER& fileHeader)
{
	BLOCK block;
	if (readBlock(block, 0x00443355) == false) { assert(false); return false; }

	U3DBitStreamRead ar;
	ar.SetDataBlock(block);
	
	ar.ReadI32(fileHeader.version);
	ar.ReadU32(fileHeader.profile);
	ar.ReadU32(fileHeader.declarationSize);
	ar.ReadU64(fileHeader.fileSize);
	ar.ReadU32(fileHeader.encoding);

	return true;
}

bool U3DFile::readPriorityUpdateBlock(BLOCK& block)
{
	if (block.blockType != PriorityUpdate) { assert(false); return false; }

	U3DBitStreamRead ar;
	ar.SetDataBlock(block);
	
	uint32 newPriority;
	ar.ReadU32(newPriority); 

	// The new priority value shall not be less than previous priority value
	if (newPriority < m_priority) { assert(false); return false; }
	m_priority = newPriority;
	return true;
}

bool U3DFile::readModifierChainBlock(BLOCK& block)
{
	if (block.blockType != ModifierChain) { assert(false); return false; }
	
	U3DBitStreamRead ar;
	ar.SetDataBlock(block);

	MODIFIER_CHAIN modChain;
	ar.ReadString(modChain.name);
	ar.ReadU32(modChain.type);
	ar.ReadU32(modChain.attributes);

	if (modChain.attributes & BoundingSphere)
	{
		ar.ReadF32(modChain.boundSphere.x);
		ar.ReadF32(modChain.boundSphere.y);
		ar.ReadF32(modChain.boundSphere.z);
		ar.ReadF32(modChain.boundSphere.radius);
	}

	if (modChain.attributes & BoundingBox)
	{
		ar.ReadF32(modChain.boundBox.xmin);
		ar.ReadF32(modChain.boundBox.ymin);
		ar.ReadF32(modChain.boundBox.zmin);
		ar.ReadF32(modChain.boundBox.xmax);
		ar.ReadF32(modChain.boundBox.ymax);
		ar.ReadF32(modChain.boundBox.zmax);
	}

	ar.AlignTo4Byte();

	ar.ReadU32(modChain.modifierCount);
	
	for (int i=0; i<(int)modChain.modifierCount; ++i)
	{
		BLOCK subBlock;
		ar.ReadBlock(subBlock);
		switch (subBlock.blockType)
		{
		case ViewNode : if (readViewNodeBlock (subBlock) == false) { assert(false); return false; } break;
		case LightNode: if (readLightNodeBlock(subBlock) == false) { assert(false); return false; } break;
		case GroupNode: if (readGroupNodeBlock(subBlock) == false) { assert(false); return false; } break;
		case ModelNode: readModelNodeBlock(subBlock); break;
		case CLODMeshDeclaration: readCLODMeshDeclaration(subBlock); break;
		}

	}

	return true;
}

void readParentNodeData(U3DBitStreamRead& ar)
{
	int parents;
	ar.ReadI32(parents);
	for (int i=0; i<parents; ++i)
	{
		string parentName;
		ar.ReadString(parentName);

		// read the transformation Matrix
		float m;
		for (int i=0; i<16; ++i) ar.ReadF32(m);
	}
}

bool U3DFile::readCLODBaseMeshBlock(BLOCK& block)
{
	if (block.blockType != CLODBaseMesh) { assert(false); return false; }

	U3DBitStreamRead ar;
	ar.SetDataBlock(block);

	string meshName;
	ar.ReadString(meshName);

	uint32 chainIndex;
	ar.ReadU32(chainIndex);

	BASE_MESH_DESCRIPTION mesh;
	ar.ReadU32(mesh.faceCount);
	ar.ReadU32(mesh.positionCount);
	ar.ReadU32(mesh.normalCount);
	ar.ReadU32(mesh.diffuseColorCount);
	ar.ReadU32(mesh.specularColorCount);
	ar.ReadU32(mesh.textureCoordCount);

	for (int i=0; i<(int)mesh.positionCount; ++i)
	{
		float x, y, z;
		ar.ReadF32(x);
		ar.ReadF32(y);
		ar.ReadF32(z);
	}

	for (int i=0; i<(int)mesh.normalCount; ++i)
	{
		float x, y, z;
		ar.ReadF32(x);
		ar.ReadF32(y);
		ar.ReadF32(z);
	}

	for (int i=0; i<(int)mesh.diffuseColorCount; ++i)
	{
		float r,g,b,a;
		ar.ReadF32(r);
		ar.ReadF32(g);
		ar.ReadF32(b);
		ar.ReadF32(a);
	}

	for (int i=0; i<(int)mesh.specularColorCount; ++i)
	{
		float r,g,b,a;
		ar.ReadF32(r);
		ar.ReadF32(g);
		ar.ReadF32(b);
		ar.ReadF32(a);
	}

	for (int i=0; i<(int)mesh.textureCoordCount; ++i)
	{
		float u, v, s, t;
		ar.ReadF32(u);
		ar.ReadF32(v);
		ar.ReadF32(s);
		ar.ReadF32(t);
	}

	for (int i=0; i<(int)mesh.faceCount; ++i)
	{
		uint32 shadingId;
		ar.ReadCompressedU32(1, shadingId);

		SHADING_DESCRIPTION& shade = m_shading[shadingId];

		for (int j=0; j<3; ++j)
		{
			uint32 positionIndex;
			ar.ReadCompressedU32(U3DConstants::StaticFull + mesh.positionCount, positionIndex);

			if ((m_maxMeshDescription.attributes & ExcludeNormals) == 0)
			{
				uint32 normalIndex;
				ar.ReadCompressedU32(U3DConstants::StaticFull + mesh.normalCount, normalIndex);
			}

			if (shade.attributes & UsePerVertexDiffuse)
			{
				uint32 diffuseIndex;
				ar.ReadCompressedU32(U3DConstants::StaticFull + mesh.diffuseColorCount, diffuseIndex);
			}

			if (shade.attributes & UsePerVertexSpecular)
			{
				uint32 specularIndex;
				ar.ReadCompressedU32(U3DConstants::StaticFull + mesh.specularColorCount, specularIndex);
			}

			for (int k=0; k<shade.textureLayerCount; ++k)
			{
				uint32 textureIndex;
				ar.ReadCompressedU32(U3DConstants::StaticFull + mesh.textureCoordCount, textureIndex);
			}
		}
	}


	return true;
}


void U3DFile::readCLODMeshDeclaration(BLOCK& block)
{
	U3DBitStreamRead ar;
	ar.SetDataBlock(block);

	string meshName;
	ar.ReadString(meshName);

	uint32 chainIndex;
	ar.ReadU32(chainIndex);

	// Max mesh description
	ar.ReadU32(m_maxMeshDescription.attributes);
	ar.ReadU32(m_maxMeshDescription.faceCount);
	ar.ReadU32(m_maxMeshDescription.positionCount);
	ar.ReadU32(m_maxMeshDescription.normalCount);
	ar.ReadU32(m_maxMeshDescription.diffuseColorCount);
	ar.ReadU32(m_maxMeshDescription.specularColorCount);
	ar.ReadU32(m_maxMeshDescription.textureCoordCount);
	ar.ReadU32(m_maxMeshDescription.shadingCount);

	m_shading.resize(m_maxMeshDescription.shadingCount);
	for (int i=0; i<(int)m_maxMeshDescription.shadingCount; ++i)
	{
		SHADING_DESCRIPTION& shade = m_shading[i];
		ar.ReadU32(shade.attributes);
		ar.ReadU32(shade.textureLayerCount);
		for (int j=0; j<(int)shade.textureLayerCount; ++j)
		{
			assert(j < MAX_TEXTURE_LAYERS);
			ar.ReadU32(shade.textureCoordDimensions[j]);
		}
		ar.ReadU32(shade.originalShadingId);
	}

	// CLOD Description
	ar.ReadU32(m_maxMeshDescription.minResolution);
	ar.ReadU32(m_maxMeshDescription.maxResolution);

	// Resource description

	// Skeleton description
}

bool U3DFile::readLightNodeBlock(BLOCK& block)
{
	U3DBitStreamRead ar;
	ar.SetDataBlock(block);

	LIGHT_NODE lightNode;
	ar.ReadString(lightNode.name);

	// read parent node data
	readParentNodeData(ar);

	ar.ReadString(lightNode.resource);

	return true;
}

bool U3DFile::readGroupNodeBlock(BLOCK& block)
{
	U3DBitStreamRead ar;
	ar.SetDataBlock(block);

	string groupName;
	ar.ReadString(groupName);

	readParentNodeData(ar);

	return true;
}

void U3DFile::readModelNodeBlock(BLOCK& block)
{
	U3DBitStreamRead ar;
	ar.SetDataBlock(block);

	string modelName;
	ar.ReadString(modelName);

	readParentNodeData(ar);

	string resourceName;
	ar.ReadString(resourceName);

	uint32 visibility;
	ar.ReadU32(visibility);
}

bool U3DFile::readViewNodeBlock(BLOCK& block)
{
	U3DBitStreamRead ar;
	ar.SetDataBlock(block);

	VIEW_NODE view;
	ar.ReadString(view.name);

	// parent noda data
	readParentNodeData(ar);

	// continue reading view node data
	ar.ReadString(view.resourceName);
	ar.ReadU32(view.attributes);
	ar.ReadF32(view.nearClip);
	ar.ReadF32(view.farClip);

	// TODO: This can also be a vector.
	float viewProjection;
	ar.ReadF32(viewProjection);

	ar.ReadF32(view.viewPort.width);
	ar.ReadF32(view.viewPort.height);
	ar.ReadF32(view.viewPort.horizontalPosition);
	ar.ReadF32(view.viewPort.verticalPosition);

	return true;
}

bool U3DFile::readBlock(BLOCK& block, int ntype)
{
	// read block type
	readBytes(&block.blockType, sizeof(int));
	if ((ntype != -1) && (block.blockType != ntype)) { assert(false); return false; }

	// read data size
	readBytes(&block.dataSize, sizeof(int));

	// read meta data size
	readBytes(&block.metaDataSize, sizeof(int));

	// read the block data
	if (block.dataSize > 0)
	{
		int nsize = (block.dataSize % 4 == 0 ? block.dataSize >> 2 : (block.dataSize >> 2) + 1);
		block.data.resize(nsize);
		readBytes(&block.data[0], 4*nsize);
	} 
	else block.data.clear();

	// read the meta data
	if (block.metaDataSize)
	{
		int nsize = (block.metaDataSize % 4 == 0 ? block.metaDataSize >> 2 : (block.metaDataSize >> 2) + 1);
		readBytes(&block.metaData[0], 4*nsize);
	} 
	else block.metaData.clear();

	return true;
}

//=============================================================================
// Swaps the order of bits in a 4-bit code
uint32 U3DConstants::Swap8[] = {0, 8, 4, 12, 2, 10, 6, 14, 1, 9, 5, 13, 3, 11, 7, 15};

//=============================================================================
U3DBitStreamWrite::U3DBitStreamWrite()
{
	m_contextManager = new U3DContextManager;
	m_high = 0x0000FFFF;
	m_low  = 0;
	m_underflow = 0;

	m_data.resize(m_dataSizeIncrement);

	m_dataPosition = 0;
	m_dataLocal = 0;
	m_dataLocalNext = 0;
	m_dataBitOffset = 0;

	m_compressed = false;
}

U3DBitStreamWrite::~U3DBitStreamWrite()
{
	delete m_contextManager;
}

//-----------------------------------------------------------------------------
// Write a U8 to the data block
void U3DBitStreamWrite::WriteU8(uint8_t uValue)
{
	uint32 symbol = (uint32) uValue;
	SwapBits8(symbol);

	bool escape = false;
	WriteSymbol(U3DConstants::Context8, symbol, escape);
}

//-----------------------------------------------------------------------------
// Write a U16 to the data block
void U3DBitStreamWrite::WriteU16(uint16 uValue)
{
	WriteU8((uint8_t)(0x00FF & uValue));
	WriteU8((uint8_t)(0x00FF & (uValue >> 8)));
}

//-----------------------------------------------------------------------------
// Write a U32 to the data block
void U3DBitStreamWrite::WriteU32(uint32 uValue)
{
	WriteU16((uint16) (0x0000FFFF & uValue));
	WriteU16((uint16) (0x0000FFFF & (uValue >> 16)));
}

//-----------------------------------------------------------------------------
// Write a U64 to the data block
void U3DBitStreamWrite::WriteU64(uint64 uValue)
{
	WriteU32((uint32) (0x00000000FFFFFFFF & uValue));
	WriteU32((uint32) (0x00000000FFFFFFFF & (uValue >> 32)));
}

//-----------------------------------------------------------------------------
// Write a I32 to the data block
void U3DBitStreamWrite::WriteI32(int32 iValue)
{
	WriteU32((uint32) iValue);
}

//-----------------------------------------------------------------------------
// Write a F32 to the data block
void U3DBitStreamWrite::WriteF32(float fValue)
{
	WriteU32(*((uint32*)(&fValue)));
}

//-----------------------------------------------------------------------------
// Write a compressed U32 to the data block
void U3DBitStreamWrite::WriteCompressedU32(uint32 context, uint32 uValue)
{
	m_compressed = true;
	bool escape = false;
	if ((context != 0) && (context < U3DConstants::MaxRange))
	{
		WriteSymbol(context, uValue, escape);
		if (escape == true)
		{
			WriteU32(uValue);
			m_contextManager->AddSymbol(context, uValue + (uint32) 1);
		}
	}
	else
	{
		WriteU32(uValue);
	}
}

//-----------------------------------------------------------------------------
// Write a compressed U16 to the data block
void U3DBitStreamWrite::WriteCompressedU16(uint32 context, uint16 uValue)
{
	m_compressed = true;
	bool escape = false;
	if ((context != 0) && (context < U3DConstants::MaxRange))
	{
		WriteSymbol(context, uValue, escape);
		if (escape == true)
		{
			WriteU16(uValue);
			m_contextManager->AddSymbol(context, uValue + (uint32) 1);
		}
	}
	else
	{
		WriteU16(uValue);
	}
}

//-----------------------------------------------------------------------------
// Write a compressed U8 to the data block
void U3DBitStreamWrite::WriteCompressedU8(uint32 context, uint8_t uValue)
{
	m_compressed = true;
	bool escape = false;
	if ((context != 0) && (context < U3DConstants::MaxRange))
	{
		WriteSymbol(context, uValue, escape);
		if (escape == true)
		{
			WriteU8(uValue);
			m_contextManager->AddSymbol(context, uValue + (uint32) 1);
		}
	}
	else
	{
		WriteU8(uValue);
	}
}

//-----------------------------------------------------------------------------
// Stores the data written by the bit stream writer in a data block
void U3DBitStreamWrite::GetDataBlock(U3DFile::BLOCK& dataBlock)
{
	// flush the arithmetic coder
	if (m_compressed)
	{
		WriteU32(0);
	}
	AlignToByte();
	uint32 numBytes = ((uint32) m_dataPosition << 2) + ((uint32) m_dataBitOffset >> 3);

	PutLocal();

	dataBlock.dataSize = numBytes;
//	dataBlock.data = m_data;
}

//-----------------------------------------------------------------------------
// Set the current position to the next uint8_t boundary
void U3DBitStreamWrite::AlignToByte()
{
	uint32 uBitCount = 0;
	GetBitCount(uBitCount);

	uBitCount = (8 - (uBitCount & 7)) & 7;
	m_dataBitOffset += uBitCount;

	if (m_dataBitOffset >= 32)
	{
		m_dataBitOffset -= 32;
		IncrementPosition();
	}
}

//-----------------------------------------------------------------------------
// Set the current position to the next 4 uint8_t boundary
void U3DBitStreamWrite::AlignTo4Byte()
{
	if (m_dataBitOffset > 0)
	{
		m_dataBitOffset = 0;
		IncrementPosition();
	}
}

//-----------------------------------------------------------------------------
// Write the given symbol to the data block in the specified context.
// escape returns as false if the symbol was written successfully.
// escape will return true when writing a dynamically compressed context when
// the symbol to write has not appeared yet in the context's histogram. 
// In this case, the escape symbol, 0, is written.
void U3DBitStreamWrite::WriteSymbol(uint32 context, uint32 symbol, bool& escape)
{
	// value is incremented because the value 0 is the escape symbol
	symbol++;

	escape = false;
	uint32 totalCumFreq = 0;
	uint32 symbolCumFreq = 0;
	uint32 symbolFreq = 0;

	totalCumFreq  = m_contextManager->GetTotalSymbolFrequency(context);
	symbolCumFreq = m_contextManager->GetCumulativeSymbolFrequency(context, symbol);
	symbolFreq    = m_contextManager->GetSymbolFrequency(context, symbol);

	if (symbolFreq == 0)
	{
		// the symbol has not occurred yet
		// write the escape symbol, 0
		symbol = 0;

		symbolCumFreq = m_contextManager->GetCumulativeSymbolFrequency(context, symbol);
		symbolFreq    = m_contextManager->GetSymbolFrequency(context, 0);
	}

	if (symbol == 0)
	{
		// the symbol is the escape symbol
		escape = true;
	}

	uint32 range = m_high + 1 - m_low;

	m_high = m_low - 1 + range*(symbolCumFreq + symbolFreq) / totalCumFreq;
	m_low  = m_low + range*symbolCumFreq / totalCumFreq;

	m_contextManager->AddSymbol(context, symbol);

	// write bits
	uint32 bit = m_low >> 15;

	uint32 highmask = m_high & U3DConstants::HalfMask;
	uint32 lowmask  = m_low  & U3DConstants::HalfMask;

	while ((m_high & U3DConstants::HalfMask) == (m_low & U3DConstants::HalfMask))
	{
		m_high &= ~U3DConstants::HalfMask;
		m_high += m_high + 1;
		WriteBit(bit);

		while (m_underflow > 0)
		{
			m_underflow--;
			WriteBit((~bit) & 1);
		}

		m_low &= ~U3DConstants::HalfMask;
		m_low += m_low;

		bit = m_low >> 15;
	}

	// check for underflow
	// Underflow occurs when the values stored in m_high and m_low differ only in the most significant bit.
	// The precision of the variables is not large enough to predict the next symbol.
	while ((0 == (m_high & U3DConstants::QuarterMask)) &&
		   (U3DConstants::QuarterMask == (m_low & U3DConstants::QuarterMask)))
	{
		m_high &= ~U3DConstants::HalfMask;
		m_high <<= 1;
		m_low <<= 1;
		m_high |= U3DConstants::HalfMask;
		m_high |= 1;
		m_low &= ~U3DConstants::HalfMask;
		m_underflow++;
	}
}

void U3DBitStreamWrite::SwapBits8(uint32& rValue)
{
	rValue = (U3DConstants::Swap8[(rValue) & 0xF] << 4) 
		   | (U3DConstants::Swap8[(rValue) >> 4]);
}

void U3DBitStreamWrite::WriteBit(uint32 bit)
{
	uint32 mask = 1;
	bit &= mask;

	m_dataLocal &= ~(mask << m_dataBitOffset);
	m_dataLocal |=  (bit  << m_dataBitOffset);

	m_dataBitOffset += 1;
	if (m_dataBitOffset >= 32)
	{
		m_dataBitOffset -= 32;
		IncrementPosition();
	}
}

//-----------------------------------------------------------------------------
// Updates the values of the datablock stored in dataLocal and dataLocalNext to the 
// next values in the datablock
void U3DBitStreamWrite::IncrementPosition()
{
	m_dataPosition++;
	CheckPosition();
	m_data[m_dataPosition - 1] = m_dataLocal;
	m_dataLocal     = m_dataLocalNext;
	m_dataLocalNext = m_data[m_dataPosition + 1];
}

//-----------------------------------------------------------------------------
// Store the initial 64 bits of the data block in m_dataLlocal and m_dataLocalNext
void U3DBitStreamWrite::GetLocal()
{
	CheckPosition();
	m_dataLocal     = m_data[m_dataPosition];
	m_dataLocalNext = m_data[m_dataPosition + 1];
}

//-----------------------------------------------------------------------------
// Stores the local values of the data to the data array
void U3DBitStreamWrite::PutLocal()
{
	m_data[m_dataPosition  ] = m_dataLocal;
	m_data[m_dataPosition+1] = m_dataLocalNext;
}

//-----------------------------------------------------------------------------
// Checks that the array allocated for writing is large enough.
// Reallocates if necessary.
void U3DBitStreamWrite::CheckPosition()
{
	if (m_dataPosition + 2 > (int)m_data.size())
	{
		AllocateDataBuffer(m_dataPosition + 2 + m_dataSizeIncrement);
	}
}

//-----------------------------------------------------------------------------
// Creates a new array for storing the data written. 
// Copies values of the old array to the new array.
void U3DBitStreamWrite::AllocateDataBuffer(int32 size)
{
	// store an old buffer if it exists
	if (m_data.empty() == false)
	{
		// I think m_data.resize(size) would do the trick as well
		vector<uint32> tmp(size);
		for (int i=0; i<(int)m_data.size(); ++i) tmp[i] = m_data[i];
		m_data = tmp;
	}
	else 
	{
		m_data.resize(size);
	}
}

//-----------------------------------------------------------------------------
// returns the number of bits written in rCount
void U3DBitStreamWrite::GetBitCount(uint32& rCount)
{
	rCount = (m_dataPosition << 5) + m_dataBitOffset;
}

//=============================================================================

uint32 U3DBitStreamRead::FastNotMask[] = {0x0000FFFF, 0x00007FFF, 0x00003FFF, 0x0001FFF, 0x00000FFF};
uint32 U3DBitStreamRead::ReadCount[] = {4,3,2,2,1,1,1,1,0,0,0,0,0,0,0,0};

U3DBitStreamRead::U3DBitStreamRead()
{
	m_contextManager = new U3DContextManager;
	m_high = 0x0000FFFF;

	m_low = 0;
	m_underflow = 0;
	m_code = 0;
	m_dataLength = 0;
	m_dataPosition = 0;
	m_dataLocal = 0;
	m_dataLocalNext = 0;
	m_dataBitOffset = 0;
}

U3DBitStreamRead::~U3DBitStreamRead()
{
}

//-----------------------------------------------------------------------------
void U3DBitStreamRead::ReadBlock(U3DFile::BLOCK& block)
{
	ReadU32(block.blockType);
	ReadU32(block.dataSize);
	ReadU32(block.metaDataSize);

	if (block.dataSize > 0)
	{
		int nsize = ((block.dataSize % 4) == 0 ? (block.dataSize >> 2) : (block.dataSize >> 2) + 1);
		block.data.resize(nsize);
		for (int i=0; i<nsize; ++i) block.data[i] = m_data[m_dataPosition + i];
		m_dataPosition += nsize;
	}

	if (block.metaDataSize > 0)
	{
		int nsize = ((block.metaDataSize% 4) == 0 ? (block.metaDataSize >> 2) : (block.metaDataSize >> 2) + 1);
		block.metaData.resize(nsize);
		for (int i=0; i<nsize; ++i) block.metaData[i] = m_data[m_dataPosition + i];
		m_dataPosition += nsize;
	}
}

//-----------------------------------------------------------------------------
// Read a U8 from the datablock associated with this bitstream
void U3DBitStreamRead::ReadU8(uint8_t& rValue)
{
	uint32 uValue = 0;
	ReadSymbol(U3DConstants::Context8, uValue);
	uValue--;
	SwapBits8(uValue);
	rValue = (uint8_t) uValue;
}

//-----------------------------------------------------------------------------
// Read a U16 from the datablock
void U3DBitStreamRead::ReadU16(uint16& rValue)
{
	uint8_t low = 0;
	uint8_t high = 0;
	ReadU8(low);
	ReadU8(high);

	rValue = (uint16)(((uint16)low) | (((uint16) high) << 8));
}

//-----------------------------------------------------------------------------
// Read a U32 from the datablock
void U3DBitStreamRead::ReadU32(uint32& rValue)
{
	uint16 low = 0;
	uint16 high = 0;
	ReadU16(low);
	ReadU16(high);

	rValue = ((uint32)low) | (((uint32) high) << 16);
}

//-----------------------------------------------------------------------------
// Read a U64 from the datablock
void U3DBitStreamRead::ReadU64(uint64& rValue)
{
	uint32 low = 0;
	uint32 high = 0;
	ReadU32(low);
	ReadU32(high);

	rValue = ((uint64)low) | (((uint64) high) << 32);
}

//-----------------------------------------------------------------------------
// Read a I32 from the datablock
void U3DBitStreamRead::ReadI32(int32& rValue)
{
	uint32 uValue = 0;
	ReadU32(uValue);
	rValue = (int32) uValue;
}

//-----------------------------------------------------------------------------
// Read a F32 from the datablock
void U3DBitStreamRead::ReadF32(float& rValue)
{
	uint32 uValue = 0;
	ReadU32(uValue);
	rValue = *((float*)(&uValue));
}

//-----------------------------------------------------------------------------
// Reads a string from the data block
void U3DBitStreamRead::ReadString(std::string& s)
{
	uint16 stringSize;
	ReadU16(stringSize);
	if (stringSize > 0)
	{
		s.resize(stringSize);
		for (int i=0; i<stringSize; ++i) 
		{
			uint8_t b;
			ReadU8(b);
			s[i] = (char) b;
		}
	}
	else s.clear();
}

//-----------------------------------------------------------------------------
// Read a compressed U32 from the datablock
void U3DBitStreamRead::ReadCompressedU32(uint32 context, uint32& rValue)
{
	uint32 symbol = 0;
	if ((context != U3DConstants::Context8) && (context < U3DConstants::MaxRange))
	{
		// the context is a compressed context
		ReadSymbol(context, symbol);
		if (symbol != 0)
		{
			// the symbol is compressed
			rValue = symbol - 1;
		}
		else
		{
			// escape character, the symbol was not compressed
			ReadU32(rValue);
			m_contextManager->AddSymbol(context, rValue + 1);
		}
	}
	else
	{
		// the context specified is uncompressed
		ReadU32(rValue);
	}
}

//-----------------------------------------------------------------------------
// Read a compressed U16 from the datablock
void U3DBitStreamRead::ReadCompressedU16(uint32 context, uint16& rValue)
{
	uint32 symbol = 0;
	if ((context != 0) && (context < U3DConstants::MaxRange))
	{
		// the context is a compressed context
		ReadSymbol(context, symbol);
		if (symbol != 0)
		{
			// the symbol is compressed
			rValue = (uint16)(symbol - 1);
		}
		else
		{
			// escape character, the symbol was not compressed
			ReadU16(rValue);
			m_contextManager->AddSymbol(context, rValue + 1);
		}
	}
	else
	{
		// the context specified is uncompressed
		ReadU16(rValue);
	}
}

//-----------------------------------------------------------------------------
// Read a compressed U8 from the datablock
void U3DBitStreamRead::ReadCompressedU8(uint32 context, uint8_t& rValue)
{
	uint32 symbol = 0;
	if ((context != 0) && (context < U3DConstants::MaxRange))
	{
		// the context is a compressed context
		ReadSymbol(context, symbol);
		if (symbol != 0)
		{
			// the symbol is compressed
			rValue = (uint8_t)(symbol - 1);
		}
		else
		{
			// escape character, the symbol was not compressed
			ReadU8(rValue);
			m_contextManager->AddSymbol(context, rValue + 1);
		}
	}
	else
	{
		// the context specified is uncompressed
		ReadU8(rValue);
	}
}

//-----------------------------------------------------------------------------
// Set the data to be read to data and get the first part of the data
// into local variables.
void U3DBitStreamRead::SetDataBlock(U3DFile::BLOCK& dataBlock)
{
	m_data = &dataBlock.data[0];
	m_dataLength = dataBlock.data.size();

	m_dataPosition = 0;
	m_dataBitOffset = 0;
	GetLocal();
}

//-----------------------------------------------------------------------------
// Reverses the bit order of an 8-bit value.
void U3DBitStreamRead::SwapBits8(uint32& rValue)
{
	rValue = (U3DConstants::Swap8[(rValue) & 0xF] << 4) | (U3DConstants::Swap8[(rValue) >> 4]);
}

//-----------------------------------------------------------------------------
// Read a symbol from the datablock using the specified context.
// The symbol 0 represents the escape value and signifies that the next symbol read
// will be uncompressed.
void U3DBitStreamRead::ReadSymbol(uint32 context, uint32& rSymbol)
{
	uint32 uValue = 0;

	// get the current bit position
	uint32 position = 0;
	GetBitCount(position);

	// read the next bit
	ReadBit(m_code);
	m_dataBitOffset += (int32) m_underflow;

	while (m_dataBitOffset >= 32)
	{
		m_dataBitOffset -= 32;
		IncrementPosition();
	}

	// read the next 15 bits
	// Note that this reverse the bit order
	uint32 temp = 0;
	Read15Bits(temp);

	// add them to m_code
	// Since Read15Bits reversed the bit order,
	// the first bit we just read now becomes bit 16;
	m_code <<= 15;
	m_code |= temp;

	// reset the last bit position
	// This is because the actual number of bits to read
	// is determined below (in bitcount)
	SeekToBit(position);

	// get the total count to calculate probabilities
	uint32 totalCumFreq = m_contextManager->GetTotalSymbolFrequency(context);

	// get the cumulative frequency of the current symbol
	uint32 range = m_high + 1 - m_low;

	// the relationship:
	// codeCumFreq <= (totalCumFreq * (m_code - m_low) / range
	// is used to calculate the cumulative frequency of the current symbol.
	// The +1 and -1 in the line below are used to counteract finite word length
	// problems resulting from the division by range
	uint32 codeCumFreq = ((totalCumFreq)*(1 + m_code - m_low) - 1)/(range);

	uValue = m_contextManager->GetSymbolFromFrequency(context, codeCumFreq);

	// update state and context
	uint32 valueCumFreq = m_contextManager->GetCumulativeSymbolFrequency(context, uValue);
	uint32 valueFreq = m_contextManager->GetSymbolFrequency(context, uValue);

	uint32 low = m_low;
	uint32 high = m_high;

	high = low - 1 + range*(valueCumFreq + valueFreq)/totalCumFreq;
	low  = low + range*(valueCumFreq) / totalCumFreq;
	m_contextManager->AddSymbol(context, uValue);

	int32 bitCount;
	uint32 maskedLow;
	uint32 maskedHigh;

	// count bits to read

	// fast count the first 4 bits
	// compare most significant 4 bits of low and high
	bitCount = (int32) ReadCount[((low >> 12) ^ (high >> 12)) & 0x0000000F];

	low &= FastNotMask[bitCount];
	high &= FastNotMask[bitCount];

	high <<= bitCount;
	low <<= bitCount;

	high |= (uint32) ((1 << bitCount) - 1);

	// regular count the rest
	maskedLow  = U3DConstants::HalfMask & low;
	maskedHigh = U3DConstants::HalfMask & high;

	while (((maskedLow | maskedHigh) == 0) || 
			((maskedLow == U3DConstants::HalfMask)
			&& (maskedHigh == U3DConstants::HalfMask)))
	{
		low  = (U3DConstants::NotHalfMask & low) << 1;
		high = ((U3DConstants::NotHalfMask & high) << 1) | 1;
		maskedLow  = U3DConstants::HalfMask & low;
		maskedHigh = U3DConstants::HalfMask & high;
		bitCount++;
	}

	uint32 savedBitsLow  = maskedLow;
	uint32 savedBitsHigh = maskedHigh;

	if (bitCount > 0)
	{
		bitCount += (int32) m_underflow;
		m_underflow = 0;
	}

	// count underflow bits
	maskedLow  = U3DConstants::QuarterMask & low;
	maskedHigh = U3DConstants::QuarterMask & high;

	uint32 underflow = 0;

	while ((maskedLow == 0x4000) && (maskedHigh == 0))
	{
		low  &= U3DConstants::NotThreeQuarterMask;
		high &= U3DConstants::NotThreeQuarterMask;

		low  += low;
		high += high;

		high |= 1;
		maskedLow  = U3DConstants::QuarterMask & low;
		maskedHigh = U3DConstants::QuarterMask & high;
		underflow++;
	}

	// store the state
	m_underflow += underflow;

	low  |= savedBitsLow;
	high |= savedBitsHigh;
	m_low = low;
	m_high = high;

	// update bit read position
	m_dataBitOffset += bitCount;

	while (m_dataBitOffset >= 32)
	{
		m_dataBitOffset -= 32;
		IncrementPosition();
	}

	// set return value
	rSymbol = uValue;
}

//-----------------------------------------------------------------------------
// returns the number of bits read in rCount
// This returns 32*m_dataPosition + m_dataBitOffset
void U3DBitStreamRead::GetBitCount(uint32& rCount)
{
	rCount = (uint32) ((m_dataPosition << 5) + m_dataBitOffset);
}

//-----------------------------------------------------------------------------
// Read the next bit in the data block.
void U3DBitStreamRead::ReadBit(uint32& rValue)
{
	uint32 uValue = 0;
	uValue = m_dataLocal >> m_dataBitOffset;

	uValue &= 1;
	m_dataBitOffset++;
	if (m_dataBitOffset >= 32)
	{
		m_dataBitOffset -= 32;
		IncrementPosition();
	}

	rValue = uValue;
}

//-----------------------------------------------------------------------------
// read the next 15 bits from the datablock
void U3DBitStreamRead::Read15Bits(uint32& rValue)
{
	// read the next 15 bits
	// (This actually reads up to 32 bits, but we only care about the first 15)
	uint32 uValue = m_dataLocal >> m_dataBitOffset;

	// if we fell off the current position, get the 
	// rest from dataLocalNext
	if (m_dataBitOffset > 17)
	{
		uValue |= (m_dataLocalNext << (32 - m_dataBitOffset));
	}

	// this effectively multiplies the value by two
	// or shifts the bits one position to the left
	// (I suspect to make room for the one bit we already read before).
	uValue += uValue;

	// flips the bit order
	// (The 0-bit that was just added now becomes bit 16, which thus should be zero)
	uValue = (U3DConstants::Swap8[(uValue >> 12) & 0xF])
		  | ((U3DConstants::Swap8[(uValue >>  8) & 0xF]) <<  4)
		  | ((U3DConstants::Swap8[(uValue >>  4) & 0xF]) <<  8)
		  | ((U3DConstants::Swap8[ uValue        & 0xF]) << 12);

	// store the (bit reversed) value
	rValue = uValue;

	// increment bit-offset
	m_dataBitOffset += 15;
	if (m_dataBitOffset >= 32)
	{
		m_dataBitOffset -= 32;
		IncrementPosition();
	}
}

//-----------------------------------------------------------------------------
// Updates the values of the data block stored in dataLocal and dataLocalNext
// to the next values in the data block
void U3DBitStreamRead::IncrementPosition()
{
	m_dataPosition++;

	m_dataLocal = m_data[m_dataPosition];
	if (m_dataLength > m_dataPosition + 1)
	{
		m_dataLocalNext = m_data[m_dataPosition + 1];
	}
	else
	{
		m_dataLocalNext = 0;
	}
}

//-----------------------------------------------------------------------------
// Sets the dataLocal, dataLocalNext and bitOffset values so that the next read will
// occur at position in the data block
void U3DBitStreamRead::SeekToBit(uint32 position)
{
	m_dataPosition = position >> 5; // i.e. divide by 32
	m_dataBitOffset = (int32) (position & 0x0000001F); // i.e. modulo 32
	GetLocal();
}

//-----------------------------------------------------------------------------
// This skips the rest of the dataLocal and moves to the next uint8_t aligned value
// TODO: Do I need to do anything with m_underflow?
void U3DBitStreamRead::AlignTo4Byte()
{
	if (m_dataBitOffset != 0)
	{
		IncrementPosition();
		m_dataBitOffset = 0;
	}
}

//-----------------------------------------------------------------------------
// store the initial 64 bits of the data block in dataLocal and dataLocalNext
void U3DBitStreamRead::GetLocal()
{
	m_dataLocal = m_data[m_dataPosition];
	if (m_dataLength > m_dataPosition + 1)
	{
		m_dataLocalNext = m_data[m_dataPosition + 1];
	}
}

//=============================================================================
U3DContextManager::U3DContextManager()
{
	m_symbolCount.resize(U3DConstants::StaticFull);
	m_cumulativeCount.resize(U3DConstants::StaticFull);
}

//-----------------------------------------------------------------------------
// Add an occurrence of the symbol to the specified context.
void U3DContextManager::AddSymbol(uint32 context, uint32 symbol)
{
	if ((context < U3DConstants::StaticFull) && 
		(context != U3DConstants::Context8) &&
		(symbol < MaximumSymbolInHistogram))
	{
		// check if dynamic. Nothing to do if static or if the symbol
		// is larger than the maximum symbol allowed in the histogram

		vector<uint16>& cumulativeCount = m_cumulativeCount[context];
		vector<uint16>& symbolCount = m_symbolCount[context];

		if (cumulativeCount.empty())
		{
			// allocate new arrays if they do not exist
			cumulativeCount.resize(symbol + ArraySizeIncr);
			symbolCount.resize(symbol + ArraySizeIncr);

			// initialize histogram
			m_cumulativeCount[context][0] = 1;
			m_symbolCount[context][0] = 1;
		}
		else if (cumulativeCount.size() <= symbol)
		{ 
			// allocate new arrays if too small
			cumulativeCount.resize(symbol + ArraySizeIncr);
			symbolCount.resize(symbol + ArraySizeIncr);
		}

		if (cumulativeCount[0] >= Elephant)
		{
			// if total number of occurances is larger than Elephant
			// scale down the values to avoid overflow
			int len = cumulativeCount.size();
			uint16 tempAccum = 0;
			for (int i=len -1; i >=0; i--)
			{
				symbolCount[i] >>= 1;
				tempAccum += symbolCount[i];
				cumulativeCount[i] = tempAccum;
			}

			// preserve the initial escape value of 1 
			// for the symbol count and cumulative conut
			symbolCount[0]++;
			cumulativeCount[0]++;
		}

		symbolCount[symbol]++;
		for (int i=0; i<= (int)symbol; ++i)
		{
			cumulativeCount[i]++;
		}
	}
}

//-----------------------------------------------------------------------------
// Get the number of occurrences of the given symbol in the context.
uint32 U3DContextManager::GetSymbolFrequency(uint32 context, uint32 symbol)
{
	// the static case is 1
	uint32 rValue = 1;
	if ((context < U3DConstants::StaticFull) && (context != U3DConstants::Context8))
	{
		// the default for the dynamic case is 0
		rValue = 0;
		if ((m_symbolCount[context].empty() == false)&&(symbol < m_symbolCount[context].size()))
		{
			rValue = (uint32) m_symbolCount[context][symbol];
		}
		else if (symbol == 0)
		{
			// if the histogram hasn't been created yet, the symbol 0 is
			// the escape value and should return 1
			rValue = 1;
		}
	}

	return rValue;
}

//-----------------------------------------------------------------------------
// Get the total number of occurrences for all of the symbols that are less
// than the given symbol in the context.
uint32 U3DContextManager::GetCumulativeSymbolFrequency(uint32 context, uint32 symbol)
{
	// the static case is just the value of the symbol
	uint32 rValue = symbol-1;
	if ((context < U3DConstants::StaticFull) && (context != U3DConstants::Context8))
	{
		rValue = 0;
		if (m_cumulativeCount[context].empty() == false)
		{
			if (symbol < m_cumulativeCount[context].size())
			{
				rValue = (uint32) (m_cumulativeCount[context][0] - m_cumulativeCount[context][symbol]);
			}
			else
				rValue = (uint32) (m_cumulativeCount[context][0]);
		}
	}

	return rValue;
}

//-----------------------------------------------------------------------------
// Get the total occurrences of all the symbols in this context.
uint32 U3DContextManager::GetTotalSymbolFrequency(uint32 context)
{
	if ((context < U3DConstants::StaticFull) && (context != U3DConstants::Context8))
	{
		uint32 rValue = 1;
		if (m_cumulativeCount[context].empty() == false)
			rValue = m_cumulativeCount[context][0];
		return rValue;
	}
	else
	{
		if (context == U3DConstants::Context8)
			return 256;
		else 
			return context - U3DConstants::StaticFull;
	}
}

//-----------------------------------------------------------------------------
// Find the symbol in a histogram that has a cumulative frequency specified.
uint32 U3DContextManager::GetSymbolFromFrequency(uint32 context, uint32 symbolFrequency)
{
	uint32 rValue = 0;
	if ((context < U3DConstants::StaticFull) && (context != U3DConstants::Context8))
	{
		rValue = 0;
		if ((m_cumulativeCount[context].empty() == false)&&
			(symbolFrequency != 0) &&
			(m_cumulativeCount[context][0] >= symbolFrequency))
		{
			uint32 i = 0;
			for (i=0; i<m_cumulativeCount[context].size(); ++i)
			{
				if (GetCumulativeSymbolFrequency(context, i) <= symbolFrequency)
					rValue = i;
				else
					break;
			}
		}
	}
	else
	{
		rValue = symbolFrequency + 1;
	}
	return rValue;
}
