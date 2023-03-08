# GNU-Radio Decoder

![](https://img.shields.io/endpoint?url=https%3A%2F%2Fhydra.hq.c3d2.de%2Fjob%2Ftlm-solutions%2Fgnuradio-decoder%2Fdefault.x86_64-linux%2Fshield) [![built with nix](https://builtwithnix.org/badge.svg)](https://builtwithnix.org)

**Contact:** <hello@tlm.solutions>

Service which connects to the software defined radio and does all the signal processing from reducing the noise floor to the conversion to diskrete bits. This piece of code also searches for the actual carriers and adjusts the frequency accordingly.

## environment variable arguments

Environment variable | description
---|---
DECODER\_FREQUENCY | float of the center frequency of the SDR
DECODER\_OFFSET | float of the offset of the signal from the center frequency
DECODER\_RF | int of RF value for SDR 
DECODER\_IF | int of IF value for SDR 
DECODER\_BB | int of BB value for SDR 
DECODER\_DEVICE\_STRING | device string for osmosdr

## building with nix flakes

```
    $ nix build .\#packages.x86_64-linux.gnuradio-decoder
```
