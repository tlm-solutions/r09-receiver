{ stdenv, fetchFromGitHub, cmake, fmt, ... }:

stdenv.mkDerivation rec {
  pname = "libenvpp";
  version = "1.0.0";

  src = fetchFromGitHub {
    owner = "ph3at";
    repo = pname;
    rev = "v${version}";
    hash = "sha256-HArpxywEOdque/CTWz3UiqncXhVS0IFW/UcdTub8Yfg=";
  };

  patches = [ ./libenvpp-fix-cmake.patch ];

  cmakeFlags = [ "-DLIBENVPP_TESTS=OFF" "-DLIBENVPP_INSTALL=ON" ];

  nativeBuildInputs = [ cmake ];
  propagatedBuildInputs = [ fmt ];

  enableParallelBuilding = true;
}
