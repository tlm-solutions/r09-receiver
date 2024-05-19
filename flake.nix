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
        r09-receiver = let
          gnuradio_unwrapped = pkgs.callPackage ./pkgs/gnuradio.nix {
            gnuradio = pkgs.gnuradio3_8;
          };
        in pkgs.callPackage ./pkgs/r09-receiver.nix {
          inherit gnuradio_unwrapped libenvpp;
          gnuradioPackages = pkgs.gnuradio3_8Packages;
        };
      in rec {
        checks = packages;
        packages = {
          inherit r09-receiver libenvpp;
          default = r09-receiver;
          docs = (pkgs.nixosOptionsDoc {
            options = (nixpkgs.lib.nixosSystem {
              inherit system;
              modules = [ self.nixosModules.default ];
            }).options.TLMS;
          }).optionsCommonMark;
        };
        devShells.default = pkgs.mkShell {
          nativeBuildInputs =
            (with packages.r09-receiver; buildInputs ++ nativeBuildInputs);
        };
      }) // {
        overlays.default = final: prev: {
          inherit (self.packages.${prev.system}) r09-receiver;
        };

        nixosModules = rec {
          default = r09-receiver;
          r09-receiver = import ./nixos-module;
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
