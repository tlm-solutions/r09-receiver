{ stdenv
, lib
, fetchFromGitHub
, cmake
, pkgconfig
, gnuradio3_8
, makeWrapper
, log4cpp
, volk
, icu
, swig
, gmp
}:

stdenv.mkDerivation {
  pname = "gr-reveng";
  version = "2021-05-22";

  src = fetchFromGitHub {
    owner = "paulgclark";
    repo = "gr-reveng";
    rev = "0a777f9ddb7e52e61cd68f9d95351ab26122a8bd";
    sha256 = "15b39l6kx143ack9dimpsj5lyqlx19vxw5cax7yfv0nc8sahi0k5";
  };

  nativeBuildInputs = [ pkgconfig ];
  buildInputs = [
    cmake gnuradio3_8.unwrapped gnuradio3_8.unwrapped.boost gnuradio3_8.unwrapped.python makeWrapper log4cpp swig gmp volk icu
  ];

  postInstall = ''
    for prog in "$out"/bin/*; do
        wrapProgram "$prog" --set PYTHONPATH $PYTHONPATH:$(toPythonPath "$out")
    done
  '';

  enableParallelBuilding = true;

  meta = with lib; {
    description = "Reverse Engineering module to gnuradio.";
    homepage = https://github.com/paulgclark/gr-reveng;
    license = licenses.gpl3Plus;
    platforms = platforms.linux;
    maintainers = with maintainers; [ marenz ];
  };
}
