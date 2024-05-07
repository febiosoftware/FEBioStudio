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

#include "VTUImport.h"
#include <GeomLib/GMeshObject.h>
#include <GeomLib/GModel.h>
#include <XML/XMLReader.h>
#ifdef HAVE_ZLIB
#include <zlib.h>

bool decompress(unsigned char* szin, uInt bufSize, unsigned char* szout, uInt maxOutSize)
{
	z_stream zstrm;

	// initialize decompression stream
	zstrm.zalloc = Z_NULL;
	zstrm.zfree = Z_NULL;
	zstrm.opaque = Z_NULL;
	zstrm.avail_in = 0;
	zstrm.next_in = Z_NULL;
	zstrm.avail_out = 0;

	// allocate inflate state
	int ret = inflateInit(&zstrm);
	if (ret != Z_OK) return false;

	// set the buffer to decompress
	zstrm.avail_in = bufSize;
	zstrm.next_in = szin;

	// run inflate() on input until output buffer not full
	do {
		zstrm.avail_out = maxOutSize;
		zstrm.next_out = szout;
		ret = inflate(&zstrm, Z_NO_FLUSH);
		assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
		switch (ret) {
		case Z_NEED_DICT:
			ret = Z_DATA_ERROR;     /* and fall through */
		case Z_DATA_ERROR:
		case Z_MEM_ERROR:
			(void)inflateEnd(&zstrm);
			return false;
		}
		uInt have = maxOutSize - zstrm.avail_out;

//		buf.append(out, have);

	} while (zstrm.avail_out == 0);

	// Finish
	(void)inflateEnd(&zstrm);

	return true;
}
#endif

unsigned char base64_decode_char(unsigned char c)
{
	if ((c >= 'A') && (c <= 'Z')) return (c - 'A');
	if ((c >= 'a') && (c <= 'z')) return (c - 'a') + 26;
	if ((c >= '0') && (c <= '9')) return (c - '0') + 52;
	if (c == '+') return 62;
	if (c == '/') return 63;
	if (c == '=') return 0;
	assert(false);
	return 0;
}

size_t base64_decode_triplet(unsigned char i0, unsigned char i1, unsigned char i2, unsigned char i3,
	unsigned char& o0, unsigned char& o1, unsigned char& o2)
{
	unsigned char d[4];
	d[0] = base64_decode_char(i0);
	d[1] = base64_decode_char(i1);
	d[2] = base64_decode_char(i2);
	d[3] = base64_decode_char(i3);

	o0 = ((d[0] << 2) & 0xFC) | ((d[1] >> 4) & 0x03);
	o1 = ((d[1] << 4) & 0xF0) | ((d[2] >> 2) & 0x0F);
	o2 = ((d[2] << 6) & 0xC0) | ((d[3] >> 0) & 0x3F);

	if (i2 == '=') return 1;
	if (i3 == '=') return 2;
	return 3;
}

size_t base64_decode(unsigned char* in, size_t l, unsigned char* o, size_t ol)
{
	size_t c = 0;
	for (size_t i = 0; i < l; i += 4)
	{
		unsigned char o0, o1, o2;
		size_t n = base64_decode_triplet(in[0], in[1], in[2], in[3], o0, o1, o2);
		in += 4;
		if (n > 0) *o++ = o0;
		if (n > 1) *o++ = o1;
		if (n > 2) *o++ = o2;
		c += n;
		if (c >= ol) break;
	}
	return c;
}

class VTKDataArray
{
public:
	enum Types
	{
		UINT8,
		INT32,
		INT64,
		FLOAT32,
		FLOAT64,
	};

	enum Format
	{
		ASCII,
		BINARY,
		APPENDED
	};

public:
	VTKDataArray()
	{
		m_type = -1;
		m_format = ASCII;
		m_numComps = 1;
		m_offset = 0;
	}

	void setFormat(const char* szformat)
	{
		if      (strcmp(szformat, "ascii"   ) == 0) m_format = Format::ASCII;
		else if (strcmp(szformat, "binary"  ) == 0) m_format = Format::BINARY;
		else if (strcmp(szformat, "appended") == 0) m_format = Format::APPENDED;
		else { assert(false); }
	}

	size_t size() const
	{
		switch (m_type)
		{
		case FLOAT32:
		case FLOAT64: return (m_values_float.size() / m_numComps); break;
		case UINT8:
		case INT32:
		case INT64:
			return (m_values_int.size() / m_numComps); break;
			break;
		default:
			assert(false);
			break;
		}
		return 0;
	}

	bool empty() const { return m_values_int.empty(); }

	void get(int n, double* v) const { *v = m_values_float[n]; }
	void get(int n, int*    v) const { *v = m_values_int[n]; }

public:
	int	m_type;
	int m_format;
	int m_numComps;
	int	m_offset;

	std::vector<double>		m_values_float;
	std::vector<int>		m_values_int;
};

template <class T> class VTKDataArrayReader
{
public:
	VTKDataArrayReader(const VTKDataArray& data) : m_data(data) {}
	T operator [] (int n) { T v; m_data.get(n, &v); return v; }
private:
	const VTKDataArray& m_data;
};

class VTKCell
{
public:
	enum { MAX_NODES = 20 };

	enum CellType
	{
		VTK_VERTEX = 1,
		VTK_POLY_VERTEX = 2,
		VTK_LINE = 3,
		VTK_POLY_LINE = 4,
		VTK_TRIANGLE = 5,
		VTK_TRIANGLE_STRIP = 6,
		VTK_POLYGON = 7,
		VTK_PIXEL = 8,
		VTK_QUAD = 9,
		VTK_TETRA = 10,
		VTK_VOXEL = 11,
		VTK_HEXAHEDRON = 12,
		VTK_WEDGE = 13,
		VTK_PYRAMID = 14
	};

	VTKCell()
	{
		m_label = 0;
		m_cellType = 0;
		m_numNodes = 0;
	}

public:
	int		m_label;
	int		m_cellType;
	int		m_numNodes;
	int		m_node[MAX_NODES];
};

class VTKPiece
{
public:
	VTKPiece() 
	{
		m_numPoints = 0;
		m_numCells = 0;
	}

	VTKPiece(const VTKPiece& piece)
	{
		m_numPoints = piece.m_numPoints;
		m_numCells = piece.m_numCells;
		m_points = piece.m_points;
		m_cell_connect = piece.m_cell_connect;
		m_cell_offsets = piece.m_cell_offsets;
		m_cell_types = piece.m_cell_types;
	}

	void operator = (const VTKPiece& piece)
	{
		m_numPoints = piece.m_numPoints;
		m_numCells = piece.m_numCells;
		m_points = piece.m_points;
		m_cell_connect = piece.m_cell_connect;
		m_cell_offsets = piece.m_cell_offsets;
		m_cell_types = piece.m_cell_types;
	}

	size_t Points() const { return m_points.size(); }
	size_t Cells() const { return m_cell_offsets.size(); }

	vec3d Point(int n) const
	{
		VTKDataArrayReader<double> p(m_points);
		return vec3d(p[3 * n], p[3 * n + 1], p[3 * n + 2]);
	}

	VTKCell Cell(int n)
	{
		VTKCell cell;

		if (m_cell_types.empty())
		{
			int n0 = (n > 0 ? m_cell_offsets.m_values_int[n - 1] : 0);
			int n1 = m_cell_offsets.m_values_int[n];
			cell.m_numNodes = n1 - n0;
			cell.m_cellType = VTKCell::VTK_POLYGON;
			int m = cell.m_numNodes;
			for (int i = 0; i < m; ++i)
			{
				cell.m_node[i] = m_cell_connect.m_values_int[n0 + i];
			}
		}
		else
		{
			m_cell_types.get(n, &cell.m_cellType);

			int n0 = (n == 0 ? 0 : m_cell_offsets.m_values_int[n - 1]);
			int n1 = m_cell_offsets.m_values_int[n];
			cell.m_numNodes = n1 - n0;
			int m = cell.m_numNodes;
			for (int i = 0; i < m; ++i)
			{
				cell.m_node[i] = m_cell_connect.m_values_int[n0 + i];
			}
		}

		return cell;
	}

public:
	int m_numPoints;
	int m_numCells;

	VTKDataArray	m_points;
	VTKDataArray	m_cell_connect;
	VTKDataArray	m_cell_offsets;
	VTKDataArray	m_cell_types;
};

//=================================================================
class VTKAppendedData
{
public:
	void SetData(const char* szdata)
	{
		m_data = szdata;
	}

	const char* GetData(size_t offset)
	{
		return m_data.data() + offset;
	}

private:
	std::string	m_data;
};

//=================================================================
class VTKModel
{
public:
	void AddPiece(const VTKPiece& piece) { m_pieces.push_back(piece); }

	size_t Pieces() const { return m_pieces.size(); }

	VTKPiece& Piece(int n) { return m_pieces[n]; }

public:
	std::vector<VTKPiece>	m_pieces;
};

class base64_stream
{
public:
	base64_stream(const char* szbuf, size_t headerSize) : m_data(szbuf), m_headerSize(headerSize) {}

	bool init()
	{
		m_bytes.clear();
		unsigned char b[9] = { 0 };
		if (m_headerSize == 32)
		{
			base64_decode((unsigned char*)m_data, 6, b, 9);
			m_bufSize = *((unsigned int*)b);
			m_bytes.assign(m_bufSize + 4, 0);
		}
		else if (m_headerSize == 64)
		{
			base64_decode((unsigned char*)m_data, 12, b, 9);
			m_bufSize = *((unsigned long int*)b);
			m_bytes.assign(m_bufSize + 8, 0);
		}
		else return false;

		// we're not sure how many bytes to read so we just make a guess
		size_t dataSize = (m_bytes.size() * 3) / 2;
		size_t nread= base64_decode((unsigned char*)m_data, dataSize, m_bytes.data(), m_bytes.size());

		return true;
	}

	void read_floats(std::vector<double>& d)
	{
		unsigned char* b = m_bytes.data();
		if (m_headerSize == 32) b += 4;
		if (m_headerSize == 64) b += 8;

		size_t m = m_bufSize / 4; assert((m_bufSize % 4) == 0);
		d.resize(m, 0.f);
		for (size_t i = 0; i < m; ++i)
		{
			d[i] = *((float*)b);
			b += 4;
		}
	}

	void read_int32(std::vector<int>& d)
	{
		unsigned char* b = m_bytes.data();
		if (m_headerSize == 32) b += 4;
		if (m_headerSize == 64) b += 8;

		size_t m = m_bufSize / 4; assert((m_bufSize % 4) == 0);
		d.resize(m, 0);
		for (size_t i = 0; i < m; ++i)
		{
			d[i] = *((int*)b);
			b += 4;
		}
	}

	void read_int64(std::vector<int>& d)
	{
		unsigned char* b = m_bytes.data();
		if (m_headerSize == 32) b += 4;
		if (m_headerSize == 64) b += 8;

		size_t m = m_bufSize / 8; assert((m_bufSize % 8) == 0);
		d.resize(m, 0);
		for (size_t i = 0; i < m; ++i)
		{
			d[i] = *((long int*)b);
			b += 8;
		}
	}

private:
	const char* m_data = nullptr;
	size_t m_headerSize = 32;
	size_t m_bufSize = 0;
	std::vector<unsigned char>	m_bytes;
};

bool VTKFileReader::ProcessProcessDataArray(VTKDataArray& ar, VTKAppendedData& data)
{
	if (ar.m_format != VTKDataArray::APPENDED) return true;

	// get the buffer at the offset
	const char* buf = data.GetData(ar.m_offset);

	// convert from base64 to binary
	size_t headerSize = (m_headerType == UInt64 ? 64 : 32);
	base64_stream b64(buf, headerSize);
	if (b64.init() == false) return false;

	if (ar.m_type == VTKDataArray::FLOAT32)
	{
		b64.read_floats(ar.m_values_float);
	}
	else if (ar.m_type == VTKDataArray::INT32)
	{
		b64.read_int32(ar.m_values_int);
	}
	else if (ar.m_type == VTKDataArray::INT64)
	{
		b64.read_int64(ar.m_values_int);
	}

	return true;
}

//=================================================================
VTKFileReader::VTKFileReader(FSProject& prj) : FSFileImport(prj) {}


//=================================================================
bool VTKFileReader::ParseFileHeader(XMLTag& tag)
{
	// read the type attribute
	const char* sztype      = tag.AttributeValue("type", true); 
	if (sztype) {
		if      (strcmp(sztype, "UnstructuredGrid") == 0) m_type = UnstructuredGrid;
		else if (strcmp(sztype, "PolyData"        ) == 0) m_type = PolyData;
		else return errf("Can't read %s vtk files", sztype);
	}
	else return errf("Missing type attribute in vtk file.");
	
	// read the version attribute
	const char* szversion   = tag.AttributeValue("version", true);
	if (szversion) m_version = szversion;

	// read the byte_order attribute
	const char* szbyteorder = tag.AttributeValue("byte_order" , true); 
	if (szbyteorder)
	{
		if      (strcmp(szbyteorder, "LittleEndian") == 0) m_byteOrder = LittleEndian;
		else if (strcmp(szbyteorder, "BigEndian"   ) == 0) m_byteOrder = BigEndian;
		else return false;
	}
	else m_byteOrder = LittleEndian; // TODO: Is this the correct default?
	
	// read header type
	const char* szhdrtype   = tag.AttributeValue("header_type", true);
	if (szhdrtype)
	{
		if      (strcmp(szhdrtype, "UInt32") == 0) m_headerType = UInt32;
		else if (strcmp(szhdrtype, "UInt64") == 0) m_headerType = UInt64;
		else return errf("Unknown header type");
	}
	else m_headerType = UInt32; // TODO: Not sure if this is the correct default
	
	// read compression scheme
	const char* szcompress  = tag.AttributeValue("compressor" , true); 
	if (szcompress) {
		if (strcmp(szcompress, "vtkZLibDataCompressor") == 0) m_compressor = ZLibCompression;
	}
	else m_compressor = NoCompression;

	return true;
}

bool VTKFileReader::ProcessDataArrays(VTKModel& vtk, VTKAppendedData& data)
{
	for (VTKPiece& piece : vtk.m_pieces)
	{
		if (ProcessProcessDataArray(piece.m_points      , data) == false) return false;
		if (ProcessProcessDataArray(piece.m_cell_offsets, data) == false) return false;
		if (ProcessProcessDataArray(piece.m_cell_connect, data) == false) return false;
		if (ProcessProcessDataArray(piece.m_cell_types  , data) == false) return false;
	}

	return true;
}

//=================================================================
VTUimport::VTUimport(FSProject& prj) : VTKFileReader(prj) { }

bool VTUimport::Load(const char* szfile)
{
	// Open the file
	XMLReader xml;
	if (xml.Open(szfile, false) == false) return false;

	// get the VTKFile tag
	XMLTag tag;
	if (xml.FindTag("VTKFile", tag) == false) return false;
	if (ParseFileHeader(tag) == false) return false;

	// This reader is for unstructured grids at this point
	if (m_type != UnstructuredGrid) return false;

	VTKModel vtk;
	VTKAppendedData data;

	// parse the file
	try {
		++tag;
		do
		{
			if (tag == "UnstructuredGrid")
			{
				if (ParseUnstructuredGrid(tag, vtk) == false) return false;
			}
			else if (tag == "AppendedData")
			{
				if (ParseAppendedData(tag, data) == false) return false;
			}
			else return false;
			++tag;
		} while (!tag.isend());
	}
	catch (...)
	{

	}
	xml.Close();

	// process the appended arrays
	if (ProcessDataArrays(vtk, data) == false) return false;

	return BuildMesh(vtk);
}

//=================================================================
VTPimport::VTPimport(FSProject& prj) : VTKFileReader(prj) { }

bool VTPimport::Load(const char* szfile)
{
	// Open the file
	XMLReader xml;
	if (xml.Open(szfile, false) == false) return false;

	// get the VTKFile tag
	XMLTag tag;
	if (xml.FindTag("VTKFile", tag) == false) return false;
	if (ParseFileHeader(tag) == false) return false;

	// this file is for PolyData only
	if (m_type != PolyData) return false;

	VTKModel vtk;
	VTKAppendedData data;

	// parse the file
	try {
		++tag;
		do
		{
			if (tag == "PolyData")
			{
				if (ParsePolyData(tag, vtk) == false) return false;
			}
			else if (tag == "AppendedData")
			{
				if (ParseAppendedData(tag, data) == false) return false;
			}
			else return false;
			++tag;
		} while (!tag.isend());
	}
	catch (...)
	{

	}

	xml.Close();

	// process the appended arrays
	if (ProcessDataArrays(vtk, data) == false) return false;

	return BuildMesh(vtk);
}


bool VTKFileReader::ParseUnstructuredGrid(XMLTag& tag, VTKModel& vtk)
{
	++tag;
	do
	{
		if (tag == "Piece")
		{
			if (ParsePiece(tag, vtk) == false) return false;
		}
		else return false;
		++tag;
	} while (!tag.isend());

	return true;
}

bool VTKFileReader::ParsePolyData(XMLTag& tag, VTKModel& vtk)
{
	++tag;
	do
	{
		if (tag == "Piece")
		{
			if (ParsePiece(tag, vtk) == false) return false;
		}
		else return false;
		++tag;
	} while (!tag.isend());

	return true;
}

bool VTKFileReader::ParsePiece(XMLTag& tag, VTKModel& vtk)
{
	VTKPiece piece;
	piece.m_numPoints = tag.AttributeValue<int>("NumberOfPoints", 0);
	piece.m_numCells  = tag.AttributeValue<int>("NumberOfCells", 0);
	if (piece.m_numCells == 0)
		piece.m_numCells = tag.AttributeValue<int>("NumberOfPolys", 0);

	++tag;
	do
	{
		if (tag == "Points")
		{
			if (ParsePoints(tag, piece) == false) return false;
		}
		else if (tag == "Cells")
		{
			if (ParseCells(tag, piece) == false) return false;
		}
		else if (tag == "Polys")
		{
			if (ParsePolys(tag, piece) == false) return false;
		}
		else tag.skip();
		++tag;
	} 
	while (!tag.isend());

	vtk.AddPiece(piece);

	return true;
}

bool VTKFileReader::ParsePoints(XMLTag& tag, VTKPiece& piece)
{
	++tag;
	do
	{
		if (tag == "DataArray")
		{
			VTKDataArray& points = piece.m_points;
			if (ParseDataArray(tag, points) == false) return false;

			if ((points.m_type != VTKDataArray::FLOAT32) &&
				(points.m_type != VTKDataArray::FLOAT64)) return false;
			if (points.m_numComps != 3) return false;
		}
		else tag.skip();
		++tag;
	} while (!tag.isend());

	return true;
}

bool VTKFileReader::ParseCells(XMLTag& tag, VTKPiece& piece)
{
	++tag;
	do
	{
		if (tag == "DataArray")
		{
			const char* szname = tag.AttributeValue("Name");
			if (strcmp(szname, "connectivity") == 0)
			{
				if (ParseDataArray(tag, piece.m_cell_connect) == false) return false;
			}
			else if (strcmp(szname, "offsets") == 0)
			{
				if (ParseDataArray(tag, piece.m_cell_offsets) == false) return false;
			}
			else if (strcmp(szname, "types") == 0)
			{
				if (ParseDataArray(tag, piece.m_cell_types) == false) return false;
				if (piece.m_cell_types.m_values_int.size() != piece.m_numCells) return errf("Error reading cell types");
			}
		}
		else tag.skip();
		++tag;
	} 
	while (!tag.isend());

	return true;
}

bool VTKFileReader::ParsePolys(XMLTag& tag, VTKPiece& piece)
{
	++tag;
	do
	{
		if (tag == "DataArray")
		{
			const char* szname = tag.AttributeValue("Name");
			if (strcmp(szname, "connectivity") == 0)
			{
				if (ParseDataArray(tag, piece.m_cell_connect) == false) return false;
			}
			else if (strcmp(szname, "offsets") == 0)
			{
				if (ParseDataArray(tag, piece.m_cell_offsets) == false) return false;
			}
			else tag.skip();
		}
		else tag.skip();
		++tag;
	} while (!tag.isend());

	return true;
}

bool VTKFileReader::ParseDataArray(XMLTag& tag, VTKDataArray& vtkDataArray)
{
	// get the format
	const char* szformat = tag.AttributeValue("Format", true);
	if (szformat == nullptr) szformat = tag.AttributeValue("format");
	if (szformat) vtkDataArray.setFormat(szformat);
	else vtkDataArray.m_format = VTKDataArray::ASCII;

	// for appended data we need an offset
	if (vtkDataArray.m_format == VTKDataArray::APPENDED)
	{
		const char* szoff = tag.AttributeValue("offset");
		vtkDataArray.m_offset = atoi(szoff);
	}

	// get the type
	const char* sztype = tag.AttributeValue("type");
	if      (strcmp(sztype, "Float32") == 0) vtkDataArray.m_type = VTKDataArray::FLOAT32;
	else if (strcmp(sztype, "Float64") == 0) vtkDataArray.m_type = VTKDataArray::FLOAT64;
	else if (strcmp(sztype, "UInt8"  ) == 0) vtkDataArray.m_type = VTKDataArray::UINT8;
	else if (strcmp(sztype, "Int64"  ) == 0) vtkDataArray.m_type = VTKDataArray::INT64;
	else if (strcmp(sztype, "Int32"  ) == 0) vtkDataArray.m_type = VTKDataArray::INT64;
	else return errf("Unknown data array type %s", sztype);

	// get the number of components
	vtkDataArray.m_numComps = tag.AttributeValue<int>("NumberOfComponents", 1);

	// get the value
	if (vtkDataArray.m_format == VTKDataArray::ASCII)
	{
		if ((vtkDataArray.m_type == VTKDataArray::FLOAT32) ||
			(vtkDataArray.m_type == VTKDataArray::FLOAT64))
		{
			tag.value(vtkDataArray.m_values_float);
		}
		else if (vtkDataArray.m_type == VTKDataArray::UINT8)
		{
			tag.value2(vtkDataArray.m_values_int);
		}
		else if (vtkDataArray.m_type == VTKDataArray::INT32)
		{
			tag.value2(vtkDataArray.m_values_int);
		}
		else if (vtkDataArray.m_type == VTKDataArray::INT64)
		{
			tag.value2(vtkDataArray.m_values_int);
		}
	}
	else if (vtkDataArray.m_format == VTKDataArray::BINARY)
	{
		// strip all white space of value
		const char* sz = tag.szvalue();
		size_t l = strlen(sz);
		std::vector<unsigned char> buf(l + 1, 0);
		unsigned char* c = buf.data();
		for (size_t i = 0; i < l; ++i)
		{
			if (isspace(sz[i]) == 0) *c++ = sz[i];
		}
		*c = 0;
		l = c - buf.data();

		// decode the buffer
		std::vector<unsigned char> out(l, 0);
		unsigned char* d = out.data();
		size_t n = base64_decode(buf.data(), l, d, l);

		// skip header (should be size of array)
		size_t headerSize = 0;
		if (m_headerType == UInt32) headerSize = 4;
		if (m_headerType == UInt64) headerSize = 8;
		d += headerSize;
		n -= headerSize;

		// process array
		if (vtkDataArray.m_type == VTKDataArray::FLOAT32)
		{
			std::vector<double>& v = vtkDataArray.m_values_float;
			size_t m = n / 4;
			v.resize(m);
			for (int i = 0; i < m; ++i)
			{
				v[i] = *((float*)d);
				d += 4;
			}
		}
		else if (vtkDataArray.m_type == VTKDataArray::FLOAT64)
		{
			std::vector<double>& v = vtkDataArray.m_values_float;
			size_t m = n / 8;
			v.resize(m);
			for (int i = 0; i < m; ++i)
			{
				v[i] = *((double*)d);
				d += 8;
			}
		}
		else if (vtkDataArray.m_type == VTKDataArray::UINT8)
		{
			std::vector<int>& v = vtkDataArray.m_values_int;
			size_t m = n;
			v.resize(m);
			for (int i = 0; i < m; ++i)
			{
				v[i] = *((unsigned char*)d);
				d += 1;
			}
		}
		else if (vtkDataArray.m_type == VTKDataArray::INT32)
		{
			std::vector<int>& v = vtkDataArray.m_values_int;
			size_t m = n / 4;
			v.resize(m);
			for (int i = 0; i < m; ++i)
			{
				v[i] = *((int*)d);
				d += 4;
			}
		}
		else if (vtkDataArray.m_type == VTKDataArray::INT64)
		{
			std::vector<int>& v = vtkDataArray.m_values_int;
			size_t m = n / 8;
			v.resize(m);
			for (int i = 0; i < m; ++i)
			{
				v[i] = *((int*)d);
				d += 8;
			}
		}
		else return false;
	}

	// There can be children, so we need to skip this tag
	tag.skip();

	return true;
}

bool VTKFileReader::BuildMesh(VTKModel& vtk)
{
	FSModel& fem = m_prj.GetFSModel();

	for (int n = 0; n < vtk.Pieces(); ++n)
	{
		VTKPiece& piece = vtk.Piece(n);
	
		// get the number of nodes and elements
		int nodes = (int) piece.Points();

		int elems = 0;
		for (int i = 0; i < piece.Cells(); i++)
		{
			VTKCell cell = piece.Cell(i);
			switch (cell.m_cellType)
			{
			case VTKCell::VTK_TRIANGLE  : elems += 1; break;
			case VTKCell::VTK_QUAD      : elems += 1; break;
			case VTKCell::VTK_TETRA     : elems += 1; break;
			case VTKCell::VTK_HEXAHEDRON: elems += 1; break;
			case VTKCell::VTK_WEDGE     : elems += 1; break;
			case VTKCell::VTK_PYRAMID   : elems += 1; break;
			case VTKCell::VTK_POLYGON:
			{
				switch (cell.m_numNodes)
				{
				case 0:
				case 1:
				case 2:
					return errf("Error trying to build mesh");
					break;
				case 3:
				case 4:
					elems += 1;
					break;
				default:
					elems += cell.m_numNodes - 2;
				}
			}
			break;
			default:
				return errf("Error trying to build mesh");
			}
		}

		// create a new mesh
		FSMesh* pm = new FSMesh();
		pm->Create(nodes, elems);

		// copy nodal data
		for (int i = 0; i < nodes; ++i)
		{
			FSNode& node = pm->Node(i);
			node.r = piece.Point(i);
		}

		// copy element data
		elems = 0;
		for (int i = 0; i < piece.Cells(); ++i)
		{
			VTKCell cell = piece.Cell(i);

			if (cell.m_cellType == VTKCell::VTK_POLYGON)
			{
				if (cell.m_numNodes == 3)
				{
					FSElement& el = pm->Element(elems++);
					el.m_gid = cell.m_label; assert(el.m_gid >= 0);
					if (el.m_gid < 0) el.m_gid = 0;
					el.SetType(FE_TRI3);
					for (int j = 0; j < 3; ++j) el.m_node[j] = cell.m_node[j];
				}
				else if (cell.m_numNodes == 4)
				{
					FSElement& el = pm->Element(elems++);
					el.m_gid = cell.m_label; assert(el.m_gid >= 0);
					if (el.m_gid < 0) el.m_gid = 0;
					el.SetType(FE_QUAD4);
					for (int j = 0; j < 4; ++j) el.m_node[j] = cell.m_node[j];
				}
				else
				{
					// Simple triangulation algorithm. Assumes polygon is convex.
					int* n = cell.m_node;
					for (int j = 0; j < cell.m_numNodes - 2; ++j)
					{
						FSElement& el = pm->Element(elems++);
						el.SetType(FE_TRI3);
						el.m_gid = cell.m_label; assert(el.m_gid >= 0);
						if (el.m_gid < 0) el.m_gid = 0;
						el.m_node[0] = n[0];
						el.m_node[1] = n[j+1];
						el.m_node[2] = n[j+2];
					}
				}
			}
			else
			{
				FSElement& el = pm->Element(elems++);
				el.m_gid = cell.m_label; assert(el.m_gid >= 0);
				if (el.m_gid < 0) el.m_gid = 0;

				switch (cell.m_cellType)
				{
				case VTKCell::VTK_TRIANGLE  : el.SetType(FE_TRI3); break;
				case VTKCell::VTK_QUAD      : el.SetType(FE_QUAD4); break;
				case VTKCell::VTK_TETRA     : el.SetType(FE_TET4); break;
				case VTKCell::VTK_HEXAHEDRON: el.SetType(FE_HEX8); break;
				case VTKCell::VTK_WEDGE     : el.SetType(FE_PENTA6); break;
				case VTKCell::VTK_PYRAMID   : el.SetType(FE_PYRA5); break;
				default:
					delete pm;
					return errf("Error trying to build mesh");
				}

				int nn = el.Nodes();
				assert(nn == cell.m_numNodes);
				for (int j = 0; j < nn; ++j) el.m_node[j] = cell.m_node[j];
			}
		}

		pm->RebuildMesh();
		GMeshObject* po = new GMeshObject(pm);
		po->Update();

		char szname[256];
		sprintf(szname, "vtkObject%d", n + 1);
		po->SetName(szname);
		fem.GetModel().AddObject(po);
	}

	return true;
}

bool VTKFileReader::ParseAppendedData(XMLTag& tag, VTKAppendedData& vtkAppendedData)
{
	const char* szenc = tag.AttributeValue("encoding");
	if (strcmp(szenc, "base64") != 0) return errf("Unrecognized encoding type in AppendedData");

	// strip all white space
	const char* sz = tag.szvalue();
	size_t l = strlen(sz);
	char* buf = new char[l + 1];
	char* c = buf;
	for (size_t i = 0; i < l; ++i)
	{
		if (isspace(sz[i]) == 0) *c++ = sz[i];
	}
	*c = 0;
	if (buf[0] != '_') { delete[] buf; return errf("invalid formatting of AppendData"); }

	vtkAppendedData.SetData(buf + 1);

	delete[] buf;

	return true;
}
