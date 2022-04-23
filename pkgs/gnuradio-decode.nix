{ pkgs, lib, gnuradio, stdenv, gnuradio_input_file }:
stdenv.mkDerivation {
  name = "gnuradio-python-soruce";
  version = "0.1.0";

  src = ../.;

  nativeBuildInputs = [ gnuradio ];

  buildPhase = ''
    HOME=$TEMPDIR
    ${gnuradio}/bin/grcc ${gnuradio_input_file}
    PYTHONENV=$(head -1 ${gnuradio}/bin/.gnuradio-companion-wrapped)
    sed -i "1s,.*,$PYTHONENV," recv_and_demod.py
  '';

  installPhase = ''
    mkdir -p $out/bin
    cp ./recv_and_demod.py $out/bin
  '';
}
