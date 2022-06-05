{ stdenv
, pkg-config
, cmake
, gnuradio
, gnuradio_unwrapped
, osmosdr
, log4cpp
, mpir
, gmpxx
, thrift
, patchelf
, frequency ? "170795000"
, offset ? "19500"
, device ? "hackrf=0"
}:
let
  decoder-dependencies = stdenv.mkDerivation {
    name = "decoder-dependencies";
    version = "0.1.0";

    src = gnuradio_unwrapped;
      
    phases = [ "installPhase" ];

    installPhase = ''
      mkdir -p $out/lib

      for module in analog digital blocks filter fft pmt runtime
      do
        cp $src/lib/libgnuradio-$module.so.3.8.5 $out/lib/libgnuradio-$module.so.3.8.5
        chmod +w $out/lib/libgnuradio-$module.so.3.8.5
      done

      for module in analog digital blocks filter fft pmt runtime
      do
        RPATH=$(${patchelf}/bin/patchelf --print-rpath $out/lib/libgnuradio-$module.so.3.8.5)

        ${patchelf}/bin/patchelf --set-rpath $out/lib:$RPATH $out/lib/libgnuradio-$module.so.3.8.5
        ${patchelf}/bin/patchelf --shrink-rpath $out/lib/libgnuradio-$module.so.3.8.5
      done
    '';
  };

  osmosdr-dependency = stdenv.mkDerivation {
    name = "osmosdr-dependency";
    version = "0.1.0";

    src = osmosdr;

    phases = [ "installPhase" ];
    
    installPhase = ''
      mkdir -p $out/lib

      echo "test"

      cp $src/lib/libgnuradio-osmosdr.so.0.2.0.0 $out/lib/libgnuradio-osmosdr.so.0.2.0.0
      cp $src/lib/libgnuradio-osmosdr.so.0.2.0 $out/lib/libgnuradio-osmosdr.so.0.2.0

      chmod +w $out/lib/libgnuradio-osmosdr.so.0.2.0

      RPATH=$(${patchelf}/bin/patchelf --print-rpath $out/lib/libgnuradio-osmosdr.so.0.2.0)

      ${patchelf}/bin/patchelf --set-rpath ${decoder-dependencies}/lib:$RPATH $out/lib/libgnuradio-osmosdr.so.0.2.0
      ${patchelf}/bin/patchelf --shrink-rpath $out/lib/libgnuradio-osmosdr.so.0.2.0
    '';
  };

  decoder = stdenv.mkDerivation {
    name = "decoder";
    version = "0.1.0";

    src = ./..;

    nativeBuildInputs = [ cmake pkg-config thrift gnuradio.python.pkgs.thrift gnuradio osmosdr ];
    buildInputs = [ log4cpp mpir gnuradio.boost.dev gmpxx.dev gnuradio.volk ];

    cmakeFlags = [ "-DOSMOSDR_DIR=${osmosdr}" ];
  };
in
stdenv.mkDerivation {
  name = "decoder-shrinked";
  version = "0.1.0";

  src = decoder;

  phases = [ "installPhase" ];

  installPhase = ''
    mkdir -p $out/bin

    cp $src/bin/gnuradio-decoder-cpp gnuradio-decoder-cpp
    chmod +w gnuradio-decoder-cpp

    RPATH=$(${patchelf}/bin/patchelf --print-rpath gnuradio-decoder-cpp)

    ${patchelf}/bin/patchelf --set-rpath ${decoder-dependencies}/lib:${osmosdr-dependency}/lib:$RPATH gnuradio-decoder-cpp
    ${patchelf}/bin/patchelf --shrink-rpath gnuradio-decoder-cpp

    cp gnuradio-decoder-cpp $out/bin/gnuradio-decoder-cpp
  '';
}
