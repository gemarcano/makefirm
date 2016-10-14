# makefirm

Simple utility to make [FIRM](https://3dbrew.org/wiki/FIRM)-like binaries 
from regular ARM11/ARM9 payloads.

# Usage

Since the amount of binaries is hardcoded to 4 (of course, less than 4 can be used anyways) `makefirm` follows this idea.
The main FIRM header is created with the initial parameters `makefirm <FIRM_out.bin> <ARM11 entrypoint> <ARM9 entrypoint>`
You can include the binaries with `<binary.bin> <load address> <0/1> (ARM9 = 0, ARM11 = 1)` as extra parameters (up to 4).

For more information, see the source code.
