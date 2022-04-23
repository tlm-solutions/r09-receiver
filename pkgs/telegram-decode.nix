{ pkgs, lib, stdenv }:
stdenv.mkDerivation {
  name = "telegram-decode";
  version = "0.1.0";

  src = ../.;

  buildInputs = [ (pkgs.python39.withPackages (ps: with ps; [ crcmod ])) ];
  propagatedBuildInputs = [ (pkgs.python39.withPackages (ps: with ps; [ crcmod ])) ];

  installPhase = ''
    mkdir -p $out/bin
    cp ./recv4.py $out/bin
  '';
}
