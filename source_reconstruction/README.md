# Reconstructed Termux `apt` Source

Cross-platform APT frontend reconstructed from the Termux AArch64 binary, using only the public `libapt-pkg` API (no `libapt-private.so` dependency).

## Prerequisites
```bash
# Linux
apt install libapt-pkg-dev g++

# Termux
pkg install clang libapt-pkg
```

## Build & Run
```bash
make          # auto-detects platform
make test     # validate all commands

# update requires root for lock files
sudo ./apt update

# other commands run as normal user
./apt list
./apt search <package>
./apt show <package>
```

## Test Results (Linux)
| Command      | Status |
|-------------|--------|
| `--help`    | ✅ 39 lines |
| `moo`       | ✅ ASCII cow |
| `list`      | ✅ 2928 packages |
| `search`    | ✅ keyword match |
| `show`      | ✅ Package info |
| `policy`    | ✅ Version policy |
| `depends`   | ✅ Dependency tree |
| `rdepends`  | ✅ Reverse deps |
| `update`    | ✅ Repo sync |
| `clean`     | ✅ Cache clear |
| `history-list`| ✅ Termux history |

## Key Fixes Over Original Reconstruction
- Null `CommandLine::ArgList` segfault in `Parse()`
- `pkgDepCache::GetCandidateVersion()` (not protected `GetCandidateVer()`)
- Double-free in `CommandLine` destructor (nullptr after delete)
- Cross-platform Termux detection via `stat()`
- Non-null-terminated `CmdL.FileList` causing APT internal crashes
