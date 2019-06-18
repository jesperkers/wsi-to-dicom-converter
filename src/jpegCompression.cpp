// Copyright 2019 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "jpegCompression.h"
#include <boost/bind/bind.hpp>
#include <boost/gil/extension/io/jpeg/old.hpp>
using namespace boost::gil;
JpegCompression::JpegCompression(int quality) {
  _quality = quality;
  _cinfo.err = jpeg_std_error(&_jerr);
  jpeg_create_compress(&_cinfo);
}

JpegCompression::~JpegCompression() { jpeg_destroy_compress(&_cinfo); }

void JpegCompression::compress(uint8_t *&output, const rgb8_view_t &view,
                               size_t &size) {
  typedef
      typename channel_type<typename rgb8_view_t::value_type>::type channel_t;
  _cinfo.image_width = (JDIMENSION)view.width();
  _cinfo.image_height = (JDIMENSION)view.height();
  _cinfo.input_components = num_channels<rgb8_view_t>::value;
  _cinfo.in_color_space = detail::jpeg_write_support<
      channel_t, typename color_space_type<rgb8_view_t>::type>::_color_space;

  unsigned char *imgd = 0;
  unsigned long outlen = 0;
  jpeg_mem_dest(&_cinfo, &imgd, &outlen);
  jpeg_set_defaults(&_cinfo);
  jpeg_set_quality(&_cinfo, _quality, TRUE);
  jpeg_start_compress(&_cinfo, TRUE);
  std::vector<
      pixel<uint8_t, layout<typename color_space_type<rgb8_view_t>::type> > >
      row(view.width());
  JSAMPLE *row_address = (JSAMPLE *)&row.front();
  for (int y = 0; y < view.height(); ++y) {
    std::copy(view.row_begin(y), view.row_end(y), row.begin());
    jpeg_write_scanlines(&_cinfo, (JSAMPARRAY)&row_address, 1) != 1;
  }
  jpeg_finish_compress(&_cinfo);
  jpeg_destroy_compress(&_cinfo);
  output = imgd;
  size = outlen;
}