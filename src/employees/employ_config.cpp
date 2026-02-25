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

// https://github.com/sea5kg/gtree

#include "employ_config.h"
#include <wsjcpp_core.h>
#include <sstream>
#include <ctime>
#include <locale>
// #include <date.h>
#include <iostream>
#include <sstream>
#include <wsjcpp_core.h>
#include <wsjcpp_yaml.h>
#include <sys/stat.h>
#include <stdio.h>

// ---------------------------------------------------------------------
// EmployConfig

REGISTRY_WJSCPP_SERVICE_LOCATOR(EmployConfig)

EmployConfig::EmployConfig()
: WsjcppEmployBase({EmployConfig::name()}, {}) {
    TAG = EmployConfig::name();
}

EmployConfig::~EmployConfig() {

}

bool EmployConfig::init(const std::string &sName, bool bSilent) {

    std::vector<std::string> vPossibleFolders;
    std::string sWorkDirFromEnv = "";
    tryLoadFromEnv("WORKDIR", sWorkDirFromEnv, "Work Directory from enviroment");
    if (sWorkDirFromEnv != "") {
        vPossibleFolders.push_back(sWorkDirFromEnv);
    }
    vPossibleFolders.push_back("./data");
    vPossibleFolders.push_back("/root/data/");
    for (int i = 0; i < vPossibleFolders.size(); i++) {
        std::string sWorkDir = vPossibleFolders[i];
        if (sWorkDir[0] != '/') {
            sWorkDir = WsjcppCore::getCurrentDirectory() + "/" + sWorkDir;
        }
        sWorkDir = WsjcppCore::doNormalizePath(sWorkDir);
        if (WsjcppCore::fileExists(sWorkDir + "/config.yml")) {
            std::cout << "Detected workdir: " << sWorkDir << std::endl;
            m_sWorkDir = sWorkDir;
            break;
        }
    }
    WsjcppLog::info(TAG, "Work Directory is " + m_sWorkDir);
    std::string sWorkDir = this->getWorkDir();
    if (!WsjcppCore::fileExists(sWorkDir + "/config.yml")) {
        WsjcppLog::err(TAG, "Config file is mist in directory " + sWorkDir);
        return false;
    }

    WsjcppYaml yamlConfig;
    std::string sError;
    std::string sConfigFile = this->getWorkDir() + "/config.yml";
    if (!yamlConfig.loadFromFile(sConfigFile, sError)) {
        WsjcppLog::err(TAG, "Could not parse " + sConfigFile + ", error: " + sError);
        return false;
    }

    if (!initLogging(yamlConfig)) {
        return false;
    }


  m_nWebPort = yamlConfig["web-port"].valInt();
  if (m_nWebPort == 0) {
    m_nWebPort = 10555;
  }

  m_sDatabaseDir = handleRelatedDirPath(yamlConfig["database-dir"].valStr(), "/dbs");
  if (!WsjcppCore::dirExists(m_sDatabaseDir)) {
    WsjcppCore::makeDir(m_sDatabaseDir);
  }
  if (!WsjcppCore::dirExists(m_sLogDir)) {
    WsjcppLog::err(TAG, "Error: Folder '" + m_sLogDir + "' does not exists and could not created, please check access rights to parent folder.");
    return false;
  }

  m_sWebDir = handleRelatedDirPath(yamlConfig["web-dir"].valStr(), "/html");
  if (!WsjcppCore::dirExists(m_sWebDir)) {
    WsjcppLog::err(TAG, "Error: Folder '" + m_sWebDir + "' does not exists and could not created, please check access rights to parent folder.");
    return false;
  }

  // TODO
  // this->doExtractFilesIfNotExists();

  return true;
}

bool EmployConfig::deinit(const std::string &sName, bool bSilent) {
    WsjcppLog::info(TAG, "deinit");
    return true;
}

const std::string &EmployConfig::getWorkDir() {
    return m_sWorkDir;
}

int EmployConfig::getWebPort() {
    return m_nWebPort;
}

const std::string &EmployConfig::getDatabaseDir() {
    return m_sDatabaseDir;
}

const std::string &EmployConfig::getLogDir() {
    return m_sLogDir;
}

const std::string &EmployConfig::getWebDir() {
  return m_sWebDir;
}

void EmployConfig::doExtractFilesIfNotExists() {
    // TODO
    if (!WsjcppCore::dirExists(m_sWorkDir + "/logs")) {
        WsjcppCore::makeDir(m_sWorkDir + "/logs");
    }

    if (!WsjcppCore::fileExists(m_sWorkDir + "/config.yml")) {
        WsjcppLog::warn(TAG, "Extracting config.yml and files");
        WsjcppLog::warn(TAG, "Extracting checker_example_*");
        const std::vector<WsjcppResourceFile*> &vFiles = WsjcppResourcesManager::list();
        std::vector<std::string> vExecutableFiles;
        for (int i = 0; i < vFiles.size(); i++) {
            std::string sFilepath = vFiles[i]->getFilename();
            if (sFilepath.rfind("./data_sample/checker_example_", 0) == 0) {
                std::vector<std::string> vPath = WsjcppCore::split(sFilepath, "/");
                std::string sDirname = vPath[2];
                vPath.erase (vPath.begin(),vPath.begin()+3);
                std::string sNewFilepath = WsjcppCore::join(vPath, "/");
                sNewFilepath = WsjcppCore::doNormalizePath(m_sWorkDir + "/" + sDirname + "/" + sNewFilepath);
                if (!WsjcppCore::fileExists(sNewFilepath)) {
                    std::cout << "Extracting file '" << sFilepath << "' to '" << sNewFilepath << "'" << std::endl;
                } else {
                    std::cout << "File '" << sNewFilepath << "' already exists. Skip." << std::endl;
                    continue;
                }

                // prepare folder
                std::string sFolder = WsjcppCore::doNormalizePath(m_sWorkDir + "/" + sDirname + "/");
                if (!WsjcppCore::dirExists(sFolder)) {
                    WsjcppCore::makeDir(sFolder);
                }

                if (!WsjcppCore::writeFile(sNewFilepath, vFiles[i]->getBuffer(), vFiles[i]->getBufferSize())) {
                    std::cout << "ERROR. Could not write file. " << std::endl;
                    continue;
                } else {
                    std::cout << "Successfully created file. " << std::endl;
                    if (chmod(sNewFilepath.c_str(), S_IRWXU|S_IRWXG) != 0) {
                        std::cout << "ERROR. Could not change permissions for. " << std::endl;
                    } else {
                        struct stat info;
                        stat(sNewFilepath.c_str(), &info);
                        printf("after chmod(), permissions are: %08x\n", info.st_mode);
                    }
                }
            }
        }

        WsjcppResourceFile* pConfigYml = WsjcppResourcesManager::get("./data_sample/config.yml");
        std::string sNewFilepath = WsjcppCore::doNormalizePath(m_sWorkDir + "/config.yml");
        if (!WsjcppCore::writeFile(sNewFilepath, pConfigYml->getBuffer(), pConfigYml->getBufferSize())) {
            std::cout << "ERROR. Could not write file. " << std::endl;
        } else {
            std::cout << "Successfully created file. " << std::endl;
        }
        // TODO extract yaml and example services files
    }

    if (!WsjcppCore::fileExists(m_sWorkDir + "/html/index.html")) {
        if (!WsjcppCore::dirExists(m_sWorkDir + "/html")) {
            WsjcppCore::makeDir(m_sWorkDir + "/html");
        }

        WsjcppLog::warn(TAG, "Extracting html/index.html and files");
        const std::vector<WsjcppResourceFile*> &vFiles = WsjcppResourcesManager::list();
        for (int i = 0; i < vFiles.size(); i++) {
            std::string sFilepath = vFiles[i]->getFilename();
            if (sFilepath.rfind("./data_sample/html/", 0) == 0) {
                std::vector<std::string> vPath = WsjcppCore::split(sFilepath, "/");
                vPath.erase (vPath.begin(),vPath.begin()+3);
                std::string sNewFilepath = WsjcppCore::join(vPath, "/");
                sNewFilepath = WsjcppCore::doNormalizePath(m_sWorkDir + "/html/" + sNewFilepath);
                if (!WsjcppCore::fileExists(sNewFilepath)) {
                    std::cout << "Extracting file '" << sFilepath << "' to '" << sNewFilepath << "'" << std::endl;
                } else {
                    std::cout << "File '" << sNewFilepath << "' already exists. Skip." << std::endl;
                    continue;
                }

                // prepare folders
                std::string sFolder = WsjcppCore::doNormalizePath(m_sWorkDir + "/html/");
                for (int p = 0; p < vPath.size()-1; p++) {
                    sFolder = WsjcppCore::doNormalizePath(sFolder + "/" + vPath[p]);
                    if (!WsjcppCore::dirExists(sFolder)) {
                        WsjcppCore::makeDir(sFolder);
                    }
                }

                if (!WsjcppCore::writeFile(sNewFilepath, vFiles[i]->getBuffer(), vFiles[i]->getBufferSize())) {
                    std::cout << "ERROR. Could not write file. " << std::endl;
                    continue;
                } else {
                    std::cout << "Successfully created file. " << std::endl;
                }
            }
        }
    }
}

bool EmployConfig::tryLoadFromEnv(const std::string &sEnvName, std::string &sValue, const std::string &sDescription) {
    if (sValue == "") { // only if not define previously (from command line param)
        if (WsjcppCore::getEnv(sEnvName, sValue)) {
            WsjcppLog::info(TAG, sDescription + ": " + sValue);
            return true;
        }
    }
    return false;
}

std::string EmployConfig::handleRelatedDirPath(const std::string &sDir, const std::string &sDefault) {
  std::string sRet = "";
  if (sDir.size() > 0 && sDir[0] != '/') {
      sRet = this->getWorkDir() + "/" + sDir;
  }
  if (sRet == "") {
      sRet = this->getWorkDir() + "/" + sDefault;
  }
  sRet = WsjcppCore::doNormalizePath(sRet);
  return sRet;
}

bool EmployConfig::initLogging(WsjcppYaml &yamlConfig) {
  m_sLogDir = handleRelatedDirPath(yamlConfig["log-dir"].valStr(), "/logs");
  if (!WsjcppCore::dirExists(m_sLogDir)) {
      WsjcppCore::makeDir(m_sLogDir);
  }
  if (!WsjcppCore::dirExists(m_sLogDir)) {
      WsjcppLog::err(TAG, "Error: Folder '" + m_sLogDir + "' does not exists and could not created, please check access rights to parent folder.");
      return false;
  }
  WsjcppLog::setPrefixLogFile("gtree");
  WsjcppLog::setLogDirectory(m_sLogDir);
  WsjcppLog::setRotationPeriodInSec(yamlConfig["log-rotation-period"].valInt());
  WsjcppLog::setEnableLogFile(true);

  WsjcppLog::info(TAG, "Logger: '" + m_sLogDir);
  return true;
}