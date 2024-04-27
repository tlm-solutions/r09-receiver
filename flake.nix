{
  description = "Transit Live Mapping Solutions - R09 Receiver";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-23.11";

    utils.url = "github:numtide/flake-utils";
  };

  outputs = inputs@{ self, utils, nixpkgs, ... }:
    utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};

        libenvpp = pkgs.callPackage ./pkgs/libenvpp.nix { };
        gnuradio-decoder = let
          gnuradio_unwrapped = pkgs.callPackage ./pkgs/gnuradio.nix {
            gnuradio = pkgs.gnuradio3_8;
          };
        in pkgs.callPackage ./pkgs/gnuradio-decoder-cpp.nix {
          inherit gnuradio_unwrapped libenvpp;
          gnuradioPackages = pkgs.gnuradio3_8Packages;
        };
      in rec {
        checks = packages;
        packages = {
          inherit gnuradio-decoder libenvpp;
          default = gnuradio-decoder;
          docs = (pkgs.nixosOptionsDoc {
            options = (nixpkgs.lib.nixosSystem {
              inherit system;
              modules = [ self.nixosModules.default ];
            }).options.TLMS;
          }).optionsCommonMark;
        };
        devShells.default = pkgs.mkShell {
          nativeBuildInputs =
            (with packages.gnuradio-decoder; buildInputs ++ nativeBuildInputs);
        };
      }) // {
        overlays.default = final: prev: {
          inherit (self.packages.${prev.system}) gnuradio-decoder;
        };

        nixosModules = rec {
          default = gnuradio-decoder;
          gnuradio-decoder = import ./nixos-module;
        };

        hydraJobs = let
          hydraSystems = [ "x86_64-linux" "aarch64-linux" ];
          hydraBlacklist = [ ];
        in builtins.foldl' (hydraJobs: system:
          builtins.foldl' (hydraJobs: pkgName:
            if builtins.elem pkgName hydraBlacklist then
              hydraJobs
            else
              nixpkgs.lib.recursiveUpdate hydraJobs {
                ${pkgName}.${system} = self.packages.${system}.${pkgName};
              }) hydraJobs (builtins.attrNames self.packages.${system})) { }
        hydraSystems;
      };
}
