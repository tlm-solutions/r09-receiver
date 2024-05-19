#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <string>

#include <gnuradio/analog/pwr_squelch_cc.h>
#include <gnuradio/analog/quadrature_demod_cf.h>
#include <gnuradio/blocks/add_const_ff.h>
#include <gnuradio/blocks/complex_to_mag_squared.h>
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
#include <gnuradio/logger.h>
#include <gnuradio/prefs.h>
#include <gnuradio/sys_paths.h>
#include <gnuradio/top_block.h>
#include <libenvpp/env.hpp>
#include <osmosdr/source.h>

#include "correlate_access_code_bb_ts_fl.h"
#include "prometheus.h"
#include "prometheus_gauge_populator.h"
#include "rational_resampler.h"

static auto print_gnuradio_diagnostics() -> void {
  const auto ver = gr::version();
  const auto c_compiler = gr::c_compiler();
  const auto cxx_compiler = gr::cxx_compiler();
  const auto compiler_flags = gr::compiler_flags();
  const auto prefs = gr::prefs::singleton()->to_string();

  std::cout << "GNU Radio Version: " << ver << "\n\n C Compiler: " << c_compiler
            << "\n\n CXX Compiler: " << cxx_compiler << "\n\n Prefs: " << prefs
            << "\n\n Compiler Flags: " << compiler_flags << "\n\n";
}

auto receiver_main(const int frequency, const int offset, const int RF,
                   const int IF, const int BB, const std::string &device_string,
                   std::shared_ptr<PrometheusExporter> prometheus_exporter)
    -> void {
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

  std::vector<gr_complex> xlat_taps = gr::filter::firdes::complex_band_pass(
      1, samp_rate, -samp_rate / (2 * decimation), samp_rate / (2 * decimation),
      transition_bw);
  gr_vector_float fir1_taps =
      gr::filter::firdes::high_pass(1.0, samp_rate / decimation, 100, 50);
  gr_vector_float fir2_taps = std::vector<float>(
      {0.002334677756186128, 0.01938096025639799, 0.14012609258404307,
       0.25997995536747043, 0.24015818184610402, 0.25997995536747043,
       0.14012609258404307, 0.01938096025639799, 0.002334677756186128});

  auto tb = gr::make_top_block("fg");

  osmosdr::source::sptr src;
  src = osmosdr::source::make(device_string);
  src->set_block_alias("src");

  src->set_sample_rate(samp_rate);
  src->set_center_freq(frequency);
  src->set_gain_mode(false, 0);
  src->set_gain(RF, "RF", 0);
  src->set_gain(IF, "IF", 0);
  src->set_gain(BB, "BB", 0);
  src->set_bandwidth(bandwidth_sdr, 0);

  auto xlat = gr::filter::freq_xlating_fir_filter_ccc::make(
      decimation, xlat_taps, offset, samp_rate);
  auto resampler = gr::filter::rational_resampler_ccc::make(
      rs_interpolation, rs_decimation, std::vector<gr_complex>(), 0.4);
  auto demod1 = gr::analog::quadrature_demod_cf::make(1.0);
  auto fir1 = gr::filter::fir_filter_fff::make(1, fir1_taps);
  auto hilbert = gr::filter::hilbert_fc::make(65);
  auto demod2 = gr::analog::quadrature_demod_cf::make(4.0);
  auto addConst = gr::blocks::add_const_ff::make(-1.5 * M_PI);
  auto fir2 = gr::filter::fir_filter_fff::make(1, fir2_taps);
  auto clockRecovery = gr::digital::clock_recovery_mm_ff::make(
      sps, 0.25f * 0.175f * 0.175f, 0.5, 0.175, 0.01);
  auto multiplyConst = gr::blocks::multiply_const_ff::make(-1.0f);
  auto slicer = gr::digital::binary_slicer_fb::make();
  // set this to 24 to receive all possible telegrams.
  // set it to 13 to receive not more data then needed for R09.16
  auto correlate = gr::reveng::correlate_access_code_bb_ts_fl::make(
      "1111110000000001", 1, "packet_len", 24);
  auto taggedStreamToPdu = gr::blocks::tagged_stream_to_pdu::make(
      gr::blocks::pdu::vector_type::byte_t, "packet_len");
  auto udp_client =
      gr::blocks::socket_pdu::make("UDP_CLIENT", "localhost", "40000");

  tb->connect(src, 0, xlat, 0);
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

  // create blocks to save the power of the current channel if prometheus
  // exporter is available
  if (prometheus_exporter) {
    auto &signal_strength = prometheus_exporter->signal_strength();
    auto &stream_signal_strength =
        signal_strength.Add({{"frequency", std::to_string(frequency + offset)},
                             {"name", "R09 Receiver Average Signal Strength"}});

    auto mag_squared = gr::blocks::complex_to_mag_squared::make();
    // averaging filter over 60 second
    unsigned tap_size = samp_rate / decimation * 60;
    std::vector<float> averaging_filter(/*count=*/tap_size,
                                        /*alloc=*/1.0 / tap_size);
    // do not decimate directly to the final frequency, since there will be some
    // jitter
    auto fir = gr::filter::fir_filter_fff::make(/*decimation=*/samp_rate / decimation / 10,
                                                averaging_filter);
    auto populator = gr::prometheus::PrometheusGaugePopulator::make(
        /*gauge=*/stream_signal_strength);

    tb->connect(xlat, 0, mag_squared, 0);
    tb->connect(mag_squared, 0, fir, 0);
    tb->connect(fir, 0, populator, 0);
  }

  print_gnuradio_diagnostics();

  tb->start();

  tb->wait();
}

auto main(int argc, char **argv) -> int {
  auto pre = env::prefix("DECODER");

  float frequency;
  float offset;
  int RF, IF, BB;
  std::string device_string, prometheus_addr;

  const auto frequency_id = pre.register_required_variable<float>("FREQUENCY");
  const auto offset_id = pre.register_required_variable<float>("OFFSET");
  const auto RF_id = pre.register_variable<int>("RF");
  const auto IF_id = pre.register_variable<int>("IF");
  const auto BB_id = pre.register_variable<int>("BB");
  const auto device_string_id =
      pre.register_variable<std::string>("DEVICE_STRING");
  const auto prometheus_addr_id =
      pre.register_variable<std::string>("PROMETHEUS_ADDRESS");

  const auto parsed_and_validated_pre = pre.parse_and_validate();

  if (parsed_and_validated_pre.ok()) {
    frequency = parsed_and_validated_pre.get(frequency_id);
    offset = parsed_and_validated_pre.get(offset_id);
    RF = parsed_and_validated_pre.get_or(RF_id, 0);
    IF = parsed_and_validated_pre.get_or(IF_id, 0);
    BB = parsed_and_validated_pre.get_or(BB_id, 0);
    device_string = parsed_and_validated_pre.get_or(device_string_id, "");
    prometheus_addr = parsed_and_validated_pre.get_or(prometheus_addr_id, "");
  } else {
    std::cout << parsed_and_validated_pre.warning_message();
    std::cout << parsed_and_validated_pre.error_message();

    return EXIT_FAILURE;
  }

  std::shared_ptr<PrometheusExporter> prometheus_exporter;

  if (!prometheus_addr.empty()) {
    prometheus_exporter = std::make_shared<PrometheusExporter>(prometheus_addr);
  }

  try {
    receiver_main(frequency, offset, RF, IF, BB, device_string,
                  prometheus_exporter);
  } catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
