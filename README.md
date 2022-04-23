# radio-conf
radio configuration running on the traffic stop boxes

```
nix-shell  -p '(gnuradio3_8.override { extraPackages = [ (callPackage ./reveng.nix {}) ]; })' -I nixpkgs=channel:nixos-21.05
```

