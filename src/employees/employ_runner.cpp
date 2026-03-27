
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
        void start(int nTimeoutMS);
    private:
        std::string TAG;
        std::string m_sDir;
        std::string m_sScript;
        std::string m_sIp;
        std::string m_sCommand;
        std::string m_sFlagId;
        std::string m_sFlag;
        int m_nTimeoutMS;
        pid_t m_nPid;
        pthread_t m_pProcessThread;
        int m_nExitCode;
        bool m_bHasError;
        bool m_bFinished;
        bool m_bFinishedByTimeout;
        std::string m_sOutput;
};

bool LocalCommandRunner::hasError() { return m_bHasError; }

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

void LocalCommandRunner::start(int nTimeoutMS) {
    m_bFinished = false;
    m_bFinishedByTimeout = false;
    m_nTimeoutMS = nTimeoutMS;
    int nTimeWait = 0;
    int nSleepMS = 100;

    pthread_create(&m_pProcessThread, NULL, &newProcessThread, (void *)this);

    while (nTimeWait < m_nTimeoutMS && !m_bFinished) {
        // Log::info(TAG, "Wait: " + std::to_string(nTimeWait) + "ms");
        std::this_thread::sleep_for(std::chrono::milliseconds(nSleepMS));
        nTimeWait = nTimeWait + nSleepMS;
    }
    if (!m_bFinished) {
        m_bHasError = true;
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
    m_bHasError = false;
    m_bFinishedByTimeout = false;
    m_bFinished = false;
    m_nPid = 0;

    int fd[2];

    if (pipe(fd) != 0) {
        m_sOutput = "Could not open pipe";
        WsjcppLog::err(TAG, m_sOutput);
        m_nExitCode = -1;
        m_bHasError = true;
        m_bFinishedByTimeout = false;
        m_bFinished = true;
        return;
    }

    pid_t nChildPid = fork();

    if(nChildPid < 0) {
        m_sOutput = "fork failed!";
        WsjcppLog::err(TAG, m_sOutput);
        m_nExitCode = -1;
        m_bHasError = true;
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
        chdir(m_sDir.c_str());
        printf("fork: Child process id=%d\n", getpid());
        printf("fork: Change dir '%s'\n", m_sDir.c_str());
        // setpgid(nChildPid, nChildPid); //Needed so negative PIDs can kill children of /bin/sh
        execlp(
            m_sScript.c_str(), //
            m_sScript.c_str(), // first argument must be same like executable file
            m_sIp.c_str(), m_sCommand.c_str(), m_sFlagId.c_str(), m_sFlag.c_str(), (char *) 0
        );
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
            m_bHasError = false;
            m_nExitCode = WEXITSTATUS(status);
        }
    } catch (std::bad_alloc& ba) {
        close(nPipeOut);
        m_bHasError = true;
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
        m_bHasError = true;
    }

    // look here https://shapeshed.com/unix-exit-codes/
    if (m_nExitCode == 1) { // Catchall for general errors
        m_bHasError = true;
    }

    if (m_nExitCode == 2) { // Misuse of shell builtins (according to Bash documentation)
        m_bHasError = true;
    }

    if (m_nExitCode == 126) { // Command invoked cannot execute
        m_bHasError = true;
    }

    if (m_nExitCode == 127) { // “command not found”
        m_bHasError = true;
    }

    if (m_nExitCode == 128) { // Invalid argument to exit
        m_bHasError = true;
    }

    if (m_nExitCode > 128 && m_nExitCode < 140) { // Fatal error signal “n”
        // 130 - Script terminated by Control-C
        m_bHasError = true;
    }

    if (m_bHasError) {
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

void EmployRunner::runCommand(CommandContext &ctx) {
    LocalCommandRunner *runner = new LocalCommandRunner();
    runner->start(ctx.timeout_ms);

    // TODO return result
}
