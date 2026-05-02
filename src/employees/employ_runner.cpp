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


#include "employ_runner.h"
#include "iemploy_config.h"
#include "wsjcpp_core.h"
#include <fstream>
#include <cstring>
#include <mutex>
#include <sstream>
#include <iostream>
#include <vector>
#include <signal.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <sys/wait.h>
#include <sysexits.h>
#include <cstring>

REGISTRY_WSJCPP_EMPLOY(EmployRunner)

// ---------------------------------------------------------------------
// LocalCommandRunner

class LocalCommandRunner {
    public:
        void run();
        bool hasError();
        int exitCode();
        bool isTimeout();
        bool isFinished();
        const std::string &outputString();
        void start(CommandContext &ctx);
    private:
        std::string TAG;
        std::string m_sDir;
        std::string m_sScript;
        std::string m_sIp;
        std::string m_sCommand;
        std::string m_sFlagId;
        std::string m_sFlag;
        std::vector<std::string> m_args;

        pid_t m_nPid;
        pthread_t m_pProcessThread;
        int m_nExitCode;
        bool m_has_error;
        bool m_bFinished;
        bool m_bFinishedByTimeout;
        std::string m_sOutput;
};

bool LocalCommandRunner::hasError() { return m_has_error; }

int LocalCommandRunner::exitCode() { return m_nExitCode; }

bool LocalCommandRunner::isTimeout() { return m_bFinishedByTimeout; }

bool LocalCommandRunner::isFinished() { return m_bFinished; }

const std::string &LocalCommandRunner::outputString() {
    return m_sOutput;
}

void* newProcessThread(void *arg) {
	// Log::info("newRequest", "");
	LocalCommandRunner *m_doRunChecker = (LocalCommandRunner *)arg;
	pthread_detach(pthread_self());
	m_doRunChecker->run();
	return 0;
}

void LocalCommandRunner::start(CommandContext &ctx) {
    // CommandContext &ctx

    m_sDir = ctx.work_dir;
    m_sScript = ctx.app_name;
    m_args = ctx.args;
    m_bFinished = false;
    m_bFinishedByTimeout = false;
    int nTimeWait = 0;
    int nSleepMS = 100;
    WsjcppLog::info(TAG, "m_sDir = : " + m_sDir);
    WsjcppLog::info(TAG, "m_sScript = : " + m_sScript);

    pthread_create(&m_pProcessThread, NULL, &newProcessThread, (void *)this);

    while (nTimeWait < ctx.timeout_ms && !m_bFinished) {
        // Log::info(TAG, "Wait: " + std::to_string(nTimeWait) + "ms");
        std::this_thread::sleep_for(std::chrono::milliseconds(nSleepMS));
        nTimeWait = nTimeWait + nSleepMS;
    }
    if (!m_bFinished) {
        m_has_error = true;
        m_bFinishedByTimeout = true;
        m_nExitCode = -1;
        m_sOutput = "timeout";
        // pthread_kill(m_pProcessThread, -9);

        if (m_nPid > 0) {
            kill(m_nPid, 9);
            // kill(m_nPid, 9);
        }
        pthread_cancel(m_pProcessThread);
    }
}

void LocalCommandRunner::run() {
    m_nExitCode = 1;
    m_sOutput = "";
    m_has_error = false;
    m_bFinishedByTimeout = false;
    m_bFinished = false;
    m_nPid = 0;

    int fd[2];

    if (pipe(fd) != 0) {
        m_sOutput = "Could not open pipe";
        WsjcppLog::err(TAG, m_sOutput);
        m_nExitCode = -1;
        m_has_error = true;
        m_bFinishedByTimeout = false;
        m_bFinished = true;
        return;
    }

    pid_t nChildPid = fork();

    if(nChildPid < 0) {
        m_sOutput = "fork failed!";
        WsjcppLog::err(TAG, m_sOutput);
        m_nExitCode = -1;
        m_has_error = true;
        m_bFinishedByTimeout = false;
        m_bFinished = true;
        return;
    } else if (nChildPid == 0) {
        // child process
        if (dup2(fd[1], STDOUT_FILENO) < 0) { // redirect from pipe to stdout
            perror("dup2");
            return;
        }
        if (dup2(STDOUT_FILENO, STDERR_FILENO) < 0) { // redirects stderr to stdout below this line.
            perror("dup2");
            return;
        }
        close(fd[0]);
        close(fd[1]);
        int res = chdir(m_sDir.c_str());
        if (res != 0) {
            printf("fork: FAILED change to work dir '%s'\n", m_sDir.c_str());
        }
        printf("fork: Child process id=%d\n", getpid());
        printf("fork: Change dir '%s'\n", m_sDir.c_str());
        // setpgid(nChildPid, nChildPid); //Needed so negative PIDs can kill children of /bin/sh
        std::vector<char*> argv_ptrs;
        argv_ptrs.push_back(const_cast<char*>(m_sScript.c_str()));
        for (const std::string& s : m_args) {
            argv_ptrs.push_back(const_cast<char*>(s.c_str()));
        }
        argv_ptrs.push_back(NULL);
        execvp(argv_ptrs[0], argv_ptrs.data());

        // execlp(
        //     m_sScript.c_str(), //
        //     m_sScript.c_str(), // first argument must be same like executable file
        //     m_sIp.c_str(), m_sCommand.c_str(), m_sFlagId.c_str(), m_sFlag.c_str(), (char *) 0
        // );
        perror("execl");
        exit(-1);
    }

    // parent process;
    // setpgid(nChildPid, ::getpid());
    close(fd[1]);
    int nPipeOut = fd[0];
    m_nPid = nChildPid;
    // Log::info(TAG, "PID: " + std::to_string(m_nPid));

    m_sOutput = "";
    const int nSizeBuffer = 4096;
    char buffer[nSizeBuffer];
    std::memset(&buffer, 0, nSizeBuffer);
    try {
        while (read(nPipeOut, buffer, nSizeBuffer-1) > 0) {
            m_sOutput += std::string(buffer);
            std::memset(&buffer, 0, nSizeBuffer);
        }
        int status;
        if (waitpid(m_nPid, &status, 0) == -1) {
            perror("waitpid() failed");
            exit(EXIT_FAILURE);
        }

        if (WIFEXITED(status)) {
            m_has_error = false;
            m_nExitCode = WEXITSTATUS(status);
        }
    } catch (std::bad_alloc& ba) {
        close(nPipeOut);
        m_has_error = true;
        m_nExitCode = -1;
        WsjcppLog::err("DoRunProcess", "bad alloc");
        return;
    }

    close(nPipeOut);

    // Log::info(TAG, "Process exit code: " + std::to_string(m_nExitCode));

    if (m_bFinishedByTimeout) {
        return;
    }

    if (m_nExitCode < 0) { // basic concept of errors
        m_has_error = true;
    }

    // look here https://shapeshed.com/unix-exit-codes/
    if (m_nExitCode == 1) { // Catchall for general errors
        m_has_error = true;
    }

    if (m_nExitCode == 2) { // Misuse of shell builtins (according to Bash documentation)
        m_has_error = true;
    }

    if (m_nExitCode == 126) { // Command invoked cannot execute
        m_has_error = true;
    }

    if (m_nExitCode == 127) { // “command not found”
        m_has_error = true;
    }

    if (m_nExitCode == 128) { // Invalid argument to exit
        m_has_error = true;
    }

    if (m_nExitCode > 128 && m_nExitCode < 140) { // Fatal error signal “n”
        // 130 - Script terminated by Control-C
        m_has_error = true;
    }

    if (m_has_error) {
        m_nExitCode = -1;
        // Log::err(TAG, m_sOutput);
    } else {
        // Log::info(TAG, "DEBUG output: " + m_sOutput);
    }
    m_bFinished = true;
    // Log::info(TAG, "Finished");
}

// ---------------------------------------------------------------------
// EmployRunner

EmployRunner::EmployRunner()
: WsjcppEmployBase({IEmployRunner::name()}, { IEmployConfig::name() }) {
    TAG = "EmployRunner";
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

void EmployRunner::runCommand(CommandContext &ctx) {
    LocalCommandRunner *runner = new LocalCommandRunner();
    runner->start(ctx);

    // TODO return result
}

