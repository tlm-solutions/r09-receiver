{
  description = "Dump DVBAG public transport radio";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-22.11";

    utils.url = "github:numtide/flake-utils";
  };

  outputs = inputs@{ self, utils, nixpkgs, ... }:
    utils.lib.eachDefaultSystem
      (system:
        let
          pkgs = nixpkgs.legacyPackages.${system};

          gnuradio-decoder =
            let
              gnuradio_unwrapped = pkgs.callPackage ./pkgs/gnuradio.nix { gnuradio = pkgs.gnuradio3_8; };
            in
            pkgs.callPackage ./pkgs/gnuradio-decoder-cpp.nix {
              gnuradio_unwrapped = gnuradio_unwrapped;
              gnuradioPackages = pkgs.gnuradio3_8Packages;
            };
        in
        rec {
          checks = packages;
          packages = {
            gnuradio-decoder = gnuradio-decoder;
            default = gnuradio-decoder;
          };
        }
      ) // {
      overlays.default = final: prev: {
        inherit (self.packages.${prev.system})
          gnuradio-decoder;
      };
      hydraJobs =
        let
          hydraSystems = [
            "x86_64-linux"
            "aarch64-linux"
          ];
          hydraBlacklist = [ ];
        in
        builtins.foldl'
          (hydraJobs: system:
            builtins.foldl'
              (hydraJobs: pkgName:
                if builtins.elem pkgName hydraBlacklist
                then hydraJobs
                else
                  nixpkgs.lib.recursiveUpdate hydraJobs {
                    ${pkgName}.${system} = self.packages.${system}.${pkgName};
                  }
              )
              hydraJobs
              (builtins.attrNames self.packages.${system})
          )
          { }
          hydraSystems;
    };
}
