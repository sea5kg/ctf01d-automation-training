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

#include "employ_users.h"
#include "web_errors.h"

#include <algorithm>
#include "employ_database.h"
#include "employ_uuids.h"

REGISTRY_WJSCPP_SERVICE_LOCATOR(EmployUsers)

EmployUsers::EmployUsers()
  : WsjcppEmployBase({EmployUsers::name()}, {EmployDatabase::name(), EmployUuids::name()}) {
  TAG = EmployUsers::name();
}

bool EmployUsers::init(const std::string &sName, bool bSilent) {
  auto dbUsers = findWsjcppEmploy<EmployDatabase>()->dbUsers();

  m_mapUserSecretToken = dbUsers->getAllUsers();
  auto it = m_mapUserSecretToken.begin();
  for (auto it = m_mapUserSecretToken.begin(); it != m_mapUserSecretToken.end(); ++it) {
    m_mapSecretTokenUser[it->second] = it->first;
  }
  return true;
}

bool EmployUsers::deinit(const std::string &sName, bool bSilent) {
  return true;
}

bool EmployUsers::createUser(const std::string &name, std::string &secret_token, std::shared_ptr<gtree::ErrorInfo> &error) {
  auto dbUsers = findWsjcppEmploy<EmployDatabase>()->dbUsers();

  if (m_mapUserSecretToken.count(name) > 0) {
    // TODO already registered
  }

  if (!dbUsers->createUser(name, secret_token)) {
    error = std::move(std::make_shared<gtree::ErrorInfo>(
      ERR_10019_COULD_NOT_CREATE_USER.replace("$email$", name)
    ));
    return false;
  }
  m_mapUserSecretToken[name] = secret_token;
  m_mapSecretTokenUser[secret_token] = name;

  return true;
}
