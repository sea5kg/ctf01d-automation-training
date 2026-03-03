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
  m_rating = nlohmann::json::array();

  m_mapUsers = dbUsers->getAllUsers();
  for (auto it = m_mapUsers.begin(); it != m_mapUsers.end(); ++it) {
    UserInfo info = it->second;
    m_mapSecretTokenUser[info.secret_token] = it->first;
    nlohmann::json rating_row;
    rating_row["name"] = info.name;
    rating_row["score"] = info.score;
    rating_row["attack"] = info.attack;
    rating_row["shtraf"] = info.shtraf;
    rating_row["tries"] = info.tries;
    rating_row["updated"] = info.updated;
    m_rating.push_back(rating_row);
  }

  sortRatingTable();

  return true;
}

bool EmployUsers::deinit(const std::string &sName, bool bSilent) {
  return true;
}

bool EmployUsers::createUser(const std::string &username, std::string &secret_token, std::shared_ptr<ctf01d::ErrorInfo> &error) {
  auto dbUsers = findWsjcppEmploy<EmployDatabase>()->dbUsers();

  if (m_mapUsers.count(username) > 0) {
    error = std::move(std::make_shared<ctf01d::ErrorInfo>(
      ERR_10018_USER_ALREADY_EXISTS.replace("$username$", username)
    ));
    return false;
  }

  UserInfo info;
  info.name = username;

  if (!dbUsers->createUser(info.name, info)) {
    error = std::move(std::make_shared<ctf01d::ErrorInfo>(
      ERR_10019_COULD_NOT_CREATE_USER.replace("$username$", username)
    ));
    return false;
  }
  m_mapUsers[info.name] = info;
  m_mapSecretTokenUser[info.secret_token] = info.name;

  secret_token = info.secret_token;

  {
    std::lock_guard<std::mutex> lock(m_mutexRating);
    nlohmann::json rating_row;
    rating_row["name"] = info.name;
    rating_row["score"] = info.score;
    rating_row["attack"] = info.attack;
    rating_row["shtraf"] = info.shtraf;
    rating_row["tries"] = info.tries;
    rating_row["updated"] = info.updated;
    m_rating.push_back(rating_row);
  }

  return true;
}

const nlohmann::json &EmployUsers::rating() {
  return m_rating;
}

void EmployUsers::sortRatingTable() {
  std::lock_guard<std::mutex> lock(m_mutexRating);

  int mutates = 1;
  while (mutates > 0) {
    mutates = 0;
    for (int i = 0; i < m_rating.size() - 1; i++) {
      int current_score = m_rating[i]["score"];
      int current_updated = m_rating[i]["updated"];
      int next_score = m_rating[i+1]["score"];
      int next_updated = m_rating[i+1]["updated"];
      if (
        (current_score < next_score)
        || (current_score == next_score && current_updated > next_updated)
      ) {
        std::swap(m_rating[i], m_rating[i+1]);
        mutates++;
      }
    }
  }
}

std::string EmployUsers::findUserByToken(const std::string &secret_token) {
  auto it = m_mapSecretTokenUser.find(secret_token);
  if (it == m_mapSecretTokenUser.end()) {
    return ""; // user not found
  }
  return it->second;
}

void EmployUsers::updateUserTries(const std::string &name) {
  auto dbUsers = findWsjcppEmploy<EmployDatabase>()->dbUsers();

  dbUsers->updateUserTries(name);

  std::lock_guard<std::mutex> lock(m_mutexRating);
  for (int i = 0; i < m_rating.size(); i++) {
    if (m_rating[i]["name"] == name) {
      int tries = m_rating[i]["tries"];
      m_rating[i]["tries"] = tries + 1;
      break;
    }
  }

}

void EmployUsers::updateUserPenaltyAndTries(const std::string &name) {
  std::lock_guard<std::mutex> lock(m_mutexRating);
  int score = 0;
  int attack = 0;
  int tries = 0;
  int penalty = 0;
  for (int i = 0; i < m_rating.size(); i++) {
    if (m_rating[i]["name"] == name) {
      score = m_rating[i]["score"];
      m_rating[i]["score"] = score - 1;
      attack = m_rating[i]["score"];
      tries = m_rating[i]["tries"];
      m_rating[i]["tries"] = tries + 1;
      penalty = m_rating[i]["shtraf"];
      m_rating[i]["shtraf"] = penalty + 1;
      break;
    }
  }
  auto dbUsers = findWsjcppEmploy<EmployDatabase>()->dbUsers();
  dbUsers->updateUserRatings(
    name,
    score,
    attack,
    penalty,
    tries
  );
}
