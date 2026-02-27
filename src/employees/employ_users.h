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

#include "web_errors.h"
#include "db_structs.h"
#include <wsjcpp_employees.h>
#include <json.hpp>

class EmployUsers : public WsjcppEmployBase {
public:
  EmployUsers();
  static std::string name() { return "EmployUsers"; }
  virtual bool init(const std::string &sName, bool bSilent);
  virtual bool deinit(const std::string &sName, bool bSilent) override;

  bool createUser(const std::string &name, std::string &secret_token, std::shared_ptr<gtree::ErrorInfo> &error);
  const nlohmann::json &rating();
  std::string findUserByToken(const std::string &secret_token);
  void updateUserTries(const std::string &name);

private:
  void sortRatingTable();

  std::mutex m_mutex;
  std::map<std::string, UserInfo> m_mapUsers;
  std::map<std::string, std::string> m_mapSecretTokenUser;

  std::mutex m_mutexRating;
  nlohmann::json m_rating;
  std::string TAG;
};
