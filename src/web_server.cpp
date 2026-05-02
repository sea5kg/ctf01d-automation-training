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

#include "web_server.h"

// #include "WebSocketServer.h"
#include "EventLoop.h"
#include "htime.h"
#include "hssl.h"
#include "hlog.h"
#include <regex>
#include <wsjcpp_core.h>

using namespace hv;


Ctf01dRequestResponse::Ctf01dRequestResponse(HttpResponse* resp) : m_resp(resp) {

};

int Ctf01dRequestResponse::response(int ret_http_code, const nlohmann::json &resp_json) {
  std::string text = resp_json.dump();
  m_resp->Data(
    (void *)(text.c_str()),
    text.length(),
    false, // nocopy - force copy
    APPLICATION_JSON
  );
  return ret_http_code;
}

WebServer::WebServer() {
  TAG = "WebServer";
  m_config = findWsjcppEmploy<IEmployConfig>();
  m_users = findWsjcppEmploy<EmployUsers>();
  m_flags = findWsjcppEmploy<EmployFlags>();
  m_db = findWsjcppEmploy<EmployDatabase>();

  {
    logger_t* pLogger = hv_default_logger();
    // logger_set_max_filesize(pLogger, 102400);
    std::string sLogDirPath = m_config->getLogDir() + "/hv";
    if (!WsjcppCore::dirExists(sLogDirPath)) {
        WsjcppCore::makeDir(sLogDirPath);
    }
    std::string sLogFilePath = sLogDirPath + "/http_" + WsjcppCore::getCurrentTimeForFilename() + ".log";
    logger_set_file(pLogger, sLogFilePath.c_str());
  }
  // m_sTeamLogoPrefix = "/team-logo/";
  // m_nTeamLogoPrefixLength = m_sTeamLogoPrefix.size();

  m_pHttpService = new HttpService();

  // static files
  m_pHttpService->document_root = m_config->getWebDir();
  m_sHtmlFolder = m_config->getWebDir();

  m_pHttpService->GET("*", std::bind(&WebServer::httpGetRequests, this, std::placeholders::_1, std::placeholders::_2));
  m_pHttpService->POST("*", std::bind(&WebServer::httpApiRequests, this, std::placeholders::_1, std::placeholders::_2));
}

hv::HttpService *WebServer::getService() {
    return m_pHttpService;
}

int WebServer::httpGetRequests(HttpRequest* req, HttpResponse* resp) {

  // remove get params from path
  std::string sRequestPath = normalizeRequestPath(req);

  if (sRequestPath == "/api" || sRequestPath.rfind("/api/", 0) == 0) {
    return httpApiRequests(req, resp);
  }

  if (sRequestPath == "/") {
    sRequestPath = "/index.html";
  }

  if (sRequestPath == "/admin") {
    return resp->Redirect("/admin/");
  }

  if (sRequestPath == "/admin/") {
    sRequestPath = "/admin/index.html";
  }

  // TODO
  WsjcppLog::info(TAG, "Request path: " + sRequestPath);
  std::string sFilePath = sRequestPath = WsjcppCore::doNormalizePath(m_sHtmlFolder + "/" + sRequestPath);
  if (WsjcppCore::fileExists(sFilePath)) { // TODO check the file exists not dir
    return resp->File(sFilePath.c_str());
  }

  // TODO cache
  std::string sResPath = "./data_sample/html" + sRequestPath;
  if (WsjcppResourcesManager::has(sResPath)) {
    WsjcppResourceFile *pFile = WsjcppResourcesManager::get(sResPath);
    resp->SetContentTypeByFilename(sResPath.c_str());
    return resp->Data((void *)pFile->getBuffer(), pFile->getBufferSize(), true, resp->content_type);
  }
  return 404; // Not found
}

int WebServer::httpApiRequests(HttpRequest* req, HttpResponse* resp) {
  auto context = std::make_shared<Ctf01dRequestResponse>(resp);
  if (req->method != HTTP_POST && req->method != HTTP_GET) {
    return context->error403(ERR_01001_ONLY_POST_OR_GET_REQUESTS);
  }

  std::string sRequestPath = normalizeRequestPath(req);
  WsjcppLog::info(TAG, "sRequestPath " + sRequestPath);

  // auto now = std::chrono::system_clock::now().time_since_epoch();
  // int nCurrentTimeSec = std::chrono::duration_cast<std::chrono::seconds>(now).count();

  // std::string auth = req->GetHeader("Authorization");
  // context->setAuth(auth);
  // WsjcppLog::info(TAG, "auth = " + auth);

  std::shared_ptr<ctf01d::ErrorInfo> error;
  if (req->method == HTTP_POST && !context->parseBodyAndCheck(req->body, error)) {
    return context->error400(error);
  }
  if (req->method == HTTP_GET) {
    nlohmann::json params;
    for (auto it = req->query_params.begin(); it != req->query_params.end(); ++it) {
      params[it->first] = it->second;
    }
    nlohmann::json request;
    request["jsonrpc"] = "2.0";
    request["params"] = params;

    if (sRequestPath == "/api/v1/signup/") {
      context->setMethodName("signup");
      request["method"] = "signup";
    }
    if (sRequestPath == "/api/v1/rating/") {
      context->setMethodName("rating");
      request["method"] = "rating";
    }

    if (sRequestPath == "/api/v1/flag/") {
      context->setMethodName("flag");
      request["method"] = "flag";
    }
    context->setRequestBody(request);
  }

  if (context->methodName() == "signup") {
    return signup(context);
  } else if (context->methodName() == "rating") {
    return rating(context);
  } else if (context->methodName() == "flag") {
    return flag(context);
  }

  return context->error404(ERR_01006_UNKNOWN_METHOD);
}

std::string WebServer::normalizeRequestPath(HttpRequest* req) {
  std::string sOriginalRequestPath = req->path;
  std::string sRequestPath;
  // remove get params from path
  std::size_t nFoundGetParams = sOriginalRequestPath.rfind("?");
  if (nFoundGetParams != std::string::npos) {
    sRequestPath = sOriginalRequestPath.substr(0, nFoundGetParams);
  } else {
    sRequestPath = sOriginalRequestPath;
  }
  sRequestPath = WsjcppCore::doNormalizePath(sRequestPath);
  return sRequestPath;
}

int WebServer::rating(std::shared_ptr<ctf01d::HandleContext> context) {
  nlohmann::json result = m_users->rating();
  return context->success(result);
}

int WebServer::signup(std::shared_ptr<ctf01d::HandleContext> context) {
  const nlohmann::json req = context->requestBody();

  if (!req["params"].is_object()) {
    return context->error400(ERR_01007_MISSING_OR_WRONG_FIELD_PARAMS);
  }

  if (!req["params"]["username"].is_string()) {
    return context->error400(ERR_10015_MISSING_FIELD_USERNAME);
  }

  std::string username = req["params"]["username"];

  if (username.length() < 3) {
    return context->error400(ERR_10030_USERNAME_TOO_SHORT.replace("$username$", username));
  }

  if (username.length() > 10) {
    return context->error400(ERR_10031_USERNAME_TOO_LONG.replace("$username$", username));
  }

  std::string secret_token;
  std::shared_ptr<ctf01d::ErrorInfo> error;
  if (!m_users->createUser(username, secret_token, error)) {
    return context->error403(error);
  }

  nlohmann::json result;
  result["username"] = username;
  result["secret_token"] = secret_token;
  return context->success(result);
}

int WebServer::flag(std::shared_ptr<ctf01d::HandleContext> context) {
  auto now = std::chrono::system_clock::now().time_since_epoch();
  long nCurrentTimeMSec = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
  int nCurrentTimeSec = nCurrentTimeMSec / 1000;
  // std::string sRequestIP = req->client_addr.ip;
  // std::string sRequestIP_MsgSuffex = " (" + sRequestIP + ")";

  if (nCurrentTimeSec < m_config->startTimeTrainingInSec()) {
    return context->error400(ERR_10036_TRAINING_NOT_STARTED_YET);
  }

  if (nCurrentTimeSec > m_config->endTimeTrainingInSec()) {
    return context->error400(ERR_10037_TRAINING_ALREADY_ENDED);
  }

  const nlohmann::json req = context->requestBody();

  if (!req["params"].is_object()) {
    return context->error400(ERR_01007_MISSING_OR_WRONG_FIELD_PARAMS);
  }

  if (!req["params"]["flag"].is_string()) {
    return context->error400(ERR_10016_MISSING_FIELD_FLAG);
  }

  if (!req["params"]["token"].is_string()) {
    return context->error400(ERR_10017_MISSING_FIELD_TOKEN);
  }

  // validate token

  std::string token = req["params"]["token"];
  token = WsjcppCore::trim(token);

  if (token.length() < 3) {
    return context->error400(ERR_10032_TOKEN_TOO_SHORT.replace("$token$", token));
  }

  if (token.length() > 10) {
    return context->error400(ERR_10033_TOKEN_TOO_LONG.replace("$token$", token));
  }

  // validate flag

  std::string flagValue = req["params"]["flag"];
  flagValue = WsjcppCore::trim(flagValue);
  flagValue = WsjcppCore::toLower(flagValue);

  if (flagValue.length() < 3) {
    // TODO failed requests
    return context->error400(ERR_10034_FLAG_TOO_SHORT.replace("$flag$", flagValue));
  }

  if (flagValue.length() > 36) {
    // TODO failed requests
    return context->error400(ERR_10035_FLAG_TOO_LONG.replace("$flag$", flagValue));
  }

  std::string username = m_users->findUserByToken(token);
  if (username == "") {
    // TODO failed requests
    return context->error404(ERR_10025_USERNAME_BY_TOKEN_NOT_FOUND.replace("$token$", token));
  }

  if (m_db->dbUserTries()->findUserFlag(username, flagValue)) {
    m_db->dbUserTries()->addUserFlag(username, flagValue, -1);
    m_users->updateUserPenaltyAndTries(username);
    return context->error400(ERR_10038_YOU_ALREADY_TRIED_THIS_FLAG.replace("$flag$", flagValue));
  }

  m_db->dbUserTries()->addUserFlag(username, flagValue, 0); // TODO if flag success -> 1
  m_users->updateUserTries(username);

  const static std::regex reFlagFormat("c01d[a-f0-9]{4,4}-[a-f0-9]{4,4}-[a-f0-9]{4,4}-[a-f0-9]{4,4}-[a-f0-9]{4,4}[0-9]{8,8}");
  if (!std::regex_match(flagValue, reFlagFormat)) {
    return context->error400(ERR_10039_FLAG_HAS_WRONG_FORMAT.replace("$flag$", flagValue));
  }

  Ctf01dFlag flag;
  if (!m_flags->findFlagLive(flagValue, flag)) {
    return context->error400(ERR_10040_FLAG_TOO_OLD_OR_NEVER_EXISTS.replace("$flag$", flagValue));
  }

  if (flag.getTimeEndInMs() < nCurrentTimeMSec) {
    return context->error400(ERR_10041_FLAG_TOO_OLD.replace("$flag$", flagValue));
  }

  m_users->incrementUserAttack(username);

  nlohmann::json result;
  result["username"] = username;
  // result["secret_token"] = secret_token;
  return context->success(result);
}
