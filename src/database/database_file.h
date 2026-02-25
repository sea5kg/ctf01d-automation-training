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

#pragma once

#include <cstring>
#include <mutex>
#include <string>
#include <vector>
#include <memory>

class DatabaseFileUpdateInfo {
public:
  DatabaseFileUpdateInfo(
    const std::string &sVersionFrom, const std::string &sVersionTo, const std::string &sDescription
  );
  const std::string &versionFrom() const;
  const std::string &versionTo() const;
  const std::string &description() const;

private:
  std::string m_sVersionFrom;
  std::string m_sVersionTo;
  std::string m_sDescription;
};

class DatabaseFile;

class DatabaseFileUpdate {
public:
  DatabaseFileUpdate(
    const std::string &sVersionFrom, const std::string &sVersionTo, const std::string &sDescription
  );
  const DatabaseFileUpdateInfo &info();
  void setWeight(int nWeight);
  int getWeight();
  virtual bool applyUpdate(DatabaseFile *pDatabaseFile) = 0;

protected:
  std::string TAG;

private:
  DatabaseFileUpdateInfo m_updateInfo;
  int m_nWeight;
};

class DatabaseSelectRows {
public:
  DatabaseSelectRows();
  ~DatabaseSelectRows();
  void setQuery(void *pQuery);
  bool next();
  std::string getString(int nColumnNumber);
  long getLong(int nColumnNumber);

private:
  // hidden type 'sqlite3_stmt *'
  void *m_pQuery;
};

class DatabaseFile {
public:
  DatabaseFile(const std::string &sFilename);
  ~DatabaseFile();
  std::string getFilename();
  std::string getFileFullpath();
  bool open();
  bool executeQuery(std::string sSqlInsert);
  int selectSumOrCount(std::string sSqlSelectCount);
  bool selectRows(std::string sSqlSelectRows, DatabaseSelectRows &selectRows);

protected:
  bool installUpdates();
  bool insertDbVersion(const DatabaseFileUpdateInfo &info);
  std::vector<std::shared_ptr<DatabaseFileUpdate>> m_vDbUpdates;
  std::string TAG;

private:
  void copyDatabaseToBackup();
  std::mutex m_mutex;

  // hidden type 'sqlite3 *'
  void *m_pDatabaseFile;
  std::string m_sFilename;
  std::string m_sFileFullpath;
  std::string m_sBaseFileBackupFullpath;
  int m_nLastBackupTime;
};
