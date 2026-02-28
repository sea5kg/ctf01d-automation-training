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

#include "db_user_tries.h"

#include <wsjcpp_core.h>
#include <wsjcpp_hashes.h>
#include <wsjcpp_sql_builder.h>

// ---------------------------------------------------------------------
// DbUserTriesUpdates

class DbUserTriesUpdate_000_001 : public DatabaseFileUpdate {
public:
  DbUserTriesUpdate_000_001() : DatabaseFileUpdate("", "v001", "Init table uuids") {}
  virtual bool applyUpdate(DatabaseFile *pDatabaseFile) override {
    // IF NOT EXISTS
    return pDatabaseFile->executeQuery("CREATE TABLE users_tries ( "
                                       "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                       "  name VARCHAR(128) NOT NULL,"
                                       "  flag VARCHAR(128) NOT NULL,"
                                       "  score INTEGER NOT NULL,"
                                       "  dt INTEGER NOT NULL"
                                       ");");
  }
};

// ---------------------------------------------------------------------
// DbUserTries

DbUserTries::DbUserTries() : DatabaseFile("user_tries.db") {
  TAG = "DbUserTries";
  m_vDbUpdates.push_back(std::make_shared<DbUserTriesUpdate_000_001>());
};

DbUserTries::~DbUserTries() {}

bool DbUserTries::findUserFlag(const std::string &username, const std::string &flag) {
  if (username.length() < 3) {
    WsjcppLog::err(TAG, "User name '" + username + "' to short");
    return false;
  }

  if (username.length() > 10) {
    WsjcppLog::err(TAG, "User name '" + username + "' to long");
    return false;
  }

  if (flag.length() < 3) {
    WsjcppLog::err(TAG, "Flag '" + flag + "' to short");
    return false;
  }

  if (username.length() > 36) {
    WsjcppLog::err(TAG, "Flag '" + flag + "' to long");
    return false;
  }

  std::lock_guard<std::mutex> lock(m_mutex);
  wsjcpp::SqlBuilder builder;
  builder.selectFrom("users_tries")
    .colum("name")
    .where()
      .equal("name", username)
      .equal("flag", flag)
  ;
  DatabaseSelectRows cur;
  if (!this->selectRows(builder.sql(), cur)) {
    return false;
  }
  if (cur.next()) {
    return true;
  }
  return false;
}

bool DbUserTries::addUserFlag(const std::string &username, const std::string &flag, int score) {
  if (username.length() < 3) {
    WsjcppLog::err(TAG, "User name '" + username + "' to short");
    return false;
  }

  if (username.length() > 10) {
    WsjcppLog::err(TAG, "User name '" + username + "' to long");
    return false;
  }

  if (flag.length() < 3) {
    WsjcppLog::err(TAG, "Flag '" + flag + "' to short");
    return false;
  }

  if (username.length() > 36) {
    WsjcppLog::err(TAG, "Flag '" + flag + "' to long");
    return false;
  }

  std::lock_guard<std::mutex> lock(m_mutex);

  wsjcpp::SqlBuilder builder;
  builder.insertInto("users")
    .addColums({
      "name",
      "flag",
      "score",
      "dt",
    })
    .val(username)
    .val(flag)
    .val(score) // score
    .val(WsjcppCore::getCurrentTimeInMilliseconds())
  ;
  return this->executeQuery(builder.sql());

}
