#include <BearSSLHelpers.h>
#include <ESP8266WebServerSecure.h>
#include "webUI.hpp"
#include "utils.hpp"
#include "logger.hpp"

BearSSL::ESP8266WebServerSecure server(443);
BearSSL::ServerSessions serverCache(4);

// TODO: client IP
#define LOG_REQUEST(code) (logger.log(LL_INFO, "`" + server.uri() + "` requested, responded with " + String(code)))

#define FAIL_ON_EMPTY(data) if (!data.length()) { \
                                server.send(500, "text/plain", "Something went wrong."); \
                                LOG_REQUEST(500); \
                                return; \
                            }

// TODO: actual login
void handle_root(void) {
    static String login = readFile("/webUI/login.html");
    FAIL_ON_EMPTY(login);
    server.send(200, "text/html", login);
    LOG_REQUEST(200);
}

void init_web_ui(void) {
    logger.log(LL_INFO, "starting webUI");

    String private_key = readFile("/private/private.key");
    if (!private_key.length()) esp_exit("failed to load private key");
    String server_cert = readFile("/private/server.crt");
    if (!private_key.length()) esp_exit("failed to load server certificate");

    server.getServer().setRSACert(new BearSSL::X509List(server_cert.c_str()), new BearSSL::PrivateKey(private_key.c_str()));
    server.getServer().setCache(&serverCache);

    server.on("/", handle_root);
    server.onNotFound([]() {
        server.send(404, "text/plain", "404");
        LOG_REQUEST(404);
    });

    server.begin();
    logger.log(LL_INFO, "webUI ready!");
}
