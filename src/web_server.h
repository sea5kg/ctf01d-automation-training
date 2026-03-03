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

#include "web_errors.h"
#include "request_handler.h"


#include <string>
#include <json.hpp>
#include <memory>
#include "HttpService.h"
#include <employ_config.h>
#include <employ_users.h>
#include <employ_database.h>

class Ctf01dRequestResponse : public ctf01d::HandleContext {
public:
  Ctf01dRequestResponse(HttpResponse* resp);
  virtual int response(int ret_http_code, const nlohmann::json &resp_json) override;
private:
  HttpResponse* m_resp;
};

class WebServer {
public:
  WebServer();
  hv::HttpService *getService();
  int httpGetRequests(HttpRequest* req, HttpResponse* resp);
  int httpApiRequests(HttpRequest* req, HttpResponse* resp);

private:
  std::string normalizeRequestPath(HttpRequest* req);

  int rating(std::shared_ptr<ctf01d::HandleContext> context);
  int signup(std::shared_ptr<ctf01d::HandleContext> context);
  int flag(std::shared_ptr<ctf01d::HandleContext> context);

  std::string TAG;
  hv::HttpService *m_pHttpService;
  EmployConfig *m_config;
  EmployDatabase *m_db;
  EmployUsers *m_users;

  std::string m_sHtmlFolder;
  std::map<std::string, std::shared_ptr<ctf01d::RequestHandler>> m_handlers;
};
