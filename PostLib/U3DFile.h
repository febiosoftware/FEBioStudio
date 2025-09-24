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
#include <string>
#include <vector>
#include <cstdint>

#ifdef WIN32
typedef unsigned __int16 uint16;
typedef __int32 int32;
typedef unsigned __int32 uint32;
typedef __int64 int64;
typedef unsigned __int64 uint64;
#endif
#ifdef __APPLE__
typedef uint16_t uint16;
typedef int32_t int32;
typedef uint32_t uint32;
typedef int64_t int64;
typedef uint64_t uint64;
#endif
#ifdef LINUX
typedef uint16_t uint16;
typedef int32_t int32;
typedef uint32_t uint32;
typedef int64_t int64;
typedef uint64_t uint64;
#endif

class U3DFile
{
public:
	// The following enums are not part of the U3D format
	// but defines by me to simplify some of the parsing
	enum { MAX_TEXTURE_LAYERS = 16 };

public:
	enum BlockType
	{
		// File Structure blocks
		ModifierChain    = 0xFFFFFF14,
		PriorityUpdate   = 0xFFFFFF15,

		// Node blocks
		GroupNode        = 0xFFFFFF21,
		ModelNode        = 0xFFFFFF22,
		LightNode        = 0xFFFFFF23,
		ViewNode         = 0xFFFFFF24,

		// geometry generator blocks
		CLODMeshDeclaration = 0xFFFFFF31,
		CLODBaseMesh        = 0xFFFFFF3b
	};

	enum ProfileFlags
	{
		Extensible    = 2,
		NoCompression = 4
	};

	enum ModifierChainType
	{
		NodeModifierChain    = 0,
		ModelResourceChain   = 1,
		TextureResourceChain = 2
	};

	enum ModifierChainAttribute
	{
		BoundingSphere = 1,
		BoundingBox    = 2
	};

	enum ViewNodeAttributes
	{
		ScreenPositionUnits    = 1,
		OrthographicProjection = 2,
		TwoPointPerspective    = 4,
		OnePointPerspective    = 8	// doc says 6 but I think that's a typo
	};

	enum MaxMeshAttributes
	{
		ExcludeNormals = 1
	};

	enum ShadingAttributes
	{
		UsePerVertexDiffuse = 1,
		UsePerVertexSpecular = 2,
		UsePerVertexDiffuseAndSpecular = 3
	};

public:
	class BLOCK
	{
	public:
		uint32	blockType;		// identifies the object type
		uint32	dataSize;		// size of the data section in bytes (does not include padding)
		uint32	metaDataSize;	// size of meta data section in bytes (does not include padding)
		std::vector<uint32>	data;			// pointer to data block
		std::vector<uint32>	metaData;		// pointer to meta data block (or zero)

	public:
		BLOCK();
		~BLOCK();
	};

	struct FILE_HEADER
	{
		int32		version;			// file version. (can be less than zero)
		uint32		profile;			// profile identifier flags
		uint32		declarationSize;	// number of bytes in the Declaration block section of file (including padding)
		uint64		fileSize;			// file size. Includes size of all blocks.		
		uint32		encoding;			// character encoding (must be 106 = UTF-8)
	};

	struct BOUNDING_SPHERE
	{
		float	x, y, z;
		float	radius;
	};

	struct BOUNDING_BOX
	{
		float	xmin, ymin, zmin;
		float	xmax, ymax, zmax;
	};

	struct MODIFIER_CHAIN
	{
		std::string	name;		// modifier chain name
		uint32		type;		// modifier chain types
		uint32		attributes;	// modifier chain attributes. Indicate presence of bounding box information

		BOUNDING_SPHERE	boundSphere;
		BOUNDING_BOX	boundBox;

		uint32		modifierCount;	// number of modifiers in chain
	};

	struct TRANSFORM_MATRIX
	{
		float	m[4][4];	// note that this is row-order so the file actually reads the transpose (since it's column order)
	};

	struct VIEW_PORT
	{
		float	width;
		float	height;
		float	horizontalPosition;
		float	verticalPosition;
	};

	struct VIEW_NODE
	{
		std::string	name;			// view node name
		std::string	resourceName;	// view resource name
		uint32		attributes;		// view node attributes

		float	nearClip;
		float	farClip;

		VIEW_PORT	viewPort;
	};

	struct LIGHT_NODE
	{
		std::string	name;		// name of light node
		std::string	resource;	// name of resource
	};

	struct MAX_MESH_DESCRIPTION
	{
		uint32	attributes;
		uint32	faceCount;
		uint32	positionCount;
		uint32	normalCount;
		uint32	diffuseColorCount;
		uint32	specularColorCount;
		uint32	textureCoordCount;
		uint32	shadingCount;

		uint32	minResolution;
		uint32	maxResolution;
	};

	struct BASE_MESH_DESCRIPTION
	{
		uint32	faceCount;
		uint32	positionCount;
		uint32	normalCount;
		uint32	diffuseColorCount;
		uint32	specularColorCount;
		uint32	textureCoordCount;
	};

	struct SHADING_DESCRIPTION
	{
		uint32	attributes;
		uint32	textureLayerCount;
		uint32	textureCoordDimensions[MAX_TEXTURE_LAYERS];
		uint32	originalShadingId;
	};

	bool readFileHeader(FILE_HEADER& fileHeader);
	bool readPriorityUpdateBlock(BLOCK& block);
	bool readModifierChainBlock(BLOCK& block);
	bool readCLODBaseMeshBlock  (BLOCK& block);

	bool readViewNodeBlock(BLOCK& block);
	bool readLightNodeBlock(BLOCK& block);
	bool readGroupNodeBlock(BLOCK& block);
	void readModelNodeBlock(BLOCK& block);
	void readCLODMeshDeclaration(BLOCK& block);

	bool readBlock(BLOCK& block, int ntype = -1);

	void readBytes(void* data, int nbytes)
	{
		fread(data, nbytes, 1, m_fp);
	}

public:
	U3DFile();
	bool Load(FILE* fp);

private:
	uint32	m_priority;
	FILE*	m_fp;

	MAX_MESH_DESCRIPTION			m_maxMeshDescription;
	std::vector<SHADING_DESCRIPTION>		m_shading;
};

class U3DContextManager;

class U3DBitStreamWrite
{
public:
	U3DBitStreamWrite();
	~U3DBitStreamWrite();

	void WriteU8(uint8_t uValue);
	void WriteU16(uint16 uValue);
	void WriteU32(uint32 uValue);
	void WriteU64(uint64 uValue);
	void WriteI32(int32 iValue);
	void WriteF32(float fValue);
	void WriteCompressedU32(uint32 context, uint32 uValue);
	void WriteCompressedU16(uint32 context, uint16 uValue);
	void WriteCompressedU8 (uint32 context, uint8_t uValue);

	void GetDataBlock(U3DFile::BLOCK& dataBlock);

	void AlignToByte();
	void AlignTo4Byte();

private:
	void WriteSymbol(uint32 context, uint32 symbol, bool& escape);
	void SwapBits8(uint32& rValue);
	void WriteBit(uint32 bit);
	void IncrementPosition();
	void GetLocal();
	void PutLocal();
	void CheckPosition();
	void AllocateDataBuffer(int32 size);
	void GetBitCount(uint32& rCount);

private:
	// the context manager handles the updates to 
	// the histograms for the compression contexts
	U3DContextManager*	m_contextManager;
					
	// high and low are the uppoer and lower limits on the probability
	uint32	m_high;	
	uint32	m_low;		

	// stores the number of bits of underflow caused by the limited range of high and low
	uint32	m_underflow;

	// This is true of a compressed value was written.
	// When the datablock is retrieved, a 32 bit 0 is written to reset 
	// the values of high, low, and underflow.
	bool	m_compressed;

	// the data section of the data block to write
	std::vector<uint32>	m_data;
	
	// the position of the datablock to write
	int32	m_dataPosition;

	// the local value of the data corresponding to the data position
	uint32	m_dataLocal;

	// the 32 bits in data after m_dataLocal
	uint32 m_dataLocalNext;

	// The offset into m_dataLocal that the next write will occur
	int32 m_dataBitOffset;

	static const int32 m_dataSizeIncrement = 0x000023F8;
};

//-----------------------------------------------------------------------------
// All uncompressed reads are read in as a sequence of U8s with the private method
// ReadSymbol in context Context8 and then built up to the appropriate size and cast
// to the appropriate type for the read call. 
// All compressed reads are for unsigned integers and are passed through to the private
// method ReadSymbol with the associated context.
class U3DBitStreamRead
{
public:
	U3DBitStreamRead();
	~U3DBitStreamRead();

public:
	void ReadU8(uint8_t& rValue);
	void ReadU16(uint16& rValue);
	void ReadU32(uint32& rValue);
	void ReadU64(uint64& rValue);
	void ReadI32(int32& rValue);
	void ReadF32(float& rValue);
	void ReadCompressedU32(uint32 context, uint32& rValue);
	void ReadCompressedU16(uint32 context, uint16& rValue);
	void ReadCompressedU8 (uint32 context, uint8_t& rValue);
	void ReadString(std::string& s);

	void SetDataBlock(U3DFile::BLOCK& dataBlock);

	void AlignTo4Byte();

	void ReadBlock(U3DFile::BLOCK& block);

private:
	void SwapBits8(uint32& rValue);
	void ReadSymbol(uint32 context, uint32& rSymbol);
	void GetBitCount(uint32& rCount);
	void ReadBit(uint32& rValue);
	void Read15Bits(uint32& rValue);
	void IncrementPosition();
	void SeekToBit(uint32 position);
	void GetLocal();

private:
	// the context manager handles the updates to the histograms 
	// for the compression contexts
	U3DContextManager*	m_contextManager;

	// high and low are the upper land lower limits on the probability
	uint32 m_high, m_low;

	// stores the number of bits of underflow caused by the limited range of high and low
	uint32 m_underflow;

	// the value as represented in the datablock
	uint32 m_code;

	// the data section of the datablock to read from
	uint32* m_data;
	uint32 m_dataLength;

	// the position currently read in the datablock specified in 32 bit increments
	uint32 m_dataPosition;

	// the local value of the data corresponding to dataPosition
	uint32 m_dataLocal;

	// the 32 bits in data after dataLocal
	uint32 m_dataLocalNext;

	// the offset into dataLocal that the next read will occur
	uint32 m_dataBitOffset;

	static uint32 FastNotMask[];
	static uint32 ReadCount[];
};

//-----------------------------------------------------------------------------
// The ContextManager is used to access the static and dynamic contexts used
// for the reading and writing of compressed data.
// Dynamic contexts are specified as 0x0001 through 0x3FFF ( = U3DConstants::MaxRange). 
// Dynamic contexts keep a histogram that stores the number of occurrences of symbols 
// that are added through the AddSymbol method.
// Static contexts represent histograms where each value between 0 and (context - 0x4000) 
// are equally likely. Static contexts histograms are not changed by the AddSymbol method.
// Context0 or Context8: context 0 is a shortcut to context 0x40FF which corresponds to values
// from 0 to 255.
// When the histogram for a dynamic context is initialized, the symbol frequency of the escape
// symbol is initialized to 1.
// Symbols larger than 0xFFFF are treated as static.
class U3DContextManager
{
public:
	U3DContextManager();

public:
	void AddSymbol(uint32 context, uint32 symbol);
	uint32 GetSymbolFrequency(uint32 context, uint32 symbol);
	uint32 GetCumulativeSymbolFrequency(uint32 context, uint32 symbol);
	uint32 GetTotalSymbolFrequency(uint32 context);
	uint32 GetSymbolFromFrequency(uint32 context, uint32 symbolFrequency);

private:
	// an array of arrays that stores the number 
	// of occurrences of each symbol for each dynamic context
	std::vector< std::vector<uint16> >	m_symbolCount;

	// an array of arrays that store the cumulative frequency of each symbol in each context.
	// the value is the number of occurences of a symbol and every symbol with a large value
	std::vector< std::vector<uint16> >	m_cumulativeCount;

	// The Elephant is a value that determines the number of symbol occurences that are stored in each
	// dynamic histogram. Limiting the number of occurrences avoids overflow of the uin16 array elements
	// and allows the histogram to adapt to changing symbol distributions in files.
	static const uint32 Elephant = 0x00001FFF;

	// the maximum value that is stored in a histogram
	static const uint32 MaximumSymbolInHistogram = 0x0000FFFF;

	// the amount to increase the size of an array when reallocating an array
	static const uint32 ArraySizeIncr = 32;
};

//-----------------------------------------------------------------------------
// U3DConstants is a class that holds constants that are needed by more than one of the objects 
class U3DConstants
{
public:
	// the context for uncompressed U8
	static const uint32 Context8 = 0;

	// contexts >= StaticFull are static contexts
	static const uint32 StaticFull = 0x00004000;

	// the largest allowable static context.
	// values written to contexts > MaxRange are written as uncompressed
	static const uint32 MaxRange = StaticFull + 0x00003FFF;

	// a default buffer size
	static const uint32 SizeBuff = 1024;

	// the initial size allocated for buffers
	static const uint32 DataSizeInitial = 0x00000010;

	// masks all but the most significant bit
	static const uint32 HalfMask = 0x00008000;

	// masks the most signifcant bit
	static const uint32 NotHalfMask = 0x00007FFF;

	// masks all but the 2nd most signifcant bit
	static const uint32 QuarterMask = 0x00004000;

	// masks the 2 most signifcant bits
	static const uint32 NotThreeQuarterMask = 0x00003FFF;

	// used to swap 8 bits in place
	static uint32 Swap8[];
};
