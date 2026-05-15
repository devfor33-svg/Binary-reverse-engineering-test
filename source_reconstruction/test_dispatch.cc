#include <iostream>
#include <cstring>
#include <vector>

class CommandLine {
public:
    std::vector<const char*> FileList;
    unsigned int FileSize() const { return FileList.size(); }
};

bool DoUpdate(CommandLine &CmdL) {
    std::cout << "Action: update" << std::endl;
    return true;
}

bool DoInstall(CommandLine &CmdL) {
    std::cout << "Action: install";
    for (unsigned i = 0; i < CmdL.FileSize(); i++)
        std::cout << " " << CmdL.FileList[i];
    std::cout << std::endl;
    return true;
}

bool DoRemove(CommandLine &CmdL) {
    std::cout << "Action: remove";
    for (unsigned i = 0; i < CmdL.FileSize(); i++)
        std::cout << " " << CmdL.FileList[i];
    std::cout << std::endl;
    return true;
}

bool DoList(CommandLine &CmdL) {
    std::cout << "Action: list" << std::endl;
    return true;
}

bool DoSearch(CommandLine &CmdL) {
    std::cout << "Action: search";
    for (unsigned i = 0; i < CmdL.FileSize(); i++)
        std::cout << " " << CmdL.FileList[i];
    std::cout << std::endl;
    return true;
}

struct Dispatch {
    const char *Match;
    bool (*Handler)(CommandLine &);
};

Dispatch DispatchTable[] = {
    {"list", &DoList},
    {"search", &DoSearch},
    {"install", &DoInstall},
    {"reinstall", &DoInstall},
    {"remove", &DoRemove},
    {"purge", &DoRemove},
    {"update", &DoUpdate},
    {nullptr, nullptr}
};

int main(int argc, const char **argv) {
    if (argc < 2) {
        std::cout << "Usage: apt command [args]" << std::endl;
        return 0;
    }

    CommandLine CmdL;
    for (int i = 2; i < argc; i++)
        CmdL.FileList.push_back(argv[i]);

    const char *Cmd = argv[1];
    for (int i = 0; DispatchTable[i].Match != nullptr; ++i) {
        if (strcmp(DispatchTable[i].Match, Cmd) == 0)
            return DispatchTable[i].Handler(CmdL) ? 0 : 1;
    }

    std::cerr << "E: Invalid operation " << Cmd << std::endl;
    return 100;
}
