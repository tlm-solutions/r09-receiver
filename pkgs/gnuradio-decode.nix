{ pkgs, lib, gnuradio, stdenv, gnuradio_input_file}:
stdenv.mkDerivation {
    name = "gnuradio-python-soruce";
    version = "0.1.0";

    src = ../.;

    buildInputs = [ gnuradio (gnuradio.unwrapped.python.withPackages (ps: [gnuradio])) ];
    propagatedBuildInputs = [ gnuradio (gnuradio.unwrapped.python.withPackages (ps: [gnuradio])) ];
  
    inherit (gnuradio) python;
    
    buildPhase = ''
      HOME=$TEMPDIR
      ${gnuradio}/bin/grcc ${gnuradio_input_file}
      patchShebangs recv_and_demod.py
    '';

    shellHook = ''
      PYTHONPATH='${gnuradio}/lib/python3.8/site-packages/'     
    '';

    # ${PYTHONPATH+":"}$PYTHONPATH
    installPhase = ''
      mkdir -p $out/bin
      cp ./recv_and_demod.py $out/bin
    '';
}
