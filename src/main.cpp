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

#include <wsjcpp_core.h>
#include <employ_config.h>
#include "web_server.h"
#include "WebSocketServer.h"  // libhv

int main(int argc, const char* argv[]) {
    WsjcppCore::initRandom();
    WsjcppLog::setEnableLogFile(false);

    WsjcppEmployeesInit empls({}, false);
    if (!empls.inited) {
        return -1;
    }

    EmployConfig *pConfig = findWsjcppEmploy<EmployConfig>();

    WsjcppLog::info("main", "Start web server on http://localhost:" + std::to_string(pConfig->getWebPort()));
    WebServer httpServer;
    hv::HttpService *pRouter = httpServer.getService();
    hv::HttpServer server(pRouter);
    server.setPort(pConfig->getWebPort());
    server.setThreadNum(1);
    server.run();

    // getLogDir()

    // websocket_server_t server;
    // server.service = pRouter;
    // server.port = 12345;
    // // server.ws = pWs;
    // websocket_server_run(&server);

    return 0;
}
