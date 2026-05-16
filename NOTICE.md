# Notice Regarding Termux and GPL Compliance

The original Termux project (https://termux.com/) is released under the **GNU General Public License v3.0** (GPLv3).

- The **reconstructed source code** in `source_reconstruction/` is an original, clean‑room reimplementation based on analysis of the binary interface and public `libapt‑pkg` API. It does **not** contain Termux’s proprietary code.
- The **patched binary** in `/bin` is a modified version of a Termux binary. If you choose to use or redistribute it, you must comply with the GPLv3 – including providing the complete corresponding source code of that binary. The original source code of Termux is available at https://github.com/termux/termux-app.

**Recommendation:** Do not distribute the patched binary. Build the reconstruction from source using the provided `Makefile`.

If you are a Termux maintainer and believe this project violates any of your rights, please open an issue – we will remove the offending parts immediately.
