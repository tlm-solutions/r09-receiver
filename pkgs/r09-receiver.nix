{ stdenv
, pkg-config
, cmake
, gnuradio_unwrapped
, gnuradioPackages
, log4cpp
, mpir
, gmp
, gmpxx
, thrift
, hackrf
, rtl-sdr
, fftwFloat
, patchelf
, libenvpp
, prometheus-cpp
, zlib
, curlFull
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

  osmosdr = gnuradioPackages.osmosdr.overrideDerivation (old: {
    gnuradio = gnuradio_unwrapped;
    buildInputs = [ log4cpp mpir gnuradio_unwrapped.boost.dev gmp hackrf rtl-sdr gnuradio_unwrapped.volk fftwFloat.dev gmpxx.dev gnuradio_unwrapped ];
    nativeBuildInputs = [ cmake pkg-config ];
    cmakeFlags = [ "-DENABLE_PYTHON=OFF" ];
    outputs = [ "out" ];
  });

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

  r09-receiver = stdenv.mkDerivation {
    name = "r09-receiver";
    version = "0.1.0";

    src = ./..;

    nativeBuildInputs = [ cmake pkg-config gnuradio_unwrapped osmosdr ];
    buildInputs = [ log4cpp mpir gnuradio_unwrapped.boost.dev gmpxx.dev gnuradio_unwrapped.volk libenvpp prometheus-cpp zlib curlFull ];

    cmakeFlags = [ "-DOSMOSDR_DIR=${osmosdr}" ];
  };
in
stdenv.mkDerivation {
  name = "r09-receiver-striped";
  version = "0.1.0";

  src = r09-receiver;

  phases = [ "installPhase" ];

  installPhase = ''
    mkdir -p $out/bin

    cp $src/bin/r09-receiver r09-receiver
    chmod +w r09-receiver

    RPATH=$(${patchelf}/bin/patchelf --print-rpath r09-receiver)

    ${patchelf}/bin/patchelf --set-rpath ${decoder-dependencies}/lib:${osmosdr-dependency}/lib:$RPATH r09-receiver
    ${patchelf}/bin/patchelf --shrink-rpath r09-receiver

    cp r09-receiver $out/bin/r09-receiver
  '';
}
