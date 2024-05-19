#ifndef PROMETHEUS_H
#define PROMETHEUS_H

#include <memory>

#include <prometheus/counter.h>
#include <prometheus/exposer.h>
#include <prometheus/registry.h>

class PrometheusExporter {
private:
  std::shared_ptr<prometheus::Registry> registry_;
  std::unique_ptr<prometheus::Exposer> exposer_;

public:
  PrometheusExporter(const std::string &host) noexcept;
  ~PrometheusExporter() noexcept = default;

  auto signal_strength() noexcept -> prometheus::Family<prometheus::Gauge> &;
};

#endif // PROMETHEUS_H
