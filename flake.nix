{
  description = "Dump DVBAG public transport radio";

  inputs = {
    utils.url = "github:numtide/flake-utils";
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-21.05";
  };

  outputs = inputs@{ self, utils, nixpkgs, ... }:
    utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};

        new_packages = {
          reveng = pkgs.callPackage ./pkgs/reveng.nix { };
          custom-gnuradio = pkgs.callPackage ./pkgs/gnuradio.nix { };

          gnuradio-decode = pkgs.callPackage ./pkgs/gnuradio-decode.nix {
            gnuradio = pkgs.callPackage ./pkgs/gnuradio.nix { };
            gnuradio_input_file = ./recv_rad1o.grc;
          };

          telegram-decode = pkgs.callPackage ./pkgs/telegram-decode.nix { };
        };

      in
      rec {
        checks = packages;
        packages = new_packages;
        overlay = (final: prev: new_packages);
      }
    ) // {
      hydraJobs =
        let
          hydraSystems = [
            "x86_64-linux"
            "aarch64-linux"
          ];
          hydraBlacklist = [
            # not an attrset
            "custom-gnuradio"
          ];
        in builtins.foldl' (hydraJobs: system:
          builtins.foldl' (hydraJobs: pkgName:
            if builtins.elem pkgName hydraBlacklist
            then hydraJobs
            else nixpkgs.lib.recursiveUpdate hydraJobs {
              ${pkgName}.${system} = self.packages.${system}.${pkgName};
            }
          ) hydraJobs (builtins.attrNames self.packages.${system})
        ) {} hydraSystems;
    };
}
