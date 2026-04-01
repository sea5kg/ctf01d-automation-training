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
#include "iemploy_config.h"
#include "iemploy_runner.h"
#include <wsjcpp_core.h>
#include <fstream>
#include <cstring>
#include <thread>

// ---------------------------------------------------------------------
// Ctf01dFlag
Ctf01dFlag::Ctf01dFlag() {
    // flag id
    m_sId = "qweRT12345";
    // flag format
    m_sValue = "c01d0000-0000-0000-0000-000000000000";
    m_nTimeStartInMs = 0;
    m_nTimeEndInMs = 0;
}

// ---------------------------------------------------------------------

void Ctf01dFlag::generateRandomFlag(int nTimeFlagLifeInMin, int nGameStartUTCInSec) {
    // __int64
    long nTimeStartInMs = WsjcppCore::getCurrentTimeInMilliseconds();
    long nTimeEndInMs = nTimeStartInMs + nTimeFlagLifeInMin*60*1000;
    setTimeStartInMs(nTimeStartInMs);
    setTimeEndInMs(nTimeEndInMs);

    generateId();
    generateValue(nGameStartUTCInSec);
}

// ---------------------------------------------------------------------

void Ctf01dFlag::generateId() {
    static const std::string sAlphabet =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";

    std::string sFlagId = "";
    int nLenId = m_sId.size();
    for (int i = 0; i < 10; ++i) {
        m_sId[i] = sAlphabet[rand() % sAlphabet.length()];
    }
}

// ---------------------------------------------------------------------

void Ctf01dFlag::setId(const std::string &sId) {
    m_sId = sId;
}

// ---------------------------------------------------------------------

std::string Ctf01dFlag::getId() const {
    return m_sId;
}

// ---------------------------------------------------------------------

void Ctf01dFlag::generateValue(int nGameStartUTCInSec) {
    // TODO redesign more freeble format
    static const std::string sAlphabet = "0123456789abcdef";
    char sUuid[37];
    memset(&sUuid, '\0', 37);
    sUuid[8] = '-';
    sUuid[13] = '-';
    sUuid[18] = '-';
    sUuid[23] = '-';

    for(int i = 4; i < 28; i++){
        if (i != 8 && i != 13 && i != 18 && i != 23) {
            m_sValue[i] = sAlphabet[rand() % sAlphabet.length()];
        }
    }

    // set timepoint
    int dt = m_nTimeStartInMs / 1000 - nGameStartUTCInSec;
    std::string sTimePoint = std::to_string(dt);
    int nTimePointLen = sTimePoint.size();
    if (nTimePointLen > 8) {
        WsjcppLog::throw_err("Ctf01dFlag::generateValue", "Really game was started more then 3 years ago ??? got value: " + sTimePoint);
    }
    int nPos = m_sValue.size() - 1;
    for (int i = nTimePointLen - 1; i >= 0; i--) {
        m_sValue[nPos] = sTimePoint[i];
        nPos--;
    }
    // 03268167

    // std::cout << "sTimePoint: " << sTimePoint << "\n";
    // this->setValue(std::string(sUuid) + sTimePoint);
}

// ---------------------------------------------------------------------

void Ctf01dFlag::setValue(const std::string &sValue) {
    // TODO validate format
    // c01d...00000000 - prefix and time
    m_sValue = sValue;
}

// ---------------------------------------------------------------------

std::string Ctf01dFlag::getValue() const {
    return m_sValue;
}

// ---------------------------------------------------------------------

void Ctf01dFlag::setTimeStartInMs(long nTimeStartInMs) {
    m_nTimeStartInMs = nTimeStartInMs;
}

// ---------------------------------------------------------------------

long Ctf01dFlag::getTimeStartInMs() const {
    return m_nTimeStartInMs;
}

// ---------------------------------------------------------------------

void Ctf01dFlag::setTimeEndInMs(long nTimeEndInMs) {
    m_nTimeEndInMs = nTimeEndInMs;
}

// ---------------------------------------------------------------------

long Ctf01dFlag::getTimeEndInMs() const {
    return m_nTimeEndInMs;
}

// ---------------------------------------------------------------------

void Ctf01dFlag::copyFrom(const Ctf01dFlag &flag) {
    this->setId(flag.getId());
    this->setValue(flag.getValue());
    this->setTimeStartInMs(flag.getTimeStartInMs());
    this->setTimeEndInMs(flag.getTimeEndInMs());
}

// ---------------------------------------------------------------------
// EmployFlags

REGISTRY_WSJCPP_EMPLOY(EmployFlags)

EmployFlags::EmployFlags()
: WsjcppEmployBase({IEmployFlags::name()}, { IEmployConfig::name() }) {
    TAG = EmployFlags::name();
    m_stop_thread = false;
}

bool EmployFlags::init(const std::string &sName, bool bSilent) {
    WsjcppLog::info(TAG, "init");
    return true;
}

bool EmployFlags::deinit(const std::string &sName, bool bSilent) {
    WsjcppLog::info(TAG, "deinit");
    return true;
}

bool EmployFlags::findFlagLive(const std::string &flagValue, Ctf01dFlag &flag) {
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
        // run gte flag
        ctx.args[0] = "GET";
        runner->runCommand(ctx);

        // TODO recalculate 
        int timesleep_ms = config->getCheckerScriptTimeSleepBetweenRunInSec()*1000;

        std::this_thread::sleep_for(std::chrono::milliseconds(timesleep_ms));
    }
}
