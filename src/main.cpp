#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <string>

#include "correlate_access_code_bb_ts_fl.h"
#include "rational_resampler.h"
#include <gnuradio/analog/frequency_modulator_fc.h>
#include <gnuradio/analog/pwr_squelch_cc.h>
#include <gnuradio/analog/quadrature_demod_cf.h>
#include <gnuradio/blocks/add_const_ff.h>
#include <gnuradio/blocks/multiply_const.h>
#include <gnuradio/blocks/socket_pdu.h>
#include <gnuradio/blocks/tagged_stream_to_pdu.h>
#include <gnuradio/blocks/wavfile_source.h>
#include <gnuradio/constants.h>
#include <gnuradio/digital/binary_slicer_fb.h>
#include <gnuradio/digital/clock_recovery_mm_ff.h>
#include <gnuradio/digital/symbol_sync_ff.h>
#include <gnuradio/filter/fir_filter_blk.h>
#include <gnuradio/filter/firdes.h>
#include <gnuradio/filter/freq_xlating_fir_filter.h>
#include <gnuradio/filter/hilbert_fc.h>
#include <gnuradio/filter/interp_fir_filter.h>
#include <gnuradio/logger.h>
#include <gnuradio/prefs.h>
#include <gnuradio/sys_paths.h>
#include <gnuradio/top_block.h>
#include <osmosdr/source.h>

#include <libenvpp/env.hpp>

int main(int argc, char **argv) {
  auto pre = env::prefix("DECODER");

  gr::top_block_sptr tb;

  // Declare our GNU Radio blocks
  gr::filter::freq_xlating_fir_filter_ccc::sptr xlat;
  gr::filter::rational_resampler_ccc::sptr resampler;
  gr::analog::quadrature_demod_cf::sptr demod1, demod2;
  gr::filter::fir_filter_fff::sptr fir1, fir2;
  gr::filter::hilbert_fc::sptr hilbert;
  gr::blocks::add_const_ff::sptr addConst;
  gr::digital::clock_recovery_mm_ff::sptr clockRecovery;
  gr::blocks::multiply_const_ff::sptr multiplyConst;
  gr::digital::binary_slicer_fb::sptr slicer;
  gr::reveng::correlate_access_code_bb_ts_fl::sptr correlate;
  gr::blocks::tagged_stream_to_pdu::sptr taggedStreamToPdu;
  gr::blocks::socket_pdu::sptr udp_client;

  float xlat_center_freq = 0;
  float samp_rate = 2000000;
  float bandwidth_sdr = 1000000;
  float bandwidth_xlat = 5000;
  float transition_bw = 1000;
  int decimation = static_cast<int>(bandwidth_sdr / bandwidth_xlat);
  float baud = 2400;
  unsigned rs_interpolation = 24;
  unsigned rs_decimation = 25;
  float sps = samp_rate / decimation / baud * (float)rs_interpolation /
              (float)rs_decimation;
  std::string wav_file;

  const auto wav_file_id = pre.register_variable<std::string>("WAV_FILE");

  const auto parsed_and_validated_pre = pre.parse_and_validate();

  if (parsed_and_validated_pre.ok()) {
    wav_file = parsed_and_validated_pre.get_or(wav_file_id, "");
  } else {
    std::cout << parsed_and_validated_pre.warning_message();
    std::cout << parsed_and_validated_pre.error_message();

    return EXIT_FAILURE;
  }

  std::vector<gr_complex> xlat_taps = gr::filter::firdes::complex_band_pass(
      1, samp_rate, -samp_rate / (2 * decimation), samp_rate / (2 * decimation),
      transition_bw);
  gr_vector_float fir1_taps =
      gr::filter::firdes::high_pass(1.0, samp_rate / decimation, 100, 50);
  gr_vector_float fir2_taps = std::vector<float>(
      {0.002334677756186128, 0.01938096025639799, 0.14012609258404307,
       0.25997995536747043, 0.24015818184610402, 0.25997995536747043,
       0.14012609258404307, 0.01938096025639799, 0.002334677756186128});

  tb = gr::make_top_block("fg");

  auto src = gr::blocks::wavfile_source::make(wav_file.c_str(), false);
  auto add_offset = gr::blocks::add_const_ff::make(0.1);
  auto fm_mod = gr::analog::frequency_modulator_fc::make(1.0);
  auto interp_fir_taps =
      gr::filter::firdes::low_pass(1.0, samp_rate, 4500, 1000);
  auto interp_fir = gr::filter::interp_fir_filter_ccf::make(
      static_cast<int>(samp_rate / 16000), interp_fir_taps);

  xlat = gr::filter::freq_xlating_fir_filter_ccc::make(
      decimation, xlat_taps, xlat_center_freq, samp_rate);
  resampler = gr::filter::rational_resampler_ccc::make(
      rs_interpolation, rs_decimation, std::vector<gr_complex>(), 0.4);
  demod1 = gr::analog::quadrature_demod_cf::make(1.0);
  fir1 = gr::filter::fir_filter_fff::make(1, fir1_taps);
  hilbert = gr::filter::hilbert_fc::make(65);
  demod2 = gr::analog::quadrature_demod_cf::make(4.0);
  addConst = gr::blocks::add_const_ff::make(-1.5 * M_PI);
  fir2 = gr::filter::fir_filter_fff::make(1, fir2_taps);
  clockRecovery = gr::digital::clock_recovery_mm_ff::make(
      sps, 0.25f * 0.175f * 0.175f, 0.5, 0.175, 0.01);
  multiplyConst = gr::blocks::multiply_const_ff::make(-1.0f);
  slicer = gr::digital::binary_slicer_fb::make();
  // set this to 24 to receive all possible telegrams.
  // set it to 13 to receive not more data then needed for R09.16
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
            << "\n\n Compiler Flags: " << compilerFlags << "\n\n";

  try {
    tb->connect(src, 0, add_offset, 0);
    tb->connect(add_offset, 0, fm_mod, 0);
    tb->connect(fm_mod, 0, interp_fir, 0);
    tb->connect(interp_fir, 0, xlat, 0);

    tb->connect(xlat, 0, resampler, 0);
    tb->connect(resampler, 0, demod1, 0);
    tb->connect(demod1, 0, fir1, 0);
    tb->connect(fir1, 0, hilbert, 0);
    tb->connect(hilbert, 0, demod2, 0);
    tb->connect(demod2, 0, addConst, 0);
    tb->connect(addConst, 0, fir2, 0);
    tb->connect(fir2, 0, clockRecovery, 0);
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

  tb->wait();

  return EXIT_SUCCESS;
}
