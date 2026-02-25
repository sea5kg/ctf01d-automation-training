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

// https://github.com/sea5kg/gtree

#pragma once

#include "gt_errors.h"
#include <string>
#include <json.hpp>

namespace gtree {

class HandleContext {
public:
  // implement send and failed
  virtual int response(int ret_http_code, const nlohmann::json &resp_json) = 0;

  // setters / getters
  const std::string &methodName();
  const std::string &getMessageId();
  void setAuth(const std::string &auth);
  const std::string &getAuth();

  bool parseBodyAndCheck(const std::string &body, std::shared_ptr<gtree::ErrorInfo> &error);
  const nlohmann::json &requestBody();

  // other method
  int success(const nlohmann::json &result);
  int failed(int ret_http_code, const ErrorInfo &error);
  int error400(const ErrorInfo &error);
  int error400(std::shared_ptr<gtree::ErrorInfo> error);
  int error401(const ErrorInfo &error);
  int error401(std::shared_ptr<gtree::ErrorInfo> error);
  int error403(const ErrorInfo &error);
  int error403(std::shared_ptr<gtree::ErrorInfo> error);
  int error404(const ErrorInfo &error);
  int error404(std::shared_ptr<gtree::ErrorInfo> error);

private:
  nlohmann::json prepareResponseJson();

  std::string m_msg_id;
  std::string m_method_name;
  std::string m_auth;
  nlohmann::json m_req_body;
};

class RequestHandler {
public:
  RequestHandler(const std::string &method_name);
  const std::string &method();
  virtual int handle(const nlohmann::json &req, std::shared_ptr<HandleContext> resp) {
    return 0;
  };

private:
  std::string m_method_name;
};

} // namespace gtree
