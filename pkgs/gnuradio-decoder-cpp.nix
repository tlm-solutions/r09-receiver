{ stdenv
, pkg-config
, cmake
, gnuradio
, osmosdr
, log4cpp
, mpir
, gmpxx
, thrift
, frequency ? "170795000"
, offset ? "19500"
, device ? "hackrf=0" }:
stdenv.mkDerivation {
  name = "gnuradio-decoder-cpp";
  version = "0.1.0";

  src = ./..;

  nativeBuildInputs = [ cmake pkg-config thrift gnuradio.python.pkgs.thrift ];
  buildInputs = [ log4cpp mpir gnuradio.boost.dev gnuradio gmpxx.dev osmosdr gnuradio.volk ];

  cmakeFlags = [ "-DOSMOSDR_DIR=${osmosdr}" ];

  # buildPhase = ''
  #   gcc -o gnuradio-decode-c
  #   HOME=$TEMPDIR
  #   cp $src flowgraph.grc
  #   sed -i 's/{{FREQUENCY}}/${frequency}/' flowgraph.grc
  #   sed -i 's/{{OFFSET}}/${offset}/' flowgraph.grc
  #   sed -i 's/{{DEVICE}}/${device}/' flowgraph.grc
  #   ${gnuradio}/bin/grcc flowgraph.grc
  #   PYTHONENV=$(head -1 ${gnuradio}/bin/.grcc-wrapped)
  #   sed -i "1s,.*,$PYTHONENV," recv_and_demod.py
  # '';

  # installPhase = ''
  #   mkdir -p $out/bin
  #   cp ./recv_and_demod.py $out/bin
  # '';
}
