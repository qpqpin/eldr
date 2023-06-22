# eldr
A small, compact ELF manual mapper.

<div align="center">
    <img src="https://github.com/qpqpin/eldr/assets/131700499/2d2baa36-c640-4fdd-944f-188a2c001321" width="450px"><br>
</div>

## Description
`eldr` is a small ELF loader that executes a given ELF file in memory. It does this via mapping the PT_LOAD segments of the ELF file
onto memory and fixes the pointers within the relocation table.

### Built with
- C

## Getting started
### Compiling
To compile `eldr`, simply execute the following command:
- `gcc -o eldr src/main.c src/elf.c src/reloc.c -ldl`

## Credits
```
https://github.com/qpqpin
```
### Contributions 🎉
###### All contributions are accepted, simply open an Issue / Pull request.
