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

#pragma once

#include <memory>

#include "gt_errors.h"
#include <wsjcpp_employees.h>

struct UserInfo  {
  std::string uuid;
  std::string email;
  std::string role;
};

struct UserSession  {
  std::string uuid;
  int expired_at;
  UserInfo user;
};


class EmployUsers : public WsjcppEmployBase {
public:
  EmployUsers();
  static std::string name() { return "EmployUsers"; }
  virtual bool init(const std::string &sName, bool bSilent);
  virtual bool deinit(const std::string &sName, bool bSilent) override;

  UserSession doLogin(const std::string &name, const std::string &pass);
  bool doLogout(const std::string &uuid);
  UserSession findSession(const std::string &uuid);
  bool createUser(const std::string &email, const std::string &pass, const std::string &role, std::shared_ptr<gtree::ErrorInfo> &error);
  bool removeUser(const std::string &email, std::shared_ptr<gtree::ErrorInfo> &error);
  bool resetUserPassword(const std::string &email, const std::string &pass, std::shared_ptr<gtree::ErrorInfo> &error);
  bool changePassword(const std::string &email, const std::string &old_pass, const std::string &new_pass, std::shared_ptr<gtree::ErrorInfo> &error);

private:
  std::mutex m_mutex;
  std::map<std::string, UserInfo> m_mapUserInfo;
  std::map<std::string, std::string> m_mapSessionUserUuidCache;
  std::map<std::string, int> m_mapSessionExpiredAt;
  std::string TAG;
};
