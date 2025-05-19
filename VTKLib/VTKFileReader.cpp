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
#include "VTKFileReader.h"
#include <XML/XMLReader.h>
using namespace VTK;

#ifdef HAVE_ZLIB
#include <zlib.h>

bool decompress(unsigned char* szin, uInt bufSize, unsigned char* szout, uInt maxOutSize, uInt& outSize)
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
		outSize = zstrm.total_out;

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
		size_t nread = base64_decode((unsigned char*)m_data, dataSize, m_bytes.data(), m_bytes.size());

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

VTKFileReader::VTKFileReader()
{
	m_type = vtkDataFileType::Invalid;
}

const VTK::vtkModel& VTKFileReader::GetVTKModel() const
{
	return m_vtk;
}

bool VTKFileReader::ProcessAppendedDataArray(vtkDataArray& ar, vtkAppendedData& data)
{
	if (ar.m_format != vtkDataArray::APPENDED) return true;

	// get the buffer at the offset
	const char* buf = data.GetData(ar.m_offset);

	// convert from base64 to binary
	size_t headerSize = (m_headerType == UInt64 ? 64 : 32);
	base64_stream b64(buf, headerSize);
	if (b64.init() == false) return false;

	if (ar.m_type == vtkDataArray::FLOAT32)
	{
		b64.read_floats(ar.m_values_float);
	}
	else if (ar.m_type == vtkDataArray::INT32)
	{
		b64.read_int32(ar.m_values_int);
	}
	else if (ar.m_type == vtkDataArray::INT64)
	{
		b64.read_int64(ar.m_values_int);
	}
	else
	{
		assert(false);
		return false;
	}

	return true;
}

bool VTKFileReader::ParseFileHeader(XMLTag& tag)
{
	// read the type attribute
	const char* sztype = tag.AttributeValue("type", true);
	if (sztype) {
		if      (strcmp(sztype, "UnstructuredGrid" ) == 0) m_type = UnstructuredGrid;
		else if (strcmp(sztype, "PolyData"         ) == 0) m_type = PolyData;
		else if (strcmp(sztype, "PUnstructuredGrid") == 0) m_type = PUnstructuredGrid;
		else if (strcmp(sztype, "Collection"       ) == 0) m_type = Collection;
		else return errf("Can't read %s vtk files", sztype);
	}
	else return errf("Missing type attribute in vtk file.");

	// read the version attribute
	const char* szversion = tag.AttributeValue("version", true);
	if (szversion) m_version = szversion;

	// read the byte_order attribute
	const char* szbyteorder = tag.AttributeValue("byte_order", true);
	if (szbyteorder)
	{
		if (strcmp(szbyteorder, "LittleEndian") == 0) m_byteOrder = LittleEndian;
		else if (strcmp(szbyteorder, "BigEndian") == 0) m_byteOrder = BigEndian;
		else return false;
	}
	else m_byteOrder = LittleEndian; // TODO: Is this the correct default?

	// read header type
	const char* szhdrtype = tag.AttributeValue("header_type", true);
	if (szhdrtype)
	{
		if (strcmp(szhdrtype, "UInt32") == 0) m_headerType = UInt32;
		else if (strcmp(szhdrtype, "UInt64") == 0) m_headerType = UInt64;
		else return errf("Unknown header type");
	}
	else m_headerType = UInt32; // TODO: Not sure if this is the correct default

	// read compression scheme
	const char* szcompress = tag.AttributeValue("compressor", true);
	if (szcompress) {
		if (strcmp(szcompress, "vtkZLibDataCompressor") == 0) m_compressor = ZLibCompression;
	}
	else m_compressor = NoCompression;

	return true;
}

bool VTKFileReader::ProcessDataArrays(vtkModel& vtk, vtkAppendedData& data)
{
	for (vtkPiece& piece : vtk.m_pieces)
	{
		if (ProcessAppendedDataArray(piece.m_points      , data) == false) return false;
		if (ProcessAppendedDataArray(piece.m_cell_offsets, data) == false) return false;
		if (ProcessAppendedDataArray(piece.m_cell_connect, data) == false) return false;
		if (ProcessAppendedDataArray(piece.m_cell_types  , data) == false) return false;
	}

	return true;
}


bool VTKFileReader::ParseUnstructuredGrid(XMLTag& tag, vtkModel& vtk)
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

bool VTKFileReader::ParsePolyData(XMLTag& tag, vtkModel& vtk)
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

bool VTKFileReader::ParsePiece(XMLTag& tag, vtkModel& vtk)
{
	vtkPiece piece;
	piece.m_numPoints = tag.AttributeValue<int>("NumberOfPoints", 0);
	piece.m_numCells = tag.AttributeValue<int>("NumberOfCells", 0);
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
		else if (tag == "PointData")
		{
			if (ParsePointData(tag, piece) == false) return false;
		}
		else if (tag == "CellData")
		{
			if (ParseCellData(tag, piece) == false) return false;
		}
		else tag.skip();
		++tag;
	} while (!tag.isend());

	vtk.AddPiece(piece);

	return true;
}

bool VTKFileReader::ParsePoints(XMLTag& tag, vtkPiece& piece)
{
	++tag;
	do
	{
		if (tag == "DataArray")
		{
			vtkDataArray& points = piece.m_points;
			if (ParseDataArray(tag, points) == false) return false;

			if ((points.m_type != vtkDataArray::FLOAT32) &&
				(points.m_type != vtkDataArray::FLOAT64)) return false;
			if (points.m_numComps != 3) return false;
		}
		else tag.skip();
		++tag;
	} while (!tag.isend());

	return true;
}

bool VTKFileReader::ParseCells(XMLTag& tag, vtkPiece& piece)
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
	} while (!tag.isend());

	return true;
}

bool VTKFileReader::ParsePolys(XMLTag& tag, vtkPiece& piece)
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

bool VTKFileReader::ParsePointData(XMLTag& tag, VTK::vtkPiece& piece)
{
	++tag;
	do {
		if (tag == "DataArray")
		{
			vtkDataArray data;
			data.m_name = tag.AttributeValue("Name");
			if (ParseDataArray(tag, data) == false) return false;
			piece.m_pointData.push_back(data);
		}
		else tag.skip();
		++tag;
	} while (!tag.isend());
	return true;
}

bool VTKFileReader::ParseCellData(XMLTag& tag, VTK::vtkPiece& piece)
{
	++tag;
	do {
		if (tag == "DataArray")
		{
			vtkDataArray data;
			data.m_name = tag.AttributeValue("Name");
			if (ParseDataArray(tag, data) == false) return false;
			piece.m_cellData.push_back(data);
		}
		else tag.skip();
		++tag;
	} while (!tag.isend());
	return true;
}

bool VTKFileReader::ParseDataArray(XMLTag& tag, vtkDataArray& vtkDataArray)
{
	// get the format
	const char* szformat = tag.AttributeValue("Format", true);
	if (szformat == nullptr) szformat = tag.AttributeValue("format");
	if (szformat) vtkDataArray.setFormat(szformat);
	else vtkDataArray.m_format = vtkDataArray::ASCII;

	// for appended data we need an offset
	if (vtkDataArray.m_format == vtkDataArray::APPENDED)
	{
		const char* szoff = tag.AttributeValue("offset");
		vtkDataArray.m_offset = atoi(szoff);
	}

	// get the type
	const char* sztype = tag.AttributeValue("type");
	if      (strcmp(sztype, "Float32") == 0) vtkDataArray.m_type = vtkDataArray::FLOAT32;
	else if (strcmp(sztype, "Float64") == 0) vtkDataArray.m_type = vtkDataArray::FLOAT64;
	else if (strcmp(sztype, "Int8"   ) == 0) vtkDataArray.m_type = vtkDataArray::INT8;
	else if (strcmp(sztype, "UInt8"  ) == 0) vtkDataArray.m_type = vtkDataArray::UINT8;
	else if (strcmp(sztype, "Int64"  ) == 0) vtkDataArray.m_type = vtkDataArray::INT64;
	else if (strcmp(sztype, "Int32"  ) == 0) vtkDataArray.m_type = vtkDataArray::INT32;
	else return errf("Unknown data array type %s", sztype);

	// get the number of components
	vtkDataArray.m_numComps = tag.AttributeValue<int>("NumberOfComponents", 1);

	// get the value
	if (vtkDataArray.m_format == vtkDataArray::ASCII)
	{
		if ((vtkDataArray.m_type == vtkDataArray::FLOAT32) ||
			(vtkDataArray.m_type == vtkDataArray::FLOAT64))
		{
			tag.value(vtkDataArray.m_values_float);
		}
		else if (vtkDataArray.m_type == vtkDataArray::INT8)
		{
			tag.value2(vtkDataArray.m_values_int);
		}
		else if (vtkDataArray.m_type == vtkDataArray::UINT8)
		{
			tag.value2(vtkDataArray.m_values_int);
		}
		else if (vtkDataArray.m_type == vtkDataArray::INT32)
		{
			tag.value2(vtkDataArray.m_values_int);
		}
		else if (vtkDataArray.m_type == vtkDataArray::INT64)
		{
			tag.value2(vtkDataArray.m_values_int);
		}
	}
	else if (vtkDataArray.m_format == vtkDataArray::BINARY)
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

		// check for compression
		std::vector<unsigned char> buf2;

		if (m_compressor == ZLibCompression)
		{
			unsigned int blocks = *((unsigned int*)d);
			unsigned int decompressedSize = *((unsigned int*)(d + 4));
			unsigned int not_sure_what_this_is = *((unsigned int*)(d + 8));
			unsigned int compressedSize = *((unsigned int*)(d + 12));

			d += 16;
			n -= 16;

			buf2.assign(decompressedSize, 0);
			unsigned char* d2 = buf2.data();
			uInt outsize = 0;

			if (decompress(d, n, d2, buf2.size(), outsize) == false) return errf("Error decompressing data");
			d = d2;
			n = outsize;
		}
		else
		{
			d += headerSize;
			n -= headerSize;
		}

		// process array
		if (vtkDataArray.m_type == vtkDataArray::FLOAT32)
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
		else if (vtkDataArray.m_type == vtkDataArray::FLOAT64)
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
		else if (vtkDataArray.m_type == vtkDataArray::INT8)
		{
			std::vector<int>& v = vtkDataArray.m_values_int;
			size_t m = n;
			v.resize(m);
			for (int i = 0; i < m; ++i)
			{
				v[i] = *((signed char*)d);
				d += 1;
			}
		}
		else if (vtkDataArray.m_type == vtkDataArray::UINT8)
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
		else if (vtkDataArray.m_type == vtkDataArray::INT32)
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
		else if (vtkDataArray.m_type == vtkDataArray::INT64)
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

bool VTKFileReader::ParseAppendedData(XMLTag& tag, vtkAppendedData& vtkAppendedData)
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
