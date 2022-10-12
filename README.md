# GNU-Radio Decoder

![](https://img.shields.io/endpoint?url=https%3A%2F%2Fhydra.hq.c3d2.de%2Fjob%2Fdvb-dump%2Fradio-conf%2Fdefault.x86_64-linux%2Fshield) [![built with nix](https://builtwithnix.org/badge.svg)](https://builtwithnix.org)

**Contact:** <dump@dvb.solutions>

Service which connects to the software defined radio and does all the signal processing from reducing the noise floor to the conversion to diskrete bits. This piece of code also searches for the actual carries and adjusts the frequency accordingly.


```
nix-shell  -p '(gnuradio3_8.override { extraPackages = [ (callPackage ./reveng.nix {}) ]; })' -I nixpkgs=channel:nixos-21.05
```

## building with nix flakes


```
    $ nix build .\#packages.x86_64-linux.custom-gnuradio
    $ nix build .\#packages.x86_64-linux.gnuradio-decode
```


