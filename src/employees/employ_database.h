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

#pragma once

#include <wsjcpp_employees.h>
#include <mutex>
#include "db_uuids.h"
#include "db_users.h"
#include "db_user_tries.h"

class EmployDatabase : public WsjcppEmployBase {
public:
  EmployDatabase();
  static std::string name() { return "EmployDatabase"; }
  virtual bool init(const std::string &sName, bool bSilent);
  virtual bool deinit(const std::string &sName, bool bSilent);

  std::shared_ptr<DbUuids> dbUuids();
  std::shared_ptr<DbUsers> dbUsers();
  std::shared_ptr<DbUserTries> dbUserTries();

private:
  std::string TAG;

  bool initDbUuids();
  bool initDbUsers();
  bool initDbUserTries();

  std::shared_ptr<DbUuids> m_dbUuids;
  std::shared_ptr<DbUsers> m_dbUsers;
  std::shared_ptr<DbUserTries> m_dbUsersTries;
};
