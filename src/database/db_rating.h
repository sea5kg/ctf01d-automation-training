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

#include "database_file.h"

#include <map>

class DbRating : public DatabaseFile {
public:
  DbRating();
  ~DbRating();

  std::pair<std::string, std::string> findUserByNameAndPass(const std::string &name, const std::string &pass);
  std::string findUserUuid(const std::string &name);
  bool createUser(const std::string &uuid, const std::string &name, const std::string &role, const std::string &pass);
  bool removeUser(const std::string &uuid);
  bool changeUserPassword(const std::string &uuid, const std::string &pass, std::string &error);

private:
  std::string findSoltByUuid(const std::string &uuid);
  std::string createRandomSolt();


  std::mutex m_mutex;
  std::string TAG;
};
