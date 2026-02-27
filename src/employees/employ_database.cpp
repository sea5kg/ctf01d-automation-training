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
  return true;
}

bool EmployDatabase::deinit(const std::string &sName, bool bSilent) {
  return true;
}

std::shared_ptr<DbUuids> EmployDatabase::dbUuids() { return m_pUuids; }

std::shared_ptr<DbUsers> EmployDatabase::dbUsers() { return m_pUsers; }

bool EmployDatabase::initDbUuids() {
  m_pUuids = std::make_shared<DbUuids>();
  if (!m_pUuids->open()) {
    return false;
  }
  WsjcppLog::ok(TAG, "Initialized " + m_pUuids->getFileFullpath());
  return true;
}

bool EmployDatabase::initDbUsers() {
  m_pUsers = std::make_shared<DbUsers>();
  if (!m_pUsers->open()) {
    return false;
  }
  WsjcppLog::ok(TAG, "Initialized " + m_pUsers->getFileFullpath());
  return true;
}

