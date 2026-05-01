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

#include "db_flags.h"

#include <wsjcpp_core.h>
#include <wsjcpp_hashes.h>
#include <wsjcpp_sql_builder.h>

// ---------------------------------------------------------------------
// DbFlagsUpdates

class DbFlagsUpdate_000_001 : public DatabaseFileUpdate {
public:
  DbFlagsUpdate_000_001() : DatabaseFileUpdate("", "v001", "Init table flags") {}
  virtual bool applyUpdate(DatabaseFile *pDatabaseFile) override {
    return pDatabaseFile->executeQuery("CREATE TABLE flags ( "
                                       "  flag_id VARCHAR(255) NOT NULL,"
                                       "  flag_value VARCHAR(255) NOT NULL,"
                                       "  start_ms INTEGER NOT NULL,"
                                       "  end_ms INTEGER NOT NULL"
                                       ");");
  }
};

class DbFlagsUpdate_001_002 : public DatabaseFileUpdate {
public:
  DbFlagsUpdate_001_002() : DatabaseFileUpdate("v001", "v002", "Create uniq index") {}
  virtual bool applyUpdate(DatabaseFile *pDatabaseFile) override {
    return pDatabaseFile->executeQuery("CREATE UNIQUE INDEX IF NOT EXISTS flag_value ON flags (flag_value)");
  }
};

// class DbFlagsUpdate_002_003 : public DatabaseFileUpdate {
// public:
//   DbFlagsUpdate_002_003() : DatabaseFileUpdate("v002", "v003", "Create index for flag_value, start_ms") {}
//   virtual bool applyUpdate(DatabaseFile *pDatabaseFile) override {
//     return pDatabaseFile->executeQuery("CREATE INDEX IF NOT EXISTS flags_start_ms_flag_value ON flags (start_ms, flag_value)");
//   }
// };

// class DbFlagsUpdate_003_004 : public DatabaseFileUpdate {
// public:
//   DbFlagsUpdate_003_004() : DatabaseFileUpdate("v003", "v004", "Create index for start_ms") {}
//   virtual bool applyUpdate(DatabaseFile *pDatabaseFile) override {
//     return pDatabaseFile->executeQuery("CREATE INDEX IF NOT EXISTS flags_start_ms ON flags (start_ms)");
//   }
// };

// ---------------------------------------------------------------------
// DbFlags

DbFlags::DbFlags() : DatabaseFile("flags.db") {
  TAG = "DbFlags";
  m_vDbUpdates.push_back(std::make_shared<DbFlagsUpdate_000_001>());
  m_vDbUpdates.push_back(std::make_shared<DbFlagsUpdate_001_002>());
  // m_vDbUpdates.push_back(std::make_shared<DbFlagsUpdate_002_003>());
  // m_vDbUpdates.push_back(std::make_shared<DbFlagsUpdate_003_004>());
};

DbFlags::~DbFlags() {}

std::map<std::string, Ctf01dFlag> DbFlags::getFlagsNotExpired() {
  std::lock_guard<std::mutex> lock(m_mutex);

  long currentTime = WsjcppCore::getCurrentTimeInMilliseconds();

  std::map<std::string, Ctf01dFlag> ret;
  wsjcpp::SqlBuilder builder;
  builder.selectFrom("flags")
    .colum("flag_id")
    .colum("flag_value")
    .colum("start_ms")
    .colum("end_ms")
    .where().lessThen("end_ms", currentTime)
  ;
  DatabaseSelectRows cur;
  if (!this->selectRows(builder.sql(), cur)) {
    return ret;
  }
  while (cur.next()) {
    std::string flag_value = cur.getString(1);
    Ctf01dFlag flag;
    flag.setId(cur.getString(0));
    flag.setValue(flag_value);
    flag.setTimeStartInMs(cur.getLong(2));
    flag.setTimeEndInMs(cur.getLong(3));
    ret[flag_value] = flag;
  }
  return ret;
}

void DbFlags::insertFlag(const Ctf01dFlag &flag) {
  // TODO
  std::lock_guard<std::mutex> lock(m_mutex);
}