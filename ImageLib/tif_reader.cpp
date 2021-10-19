/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2021 University of Utah, The Trustees of Columbia University in
University of Utah.


Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#ifdef HAS_TEEM
#include "tif_reader.h"
#include "compatibility.h"

TIFReader::TIFReader()
{
	m_resize_type = 0;
	m_resample_type = 0;
	m_alignment = 0;

	m_slice_seq = false;
	m_time_num = 0;
	m_cur_time = -1;
	m_chan_num = 0;
	m_slice_num = 0;
	m_x_size = 0;
	m_y_size = 0;

	m_valid_spc = false;
	m_xspc = 1.0;
	m_yspc = 1.0;
	m_zspc = 1.0;

	m_max_value = 0.0;
	m_scalar_scale = 1.0;

	m_batch = false;
	m_cur_batch = -1;

	m_time_id = L"_T";

	current_page_ = current_offset_ = 0;
	swap_ = false;
	isBig_ = false;
	isHyperstack_ = false;
}

TIFReader::~TIFReader()
{
	if (tiff_stream.is_open())
		tiff_stream.close();
}

void TIFReader::SetFile(std::string &file)
{
	if (!file.empty())
	{
		if (!m_path_name.empty())
			m_path_name.clear();
		m_path_name.assign(file.length(), L' ');
		copy(file.begin(), file.end(), m_path_name.begin());
	}
	m_id_string = m_path_name;
}

void TIFReader::SetFile(std::wstring &file)
{
	m_path_name = file;
	m_id_string = m_path_name;
}

int TIFReader::Preprocess()
{
	int i;

	m_4d_seq.clear();
	isHyperstack_ = false;
	isHsTimeSeq_ = false;
	imagej_raw_ = false;
	imagej_raw_possible_ = false;
	m_b_page_num = false;
	m_ull_page_num = 0;
	InvalidatePageInfo();

	//separate path and name
	std::wstring path, name;
	if (!SEP_PATH_NAME(m_path_name, path, name))
		return READER_OPEN_FAIL;
	//std::wstring suffix = GET_SUFFIX(m_path_name);

	//determine if it is an ImageJ hyperstack
	std::string img_desc;
	OpenTiff(m_path_name.c_str());
	uint64_t bits = GetTiffField(kBitsPerSampleTag);
	if (bits > 16)
	{
		CloseTiff();
		return READER_FP32_DATA;
	}
	GetImageDescription(img_desc);
	std::string search_str = "hyperstack=true";
	int64_t str_pos = img_desc.find(search_str);
	bool hyperstack = str_pos != -1;
	search_str = "frames=";
	str_pos = img_desc.find(search_str);
	hyperstack = hyperstack || str_pos != -1;
	search_str = "ImageJ=";
	str_pos = img_desc.find(search_str);
	bool imagej = str_pos != -1;
	search_str = "images=";
	bool imagej_raw = str_pos != -1;
	if (imagej && hyperstack)
	{
		//it is an ImageJ hyperstack, get information from the description
		int num;
		//channels
		search_str = "channels=";
		str_pos = img_desc.find(search_str);
		if (str_pos != -1)
			num = get_number(img_desc, str_pos + search_str.length());
		else
			num = 1;
		if (num)
			m_chan_num = num;
		else
			m_chan_num = 1;
		//slices
		search_str = "slices=";
		str_pos = img_desc.find(search_str);
		if (str_pos != -1)
			num = get_number(img_desc, str_pos + search_str.length());
		else
			num = 1;
		if (num)
			m_slice_num = num;
		else
			m_slice_num = 1;
		//frames
		search_str = "frames=";
		str_pos = img_desc.find(search_str);
		if (str_pos != -1)
			num = get_number(img_desc, str_pos + search_str.length());
		else
			num = 1;
		if (num)
			m_time_num = num;
		else
			m_time_num = 1;
		//max
		if (bits == 16)
		{
			search_str = "max=";
			str_pos = img_desc.find(search_str);
			if (str_pos != -1)
				num = get_number(img_desc, str_pos + search_str.length());
			else
				num = 0;
			if (num)
			{
				m_max_value = num;
				m_scalar_scale = 65535.0 / m_max_value;
			}
		}

		std::vector<std::wstring> list;
		if (m_time_num == 1)
		{
			if (FIND_FILES_4D(m_path_name, m_time_id, list, m_cur_time))
			{
				isHsTimeSeq_ = true;
				m_time_num = list.size();
			}
		}

		//build sequence information for the hyperstack
		isHyperstack_ = true;
		m_data_name = name;
		int page_cnt = 0;
		for (int i = 0; i < m_time_num; ++i)
		{
			TimeDataInfo info;

			info.type = 0;
			std::wstring str;
			if (isHsTimeSeq_)
			{
				page_cnt = 0;
				int64_t begin = m_path_name.find(m_time_id);
				size_t id_len = m_time_id.length();
				str = list.at(i);
				std::wstring t_num;
				for (size_t j = begin + id_len; j < str.size(); j++)
				{
					wchar_t c = str[j];
					if (iswdigit(c))
						t_num.push_back(c);
					else break;
				}
				if (t_num.size() > 0)
					info.filenumber = WSTOI(t_num);
				else
					info.filenumber = 0;
			}
			else
			{
				info.filenumber = 0;
			}

			//add slices
			for (int j = 0; j < m_slice_num; ++j)
			{
				SliceInfo sliceinfo;
				sliceinfo.slicenumber = 0;
				if (isHsTimeSeq_)
					sliceinfo.slice = str;
				sliceinfo.pagenumber = page_cnt;
				page_cnt += m_chan_num;
				info.slices.push_back(sliceinfo);
			}
			m_4d_seq.push_back(info);
		}
		if (isHsTimeSeq_ && m_4d_seq.size() > 0)
			std::sort(m_4d_seq.begin(), m_4d_seq.end(), TIFReader::tif_sort);

		m_cur_time = 0;
	}
	else if (imagej && imagej_raw)
	{
		int num;
		//slices
		search_str = "images=";
		str_pos = img_desc.find(search_str);
		if (str_pos != -1)
			num = get_number(img_desc, str_pos + search_str.length());
		else
			num = 1;
		if (num)
		{
			m_slice_num = num;
			imagej_raw_possible_ = true;
		}
		else
			m_slice_num = 1;
	}
	CloseTiff();
	
	if (!isHyperstack_)
	{
		//it is not an ImageJ hyperstack, do the usual processing
		//build 4d sequence
		//search time sequence files
		std::vector<std::wstring> list;
		if (!FIND_FILES_4D(m_path_name, m_time_id, list, m_cur_time))
		{
			TimeDataInfo info;
			SliceInfo sliceinfo;
			sliceinfo.slice = path + name;  //temporary name
			sliceinfo.slicenumber = 0;
			info.slices.push_back(sliceinfo);
			info.filenumber = 0;
			m_4d_seq.push_back(info);
			m_cur_time = 0;
		}
		else
		{
			int64_t begin = m_path_name.find(m_time_id);
			size_t id_len = m_time_id.length();
			for (size_t i = 0; i < list.size(); i++) {
				TimeDataInfo info;
				std::wstring str = list.at(i);
				std::wstring t_num;
				for (size_t j = begin + id_len; j < str.size(); j++)
				{
					wchar_t c = str[j];
					if (iswdigit(c))
						t_num.push_back(c);
					else break;
				}
				if (t_num.size() > 0)
					info.filenumber = WSTOI(t_num);
				else
					info.filenumber = 0;
				SliceInfo sliceinfo;
				sliceinfo.slice = str;
				sliceinfo.slicenumber = 0;
				info.slices.push_back(sliceinfo);
				m_4d_seq.push_back(info);
			}
		}
		if (m_4d_seq.size() > 0)
			std::sort(m_4d_seq.begin(), m_4d_seq.end(), TIFReader::tif_sort);

		//build 3d slice sequence
		for (int t = 0; t < (int)m_4d_seq.size(); t++)
		{
			std::wstring slice_str = m_4d_seq[t].slices[0].slice;

			if (m_slice_seq)
			{
				//extract common string in name
				size_t pos2 = slice_str.find_last_of(L'.');
				size_t begin2 = 0;
				int64_t end2 = -1;
				for (i = int(pos2) - 1; i >= 0; i--)
				{
					if (iswdigit(slice_str[i]) && end2 == -1)
						end2 = i;
					if (!iswdigit(slice_str[i]) && end2 != -1)
					{
						begin2 = i;
						break;
					}
				}
				if (end2 != -1)
				{
					//search slice sequence
					std::vector<std::wstring> list;
					std::wstring search_ext = slice_str.substr(end2 + 1);
					std::wstring regex = slice_str.substr(0, begin2 + 1);
					FIND_FILES(path, search_ext, list, m_cur_time, regex);
					m_4d_seq[t].type = 1;
					m_4d_seq[t].slices.clear();
					for (size_t f = 0; f < list.size(); f++) {
						size_t start_idx = begin2 + 1;
						size_t end_idx = list.at(f).find(search_ext);
						size_t size = end_idx - start_idx;
						std::wstring fileno = list.at(f).substr(start_idx, size);
						SliceInfo slice;
						slice.slice = list.at(f);
						slice.slicenumber = WSTOI(fileno);
						m_4d_seq[t].slices.push_back(slice);
					}
					if (m_4d_seq[t].slices.size() > 0)
						std::sort(m_4d_seq[t].slices.begin(),
							m_4d_seq[t].slices.end(),
							TIFReader::tif_slice_sort);
				}
			}
			else
			{
				m_4d_seq[t].type = 0;
				m_4d_seq[t].slices[0].slice = slice_str;
				if (m_4d_seq[t].slices[0].slice == m_path_name)
					m_cur_time = t;
			}
		}

		//get time number and channel number
		m_time_num = (int)m_4d_seq.size();
		if (m_4d_seq.size() > 0 &&
			m_cur_time >= 0 &&
			m_cur_time < (int)m_4d_seq.size() &&
			m_4d_seq[m_cur_time].slices.size()>0)
		{
			std::wstring tiff_name = m_4d_seq[m_cur_time].slices[0].slice;
			if (tiff_name.size() > 0)
			{
				InvalidatePageInfo();
				OpenTiff(tiff_name);
				m_chan_num = GetTiffField(kSamplesPerPixelTag);
				if (m_chan_num == 0 &&
					GetTiffField(kImageWidthTag) > 0 &&
					GetTiffField(kImageLengthTag) > 0) {
					m_chan_num = 1;
				}
				CloseTiff();
			}
			else m_chan_num = 0;
		}
		else m_chan_num = 0;
	}

	return READER_OK;
}

uint64_t TIFReader::GetTiffField(const uint64_t in_tag)
{
	if (!m_page_info.b_valid)
		ReadTiffFields();

	if (m_page_info.b_valid)
	{
		switch (in_tag)
		{
		case kSubFileTypeTag:
			if (m_page_info.b_sub_file_type)
				return m_page_info.ul_sub_file_type;
			break;
		case kImageWidthTag:
			if (m_page_info.b_image_width)
				return m_page_info.ul_image_width;
			break;
		case kImageLengthTag:
			if (m_page_info.b_image_length)
				return m_page_info.ul_image_length;
			break;
		case kBitsPerSampleTag:
			if (m_page_info.b_bits_per_sample)
				return m_page_info.us_bits_per_sample;
			break;
		case kCompressionTag:
			if (m_page_info.b_compression)
				return m_page_info.us_compression;
			break;
		case kPredictionTag:
			if (m_page_info.b_prediction)
				return m_page_info.us_prediction;
			break;
		case kPlanarConfigurationTag:
			if (m_page_info.b_planar_config)
				return m_page_info.us_planar_config;
			break;
		case kSamplesPerPixelTag:
			if (m_page_info.b_samples_per_pixel)
				return m_page_info.us_samples_per_pixel;
			break;
		case kRowsPerStripTag:
			if (m_page_info.b_rows_per_strip)
				return m_page_info.ul_rows_per_strip;
			break;
		case kTileWidthTag:
			if (m_page_info.b_tile_width)
				return m_page_info.ul_tile_width;
			break;
		case kTileLengthTag:
			if (m_page_info.b_tile_length)
				return m_page_info.ul_tile_length;
			break;
		}
	}

	return 0;
}

/*uint64_t TIFReader::GetTiffStripOffsetOrCount(uint64_t tag, uint64_t strip)
{
	//search for the offset that tells us the byte count for this strip
	uint64_t type = GetTiffField(tag, NULL, kType);
	switch (type) {
	case kLong8:
		type = 8;
		break;
	case kLong:
		type = 4;
		break;
	default:
		type = 2;
		break;
	}
	uint64_t offset = GetTiffField(tag, NULL, kOffset);
	if (offset == 0 && tag == kStripBytesCountTag)
		return 0;
	uint64_t address = GetTiffField(tag, NULL, kValueAddress);
	uint64_t cnt = GetTiffField(tag, NULL, kCount);
	// if the values do not fit into the tag data, jump to them
	if (cnt * type > (isBig_ ? 8 : 4)) {
		tiff_stream.seekg(offset + type*strip, tiff_stream.beg);
	}
	else {
		//otherwise, return the value of interest within the tag
		tiff_stream.seekg(address + type*strip, tiff_stream.beg);
	}
	uint64_t value = 0;
	uint32_t tmp = 0;
	uint16_t tmp2 = 0;
	switch (type) {
	case kLong8:
		tiff_stream.read((char*)&value, sizeof(uint64_t));
		value = (swap_) ? SwapLong(value) : value;
		break;
	case kLong:
		tiff_stream.read((char*)&tmp, sizeof(uint32_t));
		value = static_cast<uint64_t>((swap_) ? SwapWord(tmp) : tmp);
		break;
	default:
		tiff_stream.read((char*)&tmp2, sizeof(uint16_t));
		value = static_cast<uint64_t>((swap_) ? SwapShort(tmp2) : tmp2);
		break;
	}
	return value;
}*/

bool TIFReader::TagInInfo(uint16_t tag)
{
	if (tag == kSubFileTypeTag ||
		tag == kImageWidthTag ||
		tag == kImageLengthTag ||
		tag == kBitsPerSampleTag ||
		tag == kCompressionTag ||
		tag == kPredictionTag ||
		tag == kPlanarConfigurationTag ||
		tag == kImageDescriptionTag ||
		tag == kStripOffsetsTag ||
		tag == kSamplesPerPixelTag ||
		tag == kRowsPerStripTag ||
		tag == kStripBytesCountsTag ||
		tag == kXResolutionTag ||
		tag == kYResolutionTag ||
		tag == kTileWidthTag ||
		tag == kTileLengthTag ||
		tag == kTileOffsetsTag ||
		tag == kTileBytesCountsTag)
		return true;
	else
		return false;
}

void TIFReader::SetPageInfo(uint16_t tag, uint64_t answer)
{
	//save info
	switch (tag)
	{
	case kSubFileTypeTag:
		m_page_info.b_sub_file_type = true;
		m_page_info.ul_sub_file_type = static_cast<unsigned long>(answer);
		break;
	case kImageWidthTag:
		m_page_info.b_image_width = true;
		if (answer)
			m_page_info.ul_image_width = static_cast<unsigned long>(answer);
		break;
	case kImageLengthTag:
		m_page_info.b_image_length = true;
		if (answer)
			m_page_info.ul_image_length = static_cast<unsigned long>(answer);
		break;
	case kBitsPerSampleTag:
		m_page_info.b_bits_per_sample = true;
		if (answer)
			m_page_info.us_bits_per_sample = static_cast<unsigned short>(answer);
		break;
	case kCompressionTag:
		m_page_info.b_compression = true;
		m_page_info.us_compression = static_cast<unsigned short>(answer);
		break;
	case kPredictionTag:
		m_page_info.b_prediction = true;
		m_page_info.us_prediction = static_cast<unsigned short>(answer);
		break;
	case kPlanarConfigurationTag:
		m_page_info.b_planar_config = true;
		m_page_info.us_planar_config = static_cast<unsigned short>(answer);
		break;
	case kSamplesPerPixelTag:
		m_page_info.b_samples_per_pixel = true;
		if (answer)
			m_page_info.us_samples_per_pixel = static_cast<unsigned short>(answer);
		break;
	case kRowsPerStripTag:
		m_page_info.b_rows_per_strip = true;
		if (answer)
			m_page_info.ul_rows_per_strip = static_cast<unsigned long>(answer);
		break;
	case kXResolutionTag:
		m_page_info.b_x_resolution = true;
		memcpy(&(m_page_info.d_x_resolution), &answer, sizeof(double));
		break;
	case kYResolutionTag:
		m_page_info.b_y_resolution = true;
		memcpy(&(m_page_info.d_y_resolution), &answer, sizeof(double));
		break;
	case kTileWidthTag:
		m_page_info.b_tile_width = true;
		if (answer)
			m_page_info.ul_tile_width = static_cast<unsigned long>(answer);
		break;
	case kTileLengthTag:
		m_page_info.b_tile_length = true;
		if (answer)
			m_page_info.ul_tile_length = static_cast<unsigned long>(answer);
		break;
	case kStripOffsetsTag:
		m_page_info.b_strip_offsets = true;
		m_page_info.ull_strip_offsets.resize(1);
		m_page_info.ull_strip_offsets[0] = static_cast<unsigned long long>(answer);
		break;
	case kStripBytesCountsTag:
		m_page_info.b_strip_byte_counts = true;
		m_page_info.ull_strip_byte_counts.resize(1);
		m_page_info.ull_strip_byte_counts[0] = static_cast<unsigned long long>(answer);
		break;
	case kTileOffsetsTag:
		m_page_info.b_tile_offsets = true;
		m_page_info.ull_tile_offsets.resize(1);
		m_page_info.ull_tile_offsets[0] = static_cast<unsigned long long>(answer);
		break;
	case kTileBytesCountsTag:
		m_page_info.b_tile_byte_counts = true;
		m_page_info.ull_tile_byte_counts.resize(1);
		m_page_info.ull_tile_byte_counts[0] = static_cast<unsigned long long>(answer);
		break;
	}
}

void TIFReader::SetPageInfoVector(uint16_t tag, uint16_t type, uint64_t cnt, void* data)
{
	std::vector<unsigned long long>* v = 0;
	if (tag == kStripOffsetsTag)
	{
		m_page_info.b_strip_offsets = true;
		v = &(m_page_info.ull_strip_offsets);
	}
	else if (tag == kStripBytesCountsTag)
	{
		m_page_info.b_strip_byte_counts = true;
		v = &(m_page_info.ull_strip_byte_counts);
	}
	else if (tag == kTileOffsetsTag)
	{
		m_page_info.b_tile_offsets = true;
		v = &(m_page_info.ull_tile_offsets);
	}
	else if (tag == kTileBytesCountsTag)
	{
		m_page_info.b_tile_byte_counts = true;
		v = &(m_page_info.ull_tile_byte_counts);
	}
	else if (tag == kBitsPerSampleTag)
	{
		if (type == kShort)
		{
			uint16_t *data2 = (uint16*)data;
			m_page_info.b_bits_per_sample = true;
			m_page_info.us_bits_per_sample =
				swap_ ? SwapShort(data2[0]) : data2[0];
		}
		return;
	}

	v->clear();
	v->reserve(cnt);
	unsigned long long value;
	if (type == kShort)
	{
		uint16_t *data2 = (uint16_t*)data;
		for (uint64_t i = 0; i < cnt; ++i)
		{
			value = static_cast<unsigned long long>(
				swap_ ? SwapShort(data2[i]) : data2[i]);
			v->push_back(value);
		}
	}
	else if (type == kLong)
	{
		uint32_t *data2 = (uint32_t*)data;
		for (uint64_t i = 0; i < cnt; ++i)
		{
			value = static_cast<unsigned long long>(
				swap_ ? SwapWord(data2[i]) : data2[i]);
			v->push_back(value);
		}
	}
	else if (type == kLong8)
	{
		uint64_t *data2 = (uint64_t*)data;
		for (uint64_t i = 0; i < cnt; ++i)
		{
			value = static_cast<unsigned long long>(
				swap_ ? SwapLong(data2[i]) : data2[i]);
			v->push_back(value);
		}
	}
}

void TIFReader::ReadTiffFields()
{
	if (m_page_info.b_valid)
		return;

	if (!tiff_stream.is_open())
		throw std::runtime_error("TIFF File not open for reading.");
	//go to the current IFD block/page
	tiff_stream.seekg(current_offset_, tiff_stream.beg);
	uint64_t num_entries = 0;
	//how many entries are there?
	if (isBig_)
	{
		tiff_stream.read((char*)&num_entries, sizeof(uint64_t));
		if (swap_) num_entries = SwapLong(num_entries);
	}
	else
	{
		uint16_t temp;
		tiff_stream.read((char*)&temp, sizeof(uint16_t));
		if (swap_) temp = SwapShort(temp);
		num_entries = static_cast<uint64_t>(temp);
	}
	char start_off = 2, multiplier = 12;
	if (isBig_) { start_off = 8; multiplier = 20; }

	//go through all of the entries to find the one we want
	for (size_t i = 0; i < num_entries; i++)
	{
		tiff_stream.seekg(current_offset_ + start_off + multiplier*i, tiff_stream.beg);
		uint16_t tag = 0;
		//get the tag of entry
		tiff_stream.read((char*)&tag, sizeof(uint16_t));
		if (swap_) tag = SwapShort(tag);
		// This is the tag we use, grab and go.
		if (TagInInfo(tag))
		{
			//find the type
			tiff_stream.seekg(current_offset_ + start_off + multiplier*i + 2, tiff_stream.beg);
			uint16_t type = 0;
			tiff_stream.read((char*)&type, sizeof(uint16_t));
			if (swap_) type = SwapShort(type);
			//if (!buf && size == kType)
			//{
			//	//number of
			//	//return static_cast<uint64_t>(type);
			//}
			//find the count
			tiff_stream.seekg(current_offset_ + start_off + multiplier*i + 4, tiff_stream.beg);
			uint64_t cnt = 0;
			if (isBig_)
			{
				tiff_stream.read((char*)&cnt, sizeof(uint64_t));
				if (swap_) cnt = SwapLong(cnt);
			}
			else
			{
				uint32_t tmp = 0;
				tiff_stream.read((char*)&tmp, sizeof(uint32_t));
				if (swap_) tmp = SwapWord(tmp);
				cnt = static_cast<uint64_t>(tmp);
			}

			if (tag == kStripOffsetsTag)
			{
				m_page_info.b_strip_num = true;
				m_page_info.ul_strip_num = static_cast<unsigned long>(cnt);
			}
			else if (tag == kTileOffsetsTag)
			{
				m_page_info.b_tile_num = true;
				m_page_info.ul_tile_num = static_cast<unsigned long>(cnt);
				m_page_info.b_use_tiles = true;
			}

			//now get the value (different for different types.)
			char off2 = isBig_ ? 12 : 8;
			uint32_t addr32;
			uint64_t addr64 = current_offset_ + start_off + multiplier*i + off2;
			//if (size == kValueAddress) return addr;
			tiff_stream.seekg(addr64, tiff_stream.beg);

			uint64_t answer = 0;
			char* buf = 0;
			if (type == kByte)
			{
				uint8_t value = 0;
				tiff_stream.read((char*)&value, sizeof(uint8_t));
				answer = static_cast<uint64_t>(value);
			}
			else if (type == kASCII)
			{
				if (cnt > 1)
				{
					if (isBig_)
					{
						tiff_stream.read((char*)&addr64, sizeof(uint64_t));
						addr64 = swap_ ? SwapLong(addr64) : addr64;
					}
					else
					{
						tiff_stream.read((char*)&addr32, sizeof(uint32_t));
						addr64 = static_cast<uint64_t>(swap_ ? SwapWord(addr32) : addr32);
					}
					tiff_stream.seekg(addr64, tiff_stream.beg);
				}
				//read the the string
				buf = new char[cnt];
				tiff_stream.read(buf, cnt);
			}
			else if (type == kShort)
			{
				uint16_t value = 0;
				if (cnt > (isBig_ ? 4 : 2))
				{
					if (isBig_)
					{
						tiff_stream.read((char*)&addr64, sizeof(uint64_t));
						addr64 = swap_ ? SwapLong(addr64) : addr64;
					}
					else
					{
						tiff_stream.read((char*)&addr32, sizeof(uint32_t));
						addr64 = static_cast<uint64_t>(swap_ ? SwapWord(addr32) : addr32);
					}
					tiff_stream.seekg(addr64, tiff_stream.beg);
					buf = (char*)(new uint16_t[cnt]);
					tiff_stream.read((char*)buf, sizeof(uint16_t)*cnt);
				}
				else
				{
					tiff_stream.read((char*)&value, sizeof(uint16_t));
					if (swap_) value = SwapShort(value);
					answer = static_cast<uint64_t>(value);
				}
			}
			else if (type == kLong)
			{
				uint32_t value = 0;
				if (cnt > (isBig_ ? 2 : 1))
				{
					if (isBig_)
					{
						tiff_stream.read((char*)&addr64, sizeof(uint64_t));
						addr64 = swap_ ? SwapLong(addr64) : addr64;
					}
					else
					{
						tiff_stream.read((char*)&addr32, sizeof(uint32_t));
						addr64 = static_cast<uint64_t>(swap_ ? SwapWord(addr32) : addr32);
					}
					tiff_stream.seekg(addr64, tiff_stream.beg);
					buf = (char*)(new uint32_t[cnt]);
					tiff_stream.read((char*)buf, sizeof(uint32_t)*cnt);
				}
				else
				{
					tiff_stream.read((char*)&value, sizeof(uint32_t));
					if (swap_) value = SwapWord(value);
					answer = static_cast<uint64_t>(value);
				}
			}
			else if (type == kLong8)
			{
				if (cnt > 1)
				{
					tiff_stream.read((char*)&addr64, sizeof(uint64_t));
					addr64 = swap_ ? SwapLong(addr64) : addr64;
					tiff_stream.seekg(addr64, tiff_stream.beg);
					buf = (char*)(new uint64_t[cnt]);
					tiff_stream.read((char*)buf, sizeof(uint64_t)*cnt);
				}
				else
				{
					tiff_stream.read((char*)&answer, sizeof(uint64_t));
					if (swap_) answer = SwapLong(answer);
				}
			}
			else if (type == kRational)
			{
				//get the two values in the data to make a float.
				uint32_t value = 0;
				tiff_stream.read((char*)&value, sizeof(uint32_t));
				if (swap_) value = SwapWord(value);
				tiff_stream.seekg(value, tiff_stream.beg);
				uint32_t num = 0, den = 0;
				tiff_stream.read((char*)&num, sizeof(uint32_t));
				if (swap_) num = SwapWord(num);
				tiff_stream.seekg(value + sizeof(uint32_t), tiff_stream.beg);
				tiff_stream.read((char*)&den, sizeof(uint32_t));
				if (swap_) den = SwapWord(den);
				double rat = static_cast<double>(num) /
					static_cast<double>(den);
				memcpy(&answer, &rat, sizeof(double));
			}
			else
			{
				std::cerr << "Unhandled TIFF Tag type" << std::endl;
			}

			if (tag == kImageDescriptionTag && buf)
			{
				m_page_info.b_image_desc = true;
				m_page_info.s_image_desc = buf;
			}
			else if (!answer && buf)
				SetPageInfoVector(tag, type, cnt, (void*)buf);
			else
				SetPageInfo(tag, answer);
			if (buf) delete[] buf;
		}
	}
	
	//get the next page offset
	tiff_stream.seekg(current_offset_ + start_off + multiplier*num_entries, tiff_stream.beg);
	if (isBig_)
	{
		uint64_t next_offset = 0;
		tiff_stream.read((char*)&next_offset, sizeof(uint64_t));
		if (swap_)
			m_page_info.ull_next_page_offset = SwapLong(next_offset);
		else
			m_page_info.ull_next_page_offset = next_offset;
		m_page_info.b_next_page_offset = true;
	}
	else
	{
		uint32_t next_offset = 0;
		tiff_stream.read((char*)&next_offset, sizeof(uint32_t));
		if (swap_)
			m_page_info.ull_next_page_offset = static_cast<uint64_t>(SwapWord(next_offset));
		else
			m_page_info.ull_next_page_offset = static_cast<uint64_t>(next_offset);
		m_page_info.b_next_page_offset = true;
	}

	m_page_info.b_valid = true;
}

uint64_t TIFReader::GetTiffNextPageOffset()
{
	uint64_t results = 0;
	if (!tiff_stream.is_open())
		throw std::runtime_error("TIFF File not open for reading.");
	//go to the current IFD block/page
	tiff_stream.seekg(current_offset_, tiff_stream.beg);
	uint64_t num_entries = 0;
	//how many entries are there?
	if (isBig_)
	{
		tiff_stream.read((char*)&num_entries, sizeof(uint64_t));
		if (swap_) num_entries = SwapLong(num_entries);
	}
	else
	{
		uint16_t temp;
		tiff_stream.read((char*)&temp, sizeof(uint16_t));
		if (swap_) temp = SwapShort(temp);
		num_entries = static_cast<uint64_t>(temp);
	}
	char start_off = 2, multiplier = 12;
	if (isBig_) { start_off = 8; multiplier = 20; }
	//get the next page offset
	tiff_stream.seekg(current_offset_ + start_off + multiplier*num_entries, tiff_stream.beg);
	if (isBig_)
	{
		uint64_t next_offset = 0;
		tiff_stream.read((char*)&next_offset, sizeof(uint64_t));
		if (swap_)
			results = SwapLong(next_offset);
		else
			results = next_offset;
	}
	else
	{
		uint32_t next_offset = 0;
		tiff_stream.read((char*)&next_offset, sizeof(uint32_t));
		if (swap_)
			results = static_cast<uint64_t>(SwapWord(next_offset));
		else
			results = static_cast<uint64_t>(next_offset);
	}
	return results;
}

uint64_t TIFReader::TurnToPage(uint64_t page)
{
	uint64_t page_save = current_page_;
	//make sure we are on the correct page,
	//reset if ahead
	if (current_page_ > page)
		ResetTiff();
	// fast forward if we are behind.
	if (!imagej_raw_)
	{
		while (current_page_ < page)
		{
			tiff_stream.seekg(current_offset_, tiff_stream.beg);
			if (GetTiffField(kSubFileTypeTag) != 1) current_page_++;
			current_offset_ = GetTiffNextPageOffset();
		}
		if (!current_offset_)
			imagej_raw_ = true;
	}
	if (current_page_ != page_save &&
		!imagej_raw_)
		InvalidatePageInfo();
	return current_page_;
}

uint16_t TIFReader::SwapShort(uint16_t num) {
	return ((num & 0x00FF) << 8) | ((num & 0xFF00) >> 8);
}

uint32_t TIFReader::SwapWord(uint32_t num) {
	return ((num & 0x000000FF) << 24) | ((num & 0xFF000000) >> 24) |
		((num & 0x0000FF00) << 8) | ((num & 0x00FF0000) >> 8);
}

uint64_t TIFReader::SwapLong(uint64_t num) {
	return
		((num & 0x00000000000000FF) << 56) |
		((num & 0xFF00000000000000) >> 56) |
		((num & 0x000000000000FF00) << 40) |
		((num & 0x00FF000000000000) >> 40) |
		((num & 0x0000000000FF0000) << 24) |
		((num & 0x0000FF0000000000) >> 24) |
		((num & 0x00000000FF000000) << 8) |
		((num & 0x000000FF00000000) >> 8);
}

void TIFReader::SetSliceSeq(bool ss)
{
	//enable searching for slices
	m_slice_seq = ss;
}

bool TIFReader::GetSliceSeq()
{
	return m_slice_seq;
}

void TIFReader::SetTimeId(std::wstring &id)
{
	m_time_id = id;
}

std::wstring TIFReader::GetTimeId()
{
	return m_time_id;
}

void TIFReader::SetBatch(bool batch)
{
	if (batch)
	{
		//separate path and name
		std::wstring search_path = GET_PATH(m_path_name);
		std::wstring suffix = GET_SUFFIX(m_path_name);
		FIND_FILES(search_path, suffix, m_batch_list, m_cur_batch);
		m_batch = true;
	}
	else
		m_batch = false;
}

bool TIFReader::IsNewBatchFile(std::wstring name)
{
	if (m_batch_list.size() == 0)
		return true;

	for (int i = 0; i < (int)m_batch_list.size(); i++)
	{
		if (IsBatchFileIdentical(name, m_batch_list[i]))
			return false;
	}

	return true;
}

bool TIFReader::IsBatchFileIdentical(std::wstring name1, std::wstring name2)
{
	if (m_4d_seq.size() > 1)
	{
		int64_t pos = name1.find(m_time_id);
		if (pos == -1)
			return false;
		std::wstring find_str = name1.substr(0, pos + 2);
		pos = name2.find(find_str);
		if (pos == -1)
			return false;
		else
			return true;
	}
	else if (m_slice_seq)
	{
		int64_t pos = name1.find_last_of(L'.');
		int64_t begin = -1;
		int64_t end = -1;
		for (int i = int(pos) - 1; i >= 0; i--)
		{
			if (iswdigit(name1[i]) && end == -1 && begin == -1)
				end = i;
			if (!iswdigit(name1[i]) && end != -1 && begin == -1)
			{
				begin = i;
				break;
			}
		}
		if (begin == -1)
			return false;
		else
		{
			std::wstring find_str = name1.substr(0, begin + 1);
			pos = name2.find(find_str);
			if (pos == -1)
				return false;
			else
				return true;
		}
	}
	else
	{
		if (name1 == name2)
			return true;
		else
			return false;
	}
}

int TIFReader::LoadBatch(int index)
{
	int result = -1;
	if (index >= 0 && index < (int)m_batch_list.size())
	{
		m_path_name = m_batch_list[index];
		Preprocess();
		result = index;
		m_cur_batch = result;
	}
	else
		result = -1;

	return result;
}

Nrrd* TIFReader::Convert(int t, int c, bool get_max)
{
	if (t < 0 || t >= m_time_num ||
		c < 0 || c >= m_chan_num)
		return 0;

	if (isHyperstack_ && m_max_value)
		get_max = false;

	Nrrd* data = 0;
	TimeDataInfo chan_info = m_4d_seq[t];
	if (!isHyperstack_ || isHsTimeSeq_)
		m_data_name = GET_NAME(chan_info.slices[0].slice);
	data = ReadTiff(chan_info.slices, c, get_max);
	m_cur_time = t;
	return data;
}

std::wstring TIFReader::GetCurDataName(int t, int c)
{
	if (isHyperstack_ && !isHsTimeSeq_)
		return m_path_name;
	else
	{
		if (t >= 0 && t < (int64_t)m_4d_seq.size())
			return (m_4d_seq[t].slices)[0].slice;
		else
			return L"";
	}
}

std::wstring TIFReader::GetCurMaskName(int t, int c)
{
	if (isHyperstack_)
	{
		std::wostringstream woss;
		woss << m_path_name.substr(0, m_path_name.find_last_of('.'));
		if (m_time_num > 1) woss << "_T" << t;
		if (m_chan_num > 1) woss << "_C" << c;
		woss << ".msk";
		std::wstring mask_name = woss.str();
		return mask_name;
	}
	else
	{
		if (t >= 0 && t < (int64_t)m_4d_seq.size())
		{
			std::wstring data_name = (m_4d_seq[t].slices)[0].slice;
			std::wostringstream woss;
			woss << data_name.substr(0, data_name.find_last_of('.'));
			if (m_chan_num > 1) woss << "_C" << c;
			woss << ".msk";
			std::wstring mask_name = woss.str();
			return mask_name;
		}
		else
			return L"";
	}
}

std::wstring TIFReader::GetCurLabelName(int t, int c)
{
	if (isHyperstack_)
	{
		std::wostringstream woss;
		woss << m_path_name.substr(0, m_path_name.find_last_of('.'));
		if (m_time_num > 1) woss << "_T" << t;
		if (m_chan_num > 1) woss << "_C" << c;
		woss << ".lbl";
		std::wstring label_name = woss.str();
		return label_name;
	}
	else
	{
		if (t >= 0 && t < (int64_t)m_4d_seq.size())
		{
			std::wstring data_name = (m_4d_seq[t].slices)[0].slice;
			std::wostringstream woss;
			woss << data_name.substr(0, data_name.find_last_of('.'));
			if (m_chan_num > 1) woss << "_C" << c;
			woss << ".lbl";
			std::wstring label_name = woss.str();
			return label_name;
		}
		else
			return L"";
	}
}

bool TIFReader::tif_sort(const TimeDataInfo& info1, const TimeDataInfo& info2)
{
	return info1.filenumber < info2.filenumber;
}

bool TIFReader::tif_slice_sort(const SliceInfo& info1, const SliceInfo& info2)
{
	return info1.slicenumber < info2.slicenumber;
}

uint64_t TIFReader::GetNumTiffPages()
{
	if (!m_b_page_num)
	{
		uint64_t count = 0;
		uint64_t save_offset = current_offset_;
		uint64_t save_page = current_page_;
		ResetTiff();
		while (true)
		{
			// count it if it's not a thumbnail
			if (GetTiffField(kSubFileTypeTag) != 1) count++;
			current_offset_ = GetTiffNextPageOffset();
			if (current_offset_ == 0) break;
		}
		current_offset_ = save_offset;
		current_page_ = save_page;
		m_ull_page_num = count;
		m_b_page_num = true;
	}

	return m_ull_page_num;
}

void TIFReader::GetTiffStrip(uint64_t page, uint64_t strip,
	void * data, uint64_t strip_size)
{
	//make sure we are on the correct page,
	//reset if ahead
	if (current_page_ > page)
		ResetTiff();
	// fast forward if we are behind.
	if (!imagej_raw_)
	{
		while (current_page_ < page)
		{
			tiff_stream.seekg(current_offset_, tiff_stream.beg);
			if (GetTiffField(kSubFileTypeTag) != 1) current_page_++;
			current_offset_ = GetTiffNextPageOffset();
		}
	}
	//get the byte count and the strip offset to read data from.
	uint64_t byte_count = 0;
	if (current_offset_ && !imagej_raw_)
	{
		uint64_t strip_byte_count = GetTiffStripCount(strip);
		if (!strip_byte_count)
		{
			uint64_t bits = GetTiffField(kBitsPerSampleTag);
			m_page_info.ull_strip_byte_counts.resize(1);
			m_page_info.ull_strip_byte_counts[0] = strip_size * bits /8;
			m_page_info.b_strip_byte_counts = true;
			m_page_info.b_strip_offsets = true;
		}
		byte_count = GetTiffStripCount(strip);
		tiff_stream.seekg(GetTiffStripOffset(strip), tiff_stream.beg);
	}
	else
	{
		//if (m_page_info.b_strip_byte_count)
		byte_count = m_page_info.ull_strip_byte_counts[0];
		//if (m_page_info.b_strip_offset)
		tiff_stream.seekg(m_page_info.ull_strip_offsets[0] + page * byte_count, tiff_stream.beg);
		imagej_raw_ = true;
	}
	//actually read the data now
	char *temp = new char[byte_count];
	//unsigned long long pos = tiff_stream.tellg();
	tiff_stream.read((char*)temp, byte_count);
	bool eight_bits = 8 == GetTiffField(kBitsPerSampleTag);
	//get compression tag, decompress if necessary
	uint64_t tmp = GetTiffField(kCompressionTag);
	uint64_t prediction = GetTiffField(kPredictionTag);
	uint64_t samples = GetTiffField(kSamplesPerPixelTag);
	samples = samples == 0 ? 1 : samples;
	tsize_t stride = (GetTiffField(kPlanarConfigurationTag) == 2) ? 1 : samples;
	//uint64_t rows_per_strip = GetTiffField(kRowsPerStripTag,NULL,0);
	uint64_t rows_per_strip = strip_size /
		GetTiffField(kImageWidthTag) /
		samples;
	bool isCompressed = tmp == 5;
	if (isCompressed)
	{
		LZWDecode((tidata_t)temp, (tidata_t)data, strip_size);
		if (prediction == 2)
		{
			for (size_t j = 0; j < rows_per_strip; j++)
				if (eight_bits)
					DecodeAcc8((tidata_t)data + j*m_x_size*samples, m_x_size*samples, stride);
				else
					DecodeAcc16((tidata_t)data + j*m_x_size*samples * 2, m_x_size*samples * 2, stride);
		}
	}
	else
		memcpy(data, temp, byte_count);
	delete[] temp;

	//swap after decompression
	if (swap_ && !eight_bits)
	{
		short * data2 = reinterpret_cast<short*>(data);
		for (uint64_t sh = 0; sh < strip_size / 2; sh++)
			data2[sh] = SwapShort(data2[sh]);
	}
}

//read a tile
void TIFReader::GetTiffTile(uint64_t page, uint64_t tile,
	void *data, uint64_t tile_size, uint64_t tile_height)
{
	uint64_t byte_count = GetTiffTileCount(tile);
	tiff_stream.seekg(GetTiffTileOffset(tile), tiff_stream.beg);
	//actually read the data now
	char *temp = new char[byte_count];
	//unsigned long long pos = tiff_stream.tellg();
	tiff_stream.read((char*)temp, byte_count);
	bool eight_bits = 8 == GetTiffField(kBitsPerSampleTag);
	//get compression tag, decompress if necessary
	uint64_t tmp = GetTiffField(kCompressionTag);
	uint64_t prediction = GetTiffField(kPredictionTag);
	uint64_t samples = GetTiffField(kSamplesPerPixelTag);
	samples = samples == 0 ? 1 : samples;
	tsize_t stride = (GetTiffField(kPlanarConfigurationTag) == 2) ? 1 : samples;
	bool isCompressed = tmp == 5;
	if (isCompressed)
	{
		LZWDecode((tidata_t)temp, (tidata_t)data, tile_size);
		if (prediction == 2)
		{
			for (size_t j = 0; j < tile_height; j++)
				if (eight_bits)
					DecodeAcc8((tidata_t)data + j*m_x_size*samples, m_x_size*samples, stride);
				else
					DecodeAcc16((tidata_t)data + j*m_x_size*samples * 2, m_x_size*samples * 2, stride);
		}
	}
	else
		memcpy(data, temp, byte_count);
	delete[] temp;

	//swap after decompression
	if (swap_ && !eight_bits)
	{
		short * data2 = reinterpret_cast<short*>(data);
		for (uint64_t sh = 0; sh < tile_size / 2; sh++)
			data2[sh] = SwapShort(data2[sh]);
	}
}

void TIFReader::ResetTiff()
{
	if (!tiff_stream.is_open())
		throw std::runtime_error("TIFF file not open for reading.");
	//find the first IFD block/page
	if (!isBig_) {
		tiff_stream.seekg(4, tiff_stream.beg);
		uint32_t temp;
		tiff_stream.read((char*)&temp, sizeof(uint32_t));
		if (swap_) temp = SwapWord(temp);
		current_offset_ = static_cast<uint64_t>(temp);
	}
	else {
		tiff_stream.seekg(8, tiff_stream.beg);
		tiff_stream.read((char*)&current_offset_, sizeof(uint64_t));
		if (swap_) current_offset_ = SwapLong(current_offset_);
	}
	current_page_ = 0;
}

void TIFReader::OpenTiff(std::wstring name)
{
	//open the stream
#ifdef _WIN32
	tiff_stream.open(name.c_str(), std::ifstream::binary);
#else
	tiff_stream.open((ws2s(name)).c_str(), std::ifstream::binary);
#endif
	if (!tiff_stream.is_open())
		throw std::runtime_error("Unable to open TIFF File for reading.");
	tiff_stream.seekg(2, tiff_stream.beg);
	uint16_t tiff_num = 0;
	tiff_stream.read((char*)&tiff_num, sizeof(uint16_t));
	swap_ = SwapShort(tiff_num) == kRegularTiff;
	swap_ |= SwapShort(tiff_num) == kBigTiff;
	isBig_ = (SwapShort(tiff_num) == kBigTiff) || (tiff_num == kBigTiff);
	// make sure this is a proper tiff and set the state.
	if (isBig_ || tiff_num == kRegularTiff || swap_) {
		ResetTiff();
	}
	else {
		throw std::runtime_error("TIFF file formatted incorrectly. Wrong Type.");
	}
}

void TIFReader::CloseTiff() { if (tiff_stream.is_open()) tiff_stream.close(); }

void TIFReader::loadTiffInfo(size_t x, size_t y, size_t pages, int bits)
{
  tiffInfo = std::make_tuple(x,y,pages,bits);
}

std::tuple<size_t,size_t,size_t,int> TIFReader::GetTiffInfo()
{
  return tiffInfo;
}

Nrrd* TIFReader::ReadTiff(std::vector<SliceInfo> &filelist,
	int c, bool get_max)
{
	uint64_t numPages = static_cast<uint64_t>(filelist.size());
	if (numPages <= 0)
		return 0;
	std::wstring filename;
	if (isHyperstack_ && !isHsTimeSeq_)
		filename = m_path_name;
	else
		filename = filelist[0].slice;
	OpenTiff(filename.c_str());
	bool sequence = numPages > 1;
	if (!sequence)
	{
		if (get_max && !isHyperstack_ && !imagej_raw_possible_)
			numPages = GetNumTiffPages();
		else
			numPages = m_slice_num;
	}

	//cache for page info
	//reading first page
	InvalidatePageInfo();

	uint64_t width = GetTiffField(kImageWidthTag);
	uint64_t height = GetTiffField(kImageLengthTag);
	uint64_t bits = GetTiffField(kBitsPerSampleTag);
	uint64_t samples = GetTiffField(kSamplesPerPixelTag);
	if (samples == 0 && width > 0 && height > 0) samples = 1;

	double x_res = 0.0, y_res = 0.0, z_res = 0.0;
	x_res = GetTiffXResolution();
	y_res = GetTiffYResolution();

	std::string img_desc;
	GetImageDescription(img_desc);
	int64_t start = img_desc.find("spacing=");
	if (start != -1) {
		std::string spacing = img_desc.substr(start + 8);
		int64_t end = spacing.find("\n");
		if (end != -1)
			z_res = static_cast<float>(
				atof(spacing.substr(0, end).c_str()));
	}

	if (x_res > 0.0 && y_res > 0.0 && z_res > 0.0) {
		m_xspc = 1 / x_res;
		m_yspc = 1 / y_res;
		m_zspc = z_res;
		if (m_zspc < 1e-3) m_zspc = m_xspc;
		m_valid_spc = true;
	}
	else {
		m_valid_spc = false;
		m_xspc = 1.0;
		m_yspc = 1.0;
		m_zspc = 1.0;
	}

	if (m_resize_type == 1 && m_alignment > 1) {
		m_x_size = (width / m_alignment + (width%m_alignment ? 1 : 0))*m_alignment;
		m_y_size = (height / m_alignment + (height%m_alignment ? 1 : 0))*m_alignment;
	}
	else {
		m_x_size = width;
		m_y_size = height;
	}

	m_slice_num = numPages;
	int64_t pagepixels = (unsigned long long)m_x_size*(unsigned long long)m_y_size;

	if (sequence && !isHyperstack_) CloseTiff();

	Nrrd *nrrdout = nrrdNew();

	//allocate memory
	void *val = 0;
	bool eight_bit = bits == 8;

	unsigned long long total_size = (unsigned long long)m_x_size*
		(unsigned long long)m_y_size*(unsigned long long)numPages;
	//val = malloc(total_size * (eight_bit?1:2));
	val = eight_bit ? (void*)(new unsigned char[total_size]) :
		(void*)(new unsigned short[total_size]);
	if (!val)
		throw std::runtime_error("Unable to allocate memory to read TIFF.");

	int max_value = 0;

	void* buf = 0;
	uint64_t strip_size;
	uint64_t tile_size;
	uint64_t tile_w;
	uint64_t tile_h;
	uint64_t tile_w_last;//last tile width
	uint64_t x_tile_num;
	uint64_t y_tile_num;
	if (GetTiffUseTiles())
	{
		uint64_t tile_num = GetTiffTileNum();
		if (tile_num > 0)
		{
			tile_w = GetTiffField(kTileWidthTag);
			tile_h = GetTiffField(kTileLengthTag);
			tile_size = tile_w * tile_h * samples * (bits / 8);
			x_tile_num = (width + tile_w - 1) / tile_w;
			y_tile_num = (height + tile_h - 1) / tile_h;
			tile_w_last = width - tile_w * (x_tile_num - 1);
		}
		else
		{
			tile_size = height * width * samples * (bits / 8);
			tile_w = width;
			tile_h = height;
			tile_w_last = width;
			x_tile_num = y_tile_num = 1;
		}
	}
	else
	{
		uint64_t rowsperstrip = GetTiffField(kRowsPerStripTag);
		if (rowsperstrip > 0)
			strip_size = rowsperstrip * width * samples * (bits / 8);
		else
			strip_size = height * width * samples * (bits / 8);
	}

	if (isHyperstack_)
	{
		uint64_t pageindex = filelist[0].pagenumber + c;
		for (int i = 0; i < numPages; ++i)
		{
			if (!imagej_raw_)
				TurnToPage(pageindex);
			if (!imagej_raw_)
				ReadTiffFields();
			uint64_t num_strips = GetTiffStripNum();

			//read file
			for (uint64_t strip = 0; strip < num_strips; strip++)
			{
				unsigned long long valindex;
				valindex = (unsigned long long)pagepixels * (unsigned long long)i +
					(unsigned long long)strip * (unsigned long long)strip_size /
					(unsigned long long)(eight_bit ? 1 : 2);

				if (eight_bit)
					GetTiffStrip(pageindex, strip,
					(uint8_t*)val + valindex, strip_size);
				else
					GetTiffStrip(pageindex, strip,
					(uint16_t*)val + valindex, strip_size);
			}
			pageindex += m_chan_num;
			//if (!imagej_raw_)
			//	InvalidatePageInfo();
		}
	}
	else
	{
		for (uint64_t pageindex = 0; pageindex < numPages; pageindex++)
		{
			if (sequence)
			{
				filename = filelist[pageindex].slice;
				OpenTiff(filename);
				InvalidatePageInfo();
			}

			if (!imagej_raw_ && !sequence)
				TurnToPage(pageindex);
			if (!imagej_raw_)
				ReadTiffFields();

			//this is a thumbnail, skip
			if (GetTiffField(kSubFileTypeTag) == 1)
			{
				if (sequence) CloseTiff();
				continue;
			}

			//tile storage
			if (GetTiffUseTiles())
			{
				if (!buf)
					buf = malloc(tile_size);

				uint64_t num_tiles = GetTiffTileNum();
				num_tiles = num_tiles ? num_tiles : 1;

				//read file
				for (uint64_t tile = 0; tile < num_tiles; ++tile)
				{
					uint64_t valindex;
					uint64_t indexinpage;
					if (samples > 1)
					{
						GetTiffTile(sequence ? 0 : pageindex, tile, buf, tile_size, tile_h);
						int num_pixels = tile_size / samples / (eight_bit ? 1 : 2);
						uint64_t tx, ty;//tile coord
						tx = tile % x_tile_num;
						ty = tile / x_tile_num;
						indexinpage = width * ty * tile_h + tx * tile_w;
						valindex = pageindex * pagepixels + indexinpage;
						for (int i = 0; i<num_pixels; i++)
						{
							if (tx == x_tile_num - 1)
							{
								if (i % tile_w == tile_w_last)
									i += tile_w - tile_w_last;
							}
							if (i % tile_w == 0 && i)
							{
								if (tx < x_tile_num - 1)
								{
									indexinpage += width - tile_w;
									valindex += width - tile_w;
								}
								else
								{
									indexinpage += width - tile_w_last;
									valindex += width - tile_w_last;
								}
							}
							if (indexinpage >= pagepixels) break;
							if (eight_bit)
								memcpy((uint8_t*)val + valindex,
								(uint8_t*)buf + samples*i + c,
									sizeof(uint8_t));
							else
								memcpy((uint16_t*)val + valindex,
								(uint16_t*)buf + samples*i + c,
									sizeof(uint16_t));
							indexinpage++;
							valindex++;
						}
					}
					else
					{
						GetTiffTile(sequence ? 0 : pageindex, tile, buf, tile_size, tile_h);
						uint64_t tx, ty;//tile coord
						tx = tile % x_tile_num;
						ty = tile / x_tile_num;
						indexinpage = width * ty * tile_h + tx * tile_w;
						valindex = pageindex * pagepixels + indexinpage;
						//copy tile
						for (int i = 0; i < tile_h; ++i)
						{
							if (indexinpage >= pagepixels) break;
							if (tx < x_tile_num-1)
							{
								if (eight_bit)
									memcpy((uint8_t*)val + valindex,
									(uint8_t*)buf + i*tile_w,
										sizeof(uint8_t)*tile_w);
								else
									memcpy((uint16_t*)val + valindex,
									(uint16_t*)buf + i*tile_w,
										sizeof(uint16_t)*tile_w);
							}
							else
							{
								if (eight_bit)
									memcpy((uint8_t*)val + valindex,
									(uint8_t*)buf + i*tile_w,
										sizeof(uint8_t)*tile_w_last);
								else
									memcpy((uint16_t*)val + valindex,
									(uint16_t*)buf + i*tile_w,
										sizeof(uint16_t)*tile_w_last);
							}
							indexinpage += width;
							valindex += width;
						}
					}
				}
			}
			else//strip storage
			{
				if (samples > 1 && !buf)
					buf = malloc(strip_size);

				uint64_t num_strips = GetTiffStripNum();
				num_strips = num_strips ? num_strips : 1;

				//read file
				for (uint64_t strip = 0; strip < num_strips; ++strip)
				{
					long long valindex;
					int indexinpage;
					if (samples > 1)
					{
						GetTiffStrip(sequence ? 0 : pageindex, strip, buf, strip_size);
						int num_pixels = strip_size / samples / (eight_bit ? 1 : 2);
						indexinpage = strip*num_pixels;
						valindex = pageindex*pagepixels + indexinpage;
						for (int i = 0; i<num_pixels; i++)
						{
							if (indexinpage++ >= pagepixels) break;
							if (eight_bit)
								memcpy((uint8_t*)val + valindex,
									(uint8_t*)buf + samples*i + c, sizeof(uint8_t));
							else
								memcpy((uint16_t*)val + valindex,
									(uint16_t*)buf + samples*i + c, sizeof(uint16_t));
							if (!eight_bit && get_max &&
								*((uint16_t*)val + valindex) > max_value)
								max_value = *((uint16_t*)val + valindex);
							valindex++;
						}
					}
					else
					{
						valindex = pageindex*pagepixels +
							strip*strip_size / (eight_bit ? 1 : 2);
						uint64_t strip_size_used = strip_size;
						if (valindex + strip_size / (eight_bit ? 1 : 2) >= total_size)
							strip_size_used = (total_size - valindex) * (eight_bit ? 1 : 2);
						if (strip_size_used > 0)
						{
							if (eight_bit)
								GetTiffStrip(sequence ? 0 : pageindex, strip,
									(uint8_t*)val + valindex, strip_size_used);
							else
								GetTiffStrip(sequence ? 0 : pageindex, strip,
									(uint16_t*)val + valindex, strip_size_used);
						}
					}
				}
			}
			if (sequence) CloseTiff();
			//if (!imagej_raw_)
			//	InvalidatePageInfo();
		}
	}

	if (buf)
		free(buf);
	if (!sequence || isHyperstack_) CloseTiff();

	//write to nrrd
	if (eight_bit)
		nrrdWrap(nrrdout, (uint8_t*)val, nrrdTypeUChar,
			3, (size_t)m_x_size, (size_t)m_y_size, (size_t)numPages);
	else
		nrrdWrap(nrrdout, (uint16_t*)val, nrrdTypeUShort,
			3, (size_t)m_x_size, (size_t)m_y_size, (size_t)numPages);
	nrrdAxisInfoSet(nrrdout, nrrdAxisInfoSpacing, m_xspc, m_yspc, m_zspc);
	nrrdAxisInfoSet(nrrdout, nrrdAxisInfoMax, m_xspc*m_x_size,
		m_yspc*m_y_size, m_zspc*numPages);
	nrrdAxisInfoSet(nrrdout, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
	nrrdAxisInfoSet(nrrdout, nrrdAxisInfoSize, (size_t)m_x_size,
		(size_t)m_y_size, (size_t)numPages);

  rawImage = static_cast<unsigned char*>(val);

  loadTiffInfo(static_cast<size_t>(m_x_size),
               static_cast<size_t>(m_y_size),
               static_cast<size_t>(numPages),
               bits);


	if (!eight_bit) {
		if (get_max) {
			if (samples > 1)
				m_max_value = max_value;
			else {
				double value;
				unsigned long long totali = (unsigned long long)m_slice_num*
					(unsigned long long)m_x_size*(unsigned long long)m_y_size;
				for (unsigned long long i = 0; i < totali; ++i)
				{
					value = ((unsigned short*)nrrdout->data)[i];
					m_max_value = value > m_max_value ? value : m_max_value;
				}
			}
		}
		if (m_max_value > 0.0) m_scalar_scale = 65535.0 / m_max_value;
		else m_scalar_scale = 1.0;
	}
	else m_max_value = 255.0;

	return nrrdout;
}
#endif
