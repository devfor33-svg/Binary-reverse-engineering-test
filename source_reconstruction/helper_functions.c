/**
 * Helper function signatures identified in the 'apt' binary.
 * Most of these are implemented in libapt-pkg.so or libapt-private.so,
 * and now in our reconstructed main.cc using libapt-pkg directly.
 */

// Command handlers (all implemented in main.cc)
bool DoList(CommandLine &CmdL);
bool DoSearch(CommandLine &CmdL);
bool ShowPackage(CommandLine &CmdL);
bool DoInstall(CommandLine &CmdL);
bool DoRemove(CommandLine &CmdL);
bool DoUpdate(CommandLine &CmdL);
bool DoUpgrade(CommandLine &CmdL);
bool DoDistUpgrade(CommandLine &CmdL);
bool DoClean(CommandLine &CmdL);
bool DoAutoClean(CommandLine &CmdL);
bool Policy(CommandLine &CmdL);
bool Depends(CommandLine &CmdL);
bool RDepends(CommandLine &CmdL);
bool DoDownload(CommandLine &CmdL);
bool DoSource(CommandLine &CmdL);
bool DoBuildDep(CommandLine &CmdL);
bool DoChangelog(CommandLine &CmdL);
bool ShowSrcPackage(CommandLine &CmdL);
bool EditSources(CommandLine &CmdL);

// Termux-specific handlers
bool DoHistoryList(CommandLine &CmdL);
bool DoHistoryInfo(CommandLine &CmdL);
bool DoHistoryRedo(CommandLine &CmdL);
bool DoHistoryUndo(CommandLine &CmdL);
bool DoHistoryRollback(CommandLine &CmdL);
bool ModernizeSources(CommandLine &CmdL);

// Easter egg
bool DoMoo(CommandLine &CmdL);

/**
 * All handlers now use the libapt-pkg C++ API directly.
 * See main.cc for full implementations.
 *
 * Key design:
 * - pkgCacheFile for cache management
 * - pkgDepCache for dependency resolution
 * - pkgAcquire for downloading
 * - pkgPackageManager for install/remove
 * - pkgRecords for reading package descriptions
 *
 * Platform detection:
 * - isTermux() checks for /data/data/com.termux
 * - On Termux: uses /data/data/com.termux/files/usr prefix
 * - On Linux: uses standard paths (/etc/apt, /var/lib/apt)
 */
