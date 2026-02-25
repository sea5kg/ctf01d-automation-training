/* MIT License

* Copyright (c) 2019-2025 Evgenii Sopov

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
                                       "  uuid VARCHAR(36) NOT NULL,"
                                       "  name VARCHAR(128) NOT NULL,"
                                       "  pass VARCHAR(40) NOT NULL,"
                                       "  solt VARCHAR(40) NOT NULL,"
                                       "  role VARCHAR(36) NOT NULL,"
                                       "  dt INTEGER NOT NULL"
                                       ");");
  }
};

class DbUsersUpdate_001_002 : public DatabaseFileUpdate {
public:
  DbUsersUpdate_001_002() : DatabaseFileUpdate("v001", "v002", "Create uniq index") {}
  virtual bool applyUpdate(DatabaseFile *pDatabaseFile) override {
    return pDatabaseFile->executeQuery("CREATE UNIQUE INDEX IF NOT EXISTS users_col_uuid ON users (uuid)");
  }
};

class DbUsersUpdate_002_003 : public DatabaseFileUpdate {
public:
  DbUsersUpdate_002_003() : DatabaseFileUpdate("v002", "v003", "Add default user") {}
  virtual bool applyUpdate(DatabaseFile *pDatabaseFile) override {
    std::string uuid = "6d7d9de3-11ba-4c9f-beba-b34ead0e074b";
    std::string name = "admin";
    std::string pass = "admin";
    std::string role = "admin";
    std::string solt = "zAYgnGzoh2";
    std::string pass_sha1 = "8bc1dbce82b9a3072d20b50e44a28e5154d4a921"; // sha1(pass + solt)
    long dt = WsjcppCore::getCurrentTimeInMilliseconds();
    return pDatabaseFile->executeQuery("INSERT INTO users(uuid, name, pass, solt, role, dt) VALUES('" + uuid + "', '" + name + "', '" + pass_sha1 + "', '" + solt + "', '" + role + "', " + std::to_string(dt) + ")");
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

std::pair<std::string, std::string> DbUsers::findUserByNameAndPass(const std::string &name, const std::string &pass) {
  std::lock_guard<std::mutex> lock(m_mutex);

  std::string solt = "";
  {
    wsjcpp::SqlBuilder builder;
    builder.selectFrom("users")
      .colum("solt")
      .where().equal("name", name)
    ;
    DatabaseSelectRows cur;
    if (!this->selectRows(builder.sql(), cur)) {
      return std::pair<std::string, std::string>("", "");
    }
    if (!cur.next()) {
      return std::pair<std::string, std::string>("", "");
    }
    solt = cur.getString(0);
  }
  std::string sha1_pass = WsjcppHashes::getSha1ByString(pass + solt);

  std::string uuid = "";
  std::string role = "";
  {
    wsjcpp::SqlBuilder builder;
    builder.selectFrom("users")
      .colum("uuid")
      .colum("role")
      .where()
        .equal("name", name)
        .and_()
        .equal("pass", sha1_pass)
    ;

    DatabaseSelectRows cur;
    if (!this->selectRows(builder.sql(), cur)) {
      return std::pair<std::string, std::string>("", "");
    }
    if (!cur.next()) {
      return std::pair<std::string, std::string>("", "");
    }
    uuid = cur.getString(0);
    role = cur.getString(1);
  }
  return std::pair<std::string, std::string>(uuid, role);
}

std::string DbUsers::findUserUuid(const std::string &name) {
  std::lock_guard<std::mutex> lock(m_mutex);

  wsjcpp::SqlBuilder builder;
  builder.selectFrom("users")
    .colum("uuid")
    .where().equal("name", name)
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

bool DbUsers::createUser(
  const std::string &uuid,
  const std::string &name,
  const std::string &role,
  const std::string &pass
) {
  std::lock_guard<std::mutex> lock(m_mutex);

  std::string solt = createRandomSolt();

  std::string sha1_pass = WsjcppHashes::getSha1ByString(pass + solt);

  wsjcpp::SqlBuilder builder;
  builder.insertInto("users")
    .addColums({
      "uuid",
      "name",
      "pass",
      "solt",
      "role",
      "dt"
    })
    .val(uuid)
    .val(name)
    .val(sha1_pass)
    .val(solt)
    .val(role)
    .val(WsjcppCore::getCurrentTimeInMilliseconds())
  ;
  return this->executeQuery(builder.sql());
}

bool DbUsers::removeUser(const std::string &uuid) {
  std::lock_guard<std::mutex> lock(m_mutex);
  wsjcpp::SqlBuilder builder;
  builder.deleteFrom("users")
    .where().equal("uuid", uuid)
  ;
  return this->executeQuery(builder.sql());
}

bool DbUsers::changeUserPassword(const std::string &uuid, const std::string &pass, std::string &error) {
  std::lock_guard<std::mutex> lock(m_mutex);

  std::string solt = findSoltByUuid(uuid);
  if (solt == "") {
    error = "Could no find record in database (solt)";
    return false;
  }
  std::string sha1_pass = WsjcppHashes::getSha1ByString(pass + solt);

  wsjcpp::SqlBuilder builder;
  builder.update("users")
    .set("pass", sha1_pass)
    .where().equal("uuid", uuid)
  ;
  return this->executeQuery(builder.sql());
}

std::string DbUsers::findSoltByUuid(const std::string &uuid) {
  wsjcpp::SqlBuilder builder;
  builder.selectFrom("users")
    .colum("solt")
    .where().equal("uuid", uuid)
  ;
  DatabaseSelectRows cur;
  if (!this->selectRows(builder.sql(), cur)) {
    return "";
  }
  if (!cur.next()) {
    return "";
  }
  std::string solt = cur.getString(0);
  return solt;
}

std::string DbUsers::createRandomSolt() {
    return wsjcpp::Core::randomString(
      wsjcpp::Core::englishAlphabetBothCaseAndNumbers(),
      10
    );
}
