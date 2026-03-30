/**********************************************************************************
 *          Project
 *  _______ _________ _______  _______  __    ______
 * (  ____ \\__   __/(  ____ \(  __   )/  \  (  __  \
 * | (    \/   ) (   | (    \/| (  )  |\/) ) | (  \  )
 * | |         | |   | (__    | | /   |  | | | |   ) |
 * | |         | |   |  __)   | (/ /) |  | | | |   | |
 * | |         | |   | (      |   / | |  | | | |   ) |
 * | (____/\   | |   | )      |  (__) |__) (_| (__/  )
 * (_______/   )_(   |/       (_______)\____/(______/
 *
 * MIT License
 * Copyright (c) 2018-2025 Evgenii Sopov
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 ***********************************************************************************/

#ifndef EMPLOY_FLAGS_H
#define EMPLOY_FLAGS_H

#include <wsjcpp_employees.h>
#include "iemploy_flags.h"
#include <string>
#include <mutex>
#include <fstream>
#include <atomic>

class EmployFlags : public WsjcppEmployBase, public IEmployFlags {
    public:
        EmployFlags();
        virtual bool init(const std::string &sName, bool bSilent) override;
        virtual bool deinit(const std::string &sName, bool bSilent) override;

        // IEmployFlags
        virtual bool findFlagLive(const std::string &flagValue, Ctf01dFlag &flag) override;
        virtual void startThreadSendFlags() override;
        virtual void stopThreadSendFlags() override;

        void runThreadSendFlags();

    private:
        std::string TAG;
        pthread_t m_pProcessThread;
        std::atomic_bool m_stop_thread = false;
        std::mutex m_mutex_flag_lives;
};

#endif // EMPLOY_FLAGS_H
