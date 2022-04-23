{ pkgs, lib, stdenv}:
stdenv.mkDerivation {
    name = "telegram-decode";
    version = "0.1.0";

    src = ../.;

    buildInputs = [ pkgs.python39Packages.crcmod ];
    propagatedBuildInputs = [ pkgs.python39Packages.crcmod ];

    installPhase = ''
      mkdir -p $out/bin
      cp ./recv4.py $out/bin
    '';
}
