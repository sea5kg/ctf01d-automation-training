
#include "employ_runner.h"
#include <fstream>
#include <cstring>

REGISTRY_WSJCPP_EMPLOY(EmployRunner)

// ---------------------------------------------------------------------
// EmployRunner

EmployRunner::EmployRunner()
: WsjcppEmployBase({EmployRunner::name()}, { "EmployConfig" }) {
    TAG = EmployRunner::name();
}

bool EmployRunner::init(const std::string &sName, bool bSilent) {
    // WsjcppLog::info(TAG, "init");
    return true;
}

bool EmployRunner::deinit(const std::string &sName, bool bSilent) {
    // WsjcppLog::info(TAG, "deinit");
    return true;
}


