#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <string>

#include "correlate_access_code_bb_ts_fl.h"
#include <gnuradio/analog/pwr_squelch_cc.h>
#include <gnuradio/analog/quadrature_demod_cf.h>
#include <gnuradio/blocks/multiply_const.h>
#include <gnuradio/blocks/socket_pdu.h>
#include <gnuradio/blocks/tagged_stream_to_pdu.h>
#include <gnuradio/constants.h>
#include <gnuradio/digital/binary_slicer_fb.h>
#include <gnuradio/digital/clock_recovery_mm_ff.h>
#include <gnuradio/digital/symbol_sync_ff.h>
#include <gnuradio/filter/fir_filter_blk.h>
#include <gnuradio/filter/firdes.h>
#include <gnuradio/filter/freq_xlating_fir_filter.h>
#include <gnuradio/filter/hilbert_fc.h>
#include <gnuradio/filter/pfb_arb_resampler_ccf.h>
#include <gnuradio/logger.h>
#include <gnuradio/prefs.h>
#include <gnuradio/sys_paths.h>
#include <gnuradio/top_block.h>
#include <osmosdr/source.h>

void usage(char *argv0) {
	std::cout << "Usage: " << argv0 << " {frequency} {offset} [device]" << std::endl;
}

int main(int argc, char **argv) {
  gr::top_block_sptr tb;

  // Declare our GNU Radio blocks
  gr::filter::freq_xlating_fir_filter_ccc::sptr xlat;
  gr::analog::quadrature_demod_cf::sptr demod1, demod2;
  gr::filter::fir_filter_fff::sptr fir1, fir2, fir3;
  gr::filter::hilbert_fc::sptr hilbert;
  gr::digital::clock_recovery_mm_ff::sptr clockRecovery;
  gr::blocks::multiply_const_ff::sptr multiplyConst;
  gr::digital::binary_slicer_fb::sptr slicer;
  gr::reveng::correlate_access_code_bb_ts_fl::sptr correlate;
  gr::blocks::tagged_stream_to_pdu::sptr taggedStreamToPdu;
  gr::blocks::socket_pdu::sptr udp_client;

  float freq = 170795000;
  float xlat_center_freq = 19500;
  float samp_rate = 2000000;
  float bandwidth_sdr = 1000000;
  float bandwidth_xlat = 10000;
  int decimation = static_cast<int>(bandwidth_sdr / bandwidth_xlat);
  float transition_bw = 1000;
  float baud = 2400;
  float sps = samp_rate / decimation / baud;

	std::string device_string = "";

	// Argument parsing
	if (argc != 3 && argc != 4) {
		usage(*argv);
		return EXIT_FAILURE;
	}

	freq = strtof(argv[1], NULL);
	xlat_center_freq = strtof(argv[2], NULL);

	if (argc == 4) {
		device_string = std::string(argv[3]);	
	}

  std::vector<gr_complex> xlat_taps = gr::filter::firdes::complex_band_pass(
      1, samp_rate, -samp_rate / (2 * decimation), samp_rate / (2 * decimation),
      transition_bw);
  gr_vector_float fir1_taps =
      gr::filter::firdes::high_pass(1.0, samp_rate / decimation, 50, 25);
  gr_vector_float fir2_taps =
      gr::filter::firdes::high_pass(1.0, samp_rate / decimation, 1.0, 1.0);
  gr_vector_float fir3_taps = gr::filter::firdes::root_raised_cosine(
      1.0, samp_rate / decimation, sps, 0.35, 4);

  tb = gr::make_top_block("fg");

  osmosdr::source::sptr src;
  src = osmosdr::source::make(device_string);
  src->set_block_alias("src");

  src->set_sample_rate(samp_rate);
  src->set_center_freq(freq);
  src->set_gain_mode(false, 0);
  src->set_gain(14, "RF", 0);
  src->set_gain(32, "IF", 0);
  src->set_gain(50, "BB", 0);
  src->set_bandwidth(bandwidth_sdr, 0);

  xlat = gr::filter::freq_xlating_fir_filter_ccc::make(
      decimation, xlat_taps, xlat_center_freq, samp_rate);
  demod1 = gr::analog::quadrature_demod_cf::make(1.0);
  fir1 = gr::filter::fir_filter_fff::make(1, fir1_taps);
  hilbert = gr::filter::hilbert_fc::make(65);
  demod2 = gr::analog::quadrature_demod_cf::make(4.0);
  fir2 = gr::filter::fir_filter_fff::make(1, fir2_taps);
  fir3 = gr::filter::fir_filter_fff::make(1, fir3_taps);
  clockRecovery = gr::digital::clock_recovery_mm_ff::make(
      sps, 0.25f * 0.175f * 0.175f, 0.5, 0.175, 0.01);
  multiplyConst = gr::blocks::multiply_const_ff::make(-1.0f);
  slicer = gr::digital::binary_slicer_fb::make();
  correlate = gr::reveng::correlate_access_code_bb_ts_fl::make(
      "1111110000000001", 1, "packet_len", 24);
  taggedStreamToPdu = gr::blocks::tagged_stream_to_pdu::make(
      gr::blocks::pdu::vector_type::byte_t, "packet_len");
  udp_client = gr::blocks::socket_pdu::make("UDP_CLIENT", "localhost", "40000");

  std::string ver = gr::version();
  std::string cCompiler = gr::c_compiler();
  std::string cxxCompiler = gr::cxx_compiler();
  std::string compilerFlags = gr::compiler_flags();
  std::string prefs = gr::prefs::singleton()->to_string();

  std::cout << "GNU Radio Version: " << ver << "\n\n C Compiler: " << cCompiler
            << "\n\n CXX Compiler: " << cxxCompiler << "\n\n Prefs: " << prefs
            << "\n\n Compiler Flags: " << compilerFlags;

  try {
    tb->connect(src, 0, xlat, 0);
    tb->connect(xlat, 0, demod1, 0);
    tb->connect(demod1, 0, fir1, 0);
    tb->connect(fir1, 0, hilbert, 0);
    tb->connect(hilbert, 0, demod2, 0);
    tb->connect(demod2, 0, fir2, 0);
    tb->connect(fir2, 0, fir3, 0);
    tb->connect(fir3, 0, clockRecovery, 0);
    tb->connect(clockRecovery, 0, multiplyConst, 0);
    tb->connect(multiplyConst, 0, slicer, 0);
    tb->connect(slicer, 0, correlate, 0);
    tb->connect(correlate, 0, taggedStreamToPdu, 0);
    tb->msg_connect(taggedStreamToPdu, "pdus", udp_client, "pdus");
  } catch (const std::invalid_argument &e) {
    std::cerr << e.what();
    return EXIT_FAILURE;
  }

  tb->start();

  return EXIT_SUCCESS;
}
