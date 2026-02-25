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
#include "gt_errors.h"

#include <algorithm>
#include "employ_database.h"
#include "employ_uuids.h"

REGISTRY_WJSCPP_SERVICE_LOCATOR(EmployUsers)

EmployUsers::EmployUsers()
  : WsjcppEmployBase({EmployUsers::name()}, {EmployDatabase::name(), EmployUuids::name()}) {
  TAG = EmployUsers::name();
}

bool EmployUsers::init(const std::string &sName, bool bSilent) {
  auto pDbUsers = findWsjcppEmploy<EmployDatabase>()->dbUsers();
  std::pair<std::string, std::string> res = pDbUsers->findUserByNameAndPass("admin", "admin");
  if (res.first != "") {
    WsjcppLog::warn(TAG, "Found default user 'admin' with default password 'admin' please change password or remove this user.");
  }

  // TOOD cache od users from database
  // TOOD cache of sessions from database

  auto *pUuids = findWsjcppEmploy<EmployUuids>();
  pUuids->addAllowedTypesOfUuid("session");
  pUuids->addAllowedTypesOfUuid("user");
  return true;
}

bool EmployUsers::deinit(const std::string &sName, bool bSilent) {
  return true;
}

UserSession EmployUsers::doLogin(const std::string &email, const std::string &pass) {
  auto dbUsers = findWsjcppEmploy<EmployDatabase>()->dbUsers();

  UserSession session;
  std::pair<std::string, std::string> res = dbUsers->findUserByNameAndPass(email, pass);
  if (res.first == "") {
    return session;
  }
  std::string uuid = res.first;
  std::string role = res.second;
  UserInfo user;
  if (m_mapUserInfo.count(uuid)) {
    m_mapUserInfo[uuid].email = email;
    m_mapUserInfo[uuid].role = role;
    m_mapUserInfo[uuid].uuid = uuid;
    user = m_mapUserInfo[uuid];
  } else {
    user.email = email;
    user.role = role;
    user.uuid = uuid;
    m_mapUserInfo[uuid] = user;
  }

  session.user = user;

  auto *pUuids = findWsjcppEmploy<EmployUuids>();
  session.uuid = pUuids->generateNewUuid("session");
  session.expired_at = WsjcppCore::getCurrentTimeInSeconds() + 86400; // on 24h

  // WsjcppLog::info(TAG, "m_mapSessionUserUuidCache[" + session.uuid + "] = " + session.user.uuid);

  m_mapSessionUserUuidCache[session.uuid] = session.user.uuid;
  m_mapSessionExpiredAt[session.uuid] = session.expired_at;
  // TODO write session to database

  return session;
}

bool EmployUsers::doLogout(const std::string &uuid) {
  if (uuid == "") {
    return false;
  }
  // UserSession session = findSession(uuid);

  if (m_mapSessionUserUuidCache.count(uuid) > 0) {
    m_mapSessionUserUuidCache.erase(uuid);
  }

  if (m_mapSessionExpiredAt.count(uuid) > 0) {
    m_mapSessionExpiredAt.erase(uuid);
  }

  return true;
}

UserSession EmployUsers::findSession(const std::string &uuid) {
  UserSession session;
  if (uuid == "") {
    return session;
  }

  if (m_mapSessionUserUuidCache.count(uuid) == 0) {
    return session;
  }

  if (m_mapSessionExpiredAt.count(uuid) == 0) {
    m_mapSessionUserUuidCache.erase(uuid);
    return session;
  }

  int expired_at = m_mapSessionExpiredAt[uuid];
  if (expired_at < WsjcppCore::getCurrentTimeInSeconds()) {
    m_mapSessionExpiredAt.erase(uuid);
    m_mapSessionUserUuidCache.erase(uuid);
    return session;
  }

  std::string user_uuid = m_mapSessionUserUuidCache[uuid];

  UserInfo user;
  if (m_mapUserInfo.count(user_uuid) == 0) {
    return session;
  }

  session.uuid = uuid;
  session.expired_at = m_mapSessionExpiredAt[uuid];
  session.user = m_mapUserInfo[user_uuid];

  return session;
}

bool EmployUsers::createUser(const std::string &email, const std::string &pass, const std::string &role, std::shared_ptr<gtree::ErrorInfo> &error) {
  auto dbUsers = findWsjcppEmploy<EmployDatabase>()->dbUsers();

  std::string uuid = dbUsers->findUserUuid(email);
  if (uuid != "") {
    error = std::move(std::make_shared<gtree::ErrorInfo>(
      ERR_10018_USER_ALREADY_EXISTS.replace("$email$", email).replace("$uuid$", uuid)
    ));
    return false;
  }

  auto *pUuids = findWsjcppEmploy<EmployUuids>();

  std::string user_uuid = pUuids->generateNewUuid("user");

  if (!dbUsers->createUser(user_uuid, email, role, pass)) {
    error = std::move(std::make_shared<gtree::ErrorInfo>(
      ERR_10019_COULD_NOT_CREATE_USER.replace("$email$", email)
    ));
    return false;
  }

  return true;
}

bool EmployUsers::removeUser(const std::string &email, std::shared_ptr<gtree::ErrorInfo> &error) {
  auto dbUsers = findWsjcppEmploy<EmployDatabase>()->dbUsers();

  std::string user_uuid = dbUsers->findUserUuid(email);
  if (user_uuid == "") {
    error = std::move(std::make_shared<gtree::ErrorInfo>(
      ERR_10009_USER_NOT_FOUND_WITH_EMAIL.replace("$email$", email)
    ));
    return false;
  }

  if (!dbUsers->removeUser(user_uuid)) {
    error = std::move(std::make_shared<gtree::ErrorInfo>(
      ERR_10010_COULD_NOT_REMOVE_USER_WITH_EMAIL.replace("$email$", email)
    ));
    return false;
  }

  return true;
}

bool EmployUsers::resetUserPassword(const std::string &email, const std::string &pass, std::shared_ptr<gtree::ErrorInfo> &error) {
  auto dbUsers = findWsjcppEmploy<EmployDatabase>()->dbUsers();

  std::string user_uuid = dbUsers->findUserUuid(email);
  if (user_uuid == "") {
    error = std::move(std::make_shared<gtree::ErrorInfo>(
      ERR_10025_USER_NOT_FOUND_WITH_EMAIL.replace("$email$", email)
    ));
    return false;
  }
  std::string db_error;
  if (!dbUsers->changeUserPassword(user_uuid, pass, db_error)) {
    error = std::move(std::make_shared<gtree::ErrorInfo>(
      ERR_10026_COULD_NOT_UPDATE_PASSWORD_FOR.replace("$email$", email).replace("$error$", db_error)
    ));
    return false;
  }

  return true;
}

bool EmployUsers::changePassword(const std::string &email, const std::string &old_pass, const std::string &new_pass, std::shared_ptr<gtree::ErrorInfo> &error) {
  auto dbUsers = findWsjcppEmploy<EmployDatabase>()->dbUsers();

  std::string user_uuid = dbUsers->findUserUuid(email);
  if (user_uuid == "") {
    error = std::move(std::make_shared<gtree::ErrorInfo>(
      ERR_10027_USER_NOT_FOUND_WITH_EMAIL.replace("$email$", email)
    ));
    return false;
  }
  std::pair<std::string, std::string> res = dbUsers->findUserByNameAndPass(email, old_pass);
  if (res.first == "") {
    error = std::move(std::make_shared<gtree::ErrorInfo>(
      ERR_10028_WRONG_PASSWORD
    ));
    return false;
  }

  std::string db_error;
  if (!dbUsers->changeUserPassword(user_uuid, new_pass, db_error)) {
    error = std::move(std::make_shared<gtree::ErrorInfo>(
      ERR_10029_COULD_NOT_UPDATE_PASSWORD_FOR.replace("$email$", email).replace("$error$", db_error)
    ));
    return false;
  }

  return true;
}