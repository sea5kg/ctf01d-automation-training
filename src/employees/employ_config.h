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

// https://github.com/sea5kg/ctf01d-automation-training-automation-training

#pragma once

#include <wsjcpp_employees.h>
#include <wsjcpp_yaml.h>

class EmployConfig : public WsjcppEmployBase {
public:
  EmployConfig();
  ~EmployConfig();
  static std::string name() { return "EmployConfig"; }
  virtual bool init(const std::string &sName, bool bSilent) override;
  virtual bool deinit(const std::string &sName, bool bSilent) override;

  const std::string &getWorkDir();
  int getWebPort();
  const std::string &getDatabaseDir();
  const std::string &getLogDir();
  const std::string &getWebDir();

  // TODO
  void doExtractFilesIfNotExists();

private:
  bool tryLoadFromEnv(const std::string &sEnvName, std::string &sValue, const std::string &sDescription);
  std::string handleRelatedDirPath(const std::string &sDir, const std::string &sDefault);
  bool initLogging(WsjcppYaml &yamlConfig);

  std::string TAG;
  std::string m_sWorkDir;
  std::string m_sLogDir;
  std::string m_sWebDir;
  int m_nWebPort;
  int m_startTimeTraining;
  std::string m_sDatabaseDir;
};
