#ifndef PROMETHEUS_GAUGE_POPULATOR_H
#define PROMETHEUS_GAUGE_POPULATOR_H

#include <gnuradio/block.h>
#include <gnuradio/blocks/api.h>

#include <prometheus/gauge.h>

namespace gr::prometheus {

/// This block takes a float as an input and writes in into a prometheus gauge
class PrometheusGaugePopulator : virtual public block {
private:
  /// the prometheus gauge we are populating with this block
  ::prometheus::Gauge &gauge_;

public:
  using sptr = boost::shared_ptr<PrometheusGaugePopulator>;

  PrometheusGaugePopulator() = delete;

  explicit PrometheusGaugePopulator(::prometheus::Gauge &gauge);

  static auto make(::prometheus::Gauge &gauge) -> sptr;

  auto general_work(int noutput_items, gr_vector_int &ninput_items,
                    gr_vector_const_void_star &input_items,
                    gr_vector_void_star &output_items) -> int override;
};

} // namespace gr::prometheus

#endif // PROMETHEUS_GAUGE_POPULATOR_H
