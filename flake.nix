{
  description = "build script for the lingua-franca alarm clock";

  inputs = {
    utils.url = "github:numtide/flake-utils";
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-21.05";
  };

  outputs = inputs@{self, utils, nixpkgs, ...}: 
    utils.lib.eachDefaultSystem (system: let 
      pkgs = nixpkgs.legacyPackages.${system};
      in rec {
        checks = packages;
        packages = {
          reveng = nixpkgs.legacyPackages.${system}.callPackage ./pkgs/reveng.nix {};
          custom-gnuradio = nixpkgs.legacyPackages.${system}.callPackage ./pkgs/gnuradio.nix {};

          gnuradio-decode = nixpkgs.legacyPackages.${system}.callPackage ./pkgs/gnuradio-decode.nix {
            gnuradio = nixpkgs.legacyPackages.${system}.callPackage ./pkgs/gnuradio.nix {};
            gnuradio_input_file = "recv_and_demod_soapy.grc";
          };

          telegram-decode = nixpkgs.legacyPackages.${system}.callPackage ./pkgs/telegram-decode.nix {};
        };
        overlay = (final: prev: {
          reveng = packages.${system}.reveng;
          custom-gnuradio = packages.${system}.custom-gnuradio;

          gnuradio-decode = nixpkgs.legacyPackages.${system}.callPackage ./pkgs/gnuradio-decode.nix {
            gnuradio = nixpkgs.legacyPackages.${system}.callPackage ./pkgs/gnuradio.nix {};
            gnuradio_input_file = "recv_and_demod_soapy.grc";
          };

          telegram-decode = nixpkgs.legacyPackages.${system}.callPackage ./pkgs/telegram-decode.nix {};
        });
      }
    );
}
