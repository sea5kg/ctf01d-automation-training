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

#include "db_users.h"

#include <wsjcpp_core.h>
#include <wsjcpp_hashes.h>
#include <wsjcpp_sql_builder.h>

// ---------------------------------------------------------------------
// DbUsersUpdates

class DbUsersUpdate_000_001 : public DatabaseFileUpdate {
public:
  DbUsersUpdate_000_001() : DatabaseFileUpdate("", "v001", "Init table uuids") {}
  virtual bool applyUpdate(DatabaseFile *pDatabaseFile) override {
    // IF NOT EXISTS
    return pDatabaseFile->executeQuery("CREATE TABLE users ( "
                                       "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                       "  name VARCHAR(128) NOT NULL,"
                                       "  secret_token VARCHAR(128) NOT NULL,"
                                       "  score INTEGER NOT NULL,"
                                       "  attack INTEGER NOT NULL,"
                                       "  shtraf INTEGER NOT NULL,"
                                       "  tries INTEGER NOT NULL,"
                                       "  dt INTEGER NOT NULL,"
                                       "  dt_updated INTEGER NOT NULL"
                                       ");");
  }
};

class DbUsersUpdate_001_002 : public DatabaseFileUpdate {
public:
  DbUsersUpdate_001_002() : DatabaseFileUpdate("v001", "v002", "Create uniq index") {}
  virtual bool applyUpdate(DatabaseFile *pDatabaseFile) override {
    return pDatabaseFile->executeQuery("CREATE UNIQUE INDEX IF NOT EXISTS users_col_name ON users (name)");
  }
};

class DbUsersUpdate_002_003 : public DatabaseFileUpdate {
public:
  DbUsersUpdate_002_003() : DatabaseFileUpdate("v002", "v003", "Create uniq index (2)") {}
  virtual bool applyUpdate(DatabaseFile *pDatabaseFile) override {
    return pDatabaseFile->executeQuery("CREATE UNIQUE INDEX IF NOT EXISTS users_secret_token ON users (secret_token)");
  }
};

// ---------------------------------------------------------------------
// DbUsers

DbUsers::DbUsers() : DatabaseFile("users.db") {
  TAG = "DbUsers";
  m_vDbUpdates.push_back(std::make_shared<DbUsersUpdate_000_001>());
  m_vDbUpdates.push_back(std::make_shared<DbUsersUpdate_001_002>());
  m_vDbUpdates.push_back(std::make_shared<DbUsersUpdate_002_003>());
};

DbUsers::~DbUsers() {}


std::map<std::string, std::string> DbUsers::getAllUsers() {
  std::lock_guard<std::mutex> lock(m_mutex);

  std::map<std::string, std::string> ret;
  wsjcpp::SqlBuilder builder;
  builder.selectFrom("users")
    .colum("name")
    .colum("secret_token")
    // .where().equal("name", name)
  ;
  DatabaseSelectRows cur;
  if (!this->selectRows(builder.sql(), cur)) {
    return ret;
  }
  while (cur.next()) {
    std::string name = cur.getString(0);
    std::string secret_token = cur.getString(1);
    ret[name] = secret_token;
  }
  return ret;
}

// std::pair<std::string, std::string> DbUsers::findUserByNameAndPass(const std::string &name, const std::string &pass) {
//   std::lock_guard<std::mutex> lock(m_mutex);

//   std::string solt = "";
//   {
//     wsjcpp::SqlBuilder builder;
//     builder.selectFrom("users")
//       .colum("solt")
//       .where().equal("name", name)
//     ;
//     DatabaseSelectRows cur;
//     if (!this->selectRows(builder.sql(), cur)) {
//       return std::pair<std::string, std::string>("", "");
//     }
//     if (!cur.next()) {
//       return std::pair<std::string, std::string>("", "");
//     }
//     solt = cur.getString(0);
//   }
//   std::string sha1_pass = WsjcppHashes::getSha1ByString(pass + solt);

//   std::string uuid = "";
//   std::string role = "";
//   {
//     wsjcpp::SqlBuilder builder;
//     builder.selectFrom("users")
//       .colum("uuid")
//       .colum("role")
//       .where()
//         .equal("name", name)
//         .and_()
//         .equal("pass", sha1_pass)
//     ;

//     DatabaseSelectRows cur;
//     if (!this->selectRows(builder.sql(), cur)) {
//       return std::pair<std::string, std::string>("", "");
//     }
//     if (!cur.next()) {
//       return std::pair<std::string, std::string>("", "");
//     }
//     uuid = cur.getString(0);
//     role = cur.getString(1);
//   }
//   return std::pair<std::string, std::string>(uuid, role);
// }

std::string DbUsers::findUserBySecretToken(const std::string &secret_token) {
  std::lock_guard<std::mutex> lock(m_mutex);
  return unsafe_findUserBySecretToken(secret_token);
}

bool DbUsers::createUser(const std::string &name, std::string &secret_token) {
  std::lock_guard<std::mutex> lock(m_mutex);

  if (name.length() < 3) {
    WsjcppLog::err(TAG, "User name '" + name + "' to short");
    return false;
  }

  {
    wsjcpp::SqlBuilder builder;
    builder.selectFrom("users")
      .colum("name")
      .where().equal("name", name)
    ;
    DatabaseSelectRows cur;
    if (!this->selectRows(builder.sql(), cur)) {
      WsjcppLog::err(TAG, "Problem with database");
      return false;
    }
    if (cur.next()) {
      WsjcppLog::err(TAG, "User '" + name + "' aready exists");
      return false;
    }
  }

  std::string random_secret_token = wsjcpp::Core::randomString(wsjcpp::Core::englishAlphabetBothCaseAndNumbers(), 5);
  std::string user_name = unsafe_findUserBySecretToken(random_secret_token);

  while (user_name != "") {
    random_secret_token = wsjcpp::Core::randomString(wsjcpp::Core::englishAlphabetBothCaseAndNumbers(), 5);
    user_name = unsafe_findUserBySecretToken(random_secret_token);
  }

  // return value
  secret_token = random_secret_token;

  wsjcpp::SqlBuilder builder;
  builder.insertInto("users")
    .addColums({
      "name",
      "secret_token",
      "dt"
    })
    .val(name)
    .val(secret_token)
    .val(WsjcppCore::getCurrentTimeInMilliseconds())
  ;
  return this->executeQuery(builder.sql());
}

std::string DbUsers::unsafe_findUserBySecretToken(const std::string &secret_token) {
  wsjcpp::SqlBuilder builder;
  builder.selectFrom("users")
    .colum("name")
    .where().equal("secret_token", secret_token)
  ;

  DatabaseSelectRows cur;
  if (!this->selectRows(builder.sql(), cur)) {
    return "";
  }
  if (cur.next()) {
    return cur.getString(0);
  }
  return "";
}
