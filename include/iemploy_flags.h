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

#include <string>

class Ctf01dFlag {
public:
  Ctf01dFlag();
  void generateRandomFlag(int nTimeFlagLifeInMin, int nGameStartUTCInSec);

  void generateId();
  void setId(const std::string &sId);
  std::string getId() const;

  void generateValue(int nGameStartUTCInSec);
  void setValue(const std::string &sValue);
  std::string getValue() const;

  void setTimeStartInMs(long nTimeStart);
  long getTimeStartInMs() const;

  void setTimeEndInMs(long nTimeEnd);
  long getTimeEndInMs() const;

  void copyFrom(const Ctf01dFlag &flag);

private:
  std::string m_sId;
  std::string m_sValue;
  long m_nTimeStartInMs;
  long m_nTimeEndInMs;
};

class IEmployFlags {
public:
  static std::string name() { return "IEmployFlags"; }
  virtual bool findFlagLive(const std::string &flagValue, Ctf01dFlag &flag) = 0;
  virtual void startThreadSendFlags() = 0;
  virtual void stopThreadSendFlags() = 0;
};