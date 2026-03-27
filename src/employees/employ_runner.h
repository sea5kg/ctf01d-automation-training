#pragma once

#include <wsjcpp_employees.h>
#include <string>
#include <mutex>
#include <fstream>


struct CommandContext {
    std::string &work_dir;
    std::string &script_name;
    std::vector<std::string> &args;
    int timeout_ms; // infinity
};

class EmployRunner : public WsjcppEmployBase {
    public:
        EmployRunner();
        static std::string name() { return "EmployRunner"; }
        virtual bool init(const std::string &sName, bool bSilent) override;
        virtual bool deinit(const std::string &sName, bool bSilent) override;

        void runCommand(CommandContext &ctx);

    private:
        std::string TAG;
};
