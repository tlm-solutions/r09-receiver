{ pkgs, gnuradio, gnuradio_unwrapped, gnuradioPackages, ... }:
let
  gnuradio_wrapped_unwrapped = gnuradio.overrideDerivation(old: {
    unwrapped = gnuradio_unwrapped;
  });
in
(gnuradioPackages.osmosdr.overrideDerivation(old: {
  gnuradio = gnuradio_unwrapped;
  buildInputs = with pkgs; [ log4cpp mpir gnuradio_unwrapped.boost.dev gmp icu hackrf rtl-sdr gnuradio_unwrapped.volk fftwFloat.dev gmpxx.dev ];
  nativeBuildInputs = with pkgs; [ cmake pkgconfig thrift gnuradio_unwrapped.python.pkgs.thrift gnuradio_unwrapped.python.pkgs.Mako swig gnuradio_wrapped_unwrapped ];
  outputs = [ "out" ];
}))
