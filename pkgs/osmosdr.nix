{ pkgs, gnuradio, gnuradio_unwrapped, gnuradioPackages, ... }:
(gnuradioPackages.osmosdr.overrideDerivation(old: {
  gnuradio = gnuradio_unwrapped;
  buildInputs = with pkgs; [ log4cpp mpir gnuradio_unwrapped.boost.dev gmp icu hackrf rtl-sdr gnuradio_unwrapped.volk fftwFloat.dev gmpxx.dev ];
  nativeBuildInputs = with pkgs; [ cmake pkgconfig thrift gnuradio.python.pkgs.thrift gnuradio.python.pkgs.Mako swig gnuradio ];
  outputs = [ "out" ];
}))
