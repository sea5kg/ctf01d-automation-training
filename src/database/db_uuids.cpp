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

#include "db_uuids.h"

#include <wsjcpp_core.h>
#include <wsjcpp_sql_builder.h>

// ---------------------------------------------------------------------
// DbUuidsUpdates

class DbUuidsUpdate_000_001 : public DatabaseFileUpdate {
public:
  DbUuidsUpdate_000_001() : DatabaseFileUpdate("", "v001", "Init table uuids") {}
  virtual bool applyUpdate(DatabaseFile *pDatabaseFile) override {
    // IF NOT EXISTS
    return pDatabaseFile->executeQuery("CREATE TABLE uuids ( "
                                       "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                       "  uuid VARCHAR(36) NOT NULL,"
                                       "  typeobj VARCHAR(36) NOT NULL,"
                                       "  dt INTEGER NOT NULL"
                                       ");");
  }
};

class DbUuidsUpdate_001_002 : public DatabaseFileUpdate {
public:
  DbUuidsUpdate_001_002() : DatabaseFileUpdate("v001", "v002", "Create uniq index") {}
  virtual bool applyUpdate(DatabaseFile *pDatabaseFile) override {
    return pDatabaseFile->executeQuery("CREATE UNIQUE INDEX IF NOT EXISTS uuids_col_uuid ON uuids (uuid)");
  }
};

class DbUuidsUpdate_002_003 : public DatabaseFileUpdate {
public:
  DbUuidsUpdate_002_003() : DatabaseFileUpdate("v002", "v003", "Add default user") {}
  virtual bool applyUpdate(DatabaseFile *pDatabaseFile) override {
    std::string uuid = "6d7d9de3-11ba-4c9f-beba-b34ead0e074b";
    std::string typeobj = "user";
    long dt = WsjcppCore::getCurrentTimeInMilliseconds();
    return pDatabaseFile->executeQuery("INSERT INTO uuids(uuid, typeobj, dt) VALUES('" + uuid + "', '" + typeobj + "', " + std::to_string(dt) + ")");
  }
};

// ---------------------------------------------------------------------
// DbUuids

DbUuids::DbUuids() : DatabaseFile("uuids.db") {
  TAG = "DbUuids";
  m_vDbUpdates.push_back(std::make_shared<DbUuidsUpdate_000_001>());
  m_vDbUpdates.push_back(std::make_shared<DbUuidsUpdate_001_002>());
  m_vDbUpdates.push_back(std::make_shared<DbUuidsUpdate_002_003>());
};

DbUuids::~DbUuids() {}

std::map<std::string, std::string> DbUuids::getAllRecords() {
  std::lock_guard<std::mutex> lock(m_mutex);

  std::map<std::string, std::string> mapUuids;
  std::string sSql = "SELECT uuid, typeobj FROM uuids;";
  DatabaseSelectRows cur;
  if (this->selectRows(sSql, cur)) {
    while (cur.next()) {
      mapUuids[cur.getString(0)] = cur.getString(1);
    }
  }
  return mapUuids;
}

bool DbUuids::insertUuid(const std::string &sUuid, const std::string &sTypeOfObject) {
  std::lock_guard<std::mutex> lock(m_mutex);

  wsjcpp::SqlBuilder builder;
  builder.insertInto("uuids")
    .addColums({"uuid", "typeobj", "dt"})
    .val(sUuid)
    .val(sTypeOfObject)
    .val(WsjcppCore::getCurrentTimeInMilliseconds())
  ;

  if (!this->executeQuery(builder.sql())) {
    WsjcppLog::err(TAG, "Could not insert " + builder.sql());
    return false;
  }
  return true;
}

bool DbUuids::deleteUuid(const std::string &sUuid) {
  std::lock_guard<std::mutex> lock(m_mutex);
  // TODO
  // DatabaseSqlQueryDelete sql("uuids");
  // sql.add("uuid", sUuid);

  // if (!this->executeQuery(sql.getSql())) {
  //   WsjcppLog::err(TAG, "Could not insert " + sql.getSql());
  //   return false;
  // }
  return true;
}
