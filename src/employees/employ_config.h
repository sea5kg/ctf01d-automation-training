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

// https://github.com/sea5kg/ctf01d-automation-training

#pragma once

#include "iemploy_config.h"

#include <wsjcpp_employees.h>
#include <wsjcpp_yaml.h>

class EmployConfig : public WsjcppEmployBase, public IEmployConfig {
public:
  EmployConfig();
  ~EmployConfig();

  virtual bool init(const std::string &sName, bool bSilent) override;
  virtual bool deinit(const std::string &sName, bool bSilent) override;

  virtual const std::string &getWorkDir() override;
  virtual int getWebPort() override;
  virtual const std::string &getDatabaseDir() override;
  virtual const std::string &getLogDir() override;
  virtual const std::string &getWebDir() override;

  virtual int startTimeTrainingInSec() override;
  virtual int endTimeTrainingInSec() override;
  virtual int flagLifeTimeInMin() override;

  virtual const std::string &getCheckerType() override;
  virtual const std::string &getCheckerTargetHost() override;
  virtual const std::string &getCheckerWorkDir() override;
  virtual const std::string &getCheckerScriptPath() override;
  virtual int getCheckerScriptWaitInSec() override;
  virtual int getCheckerScriptRoundInSec() override;

  // TODO
  void doExtractFilesIfNotExists();

private:
  bool tryLoadFromEnv(const std::string &sEnvName, std::string &sValue, const std::string &sDescription);
  std::string handleRelatedDirPath(const std::string &sDir, const std::string &sDefault);
  bool initLogging(WsjcppYaml &yamlConfig);
  bool readTimesTraining(const std::string &configFilepath, WsjcppYaml &yamlConfig);
  bool readFlagConfig(const std::string &configFilepath, WsjcppYaml &yamlConfig);
  bool readCheckerConfig(const std::string &configFilepath, WsjcppYaml &yamlConfig);
  std::string secondsToFormattedDateTime(int seconds);
  int formattedDateTimeToSeconds(const std::string &dt);
  void createParamConfigDatetime(WsjcppYaml &yamlConfig, const std::string &name, int value_seconds, const std::string &comment);

  std::string TAG;
  std::string m_sWorkDir;
  std::string m_sLogDir;
  std::string m_sWebDir;
  int m_nWebPort;
  int m_start_time_training_in_sec;
  int m_endTimeTraining;
  std::string m_sDatabaseDir;
  int m_flag_lifetime_in_min = 1;
  std::string m_checker_type = "local-script";
  std::string m_checker_target_host = "localhost";
  std::string m_checker_work_dir = "./checker";
  std::string m_checker_script_path = "./example_checker.py";
  int m_checker_script_wait_in_sec = 5;
  int m_checker_script_round_in_sec = 15;

};
