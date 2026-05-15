apt: ELF 64-bit LSB shared object, ARM aarch64, version 1 (SYSV), dynamically linked, interpreter /system/bin/linker64, for Android 30, built by NDK r29 (14206865), stripped
d9b56ec48daabb800fabd91099ce689778ffa5ebefa78db62190b55ced210921  apt
ELF Header:
  Magic:   7f 45 4c 46 02 01 01 00 00 00 00 00 00 00 00 00 
  Class:                             ELF64
  Data:                              2's complement, little endian
  Version:                           1 (current)
  OS/ABI:                            UNIX - System V
  ABI Version:                       0
  Type:                              DYN (Shared object file)
  Machine:                           AArch64
  Version:                           0x1
  Entry point address:               0x8234
  Start of program headers:          64 (bytes into file)
  Start of section headers:          39432 (bytes into file)
  Flags:                             0x0
  Size of this header:               64 (bytes)
  Size of program headers:           56 (bytes)
  Number of program headers:         12
  Size of section headers:           64 (bytes)
  Number of section headers:         27
  Section header string table index: 26
### Section Headers
There are 27 section headers, starting at offset 0x9a08:

Section Headers:
  [Nr] Name              Type             Address           Offset
       Size              EntSize          Flags  Link  Info  Align
  [ 0]                   NULL             0000000000000000  00000000
       0000000000000000  0000000000000000           0     0     0
  [ 1] .interp           PROGBITS         00000000000002e0  000002e0
       0000000000000015  0000000000000000   A       0     0     1
  [ 2] .note.androi[...] NOTE             00000000000002f8  000002f8
       0000000000000098  0000000000000000   A       0     0     4
  [ 3] .dynsym           DYNSYM           0000000000000390  00000390
       0000000000000870  0000000000000018   A       7     1     8
  [ 4] .gnu.version      VERSYM           0000000000000c00  00000c00
       00000000000000b4  0000000000000002   A       3     0     2
  [ 5] .gnu.version_r    VERNEED          0000000000000cb4  00000cb4
       0000000000000060  0000000000000000   A       7     3     4
  [ 6] .gnu.hash         GNU_HASH         0000000000000d18  00000d18
       000000000000001c  0000000000000000   A       3     0     8
  [ 7] .dynstr           STRTAB           0000000000000d34  00000d34
       0000000000000a8e  0000000000000000   A       0     0     1
  [ 8] .rela.dyn         RELA             00000000000017c8  000017c8
       0000000000000438  0000000000000018   A       3     0     8
  [ 9] .relr.dyn         ANDROID_RELR     0000000000001c00  00001c00
       0000000000000038  0000000000000008   A       0     0     8
  [10] .rela.plt         RELA             0000000000001c38  00001c38
       0000000000000528  0000000000000018  AI       3    21     8
  [11] .gcc_except_table PROGBITS         0000000000002160  00002160
       00000000000000d0  0000000000000000   A       0     0     4
  [12] .rodata           PROGBITS         0000000000002230  00002230
       00000000000012e0  0000000000000000 AMS       0     0     4
  [13] .eh_frame_hdr     PROGBITS         0000000000003510  00003510
       00000000000001e4  0000000000000000   A       0     0     4
  [14] .eh_frame         PROGBITS         00000000000036f8  000036f8
       0000000000000b3c  0000000000000000   A       0     0     8
  [15] .text             PROGBITS         0000000000008234  00004234
       0000000000004934  0000000000000000  AX       0     0     4
  [16] .plt              PROGBITS         000000000000cb70  00008b70
       0000000000000390  0000000000000000  AX       0     0     16
  [17] .tdata            PROGBITS         0000000000010f00  00008f00
       0000000000000000  0000000000000000 WAT       0     0     64
  [18] .data.rel.ro      PROGBITS         0000000000010f00  00008f00
       00000000000004a8  0000000000000000  WA       0     0     8
  [19] .dynamic          DYNAMIC          00000000000113a8  000093a8
       00000000000001c0  0000000000000010  WA       7     0     8
  [20] .got              PROGBITS         0000000000011568  00009568
       0000000000000048  0000000000000000  WA       0     0     8
  [21] .got.plt          PROGBITS         00000000000115b0  000095b0
       00000000000001d0  0000000000000000  WA       0     0     8
  [22] .relro_padding    NOBITS           0000000000011780  00009780
       0000000000000880  0000000000000000  WA       0     0     1
  [23] .data             PROGBITS         0000000000015780  00009780
       0000000000000038  0000000000000000  WA       0     0     8
  [24] .bss              NOBITS           00000000000157b8  000097b8
       0000000000000848  0000000000000000  WA       0     0     8
  [25] .comment          PROGBITS         0000000000000000  000097b8
       000000000000014f  0000000000000001  MS       0     0     1
  [26] .shstrtab         STRTAB           0000000000000000  00009907
       0000000000000101  0000000000000000           0     0     1
Key to Flags:
  W (write), A (alloc), X (execute), M (merge), S (strings), I (info),
  L (link order), O (extra OS processing required), G (group), T (TLS),
  C (compressed), x (unknown), o (OS specific), E (exclude),
  D (mbind), p (processor specific)
