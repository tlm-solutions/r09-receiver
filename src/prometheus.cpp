#include "prometheus.h"

PrometheusExporter::PrometheusExporter(
    const std::string &prometheus_host) noexcept {
  exposer_ = std::make_unique<prometheus::Exposer>(prometheus_host);
  registry_ = std::make_shared<prometheus::Registry>();

  exposer_->RegisterCollectable(registry_);
}

auto PrometheusExporter::signal_strength() noexcept
    -> prometheus::Family<prometheus::Gauge> & {
  return prometheus::BuildGauge()
      .Name("signal_strength")
      .Help("Current Signal Strength")
      .Register(*registry_);
}
