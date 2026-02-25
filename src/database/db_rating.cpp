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

#include "db_rating.h"

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
    return pDatabaseFile->executeQuery("CREATE TABLE rating ( "
                                       "  name VARCHAR(128) NOT NULL,"
                                       "  score INTEGER NOT NULL"
                                       "  attack INTEGER NOT NULL"
                                       "  shtraf INTEGER NOT NULL"
                                       "  tries INTEGER NOT NULL"
                                       "  dt_updated INTEGER NOT NULL"
                                       ");");
  }
};

class DbUsersUpdate_001_002 : public DatabaseFileUpdate {
public:
  DbUsersUpdate_001_002() : DatabaseFileUpdate("v001", "v002", "Create uniq index") {}
  virtual bool applyUpdate(DatabaseFile *pDatabaseFile) override {
    return pDatabaseFile->executeQuery("CREATE UNIQUE INDEX IF NOT EXISTS users_col_uuid ON users (name)");
  }
};

// ---------------------------------------------------------------------
// DbRating

DbRating::DbRating() : DatabaseFile("rating.db") {
  TAG = "DbRating";
  m_vDbUpdates.push_back(std::make_shared<DbUsersUpdate_000_001>());
  m_vDbUpdates.push_back(std::make_shared<DbUsersUpdate_001_002>());
};

DbRating::~DbRating() {}

std::pair<std::string, std::string> DbRating::findUserByNameAndPass(const std::string &name, const std::string &pass) {
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

std::string DbRating::findUserUuid(const std::string &name) {
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

bool DbRating::createUser(
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

bool DbRating::removeUser(const std::string &uuid) {
  std::lock_guard<std::mutex> lock(m_mutex);
  wsjcpp::SqlBuilder builder;
  builder.deleteFrom("users")
    .where().equal("uuid", uuid)
  ;
  return this->executeQuery(builder.sql());
}

bool DbRating::changeUserPassword(const std::string &uuid, const std::string &pass, std::string &error) {
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

std::string DbRating::findSoltByUuid(const std::string &uuid) {
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

std::string DbRating::createRandomSolt() {
    return wsjcpp::Core::randomString(
      wsjcpp::Core::englishAlphabetBothCaseAndNumbers(),
      10
    );
}
