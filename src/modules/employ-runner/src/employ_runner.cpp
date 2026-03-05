
#include "employ_runner.h"
#include "iemploy_config.h"
#include "wsjcpp_core.h"
#include <fstream>
#include <cstring>

REGISTRY_WSJCPP_EMPLOY(EmployRunner)

// ---------------------------------------------------------------------
// EmployRunner

EmployRunner::EmployRunner()
: WsjcppEmployBase({EmployRunner::name()}, { IEmployConfig::name() }) {
    TAG = EmployRunner::name();
}

bool EmployRunner::init(const std::string &sName, bool bSilent) {
    auto p = findWsjcppEmploy<IEmployConfig>();
    std::cout << "p->getLogDir(): "<< p->getLogDir() << std::endl;
    WsjcppLog::info(TAG, "init");
    return true;
}

bool EmployRunner::deinit(const std::string &sName, bool bSilent) {
    // WsjcppLog::info(TAG, "deinit");
    return true;
}


