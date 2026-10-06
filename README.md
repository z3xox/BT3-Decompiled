# BT3-Decompiled

A matching decompilation of **Dragon Ball Z: Budokai Tenkaichi 3** for the PlayStation 2 (USA release,
SLUS-21678): the game's code rewritten as C that compiles, with the compiler the developers used, to exactly
the bytes on the disc.

This repository contains **no game data and no copy of the game**. To build it you need your own disc image
of that release; the build checks its result against it.

## Status

| Binary | Game code in matching C | Still in assembly |
|---|---|---|
| `SLUS_216.78` (main program) | 94.96% | 45 functions |
| `BIN/DBZP.BIN` (menu program) | 99.24% | 4 functions |

Both rebuilt binaries are byte-identical to the originals. The percentages count the game's own code; Sony's
SDK libraries and the CRI sound libraries linked into the executable are out of scope. The nine vertex
programs (VU1 microcode) are annotated listings that assemble back to the original bytes.

`python3 scripts/progress.py` prints the current numbers.

## Building

You need:

- your disc image, placed in the repository's top folder as
  `Dragon Ball Z - Budokai Tenkaichi 3 (USA) (En,Ja).iso` (the name `configure.py` looks for);
- `7z`, `ninja`, Python 3 with [splat](https://github.com/ethteck/splat) (`pip install "splat64[mips]"`) in `.venv`;
- in `tools/` (not part of this repository): Sony's `ee-gcc` 2.96 (the build in the
  [decompme compilers](https://github.com/decompme/compilers) release) and a MIPS binutils for the PS2
  (`mips-ps2-decompals-*`). `configure.py` has the exact paths it expects.

```
.venv/bin/python configure.py && ninja
cmp build/SLUS_216.78.rom disc/SLUS_216.78.rom && cmp build/DBZP.BIN disc/BIN/DBZP.BIN
```

`configure.py` extracts the two programs from your disc image, splits them, and writes the build file; `ninja`
compiles, links and compares.

## Layout

| Path | Contents |
|---|---|
| `src/`, `include/` | The decompiled code: `sys/` (engine), `battle/` (fights), `menu/` (the menu program), `vu1/` (vertex programs) |
| `config/` | How the binaries are split, and the symbol names (`config/symbols/`) |
| `scripts/` | Progress report, per-function diff, the VU1 assembler, the permuter driver |
| `docs/` | What has been learned about the game: start at [docs/README.md](docs/README.md) |

Names of functions, variables and files are ours, chosen from what the code does; the original names are not
known. The documentation marks what is verified by a byte match and what is inferred.

A function that does not match yet stays as the original assembly, with the best C attempt kept next to it.

## Related

The PC port built on this decompilation is a separate repository, **Tenkaichi3Decomp**. This repository holds
PS2 code only.

## Licence

What is ours in this repository (the names, comments, documentation, scripts and build configuration, and the
work of reconstruction) is dedicated to the public domain under [CC0 1.0](LICENSE). That dedication cannot and
does not cover the game itself: the program this code reproduces belongs to its rights holders.

## Legal

This project is not affiliated with or endorsed by the game's developers, publishers or rights holders. It
exists for preservation, study and interoperability. All rights to the game belong to their owners. You need
your own legally obtained copy of the game to use anything here.
