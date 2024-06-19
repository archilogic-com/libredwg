/*****************************************************************************/
/*  LibreDWG - free implementation of the DWG file format                    */
/*                                                                           */
/*  Copyright (C) 2018-2023 Free Software Foundation, Inc.                   */
/*                                                                           */
/*  This library is free software, licensed under the terms of the GNU       */
/*  General Public License as published by the Free Software Foundation,     */
/*  either version 3 of the License, or (at your option) any later version.  */
/*  You should have received a copy of the GNU General Public License        */
/*  along with this program.  If not, see <http://www.gnu.org/licenses/>.    */
/*****************************************************************************/

/* Simple WASM API to convert DWG to DXF.
 * 
 * written by Frank van Leeuwen
 * heavily modified by Milan Redele
 */
#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#else
#define EMSCRIPTEN_KEEPALIVE
#endif

#include "../src/config.h"
#include <dwg.h>
#include "decode.h"
#include "out_dxf.h"
#include <logging.h>


Dwg_Data dwg;
int readError = DWG_ERR_IOERROR;

EMSCRIPTEN_KEEPALIVE
int test_add(int a, int b) {
  return a + b;
}

EMSCRIPTEN_KEEPALIVE
int test_array_sum(char* arr, size_t size) {
  int sum = 0;
  for (size_t i=0; i<size; i++){
    sum += arr[i];
  }
  return sum;
}

EMSCRIPTEN_KEEPALIVE
int read(unsigned char* buf, size_t size) {
  dwg_free(&dwg);
  readError = DWG_ERR_IOERROR;

  Bit_Chain dat = EMPTY_CHAIN (0);
  dat.chain = buf;
  dat.size = size;
  dat.chain[dat.size] = '\0'; // buf must be 1 byte longer than the dwg size
  memset (&dwg, 0, sizeof (Dwg_Data));
  
  int error = dwg_decode (&dat, &dwg);

  readError = error;
  return error;
}

EMSCRIPTEN_KEEPALIVE
int write(unsigned char* buf, size_t size, size_t* result_size) {
  if (readError >= DWG_ERR_CRITICAL){
    LOG_ERROR("There was an error during dwg parsing, can't proceed.")
    return DWG_ERR_IOERROR;
  }

  Bit_Chain dat = EMPTY_CHAIN (0);
  dat.version = dwg.header.version;
  dat.from_version = dwg.header.from_version;
  dat.fh = fmemopen(buf, size, "w");
  
  int error = 0;
  
  error = dwg_write_dxf(&dat, &dwg);

  *result_size = (size_t) ftell(dat.fh);
  
  fclose(dat.fh);

  return error;
}