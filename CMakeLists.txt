cmake_minimum_required(VERSION 3.22)

add_executable(r09-receiver
               src/correlate_access_code_bb_ts_fl_impl.cpp
               src/prometheus.cpp
               src/prometheus_gauge_populator.cpp
               src/r09-receiver.cpp
               src/rational_resampler_impl.cc)

target_compile_options(r09-receiver PUBLIC -std=c++17 -Wall)

include_directories(src)

find_package(Gnuradio "3.8" REQUIRED)
find_package(Boost REQUIRED)
find_package(Volk REQUIRED)
find_package(libenvpp REQUIRED)
find_package(prometheus-cpp CONFIG REQUIRED)

include(${OSMOSDR_DIR}/lib/cmake/osmosdr/gnuradio-osmosdrConfig.cmake)

include_directories(${GNURADIO_ALL_INCLUDE_DIRS})

target_link_libraries(r09-receiver log4cpp gnuradio-digital gnuradio-analog gnuradio-filter gnuradio-blocks gnuradio-fft gnuradio-runtime gnuradio-pmt volk gnuradio-osmosdr libenvpp::libenvpp prometheus-cpp::pull)

install(TARGETS r09-receiver DESTINATION bin)
