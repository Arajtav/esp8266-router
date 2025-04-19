#include <ESP8266WebServer.h>
#include "webUI.hpp"
#include "utils.hpp"
#include "logger.hpp"

ESP8266WebServer server(80);

// TODO: client IP
#define LOG_REQUEST(code) (logger.log(LL_INFO, "`" + server.uri() + "` requested, responded with " + String(code)))

#define FAIL_ON_EMPTY(data) if (!data.length()) { \
                                server.send(500, "text/plain", "Something went wrong."); \
                                LOG_REQUEST(500); \
                                return; \
                            }

// TODO: actual login
void handle_root(void) {
    String data = readFile("/webUI/login.html");
    FAIL_ON_EMPTY(data);
    server.send(200, "text/html", data);
    LOG_REQUEST(200);
}

void init_web_ui(void) {
    server.on("/", handle_root);
    server.onNotFound([]() {
        server.send(404, "text/plain", "404");
        LOG_REQUEST(404);
    });
    server.begin();
}
