/* -*- c++ -*- */
/*
 * Copyright 2014 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * GNU Radio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * GNU Radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include "correlate_access_code_bb_ts_fl_impl.h"
#include <boost/format.hpp>
#include <cstdio>
#include <gnuradio/io_signature.h>
#include <iostream>
#include <stdexcept>
#include <volk/volk.h>

namespace gr {
namespace reveng {

correlate_access_code_bb_ts_fl::sptr
correlate_access_code_bb_ts_fl::make(const std::string &access_code,
                                     int threshold, const std::string &tag_name,
                                     int len) {
  return gnuradio::get_initial_sptr(new correlate_access_code_bb_ts_fl_impl(
      access_code, threshold, tag_name, len));
}

correlate_access_code_bb_ts_fl_impl::correlate_access_code_bb_ts_fl_impl(
    const std::string &access_code, int threshold, const std::string &tag_name,
    int len)
    : block("correlate_access_code_bb_ts_fl",
            io_signature::make(1, 1, sizeof(char)),
            io_signature::make(1, 1, sizeof(char))),
      d_data_reg(0), d_mask(0), d_threshold(threshold), d_len(0) {
  set_tag_propagation_policy(TPP_DONT);
	//set_ninput_items_required(len * 8);
	set_min_noutput_items(len * 8);
	set_output_multiple(len * 8);

  if (!set_access_code(access_code)) {
    GR_LOG_ERROR(d_logger, "access_code is > 64 bits");
    throw std::out_of_range("access_code is > 64 bits");
  }

  std::stringstream str;
  str << name() << unique_id();
  d_me = pmt::string_to_symbol(str.str());
  d_key = pmt::string_to_symbol(tag_name);

	d_pkt_len = 8 * len;
}

correlate_access_code_bb_ts_fl_impl::~correlate_access_code_bb_ts_fl_impl() {}

bool correlate_access_code_bb_ts_fl_impl::set_access_code(
    const std::string &access_code) {
  d_len = access_code.length(); // # of bytes in string
  if (d_len > 64)
    return false;

  // set len least significant bits to 1.
  d_mask = ((~0ULL) >> (64 - d_len));

  d_access_code = 0;
  for (unsigned i = 0; i < d_len; i++) {
    d_access_code = (d_access_code << 1) | (access_code[i] & 1);
  }

  GR_LOG_DEBUG(d_logger, boost::format("Access code: %llx") % d_access_code);
  GR_LOG_DEBUG(d_logger, boost::format("Mask: %llx") % d_mask);

  return true;
}

unsigned long long correlate_access_code_bb_ts_fl_impl::access_code() const {
  return d_access_code;
}

int correlate_access_code_bb_ts_fl_impl::general_work(
    int noutput_items, gr_vector_int &ninput_items,
    gr_vector_const_void_star &input_items, gr_vector_void_star &output_items) {

	// Streaming correlate access code.
  const unsigned char *in = (const unsigned char *)input_items[0];
  unsigned char *out = (unsigned char *)output_items[0];

  uint64_t abs_out_sample_cnt = nitems_written(0);

	int nprod = 0;

		// shift in new data
		d_data_reg = (d_data_reg << 1) | (in[0] & 0x1);

		// compute hamming distance between desired access code and current data
		uint64_t wrong_bits = 0;
		uint64_t nwrong = d_threshold + 1;

		wrong_bits = (d_data_reg ^ d_access_code) & d_mask;
		volk_64u_popcnt(&nwrong, wrong_bits);

		if (nwrong <= d_threshold) {
				if (d_pkt_len > noutput_items) {
				GR_LOG_FATAL(d_logger,
						boost::format("cannot write tagged stream not enough output_items available"));
						return 0;
				}

				GR_LOG_DEBUG(d_logger,
										 boost::format("writing tag at sample %llu") %
												 (abs_out_sample_cnt));
        // MAKE A TAG OUT OF THIS AND UPDATE OFFSET
        add_item_tag(0,                          // stream ID
										 abs_out_sample_cnt, // sample
                     d_key,                      // length key
                     pmt::from_long(d_pkt_len),  // length data
                     d_me);                      // block src id

				for (int j = 0; j < d_pkt_len; j++) {
					out[nprod++] = in[j];
				}
		}

	consume_each(1);
  return nprod;
}

} /* namespace reveng */
} /* namespace gr */
