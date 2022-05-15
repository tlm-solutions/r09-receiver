{ pkgs, lib, gnuradio, stdenv, gnuradio_input_file, frequency ? "170795000", offset ? "19500", device ? "hackrf=0" }:
stdenv.mkDerivation {
  name = "gnuradio-python-source";
  version = "0.1.0";

  src = gnuradio_input_file;

  phases = [ "buildPhase" "installPhase" ];

  nativeBuildInputs = [ gnuradio ];

  buildPhase = ''
    HOME=$TEMPDIR
    cp $src flowgraph.grc
    sed -i 's/{{FREQUENCY}}/${frequency}/' flowgraph.grc
    sed -i 's/{{OFFSET}}/${offset}/' flowgraph.grc
    sed -i 's/{{DEVICE}}/${device}/' flowgraph.grc
    ${gnuradio}/bin/grcc flowgraph.grc
    PYTHONENV=$(head -1 ${gnuradio}/bin/.grcc-wrapped)
    sed -i "1s,.*,$PYTHONENV," recv_and_demod.py
  '';

  installPhase = ''
    mkdir -p $out/bin
    cp ./recv_and_demod.py $out/bin
  '';
}
