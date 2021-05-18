/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2018 Scientific Computing and Imaging Institute,
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
#ifndef BASE_READER_H
#define BASE_READER_H

#ifdef HAS_TEEM

#include <string>
#include <teem/nrrd.h>
#include <vector>
#include <sstream>

//using namespace std;

//#ifdef STATIC_COMPILE
  #define nrrdWrap nrrdWrap_va
  #define nrrdAxisInfoSet nrrdAxisInfoSet_va
//#endif

//error codes
//return to notify caller if fail
#define READER_OK	0
#define READER_OPEN_FAIL	1
#define READER_FORMAT_ERROR	2
#define READER_EMPTY_DATA	3
#define READER_FP32_DATA	4
#define READER_JAVA_ARRAY_SIZE_ERROR 5

class BaseReader
{
  public:
	//BaseReader();
	virtual ~BaseReader() {};

	//get the reader type
	//see the header of each reader implementation for the actual value
	virtual int GetType() = 0;	//get reader type

	//set the file name and path to open and read
	virtual void SetFile(std::string &file) = 0;
	//set the file name and path to open and read (in wide string for foreign languages)
	virtual void SetFile(std::wstring &file) = 0;
	//set reader flag to read a 3D stack as a file sequence, each file being a slice
	//in UI, it is set in the open file dialog as "read a sequence as z slices..."
	//not all reader types use this flag, basically for a tiff sequence
	virtual void SetSliceSeq(bool ss) = 0;
	//get the reader flag to read z stack sequence
	virtual bool GetSliceSeq() = 0;
	//set time identifier
	//time identifier is a string to identify each file in a sequence as a time point
	//in UI, it is set in the open file dialog as option "time sequence identifier"
	//default value is "_T", which means any digits after the string in a file name is used as its time value
	virtual void SetTimeId(std::wstring &id) = 0;
	//get current time identifier
	virtual std::wstring GetTimeId() = 0;
	//preprocess the file
	//get the structure of the data without reading actual volume
	virtual int Preprocess() = 0;
	//a sequence of similar files can be put in one folder
	//once batch mode is turn on, those files can be "played back" as if they were in a time sequence
	//a time sequence assumes all files are of the same size, therefore no memory releasing (simple replacing) when time changes
	//a batch sequence can have files with different sizes. memory is released every time when time changes
	//batch mode is set in the movie export panel when time sequence is checked for a non-time-sequence
	virtual void SetBatch(bool batch) = 0;
	//get batch mode
	virtual bool GetBatch() = 0;
	//batch sequence is determined when SetBatch(true) is called
	//a stl vector containing file names is created
	//LoadBatch() loads a new file in the vector to replace the current one
	//usually called when the time value is changed from the movie export panel
	virtual int LoadBatch(int index) = 0;
	//similar to LoadBatch(). instead of using an absolute index value
	//it uses an offset value, telling the reader to load a file n indices after/before the current one
	virtual int LoadOffset(int offset);
	//get the current offest index value
	virtual int GetOffset();
	//read the acutual volume data from file and convert it to a nrrd
	//if get_max is true, it will find the maximum value within the volume
	//the max value is used to change the silder range in UI
	virtual Nrrd* Convert(bool get_max);
	//c is the channel index to load
	virtual Nrrd* Convert(int c, bool get_max);
	//t is the time point value to load
	virtual Nrrd* Convert(int t, int c, bool get_max) = 0;
	//for a time sequence, get the file name for specified time and channel
	virtual std::wstring GetCurDataName(int t, int c) = 0;
	//for a 4d sequence, get the file name for the mask of specified time and channel
	virtual std::wstring GetCurMaskName(int t, int c) = 0;
	//for a 4d sequence, get the file name for the label of specified time and channel
	virtual std::wstring GetCurLabelName(int t, int c) = 0;

	//get a string for only the path of the file
	virtual std::wstring GetPathName() = 0;
	//get a string for only the file name without its path
	virtual std::wstring GetDataName() = 0;
	//get the total number of time points
	virtual int GetTimeNum() = 0;
	//get the time point value of last/current loaded file
	virtual int GetCurTime() = 0;
	//get the total number of channels
	virtual int GetChanNum() = 0;
	//get the excitation wave length for a channel, read from meta data
	//the wave length value can be used to set the color of a channel
	//the settings are set in the setting dialog
	//not all file formats can provide this information
	virtual double GetExcitationWavelength(int chan) = 0;
	//get the total number of z slices in a stack
	virtual int GetSliceNum() = 0;
	//get the number of voxels/pixels of the x dimension
	virtual int GetXSize() = 0;
	//get the number of voxels/pixels in the y dimension
	virtual int GetYSize() = 0;
	//sometimes the spacing info in the file is incorrect (too small or big)
	//if the flag is set to false, default spacing values will be applied after reading
	virtual bool IsSpcInfoValid() = 0;
	//get the x spacing value, the physical size of the voxel
	virtual double GetXSpc() = 0;
	//get the y spacing value
	virtual double GetYSpc() = 0;
	//get the z spacing value
	virtual double GetZSpc() = 0;
	//get the max intensity value
	//for 8-bit data, the max value is usually 255
	//for 16-bit data, the microscope sensor may not generate full range, a smaller number is usually used
	virtual double GetMaxValue() = 0;
	//intensity values (0-255 for 8-bit, 0-max value for 16-bit) are normalized to (0.0-1.0) for OpenGL
	//the scalar-value scaling factor is usually the inverse of the max value (1/max) for 16-bit data
	virtual double GetScalarScale() = 0;
	//get the total number of files in a batch when batch is turned on with SetBatch(true)
	virtual int GetBatchNum() = 0;
	//get the index of the current file when the batch mode is turned on
	virtual int GetCurBatch() = 0;

	//check if two readers are the same
	bool operator==(BaseReader& reader)
	{
      return m_id_string == reader.m_id_string;
	}
	//another way to check if two readers are the same
	bool Match(std::wstring &id_string)
	{
      return m_id_string == id_string;
	}

	//a volume can be resized using a sampler
	//this is usually used when the mask of a volume has a different size
	//it can also change the size of a volume when it is resaved
	//this sets the type of resizing
	void SetResize(int type)
	{
      m_resize_type = type;
	}
	//get the type of resizing
	int GetResize()
	{
      return m_resize_type;
	}
	//specify the sampler to use for resizing
	void SetResample(int type)
	{
      m_resample_type = type;
	}
	//get the current smapler's type
	int GetResample()
	{
      return m_resample_type;
	}
	void SetAlignment(int alignment)
	{
      m_alignment = alignment;
	}
	int GetAlignment()
	{
      return m_alignment;
	}

	static std::string GetError(int code);

  protected:
	std::wstring m_id_string;	//the path and file name used to read files
	//resizing
	int m_resize_type;		//0: no resizing; 1: padding; 2: resampling
	int m_resample_type;	//0: nearest neighbour; 1: linear
	int m_alignment;		//padding alignment

	//3d batch
	bool m_batch;
	std::vector<std::wstring> m_batch_list;
	int m_cur_batch;
	
	std::wstring m_path_name;

	std::wstring m_info;

	//all the lzw decoding stuff
	#define MAXCODE(n)	((1L<<(n))-1)
	#define	BITS_MIN	9		/* start with 9 bits */
	#define	BITS_MAX	12		/* max of 12 bit strings */
	#define	CODE_CLEAR	256		/* code to clear string table */
	#define	CODE_EOI	257		/* end-of-information code */
	#define CODE_FIRST	258		/* first free code entry */
	#define	CSIZE		(MAXCODE(BITS_MAX)+1024L)
	typedef	unsigned short uint16;	/* sizeof (uint16) must == 2 */
	typedef	int int32;
	typedef	int32 tsize_t;		/* i/o size in bytes */
	typedef	unsigned char tidataval_t;	/* internal image data value type */
	typedef	tidataval_t* tidata_t;		/* reference to internal image data */
	typedef	uint16 tsample_t;			/* sample number */
	typedef uint16 hcode_t;			/* codes fit in 16 bits */
	typedef struct code_ent
	{
      struct code_ent *next;
      unsigned short	length;		/* string len, including this token */
      unsigned char	value;		/* data value */
      unsigned char	firstchar;	/* first token of string */
	} code_t;

	typedef struct
	{
      long	hash;
      hcode_t	code;
	} hash_t;

	typedef struct
	{
      int		predictor;	/* predictor tag value */
      int		stride;		/* sample stride over data */
      tsize_t		rowsize;	/* tile/strip row size */

      unsigned short	nbits;		/* # of bits/code */
      unsigned short	maxcode;	/* maximum code for lzw_nbits */
      unsigned short	free_ent;	/* next free entry in hash table */
      long		nextdata;	/* next bits of i/o */
      long		nextbits;	/* # of valid bits in lzw_nextdata */

      /* Decoding specific data */
      long	dec_nbitsmask;		/* lzw_nbits 1 bits, right adjusted */
      long	dec_restart;		/* restart count */
      long	dec_bitsleft;		/* available bits in raw data */
      code_t*	dec_codep;		/* current recognized code */
      code_t*	dec_oldcodep;		/* previously recognized code */
      code_t*	dec_free_entp;		/* next free entry */
      code_t*	dec_maxcodep;		/* max available entry */
      code_t*	dec_codetab;		/* kept separate for small machines */
	} LZWCodecState;
	#define	NextCode(_tif, _sp, _bp, _code, _get) {				\
      if ((_sp)->dec_bitsleft < nbits) {				\
        _code = CODE_EOI;					\
      } else {							\
        _get(_sp,_bp,_code);					\
        (_sp)->dec_bitsleft -= nbits;				\
      }								\
	}
	#define	GetNextCode(sp, bp, code) {				\
      nextdata = (nextdata<<8) | *(bp)++;			\
      nextbits += 8;						\
      if (nextbits < nbits) {					\
        nextdata = (nextdata<<8) | *(bp)++;		\
        nextbits += 8;					\
      }							\
      code = (hcode_t)((nextdata >> (nextbits-nbits)) & nbitsmask);	\
      nextbits -= nbits;					\
	}
	#define REPEAT4(n, op)		\
      switch (n) {		\
      default: { int i; for (i = n-4; i > 0; i--) { op; } } \
      case 4:  op;		\
      case 3:  op;		\
      case 2:  op;		\
      case 1:  op;		\
      case 0:  ;			\
	}

	int LZWDecode(tidata_t tif, tidata_t op0, tsize_t occ0);
	void DecodeAcc8(tidata_t cp0, tsize_t cc, tsize_t stride);
	void DecodeAcc16(tidata_t cp0, tsize_t cc, tsize_t stride);

	//read number after a position in a string
	int get_number(std::string &str, int64_t pos);
};
#endif
#endif
