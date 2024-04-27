{ gnuradio }:
gnuradio.unwrapped.override {
  features = {
    basic = true;
    volk = true;
    doxygen = false;
    sphinx = false;
    python-support = false;
    testing-support = false;
    gnuradio-runtime = true;
    gr-ctrlport = false;
    gnuradio-companion = false;
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
}
