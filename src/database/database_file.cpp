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

#include "database_file.h"

#include "employ_config.h"
#include <wsjcpp_core.h>
#include <algorithm>
#include <sqlite3.h>

// ---------------------------------------------------------------------
// DatabaseFileUpdateInfo

DatabaseFileUpdateInfo::DatabaseFileUpdateInfo(
  const std::string &sVersionFrom, const std::string &sVersionTo, const std::string &sDescription
)
  : m_sVersionFrom(sVersionFrom), m_sVersionTo(sVersionTo), m_sDescription(sDescription) {}

const std::string &DatabaseFileUpdateInfo::versionFrom() const { return m_sVersionFrom; }

const std::string &DatabaseFileUpdateInfo::versionTo() const { return m_sVersionTo; }

const std::string &DatabaseFileUpdateInfo::description() const { return m_sDescription; }

// ---------------------------------------------------------------------
// DatabaseFileUpdate

DatabaseFileUpdate::DatabaseFileUpdate(
  const std::string &sVersionFrom, const std::string &sVersionTo, const std::string &sDescription
)
  : m_updateInfo(sVersionFrom, sVersionTo, sDescription) {}

const DatabaseFileUpdateInfo &DatabaseFileUpdate::info() { return m_updateInfo; };

void DatabaseFileUpdate::setWeight(int nWeight) { m_nWeight = nWeight; }

int DatabaseFileUpdate::getWeight() { return m_nWeight; }

// ---------------------------------------------------------------------
// DatabaseSelectRows

DatabaseSelectRows::DatabaseSelectRows() { m_pQuery = nullptr; }

DatabaseSelectRows::~DatabaseSelectRows() {
  if (m_pQuery != nullptr) {
    sqlite3_finalize((sqlite3_stmt *)m_pQuery);
  }
}

void DatabaseSelectRows::setQuery(void *pQuery) { m_pQuery = pQuery; }

bool DatabaseSelectRows::next() { return sqlite3_step((sqlite3_stmt *)m_pQuery) == SQLITE_ROW; }

std::string DatabaseSelectRows::getString(int nColumnNumber) {
  return std::string((const char *)sqlite3_column_text((sqlite3_stmt *)m_pQuery, nColumnNumber));
}

long DatabaseSelectRows::getLong(int nColumnNumber) {
  return sqlite3_column_int64((sqlite3_stmt *)m_pQuery, nColumnNumber);
}

// ---------------------------------------------------------------------
// DatabaseFile

DatabaseFile::DatabaseFile(const std::string &sFilename) {
  TAG = "DatabaseFile-" + sFilename;
  m_pDatabaseFile = nullptr;
  m_sFilename = sFilename;
  m_nLastBackupTime = 0;
  EmployConfig *pConfig = findWsjcppEmploy<EmployConfig>();
  std::string sDatabaseDir = pConfig->getDatabaseDir();
  m_sFileFullpath = sDatabaseDir + "/" + m_sFilename;

  std::string sDatabaseBackupDir = sDatabaseDir + "/backups";
  if (!WsjcppCore::dirExists(sDatabaseBackupDir)) {
    !WsjcppCore::makeDir(sDatabaseBackupDir);
  }
  m_sBaseFileBackupFullpath = sDatabaseBackupDir + "/" + m_sFilename;
};

DatabaseFile::~DatabaseFile() {
  if (m_pDatabaseFile != nullptr) {
    sqlite3 *db = (sqlite3 *)m_pDatabaseFile;
    sqlite3_close(db);
  }
}

std::string DatabaseFile::getFilename() { return m_sFilename; }

std::string DatabaseFile::getFileFullpath() { return m_sFileFullpath; }

bool DatabaseFile::open() {
  // TODO if ccould not open but has backup try open backup
  // open connection to a DB
  sqlite3 *db = (sqlite3 *)m_pDatabaseFile;
  int nRet = sqlite3_open_v2(m_sFileFullpath.c_str(), &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
  if (nRet != SQLITE_OK) {
    WsjcppLog::throw_err(TAG, "Failed to open conn: " + std::to_string(nRet));
    return false;
  }
  m_pDatabaseFile = db;

  const std::string sSqlCheckVersionTable =
    "SELECT count(*) FROM sqlite_master WHERE type='table' AND name='db_version';";

  int nCount = selectSumOrCount(sSqlCheckVersionTable.c_str());
  if (nCount == 0) {
    // create db_version
    const std::string sSqlCreateDbVersion = "CREATE TABLE IF NOT EXISTS db_version ( "
                                            "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                            "  version_from VARCHAR(64),"
                                            "  version_to VARCHAR(64),"
                                            "  dt INTEGER NOT NULL,"
                                            "  description VARCHAR(2048) NOT NULL"
                                            ");";
    char *zErrMsg = 0;
    nRet = sqlite3_exec(db, sSqlCreateDbVersion.c_str(), 0, 0, &zErrMsg);
    if (nRet != SQLITE_OK) {
      WsjcppLog::throw_err(TAG, "Problem with create table: " + std::string(zErrMsg));
      return false;
    }
    WsjcppLog::info(TAG, "Created table db_version in " + m_sFileFullpath);
  }

  if (!installUpdates()) {
    WsjcppLog::throw_err(TAG, "Problem with install updates");
    return false;
  }

  WsjcppLog::ok(TAG, "Opened database file " + m_sFileFullpath);
  copyDatabaseToBackup();
  return true;
}

bool DatabaseFile::executeQuery(std::string sSqlInsert) {
  copyDatabaseToBackup();
  char *zErrMsg = 0;
  sqlite3 *db = (sqlite3 *)m_pDatabaseFile;
  int nRet = sqlite3_exec(db, sSqlInsert.c_str(), 0, 0, &zErrMsg);
  if (nRet != SQLITE_OK) {
    WsjcppLog::throw_err(TAG, "Problem with insert: " + std::string(zErrMsg) + "\n SQL-query: " + sSqlInsert);
    return false;
  }
  return true;
}

int DatabaseFile::selectSumOrCount(std::string sSqlSelectCount) {
  // copyDatabaseToBackup();
  sqlite3 *db = (sqlite3 *)m_pDatabaseFile;
  sqlite3_stmt *pQuery = nullptr;
  int ret = sqlite3_prepare_v2(db, sSqlSelectCount.c_str(), -1, &pQuery, NULL);
  // prepare the statement
  if (ret != SQLITE_OK) {
    WsjcppLog::throw_err(
      TAG, "Failed to prepare select count: " + std::string(sqlite3_errmsg(db)) + "\n SQL-query: " + sSqlSelectCount
    );
  }
  // step to 1st row of data
  ret = sqlite3_step(pQuery);
  if (ret != SQLITE_ROW) { // see documentation, this can return more values as success
    WsjcppLog::throw_err(
      TAG,
      "Failed to step for select count or sum: " + std::string(sqlite3_errmsg(db)) + "\n SQL-query: " + sSqlSelectCount
    );
  }
  int nRet = sqlite3_column_int(pQuery, 0);
  if (pQuery != nullptr)
    sqlite3_finalize(pQuery);
  return nRet;
}

bool DatabaseFile::selectRows(std::string sSqlSelectRows, DatabaseSelectRows &selectRows) {
  // copyDatabaseToBackup();
  sqlite3 *db = (sqlite3 *)m_pDatabaseFile;
  sqlite3_stmt *pQuery = nullptr;
  int nRet = sqlite3_prepare_v2(db, sSqlSelectRows.c_str(), -1, &pQuery, NULL);
  // prepare the statement
  if (nRet != SQLITE_OK) {
    WsjcppLog::throw_err(
      TAG, "Failed to prepare select rows: " + std::string(sqlite3_errmsg(db)) + "\n SQL-query: " + sSqlSelectRows
    );
    return false;
  }
  selectRows.setQuery((void *)pQuery);
  return true;
}

bool DatabaseFile::installUpdates() {
  sqlite3 *db = (sqlite3 *)m_pDatabaseFile;

  // Installed updates
  std::vector<DatabaseFileUpdateInfo> installedUpdates;
  {
    sqlite3_stmt *pQuery = nullptr;
    const std::string sSqlCurrentVersion =
      "SELECT version_from, version_to, description FROM db_version ORDER BY rowid";
    int ret = sqlite3_prepare_v2(db, sSqlCurrentVersion.c_str(), -1, &pQuery, NULL);
    if (ret != SQLITE_OK) {
      WsjcppLog::throw_err(
        TAG, "Failed to prepare: " + std::string(sqlite3_errmsg(db)) + "\n SQL-query: " + sSqlCurrentVersion
      );
      return false;
    }
    ret = sqlite3_step(pQuery);
    while (ret == SQLITE_ROW) {
      DatabaseFileUpdateInfo info(
        std::string((const char *)sqlite3_column_text(pQuery, 0)), // from
        std::string((const char *)sqlite3_column_text(pQuery, 1)), // to
        std::string((const char *)sqlite3_column_text(pQuery, 2))  // secr
      );
      installedUpdates.push_back(info);
      ret = sqlite3_step(pQuery);
    }
    if (pQuery != nullptr)
      sqlite3_finalize(pQuery);
  }

  std::vector<std::string> installedVersionsTo;
  installedVersionsTo.push_back(""); // start version
  for (int i = 0; i < installedUpdates.size(); i++) {
    installedVersionsTo.push_back(installedUpdates[i].versionTo());
  }

  // install updates
  bool bInstalledNewUpdates = true;
  while (bInstalledNewUpdates) {
    bInstalledNewUpdates = false;
    std::vector<std::string> installedNewUpdates;

    for (const auto& upd : m_vDbUpdates) {
      // DatabaseFileUpdate *pUpdate = m_vDbUpdates[i];
      const std::string &sVersionFrom = upd->info().versionFrom();
      const std::string &sVersionTo = upd->info().versionTo();

      for (int iv = 0; iv < installedVersionsTo.size(); iv++) {
        if (sVersionFrom == installedVersionsTo[iv]) {
          if (std::find(installedVersionsTo.begin(), installedVersionsTo.end(), sVersionTo) == installedVersionsTo.end()) {
            if (!upd->applyUpdate(this)) {
              return false;
            }
            if (!insertDbVersion(upd->info())) {
              return false;
            }
            WsjcppLog::ok(TAG, "Installed update " + sVersionTo);
            installedNewUpdates.push_back(sVersionTo);
          } else {
            // skip update
          }
        }
      }
    }
    bInstalledNewUpdates = installedNewUpdates.size() > 0;
    for (int i = 0; i < installedNewUpdates.size(); i++) {
      installedVersionsTo.push_back(installedNewUpdates[i]);
    }
  }

  return true;
}

bool DatabaseFile::insertDbVersion(const DatabaseFileUpdateInfo &info) {
  // TODO escaping
  std::string sSqlDbVersion = "INSERT INTO db_version(version_from, version_to, description, dt) VALUES(\"" +
                              info.versionFrom() + "\", \"" + info.versionTo() + "\", \"" + info.description() +
                              "\", " + std::to_string(WsjcppCore::getCurrentTimeInMilliseconds()) + ")";
  return this->executeQuery(sSqlDbVersion);
}

void DatabaseFile::copyDatabaseToBackup() {
  std::lock_guard<std::mutex> lock(m_mutex);
  // every 1 minutes make backup
  int nCurrentTime = WsjcppCore::getCurrentTimeInSeconds();
  if (nCurrentTime - m_nLastBackupTime < 60) {
    return;
  }
  m_nLastBackupTime = nCurrentTime;

  int nMaxBackupsFiles = 9;
  WsjcppLog::info(TAG, "Start backup for " + m_sFileFullpath);
  std::string sFilebackup = m_sBaseFileBackupFullpath + "." + std::to_string(nMaxBackupsFiles);
  if (WsjcppCore::fileExists(sFilebackup)) {
    WsjcppCore::removeFile(sFilebackup);
  }
  for (int i = nMaxBackupsFiles - 1; i >= 0; i--) {
    std::string sFilebackupFrom = m_sBaseFileBackupFullpath + "." + std::to_string(i);
    std::string sFilebackupTo = m_sBaseFileBackupFullpath + "." + std::to_string(i + 1);
    if (WsjcppCore::fileExists(sFilebackupFrom)) {
      if (std::rename(sFilebackupFrom.c_str(), sFilebackupTo.c_str())) {
        WsjcppLog::throw_err(TAG, "Could not rename from " + sFilebackupFrom + " to " + sFilebackupTo);
      }
    }
  }
  sFilebackup = m_sBaseFileBackupFullpath + "." + std::to_string(0);
  if (!WsjcppCore::copyFile(m_sFileFullpath, sFilebackup)) {
    WsjcppLog::throw_err(TAG, "Failed copy file to backup for " + m_sFileFullpath);
  }
  WsjcppLog::info(TAG, "Backup done for " + m_sFileFullpath);
}
