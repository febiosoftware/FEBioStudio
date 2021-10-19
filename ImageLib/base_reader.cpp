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
/*
 * Copyright (c) 1988-1997 Sam Leffler
 * Copyright (c) 1991-1997 Silicon Graphics, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that (i) the above copyright notices and this permission notice appear in
 * all copies of the software and related documentation, and (ii) the names of
 * Sam Leffler and Silicon Graphics may not be used in any advertising or
 * publicity relating to the software without the specific, prior written
 * permission of Sam Leffler and Silicon Graphics.
 *
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
 *
 * IN NO EVENT SHALL SAM LEFFLER OR SILICON GRAPHICS BE LIABLE FOR
 * ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF
 * LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

#ifdef HAS_TEEM
#include "base_reader.h"

int BaseReader::LZWDecode(tidata_t tif, tidata_t op0, tsize_t occ0)
{
  //initialize codec state
  LZWCodecState *sp = new LZWCodecState;
  //sp->predictor = m_predictor;
  sp->stride = 1;
  //sp->rowsize = m_x_size;
  sp->dec_codetab = new code_t[CSIZE*sizeof(code_t)];
  int icode = 255;
  do
  {
    sp->dec_codetab[icode].value = icode;
    sp->dec_codetab[icode].firstchar = icode;
    sp->dec_codetab[icode].length = 1;
    sp->dec_codetab[icode].next = NULL;
  } while (icode--);

  sp->maxcode = MAXCODE(BITS_MIN)-1;
  sp->nbits = BITS_MIN;
  sp->nextbits = 0;
  sp->nextdata = 0;
  sp->dec_restart = 0;
  sp->dec_nbitsmask = MAXCODE(BITS_MIN);
  sp->dec_bitsleft = occ0 << 3;
  sp->dec_free_entp = sp->dec_codetab + CODE_FIRST;
  memset(sp->dec_free_entp, 0, (CSIZE-CODE_FIRST)*sizeof(code_t));
  sp->dec_oldcodep = &sp->dec_codetab[-1];
  sp->dec_maxcodep = &sp->dec_codetab[sp->dec_nbitsmask-1];

  char *op = (char*) op0;
  long occ = (long) occ0;
  char *tp;
  unsigned char *bp;
  hcode_t code;
  int len;
  long nbits, nextbits, nextdata, nbitsmask;
  code_t *codep, *free_entp, *maxcodep, *oldcodep;

  bp = (unsigned char *)tif;
  nbits = sp->nbits;
  nextdata = sp->nextdata;
  nextbits = sp->nextbits;
  nbitsmask = sp->dec_nbitsmask;
  oldcodep = sp->dec_oldcodep;
  free_entp = sp->dec_free_entp;
  maxcodep = sp->dec_maxcodep;

  while (occ > 0)
  {
    GetNextCode(sp, bp, code);
    //NextCode(tif, sp, bp, code, GetNextCode);
    if (code == CODE_EOI)
      break;
    if (code == CODE_CLEAR)
    {
      free_entp = sp->dec_codetab + CODE_FIRST;
      nbits = BITS_MIN;
      nbitsmask = MAXCODE(BITS_MIN);
      maxcodep = sp->dec_codetab + nbitsmask-1;
      GetNextCode(sp, bp, code);
      //NextCode(tif, sp, bp, code, GetNextCode);
      if (code == CODE_EOI)
        break;
      *op++ = (char)code, occ--;
      oldcodep = sp->dec_codetab + code;
      continue;
    }
    codep = sp->dec_codetab + code;

    /*
    * Add the new entry to the code table.
    */
    if (free_entp < &sp->dec_codetab[0] ||
    free_entp >= &sp->dec_codetab[CSIZE])
      return 0;

    free_entp->next = oldcodep;
    if (free_entp->next < &sp->dec_codetab[0] ||
    free_entp->next >= &sp->dec_codetab[CSIZE])
      return 0;

    free_entp->firstchar = free_entp->next->firstchar;
    free_entp->length = free_entp->next->length+1;
    free_entp->value = (codep < free_entp) ?
    codep->firstchar : free_entp->firstchar;
    if (++free_entp > maxcodep)
    {
      if (++nbits > BITS_MAX)		/* should not happen */
      nbits = BITS_MAX;
      nbitsmask = MAXCODE(nbits);
      maxcodep = sp->dec_codetab + nbitsmask-1;
    }
    oldcodep = codep;
    if (code >= 256)
    {
      /*
      * Code maps to a string, copy string
      * value to output (written in reverse).
      */
      if(codep->length == 0)
        return 0;
      if (codep->length > occ)
      {
        /*
         * String is too long for decode buffer,
         * locate portion that will fit, copy to
         * the decode buffer, and setup restart
         * logic for the next decoding call.
         */
        sp->dec_codep = codep;
        do
        {
                codep = codep->next;
        } while (codep && codep->length > occ);
        if (codep)
        {
          sp->dec_restart = occ;
          tp = op + occ;
          do
          {
            *--tp = codep->value;
            codep = codep->next;
          } while (--occ && codep);
        }
        break;
      }
    len = codep->length;
    tp = op + len;
    do
    {
    int t;
    --tp;
    t = codep->value;
    codep = codep->next;
    *tp = t;
    } while (codep && tp > op);
      if (codep)
        break;
      op += len, occ -= len;
    } else
    *op++ = (char)code, occ--;
  }

  delete []sp->dec_codetab;
  delete sp;

  if (occ > 0)
    return 0;

  return (1);
}

void BaseReader::DecodeAcc8(tidata_t cp0, tsize_t cc, tsize_t stride)
{
  char* cp = (char*) cp0;
  if((cc%stride)!=0) return;
  if (cc > stride)
  {
    /*
     * Pipeline the most common cases.
     */
    if (stride == 3)
    {
      unsigned int cr = cp[0];
      unsigned int cg = cp[1];
      unsigned int cb = cp[2];
      cc -= 3;
      cp += 3;
      while (cc>0)
      {
        cp[0] = (char) (cr += cp[0]);
        cp[1] = (char) (cg += cp[1]);
        cp[2] = (char) (cb += cp[2]);
        cc -= 3;
        cp += 3;
      }
    }
    else if (stride == 4)
    {
      unsigned int cr = cp[0];
      unsigned int cg = cp[1];
      unsigned int cb = cp[2];
      unsigned int ca = cp[3];
      cc -= 4;
      cp += 4;
      while (cc>0)
      {
        cp[0] = (char) (cr += cp[0]);
        cp[1] = (char) (cg += cp[1]);
        cp[2] = (char) (cb += cp[2]);
        cp[3] = (char) (ca += cp[3]);
        cc -= 4;
        cp += 4;
      }
    }
    else
    {
      cc -= stride;
      do {
        REPEAT4(stride, cp[stride] = (char)(cp[stride] + *cp); cp++)
        cc -= stride;
      } while (cc>0);
    }
  }
}

void BaseReader::DecodeAcc16(tidata_t cp0, tsize_t cc, tsize_t stride)
{
  uint16* wp = (uint16*) cp0;
  tsize_t wc = cc / 2;

  if((cc%(2*stride))!=0) return;

  if (wc > stride)
  {
    wc -= stride;
    do {
      REPEAT4(stride, wp[stride] += wp[0]; wp++) wc -= stride;
    } while (wc > 0);
  }
}

Nrrd* BaseReader::Convert(bool get_max) { return Convert(0,get_max); }

Nrrd* BaseReader::Convert(int c, bool get_max) { return Convert(0,c,get_max); }

int BaseReader::LoadOffset(int offset)
{
  if (m_batch_list.size() <=1) return -1;
  int result = offset % m_batch_list.size();
  m_path_name = m_batch_list[result];
  Preprocess();
  m_cur_batch = result;
  return result;
}

int BaseReader::GetOffset() { return m_cur_batch; }

int BaseReader::get_number(std::string &str, int64_t pos)
{
  std::string num_str;
  for (int64_t i = pos; i < str.length(); ++i)
  {
    if (isdigit(str[i]))
      num_str.push_back(str[i]);
    else
      break;
  }
  if (num_str != "")
    return atoi(num_str.c_str());
  else
    return 0;
}

std::string BaseReader::GetError(int code)
{
  std::string err_str;
  switch (code)
  {
    case READER_OK:
      err_str = "No Error.";
      break;
    case READER_OPEN_FAIL:
      err_str = "Cannot open file.";
      break;
    case READER_FORMAT_ERROR:
      err_str = "Cannot understand file format.";
      break;
    case READER_EMPTY_DATA:
      err_str = "File is empty.";
      break;
    case READER_FP32_DATA:
      err_str = "Floating point data are unsupported.";
      break;
    case READER_JAVA_ARRAY_SIZE_ERROR:
      err_str = "File contains too many voxels for Java.\n" \
      "Max is 2^31-1 (2,147,483,647) voxels.";
      break;
  }
  return err_str;
}
#endif
