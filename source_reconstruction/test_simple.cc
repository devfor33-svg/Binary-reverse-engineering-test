#include <apt-pkg/init.h>
#include <apt-pkg/configuration.h>
#include <apt-pkg/pkgsystem.h>
#include <apt-pkg/error.h>
#include <iostream>
using namespace std;
int main() {
    cerr << "Before pkgInitConfig" << endl;
    cerr << "_config ptr: " << _config << endl;
    if (pkgInitConfig(*_config) == false) {
        cerr << "pkgInitConfig failed" << endl;
        if (_error->PendingError()) _error->DumpErrors(cerr);
        return 1;
    }
    cerr << "After pkgInitConfig" << endl;
    pkgSystem *sys = _system;
    cerr << "_system ptr: " << (void*)sys << endl;
    if (pkgInitSystem(*_config, sys) == false) {
        cerr << "pkgInitSystem failed" << endl;
        if (_error->PendingError()) _error->DumpErrors(cerr);
        return 1;
    }
    cerr << "After pkgInitSystem" << endl;
    cout << "OK" << endl;
    return 0;
}
