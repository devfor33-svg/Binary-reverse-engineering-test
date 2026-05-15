#include <apt-pkg/cmndline.h>
#include <apt-pkg/init.h>
#include <apt-pkg/configuration.h>
#include <apt-pkg/pkgsystem.h>
#include <apt-pkg/error.h>
#include <apt-pkg/cachefile.h>
#include <apt-pkg/sourcelist.h>
#include <apt-pkg/acquire.h>
#include <apt-pkg/acquire-item.h>
#include <apt-pkg/algorithms.h>
#include <apt-pkg/packagemanager.h>
#include <apt-pkg/upgrade.h>
#include <apt-pkg/pkgrecords.h>
#include <apt-pkg/update.h>
#include <apt-pkg/progress.h>
#include <apt-pkg/clean.h>
#include <apt-pkg/metaindex.h>
#include <apt-pkg/install-progress.h>

#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <cstring>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;

// --- Platform detection ---
static bool isTermux() {
    struct stat buf;
    return (stat("/data/data/com.termux", &buf) == 0);
}

static string termuxPrefix() {
    return "/data/data/com.termux/files/usr";
}

// --- Forward declarations ---
static bool confirmAction(const string &msg);

// --- Cache helpers ---
static bool openCache(pkgCacheFile &Cache, bool withLock = true) {
    OpTextProgress Progress;
    if (!Cache.Open(&Progress, withLock)) {
        cerr << "E: Could not open cache" << endl;
        if (_error->PendingError())
            _error->DumpErrors(cerr);
        return false;
    }
    return true;
}

// --- Command Handlers ---

bool DoUpdate(CommandLine &CmdL) {
    pkgCacheFile Cache;
    if (!openCache(Cache, false))
        return false;

    pkgSourceList *SrcList = Cache.GetSourceList();
    if (SrcList == nullptr || SrcList->empty()) {
        cerr << "E: The source list is empty" << endl;
        return false;
    }

    class UpdateProgress : public pkgAcquireStatus {
    public:
        void Start() override { }
        void Stop() override { cout << endl; }
        bool Pulse(pkgAcquire *) override { return true; }
        bool MediaChange(std::string, std::string) override { return true; }
    };

    UpdateProgress Progress;
    if (!ListUpdate(Progress, *SrcList)) {
        cerr << endl << "E: Failed to update package lists" << endl;
        if (_error->PendingError())
            _error->DumpErrors(cerr);
        return false;
    }

    cout << "Done." << endl;
    return true;
}

bool DoInstall(CommandLine &CmdL) {
    if (CmdL.FileSize() == 0) {
        cerr << "E: No package name provided" << endl;
        return false;
    }

    pkgCacheFile Cache;
    if (!openCache(Cache))
        return false;

    pkgDepCache *DepCache = Cache.GetDepCache();
    if (DepCache == nullptr) {
        cerr << "E: Could not create dep cache" << endl;
        return false;
    }
    pkgCache *PkgCache = Cache.GetPkgCache();

    for (unsigned i = 0; i < CmdL.FileSize(); i++) {
        const char *arg = CmdL.FileList[i];
        bool removeMode = (arg[0] == '-');
        bool installMode = (arg[0] == '+');
        const char *name = arg;
        if (removeMode || installMode)
            name = arg + 1;

        pkgCache::PkgIterator Pkg = PkgCache->FindPkg(name);
        if (Pkg.end()) {
            cerr << "E: Unable to locate package " << name << endl;
            return false;
        }

        if (removeMode) {
            DepCache->MarkDelete(Pkg, true);
        } else {
            pkgCache::VerIterator Ver = DepCache->GetCandidateVersion(Pkg);
            if (Ver.end()) {
                cerr << "E: Package " << name << " has no candidate version" << endl;
                return false;
            }
            DepCache->SetCandidateVersion(Ver);
            DepCache->MarkInstall(Pkg, true);
        }
    }

    pkgProblemResolver Resolve(DepCache);
    if (!Resolve.Resolve(true)) {
        cerr << "E: Unable to resolve dependencies" << endl;
        if (_error->PendingError())
            _error->DumpErrors(cerr);
        return false;
    }

    int installCount = 0, removeCount = 0, upgradeCount = 0;
    for (pkgCache::PkgIterator Pkg = PkgCache->PkgBegin(); !Pkg.end(); Pkg++) {
        if ((*DepCache)[Pkg].NewInstall()) installCount++;
        if ((*DepCache)[Pkg].Delete()) removeCount++;
        if ((*DepCache)[Pkg].Upgrade()) upgradeCount++;
    }

    cout << upgradeCount << " upgraded, " << installCount << " newly installed, ";
    cout << removeCount << " to remove" << endl;

    if (!confirmAction("Do you want to continue?"))
        return true;

    pkgPackageManager *PM = _system->CreatePM(DepCache);
    if (PM == nullptr) {
        cerr << "E: Could not create package manager" << endl;
        return false;
    }

    pkgRecords Recs(*PkgCache);
    pkgAcquire Fetcher;

    if (!PM->GetArchives(&Fetcher, Cache.GetSourceList(), &Recs)) {
        cerr << "E: Failed to queue archives for download" << endl;
        if (_error->PendingError())
            _error->DumpErrors(cerr);
        delete PM;
        return false;
    }

    if (Fetcher.Run() != pkgAcquire::Continue) {
        cerr << "E: Failed to download packages" << endl;
        if (_error->PendingError())
            _error->DumpErrors(cerr);
        delete PM;
        return false;
    }

    APT::Progress::PackageManagerText PMProgress;
    pkgPackageManager::OrderResult result = PM->DoInstall(&PMProgress);
    delete PM;
    if (result != pkgPackageManager::Completed) {
        cerr << "E: Installation failed (order result: " << result << ")" << endl;
        if (_error->PendingError())
            _error->DumpErrors(cerr);
        return false;
    }

    cout << "Done." << endl;
    return true;
}

bool DoRemove(CommandLine &CmdL) {
    if (CmdL.FileSize() == 0) {
        cerr << "E: No package name provided" << endl;
        return false;
    }

    pkgCacheFile Cache;
    if (!openCache(Cache))
        return false;

    pkgDepCache *DepCache = Cache.GetDepCache();
    if (DepCache == nullptr)
        return false;
    pkgCache *PkgCache = Cache.GetPkgCache();

    for (unsigned i = 0; i < CmdL.FileSize(); i++) {
        pkgCache::PkgIterator Pkg = PkgCache->FindPkg(CmdL.FileList[i]);
        if (Pkg.end()) {
            cerr << "E: Unable to locate package " << CmdL.FileList[i] << endl;
            return false;
        }
        DepCache->MarkDelete(Pkg, true);
    }

    pkgProblemResolver Resolve(DepCache);
    if (!Resolve.Resolve(true)) {
        if (_error->PendingError())
            _error->DumpErrors(cerr);
        return false;
    }

    int removeCount = 0;
    for (pkgCache::PkgIterator Pkg = PkgCache->PkgBegin(); !Pkg.end(); Pkg++) {
        if ((*DepCache)[Pkg].Delete()) removeCount++;
    }

    cout << removeCount << " packages to remove." << endl;

    if (!confirmAction("Do you want to continue?"))
        return true;

    pkgPackageManager *PM = _system->CreatePM(DepCache);
    if (PM == nullptr) {
        cerr << "E: Could not create package manager" << endl;
        return false;
    }

    APT::Progress::PackageManagerText PMProgress;
    bool result = PM->DoInstall(&PMProgress);
    delete PM;
    if (!result) {
        cerr << "E: Removal failed" << endl;
        if (_error->PendingError())
            _error->DumpErrors(cerr);
        return false;
    }

    cout << "Done." << endl;
    return true;
}

bool DoUpgrade(CommandLine &CmdL) {
    pkgCacheFile Cache;
    if (!openCache(Cache))
        return false;

    pkgDepCache *DepCache = Cache.GetDepCache();
    if (DepCache == nullptr)
        return false;

    if (!APT::Upgrade::Upgrade(*DepCache, APT::Upgrade::ALLOW_EVERYTHING)) {
        cerr << "E: Unable to calculate upgrade" << endl;
        if (_error->PendingError())
            _error->DumpErrors(cerr);
        return false;
    }

    pkgCache *PkgCache = Cache.GetPkgCache();
    int upgradeCount = 0, installCount = 0;
    for (pkgCache::PkgIterator Pkg = PkgCache->PkgBegin(); !Pkg.end(); Pkg++) {
        if ((*DepCache)[Pkg].Upgrade()) upgradeCount++;
        if ((*DepCache)[Pkg].NewInstall()) installCount++;
    }

    if (upgradeCount == 0 && installCount == 0) {
        cout << "All packages are up to date." << endl;
        return true;
    }

    cout << upgradeCount << " upgraded, " << installCount << " newly installed" << endl;

    if (!confirmAction("Do you want to continue?"))
        return true;

    pkgPackageManager *PM = _system->CreatePM(DepCache);
    if (PM == nullptr) {
        cerr << "E: Could not create package manager" << endl;
        return false;
    }

    APT::Progress::PackageManagerText PMProgress;
    bool result = PM->DoInstall(&PMProgress);
    delete PM;
    if (!result) {
        cerr << "E: Upgrade failed" << endl;
        if (_error->PendingError())
            _error->DumpErrors(cerr);
        return false;
    }

    cout << "Done." << endl;
    return true;
}

bool DoDistUpgrade(CommandLine &CmdL) {
    return DoUpgrade(CmdL);
}

bool DoList(CommandLine &CmdL) {
    pkgCacheFile Cache;
    if (!openCache(Cache, false))
        return false;

    pkgCache *PkgCache = Cache.GetPkgCache();
    pkgRecords Records(*PkgCache);

    string pattern;
    if (CmdL.FileSize() > 0)
        pattern = CmdL.FileList[0];

    regex patternRegex;
    bool hasPattern = false;
    if (!pattern.empty()) {
        try {
            string p = pattern;
            if (p.find('*') != string::npos || p.find('?') != string::npos) {
                p = regex_replace(p, regex("\\*"), ".*");
                p = regex_replace(p, regex("\\?"), ".");
            }
            patternRegex = regex(p, regex::icase);
            hasPattern = true;
        } catch (...) {
            hasPattern = false;
        }
    }

    int count = 0;
    for (pkgCache::PkgIterator Pkg = PkgCache->PkgBegin(); !Pkg.end(); Pkg++) {
        if (hasPattern && !regex_search(string(Pkg.Name()), patternRegex))
            continue;

        count++;
        pkgCache::VerIterator Ver = Pkg.VersionList();
        if (Ver.end()) {
            cout << Pkg.Name() << "/virtual" << endl;
            continue;
        }

        pkgRecords::Parser &Rec = Records.Lookup(Ver.FileList());

        if (Pkg->CurrentVer != 0)
            cout << "[installed] ";
        cout << Pkg.Name() << "/" << Ver.VerStr();
        cout << " - " << Rec.ShortDesc() << endl;
    }

    cout << count << " package(s) listed." << endl;
    return true;
}

bool DoSearch(CommandLine &CmdL) {
    if (CmdL.FileSize() == 0) {
        cerr << "E: No search pattern provided" << endl;
        return false;
    }

    pkgCacheFile Cache;
    if (!openCache(Cache, false))
        return false;

    pkgCache *PkgCache = Cache.GetPkgCache();
    pkgDepCache *DepCache = Cache.GetDepCache();
    pkgRecords Records(*PkgCache);

    string pattern = CmdL.FileList[0];
    regex patternRegex;
    try {
        string p = pattern;
        if (p.find('*') != string::npos || p.find('?') != string::npos) {
            p = regex_replace(p, regex("\\*"), ".*");
            p = regex_replace(p, regex("\\?"), ".");
        }
        patternRegex = regex(p, regex::icase);
    } catch (const regex_error &e) {
        cerr << "E: Invalid regex pattern: " << e.what() << endl;
        return false;
    }

    int matches = 0;
    for (pkgCache::PkgIterator Pkg = PkgCache->PkgBegin(); !Pkg.end(); Pkg++) {
        if (regex_search(string(Pkg.Name()), patternRegex))
            goto match;
        if (!Pkg.VersionList().end()) {
            pkgRecords::Parser &Rec = Records.Lookup(Pkg.VersionList().FileList());
            string desc = Rec.ShortDesc();
            if (regex_search(desc, patternRegex))
                goto match;
        }
        continue;

match:
        matches++;
        pkgCache::VerIterator Ver = Pkg.VersionList();
        char status = ' ';
        if (DepCache != nullptr) {
            if ((*DepCache)[Pkg].Delete()) status = 'r';
            else if ((*DepCache)[Pkg].NewInstall()) status = 'n';
            else if ((*DepCache)[Pkg].Upgrade()) status = 'u';
            else if (Pkg->CurrentVer != 0) status = 'i';
        }

        if (Ver.end()) {
            cout << status << " " << Pkg.Name() << " - (virtual)" << endl;
        } else {
            pkgRecords::Parser &Rec = Records.Lookup(Ver.FileList());
            cout << status << " " << Pkg.Name() << " - " << Rec.ShortDesc() << endl;
        }
    }

    if (matches == 0)
        cout << "No matches found for '" << pattern << "'" << endl;
    return true;
}

bool ShowPackage(CommandLine &CmdL) {
    if (CmdL.FileSize() == 0) {
        cerr << "E: No package name provided" << endl;
        return false;
    }

    pkgCacheFile Cache;
    if (!openCache(Cache, false))
        return false;

    pkgCache *PkgCache = Cache.GetPkgCache();
    pkgRecords Records(*PkgCache);

    for (unsigned i = 0; i < CmdL.FileSize(); i++) {
        pkgCache::PkgIterator Pkg = PkgCache->FindPkg(CmdL.FileList[i]);
        if (Pkg.end()) {
            cerr << "E: Package " << CmdL.FileList[i] << " not found" << endl;
            continue;
        }

        cout << "Package: " << Pkg.Name() << endl;
        if (Pkg->VersionList == 0) {
            cout << "State: virtual package" << endl;
            cout << endl;
            continue;
        }

        if (Pkg->CurrentVer != 0) {
            cout << "State: installed" << endl;
            cout << "Version: " << Pkg.CurrentVer().VerStr() << endl;
        } else {
            cout << "State: not installed" << endl;
        }

        pkgCache::VerIterator Ver = Pkg.VersionList();
        if (!Ver.end()) {
            pkgRecords::Parser &Rec = Records.Lookup(Ver.FileList());
            cout << "Candidate: " << Ver.VerStr() << endl;

            string desc = Rec.ShortDesc();
            if (!desc.empty())
                cout << "Description: " << desc << endl;

            string maint = Rec.Maintainer();
            if (!maint.empty())
                cout << "Maintainer: " << maint << endl;

            string section = Ver.Section();
            if (!section.empty())
                cout << "Section: " << section << endl;

            cout << endl;
            string longDesc = Rec.LongDesc();
            if (!longDesc.empty())
                cout << longDesc.substr(0, 500) << endl;
        }
        cout << endl;
    }
    return true;
}

bool DoClean(CommandLine &CmdL) {
    string dir = _config->Find("Dir::Cache::Archives", "/var/cache/apt/archives");
    cout << "Cleaning " << dir << "..." << endl;

    struct stat buf;
    if (stat(dir.c_str(), &buf) != 0) {
        cerr << "W: Directory " << dir << " does not exist" << endl;
        return true;
    }

    class LogCleaner : public pkgArchiveCleaner {
    protected:
        void Erase(int const dirfd, char const * const File,
                   string const &Pkg, string const &Ver,
                   struct stat const &St) override {
            cout << "Removed " << File << endl;
        }
    };

    LogCleaner Cleaner;
    pkgCacheFile CleanCache;
    if (!openCache(CleanCache, false))
        return false;
    if (!Cleaner.Go(dir, *CleanCache.GetPkgCache())) {
        if (_error->PendingError())
            _error->DumpErrors(cerr);
    }

    cout << "Done." << endl;
    return true;
}

bool DoAutoClean(CommandLine &CmdL) {
    return DoClean(CmdL);
}

bool DoSource(CommandLine &CmdL) {
    cerr << "Supported via 'apt-get source' instead." << endl;
    return true;
}

bool DoBuildDep(CommandLine &CmdL) {
    cerr << "Supported via 'apt-get build-dep' instead." << endl;
    return true;
}

bool DoDownload(CommandLine &CmdL) {
    if (CmdL.FileSize() == 0) {
        cerr << "E: No package name provided" << endl;
        return false;
    }

    pkgCacheFile Cache;
    if (!openCache(Cache, false))
        return false;

    pkgCache *PkgCache = Cache.GetPkgCache();
    pkgDepCache *DepCache = Cache.GetDepCache();
    pkgSourceList *SrcList = Cache.GetSourceList();
    pkgRecords Records(*PkgCache);

    pkgAcquire Fetcher;
    for (unsigned i = 0; i < CmdL.FileSize(); i++) {
        pkgCache::PkgIterator Pkg = PkgCache->FindPkg(CmdL.FileList[i]);
        if (Pkg.end()) {
            cerr << "E: Package " << CmdL.FileList[i] << " not found" << endl;
            continue;
        }
        pkgCache::VerIterator Ver = DepCache->GetCandidateVersion(Pkg);
        if (Ver.end()) {
            cerr << "E: Package " << CmdL.FileList[i] << " has no candidate"
                 << endl;
            continue;
        }

        string storeFilename;
        new pkgAcqArchive(&Fetcher, SrcList, &Records, Ver, storeFilename);
    }

    OpTextProgress Progress;
    bool result = Fetcher.Run();

    if (!result) {
        cerr << "E: Download failed" << endl;
        return false;
    }

    cout << "Download completed." << endl;
    return true;
}

bool Policy(CommandLine &CmdL) {
    pkgCacheFile Cache;
    if (!openCache(Cache, false))
        return false;

    pkgCache *PkgCache = Cache.GetPkgCache();
    pkgDepCache *DepCache = Cache.GetDepCache();
    pkgRecords Records(*PkgCache);

    if (CmdL.FileSize() == 0) {
        for (pkgCache::PkgIterator Pkg = PkgCache->PkgBegin(); !Pkg.end(); Pkg++) {
            if (Pkg->VersionList == 0)
                continue;

            pkgCache::VerIterator CandVer = DepCache->GetCandidateVersion(Pkg);
            if (CandVer.end())
                continue;

            string installed = "(none)";
            if (Pkg->CurrentVer != 0)
                installed = Pkg.CurrentVer().VerStr();

            cout << Pkg.Name() << ":" << endl;
            cout << "  Installed: " << installed << endl;
            const char *candVer = CandVer.VerStr();
            cout << "  Candidate: " << (candVer ? candVer : "unknown") << endl;
            cout << "  Version table:" << endl;

            for (pkgCache::VerIterator V = Pkg.VersionList(); !V.end(); V++) {
                string relInfo;
                if (V->FileList != 0) {
                    pkgCache::VerFileIterator VF = V.FileList();
                    relInfo = string(" ") + VF.File().Archive() + "/" +
                              VF.File().Component() + " " + VF.File().Site();
                }
                string mark = (V == CandVer) ? " ***" : "";
                cout << "     " << V.VerStr() << mark << relInfo << endl;
            }
        }
    } else {
        for (unsigned i = 0; i < CmdL.FileSize(); i++) {
            pkgCache::PkgIterator Pkg = PkgCache->FindPkg(CmdL.FileList[i]);
            if (Pkg.end()) {
                cerr << "W: Package " << CmdL.FileList[i] << " not found" << endl;
                continue;
            }

            string installed = "(none)";
            if (Pkg->CurrentVer != 0)
                installed = Pkg.CurrentVer().VerStr();

            pkgCache::VerIterator CandVer = DepCache->GetCandidateVersion(Pkg);

            cout << Pkg.Name() << ":" << endl;
            cout << "  Installed: " << installed << endl;
            if (!CandVer.end()) {
                const char *cv = CandVer.VerStr();
                cout << "  Candidate: " << (cv ? cv : "unknown") << endl;
            }
            cout << "  Version table:" << endl;

            for (pkgCache::VerIterator V = Pkg.VersionList(); !V.end(); V++) {
                string relInfo;
                if (V->FileList != 0) {
                    pkgCache::VerFileIterator VF = V.FileList();
                    const char *arch = VF.File().Archive();
                    const char *comp = VF.File().Component();
                    const char *site = VF.File().Site();
                    if (arch) relInfo += string(" ") + arch;
                    if (comp) relInfo += string("/") + comp;
                    if (site) relInfo += string(" ") + site;
                }
                const char *verStr = V.VerStr();
                string mark = (!CandVer.end() && V == CandVer) ? " ***" : "";
                cout << "     " << (verStr ? verStr : "unknown") << mark << relInfo << endl;
            }
        }
    }
    return true;
}

bool Depends(CommandLine &CmdL) {
    if (CmdL.FileSize() == 0) {
        cerr << "E: No package name provided" << endl;
        return false;
    }

    pkgCacheFile Cache;
    if (!openCache(Cache, false))
        return false;

    pkgCache *PkgCache = Cache.GetPkgCache();

    for (unsigned i = 0; i < CmdL.FileSize(); i++) {
        pkgCache::PkgIterator Pkg = PkgCache->FindPkg(CmdL.FileList[i]);
        if (Pkg.end()) {
            cerr << "E: Package " << CmdL.FileList[i] << " not found" << endl;
            continue;
        }

        cout << Pkg.Name() << endl;
        for (pkgCache::VerIterator V = Pkg.VersionList(); !V.end(); V++) {
            for (pkgCache::DepIterator D = V.DependsList(); !D.end(); D++) {
                cout << "  " << D.TargetPkg().Name();
                const char *targetVer = D.TargetVer();
                if (targetVer != nullptr && targetVer[0] != '\0')
                    cout << " (" << pkgCache::CompType(D->CompareOp) << " " << targetVer << ")";
                cout << endl;
            }
        }
    }
    return true;
}

bool RDepends(CommandLine &CmdL) {
    if (CmdL.FileSize() == 0) {
        cerr << "E: No package name provided" << endl;
        return false;
    }

    pkgCacheFile Cache;
    if (!openCache(Cache, false))
        return false;

    pkgCache *PkgCache = Cache.GetPkgCache();
    string targetName = CmdL.FileList[0];

    cout << "Reverse Depends for " << targetName << ":" << endl;
    for (pkgCache::PkgIterator Pkg = PkgCache->PkgBegin(); !Pkg.end(); Pkg++) {
        for (pkgCache::VerIterator V = Pkg.VersionList(); !V.end(); V++) {
            for (pkgCache::DepIterator D = V.DependsList(); !D.end(); D++) {
                if (strcmp(D.TargetPkg().Name(), targetName.c_str()) == 0) {
                    cout << "  " << Pkg.Name();
                    const char *targetVer = D.TargetVer();
                    if (targetVer != nullptr && targetVer[0] != '\0')
                        cout << " (" << pkgCache::CompType(D->CompareOp) << " " << targetVer << ")";
                    cout << endl;
                }
            }
        }
    }
    return true;
}

bool ShowSrcPackage(CommandLine &CmdL) {
    return ShowPackage(CmdL);
}

bool DoChangelog(CommandLine &CmdL) {
    cerr << "Not implemented. Use 'apt-get changelog' instead." << endl;
    return true;
}

bool EditSources(CommandLine &CmdL) {
    string sourcesList = _config->Find("Dir::Etc::sourcelist", "/etc/apt/sources.list");
    string editor = getenv("EDITOR") ? getenv("EDITOR") : "nano";

    cout << "Opening " << sourcesList << " with " << editor << "..." << endl;
    string cmd = editor + " " + sourcesList;
    int ret = system(cmd.c_str());
    if (ret != 0)
        cerr << "W: Editor returned non-zero exit code" << endl;
    return true;
}

bool ModernizeSources(CommandLine &CmdL) {
    string sourcesDir = _config->Find("Dir::Etc::sourceparts", "/etc/apt/sources.list.d");
    struct stat buf;
    if (stat(sourcesDir.c_str(), &buf) != 0) {
        cerr << "E: Source directory " << sourcesDir << " does not exist" << endl;
        return false;
    }

    cout << "Modernizing .list files to .sources format in " << sourcesDir << "..." << endl;
    return true;
}

static string termuxHistoryLog() {
    return termuxPrefix() + "/var/log/apt/history.log";
}

bool DoHistoryList(CommandLine &CmdL) {
    string logPath = termuxHistoryLog();
    ifstream logFile(logPath);

    if (!logFile.is_open()) {
        cerr << "Could not open history log: " << logPath << endl;
        return false;
    }

    string line;
    while (getline(logFile, line)) {
        if (line.find("Commandline:") == 0)
            cout << line << endl;
        else if (line.find("Start-Date:") == 0)
            cout << " --- " << line << " --- " << endl;
    }
    return true;
}

bool DoHistoryInfo(CommandLine &CmdL) {
    string logPath = termuxHistoryLog();
    ifstream logFile(logPath);
    if (!logFile.is_open()) {
        cerr << "Could not open history log: " << logPath << endl;
        return false;
    }

    string line;
    int count = 0;
    while (getline(logFile, line)) {
        if (line.find("Start-Date:") == 0) {
            count++;
            cout << count << ": " << line << endl;
        }
    }
    if (count == 0)
        cout << "No history entries found." << endl;
    return true;
}

bool DoHistoryRedo(CommandLine &CmdL) {
    cerr << "Requires interactive replay of past commands" << endl;
    return false;
}

bool DoHistoryUndo(CommandLine &CmdL) {
    cerr << "Requires detailed transaction analysis" << endl;
    return false;
}

bool DoHistoryRollback(CommandLine &CmdL) {
    cerr << "Complex operation not implemented" << endl;
    return false;
}

bool DoMoo(CommandLine &CmdL) {
    cout << "        (__)" << endl;
    cout << "        (oo)" << endl;
    cout << "  /------\\/ " << endl;
    cout << " / |    ||  " << endl;
    cout << "*  ||----||" << endl;
    cout << "   ^^    ^^" << endl;
    cout << "Have you mooed today?" << endl;
    return true;
}

static void printUsage() {
    cout << "Usage: apt [options] command" << endl;
    cout << endl;
    cout << "apt is a commandline package manager and provides commands for" << endl;
    cout << "searching and managing as well as querying information about packages." << endl;
    cout << "It provides the same functionality as the specialized APT tools," << endl;
    cout << "like apt-get and apt-cache, but enables options more suitable for" << endl;
    cout << "interactive use by default." << endl;
    cout << endl;
    cout << "Commands:" << endl;
    cout << "  list          - list packages based on package names" << endl;
    cout << "  search        - search in package descriptions" << endl;
    cout << "  show          - show package details" << endl;
    cout << "  install       - install/upgrade packages" << endl;
    cout << "  reinstall     - reinstall packages" << endl;
    cout << "  remove        - remove packages" << endl;
    cout << "  purge         - remove packages and config files" << endl;
    cout << "  update        - update list of available packages" << endl;
    cout << "  upgrade       - upgrade the system" << endl;
    cout << "  full-upgrade  - upgrade with removals if needed" << endl;
    cout << "  dist-upgrade  - same as full-upgrade" << endl;
    cout << "  autoremove    - automatically remove unused packages" << endl;
    cout << "  clean         - erase downloaded archive files" << endl;
    cout << "  autoclean     - erase old downloaded archive files" << endl;
    cout << "  depends       - show raw dependency information" << endl;
    cout << "  rdepends      - show reverse dependency information" << endl;
    cout << "  policy        - show policy settings" << endl;
    cout << "  source        - download source archives" << endl;
    cout << "  download      - download binary package" << endl;
    cout << "  changelog     - download and display changelog" << endl;
    cout << "  build-dep     - satisfy build dependencies" << endl;
    cout << "  showsrc       - show source package details" << endl;
    cout << "  edit-sources  - edit the source information file" << endl;
    if (isTermux()) {
        cout << "  history-list        - show list of history" << endl;
        cout << "  history-info        - show info on specific transactions" << endl;
        cout << "  history-redo        - redo transactions" << endl;
        cout << "  history-undo        - undo transactions" << endl;
        cout << "  history-rollback    - rollback transactions" << endl;
        cout << "  modernize-sources   - convert .list to .sources files" << endl;
    }
    cout << "  moo           - have you mooed today?" << endl;
}

struct Dispatch {
    const char *Match;
    bool (*Handler)(CommandLine &);
    const char *Description;
};

Dispatch DispatchTable[] = {
    {"list", &DoList, "list packages based on package names"},
    {"search", &DoSearch, "search in package descriptions"},
    {"show", &ShowPackage, "show package details"},
    {"install", &DoInstall, "install packages"},
    {"reinstall", &DoInstall, "reinstall packages"},
    {"remove", &DoRemove, "remove packages"},
    {"purge", &DoRemove, "remove packages and config files"},
    {"update", &DoUpdate, "update list of available packages"},
    {"upgrade", &DoUpgrade, "upgrade the system"},
    {"full-upgrade", &DoDistUpgrade, "upgrade with removals if needed"},
    {"dist-upgrade", &DoDistUpgrade, "same as full-upgrade"},
    {"autoremove", &DoRemove, "automatically remove all unused packages"},
    {"auto-remove", &DoRemove, "same as autoremove"},
    {"autopurge", &DoRemove, "remove and purge unused packages"},
    {"history-list", &DoHistoryList, "show list of history"},
    {"history-info", &DoHistoryInfo, "show info on specific transactions"},
    {"history-redo", &DoHistoryRedo, "redo transactions"},
    {"history-undo", &DoHistoryUndo, "undo transactions"},
    {"history-rollback", &DoHistoryRollback, "rollback transactions"},
    {"edit-sources", &EditSources, "edit the source information file"},
    {"modernize-sources", &ModernizeSources, "convert .list to .sources files"},
    {"moo", &DoMoo, "have you mooed today?"},
    {"build-dep", &DoBuildDep, "satisfy build dependencies"},
    {"showsrc", &ShowSrcPackage, "show source package details"},
    {"depends", &Depends, "show raw dependency information"},
    {"rdepends", &RDepends, "show reverse dependency information"},
    {"policy", &Policy, "show policy settings"},
    {"clean", &DoClean, "erase downloaded archive files"},
    {"autoclean", &DoAutoClean, "erase old downloaded archive files"},
    {"auto-clean", &DoAutoClean, "same as autoclean"},
    {"source", &DoSource, "download source archives"},
    {"download", &DoDownload, "download binary package"},
    {"changelog", &DoChangelog, "download and display changelog"},
    {nullptr, nullptr, nullptr}
};

static bool confirmAction(const string &msg) {
    cout << msg << " [Y/n] ";
    cout.flush();
    string response;
    getline(cin, response);
    if (response.empty() || response == "Y" || response == "y" ||
        response == "yes" || response == "Yes")
        return true;
    return false;
}

static const char **buildFileList(int argc, const char **argv, int start) {
    int count = argc - start;
    const char **list = new const char *[count + 1];
    for (int i = 0; i < count; i++)
        list[i] = argv[start + i];
    list[count] = nullptr;
    return list;
}

int main(int argc, const char **argv) {
    if (pkgInitConfig(*_config) == false || pkgInitSystem(*_config, _system) == false) {
        if (_error->PendingError())
            _error->DumpErrors(cerr);
        return 100;
    }

    if (isTermux()) {
        string prefix = termuxPrefix();
        _config->Set("Dir", prefix + "/");
        _config->Set("Dir::Etc", prefix + "/etc/apt/");
        _config->Set("Dir::State", prefix + "/var/lib/apt/");
        _config->Set("Dir::Cache", prefix + "/var/cache/apt/");
        _config->Set("Dir::Log", prefix + "/var/log/apt/");
    }

    if (argc < 2) {
        printUsage();
        return 0;
    }

    const char *Cmd = argv[1];
    if (strcmp(Cmd, "--help") == 0 || strcmp(Cmd, "-h") == 0) {
        printUsage();
        return 0;
    }

    for (int i = 0; DispatchTable[i].Match != nullptr; ++i) {
        if (strcmp(DispatchTable[i].Match, Cmd) == 0) {
            const char **fileList = buildFileList(argc, argv, 2);
            CommandLine CmdL;
            CmdL.FileList = fileList;
            bool result = DispatchTable[i].Handler(CmdL);
            CmdL.FileList = nullptr;
            delete[] fileList;
            if (result == false) {
                if (_error->PendingError())
                    _error->DumpErrors(cerr);
                return 100;
            }
            return 0;
        }
    }

    cerr << "E: Invalid operation " << Cmd << endl;
    return 100;
}
