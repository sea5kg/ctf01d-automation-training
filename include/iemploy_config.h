#pragma once

#include <string>

class IEmployConfig {
public:
  static std::string name() { return "IEmployConfig"; }

  virtual const std::string &getWorkDir() = 0;
  virtual int getWebPort() = 0;
  virtual const std::string &getDatabaseDir() = 0;
  virtual const std::string &getLogDir() = 0;
  virtual const std::string &getWebDir() = 0;

  virtual int startTimeTraining() = 0;
  virtual int endTimeTraining() = 0;
};
