{ pkgs, gnuradio3_8, ... }:
let
  gnuradio3_8_unwrapped = gnuradio3_8.unwrapped.override {
    features = {
      basic = true;
      volk = true;
      doxygen = false;
      sphinx = false;
      python-support = true;
      testing-support = false;
      gnuradio-runtime = false;
      gr-ctrlport = true;
      gnuradio-companion = true;
      gr-blocks = true;
      gr-fec = false;
      gr-fft = true;
      gr-dtv = false;
      gr-trellis = false;
      gr-audio = false;
      gr-zeromq = false;
      gr-uhd = false;
      gr-modtool = false;
      gr-video-sdl = false;
      gr-vocoder = false;
      examples = false;
      gr-utils = false;
      gr-qtgui = false;
      gr-blocktool = false;
      gr-wavelet = false;
    };
    versionAttr = {
      major = "3.8";
      minor = "3";
      patch = "0";
    };
    boost = pkgs.boost173;
  };
in
(gnuradio3_8.override {
  unwrapped = gnuradio3_8_unwrapped;

  extraPackages = [
    (pkgs.callPackage ./reveng.nix { unwrapped = gnuradio3_8_unwrapped; })
    (pkgs.gnuradio3_8Packages.osmosdr.overrideAttrs (old: rec {
      buildInputs = with pkgs; [ log4cpp mpir boost17x fftwFloat gmp icu hackrf rtl-sdr gnuradio3_8_unwrapped volk thrift gnuradio3_8_unwrapped.python.pkgs.thrift ];
      outputs = [ "out" "dev" ];
    }))
  ];
})
