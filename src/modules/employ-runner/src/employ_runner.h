#pragma once

#include <wsjcpp_employees.h>
#include <string>
#include <mutex>
#include <fstream>

class EmployRunner : public WsjcppEmployBase {
    public:
        EmployRunner();
        static std::string name() { return "EmployRunner"; }
        virtual bool init(const std::string &sName, bool bSilent) override;
        virtual bool deinit(const std::string &sName, bool bSilent) override;

    private:
        std::string TAG;
};
