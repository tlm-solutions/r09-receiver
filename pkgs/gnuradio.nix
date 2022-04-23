{ pkgs, config, gnuradio3_8, ... }: 
(gnuradio3_8.override {
    extraPackages = [
      (pkgs.callPackage ./reveng.nix {})
    ];
})
