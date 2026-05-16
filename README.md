# Termux 'apt' Binary — Reverse Engineering & Reconstruction

## Overview
Full static analysis and source-level reconstruction of the custom Termux `apt` (APT frontend) binary for AArch64. The reconstructed C++ implementation replicates all Termux-specific commands and works cross-platform (Linux + Termux).

## Status
| Component | Status |
|-----------|--------|
| Static analysis (disassembly, symbols, strings) | ✅ Complete |
| Source reconstruction (`main.cc`) | ✅ Complete |
| Cross-platform build (glibc Linux + Bionic Termux) | ✅ Verified |
| Zero compiler warnings (`-Wall -O2`) | ✅ Clean |
| All 11 test commands pass | ✅ Verified |

## Architecture
- **Original binary:** AArch64 ELF, linked against `libapt-pkg.so` and `libapt-private.so`
- **Reconstruction:** Pure C++17 using `libapt-pkg` public API — no `libapt-private.so` dependency
- **Termux detection:** Runtime `stat("/data/data/com.termux")` for automatic path configuration

## Folder Structure
```
├── bin/                    # Patched original binary (root-bypass NOPped)
├── iocs/                   # YARA rules, extracted strings, network/path IOCs
├── reports/                # Detailed analysis reports
│   ├── decompiled/         # Ghidra/IDA pseudocode
│   ├── disassembly.txt     # Full disassembly
│   ├── symbols.txt         # Symbol table dump
│   └── live_test_report.md # Live testing results
└── source_reconstruction/  # Reconstructed C++ source
    ├── main.cc             # Full apt frontend implementation
    ├── Makefile            # Cross-platform build system
    ├── setup.sh            # Auto-detection and config script
    ├── termux.sources      # Termux official repo config (DEB822)
    ├── helper_functions.c  # C function declarations from binary
    └── test_dispatch.cc    # Dispatch table test mock
```

## Reconstructed Commands
`update`, `install`, `remove`, `upgrade`, `dist-upgrade`, `full-upgrade`, `list`, `search`, `show`, `depends`, `rdepends`, `policy`, `clean`, `autoclean`, `autoremove`, `download`, `source`, `changelog`, `history-list`, `history-info`, `modernize-sources`, `moo`, `help`

## Build & Test
```bash
cd source_reconstruction
make        # auto-detects environment
make test   # runs all 11 command tests
```


> **⚠️ LEGAL NOTICE**  
> This project is for **educational reverse engineering** only. 
> This project is **not affiliated with Termux**.
