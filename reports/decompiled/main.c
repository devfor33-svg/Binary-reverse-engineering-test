/* Decompiled with manual analysis of AArch64 disassembly */
#include <apt-pkg/cmndline.h>
#include <apt-pkg/configuration.h>
#include <apt-pkg/init.h>
#include <apt-pkg/pkgsystem.h>
#include <iostream>

int main(int argc, const char **argv) {
    // Local variables on stack
    CommandLine CmdL;
    Configuration *Config = _config;
    pkgSystem *Sys = _system;

    // Register 0x22 (TPIDR_EL0) used for stack canary
    // getuid() check at 0x8360
    if (getuid() != 0) {
        // Log info or handle non-root execution
    }

    // Constructor for CommandLine at 0x836c
    // _ZN11CommandLineC1Ev(&CmdL)
    
    // ParseCommandLine call at 0x83a8
    // Signature: ParseCommandLine(CmdL, ..., argc, argv, ...)
    if (!ParseCommandLine(CmdL, 0, Config, &Sys, argc, argv, NULL, NULL)) {
        return 100;
    }

    // Initialize Signals and Output
    InitSignals();
    InitOutput(std::cout.rdbuf());

    // Check environment context
    CheckIfCalledByScript(argc, argv);
    CheckIfSimulateMode(CmdL);

    // Dispatching logic at 0x8438
    // DispatchCommandLine(CmdL, DispatchTable)
    int result = DispatchCommandLine(CmdL, NULL);

    // Destructor for CommandLine at 0x8460
    // _ZN11CommandLineD1Ev(&CmdL)

    return result;
}
