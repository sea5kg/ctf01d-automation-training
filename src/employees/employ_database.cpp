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

// https://github.com/sea5kg/ctf01d-automation-training-automation-training

#include "employ_database.h"
#include "employ_config.h"
#include <sqlite3.h>

// ---------------------------------------------------------------------
// EmployDatabase

REGISTRY_WJSCPP_SERVICE_LOCATOR(EmployDatabase)

EmployDatabase::EmployDatabase() : WsjcppEmployBase({EmployDatabase::name()}, {EmployConfig::name()}) {
  TAG = EmployDatabase::name();
}

bool EmployDatabase::init(const std::string &sName, bool bSilent) {
  // new database format
  int nRet = 0;
  if (SQLITE_OK != (nRet = sqlite3_initialize())) {
    WsjcppLog::throw_err(TAG, "Failed to initialize build-in sqlite3 library: " + std::to_string(nRet));
    return false;
  }
  // TODO via list pointers or something like
  WsjcppLog::ok(TAG, "Initialize build-in sqlite3 library");
  if (!this->initDbUuids()) {
    return false;
  }
  if (!this->initDbUsers()) {
    return false;
  }
  if (!this->initDbUserTries()) {
    return false;
  }
  return true;
}

bool EmployDatabase::deinit(const std::string &sName, bool bSilent) {
  return true;
}

std::shared_ptr<DbUuids> EmployDatabase::dbUuids() { return m_dbUuids; }

std::shared_ptr<DbUsers> EmployDatabase::dbUsers() { return m_dbUsers; }

std::shared_ptr<DbUserTries> EmployDatabase::dbUserTries() { return m_dbUsersTries; }

bool EmployDatabase::initDbUuids() {
  m_dbUuids = std::make_shared<DbUuids>();
  if (!m_dbUuids->open()) {
    return false;
  }
  WsjcppLog::ok(TAG, "Initialized " + m_dbUuids->getFileFullpath());
  return true;
}

bool EmployDatabase::initDbUsers() {
  m_dbUsers = std::make_shared<DbUsers>();
  if (!m_dbUsers->open()) {
    return false;
  }
  WsjcppLog::ok(TAG, "Initialized " + m_dbUsers->getFileFullpath());
  return true;
}

bool EmployDatabase::initDbUserTries() {
  m_dbUsersTries = std::make_shared<DbUserTries>();
  if (!m_dbUsersTries->open()) {
    return false;
  }
  WsjcppLog::ok(TAG, "Initialized " + m_dbUsersTries->getFileFullpath());
  return true;
}
