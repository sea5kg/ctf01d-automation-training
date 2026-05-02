/* MIT License

* Copyright (c) 2015-2026 Evgenii Sopov

* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:

* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.

* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

// https://github.com/sea5kg/ctf01d-automation-training

#include "employ_flags.h"
#include "employ_database.h"
#include "iemploy_config.h"
#include "iemploy_runner.h"
#include <wsjcpp_core.h>
#include <fstream>
#include <cstring>
#include <thread>
#include <algorithm>

// ---------------------------------------------------------------------
// EmployFlags

REGISTRY_WSJCPP_EMPLOY(EmployFlags)

EmployFlags::EmployFlags()
: WsjcppEmployBase({IEmployFlags::name()}, { IEmployConfig::name(), EmployDatabase::name() }) {
    TAG = EmployFlags::name();
    m_stop_thread = false;
}

bool EmployFlags::init(const std::string &sName, bool bSilent) {
  WsjcppLog::info(TAG, "init");

  // caching previously flag lives
  {
    std::lock_guard<std::mutex> lock(m_mutex_flag_lives);
    auto *dbs = findWsjcppEmploy<EmployDatabase>();
    m_flag_lives = dbs->dbFlags()->getFlagsNotExpired();

    WsjcppLog::info(TAG, "Flags Live Size (On start): " + std::to_string(m_flag_lives.size()));
  }
  return true;
}

bool EmployFlags::deinit(const std::string &sName, bool bSilent) {
    WsjcppLog::info(TAG, "deinit");
    return true;
}

bool EmployFlags::findFlagLive(const std::string &flagValue, Ctf01dFlag &flag) {
    std::lock_guard<std::mutex> lock(m_mutex_flag_lives);
    if (m_flag_lives.count(flagValue) > 0) {
        flag.copyFrom(m_flag_lives[flagValue]);
        return true;
    }
    return false;
}

void* newProcessThread_Flags(void *arg) {
	// Log::info("newRequest", "");
	EmployFlags *m_flags = (EmployFlags *)arg;
	pthread_detach(pthread_self());
	m_flags->runThreadSendFlags();
	return 0;
}

void EmployFlags::startThreadSendFlags() {
    m_stop_thread = false;
    pthread_create(&m_pProcessThread, NULL, &newProcessThread_Flags, (void *)this);
}

void EmployFlags::stopThreadSendFlags() {
    m_stop_thread = true;
}

void EmployFlags::runThreadSendFlags() {
    auto *config = findWsjcppEmploy<IEmployConfig>();
    auto *runner = findWsjcppEmploy<IEmployRunner>();

    while (!m_stop_thread) {
        std::chrono::time_point<std::chrono::system_clock> start, end;
        start = std::chrono::system_clock::now();

        Ctf01dFlag new_flag;

        new_flag.generateRandomFlag(config->flagLifeTimeInMin(), config->startTimeTrainingInSec());

        WsjcppLog::warn(TAG, "runThreadSendFlags - TODO: " + new_flag.getValue());

        // std::lock_guard<std::mutex> lock(m_mutex_flag_lives);

        // fill context
        CommandContext ctx;
        ctx.work_dir = config->getCheckerWorkDir();
        ctx.app_name = config->getCheckerScriptPath();
        ctx.timeout_ms = config->getCheckerScriptWaitInSec() * 1000;
        ctx.args.push_back("PUT");
        ctx.args.push_back(config->getCheckerTargetHost());
        ctx.args.push_back(new_flag.getId());
        ctx.args.push_back(new_flag.getValue());

        // run put flag
        runner->runCommand(ctx);
        // TODO process errors

        // TODO if success putted
        // run get flag
        ctx.args[0] = "GET";
        runner->runCommand(ctx);

        // caching previously flag lives
        {
          std::lock_guard<std::mutex> lock(m_mutex_flag_lives);
          auto *dbs = findWsjcppEmploy<EmployDatabase>();
          if (!dbs->dbFlags()->insertFlag(new_flag)) {
            WsjcppLog::err(TAG, "Some problems with insert flag to database [" + new_flag.getId() + ":" + new_flag.getValue() + "]");
          } else {
            m_flag_lives[new_flag.getValue()] = new_flag;
          }

          // cleanup outdated flags
          long currentTime = WsjcppCore::getCurrentTimeInMilliseconds();
          auto it = m_flag_lives.begin();
          while (it != m_flag_lives.end()) {
            Ctf01dFlag fl = it->second;
            if (fl.getTimeEndInMs() < currentTime) {
              // WsjcppLog::info(TAG, "Erase flag: fl.getTimeEndInMs() < currentTime [" + std::to_string(fl.getTimeEndInMs()) + " < " + std::to_string(currentTime) + "]");
              it = m_flag_lives.erase(it);
            } else {
              ++it;
            }
          }
        }
        WsjcppLog::info(TAG, "Flags Live Size: " + std::to_string(m_flag_lives.size()));

        // sleep
        end = std::chrono::system_clock::now();
        int elapsed_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();
        int ms_sleep = config->getCheckerScriptRoundInSec()*1000;
        WsjcppLog::info(TAG, "Elapsed milliseconds: " + std::to_string(elapsed_milliseconds) + "ms");
        std::this_thread::sleep_for(std::chrono::milliseconds(ms_sleep - elapsed_milliseconds));
    }
}
