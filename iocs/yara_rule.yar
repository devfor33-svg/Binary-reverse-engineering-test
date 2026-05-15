rule apt_termux_binary {
    meta:
        description = "Detects the Termux version of the apt package manager binary"
        author = "Gemini CLI Reverse Engineer"
        date = "2026-05-15"
        hash = "d9b56ec48daabb800fabd91099ce689778ffa5ebefa78db62190b55ced210921"

    strings:
        $s1 = "libapt-pkg.so"
        $s2 = "libapt-private.so"
        $s3 = "/data/data/com.termux/files/usr/lib"
        $s4 = "modernize-sources"
        $s5 = "history-list"
        $s6 = "DoHistoryList"
        $s7 = "ModernizeSources"

    condition:
        uint32(0) == 0x464c457f and all of them
}
