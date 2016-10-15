# makefirm

Simple utility to make [FIRM](https://3dbrew.org/wiki/FIRM)-like binaries 
from regular ARM11/ARM9 payloads.

# Usage

Since the amount of binaries is hardcoded to 4 in official FIRMs, `makefirm` follows this limitation.

The main FIRM header is created with the initial parameters `<FIRM_out.bin> <ARM11 entrypoint> <ARM9 entrypoint>`

For each binary you want to include [1-4] you can use `<payload.bin> <load address> <0/1> (ARM9 = 0, ARM11 = 1)` as extra parameters to `makefirm`.


For more information, see the source code.


###### Note: Obviously, the RSA signature is not calculated since we don't have the key. If this ever changes (doubt it) I'll modify `makefirm` to accept it.

# Credits

fox8091 for the idea and some info, Brad Conte for the pd implementation of the SHA256 hashing algorithm and 3dbrew for the documentation.
