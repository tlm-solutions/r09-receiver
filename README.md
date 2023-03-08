# GNU-Radio Decoder

![](https://img.shields.io/endpoint?url=https%3A%2F%2Fhydra.hq.c3d2.de%2Fjob%2Fdvb-dump%2Fradio-conf%2Fdefault.x86_64-linux%2Fshield) [![built with nix](https://builtwithnix.org/badge.svg)](https://builtwithnix.org)

**Contact:** <hello@tlm.solutions>

Service which connects to the software defined radio and does all the signal processing from reducing the noise floor to the conversion to diskrete bits. This piece of code also searches for the actual carriers and adjusts the frequency accordingly.

## building with nix flakes

```
    $ nix build .\#packages.x86_64-linux.gnuradio-decoder
```
