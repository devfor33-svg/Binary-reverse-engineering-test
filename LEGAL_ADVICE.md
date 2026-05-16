# Legal basis for this project

This project is a personal, non‑commercial educational exercise.

1. **Right to reverse engineer** – Under EU Directive 2009/24/EC and US 17 USC § 1201(f), reverse engineering is lawful when necessary to achieve interoperability. Here, the goal was to create a standalone `apt` frontend that works with Termux repositories – an interoperable replacement.
2. **No trade secret misappropriation** – All analysis was performed on publicly available binaries using standard static analysis tools. No non‑public Termux source code was accessed.
3. **Clean room reconstruction** – The final C++ code was written by referencing only the decompiled pseudocode (which is not copyrightable under *Sega v. Accolade*) and public API documentation.
4. **Potential risk** – The patched binary `/bin/apt_patched` is a direct modification of Termux’s binary. This **does** constitute a derivative work and may violate Termux’s copyright if distributed without source. **Users are advised to delete it and build from source instead.**

If Termux requests removal of the patched binary, we will comply immediately.
