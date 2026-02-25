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

// https://github.com/sea5kg/gtree

#pragma once

#include <wsjcpp_employees.h>

class EmployUuids : public WsjcppEmployBase {
public:
  EmployUuids();
  static std::string name() { return "EmployUuids"; }
  virtual bool init(const std::string &sName, bool bSilent);
  virtual bool deinit(const std::string &sName, bool bSilent) override;

  void addAllowedTypesOfUuid(const std::string &sTypeOfObject);
  const std::vector<std::string> &getAllowedTypesOfUuid();

  std::string generateNewUuid(const std::string &sTypeOfObject);
  bool hasUuid(const std::string &sUuid);
  std::string getTypeOfObject(const std::string &sUuid);
  bool removeUuid(const std::string &sUuid);

private:
  std::mutex m_mutex;
  std::vector<std::string> m_vAllowedTypes;
  std::map<std::string, std::string> m_mapGlobalUuids;
  std::string TAG;
};
