#include "build_info.h"
#include <BearSSLHelpers.h>
#include <ESP8266WebServerSecure.h>
#include "webUI.hpp"
#include "utils.hpp"
#include "logger.hpp"
#include "settings.hpp"

BearSSL::ESP8266WebServerSecure server(443);
BearSSL::ServerSessions serverCache(1);

// when possible, stream files to not OOM
// TODO: clean all of that up
#define SEND_LOG_RETURN(code, contentType, content) { \
                                                        server.send(code, contentType, content); \
                                                        logger.log(LL_INFO, "`" + server.uri() + "` requested, responded with " + String(code)); \
                                                        return; \
                                                    }
#define FAIL_ON_EMPTY(data) if (!data.length()) SEND_LOG_RETURN(500, "text/plain", "Something went wrong.");
#define STREAM_LOG_RETURN(fn, type) { \
                                    File file = LittleFS.open(fn, "r"); \
                                    if (!file) { \
                                        logger.log(LL_ERROR, String("Failed to open file `") + fn + "`"); \
                                        SEND_LOG_RETURN(500, "text/plain", "Something went wrong."); \
                                    } \
                                    server.streamFile(file, type); \
                                    file.close(); \
                                    logger.log(LL_INFO, "`" + server.uri() + "` requested, responded with 200"); \
                                    return; \
                                }
#define FAIL_ON_AUTH_MISS() if (!is_authenticated()) { \
                                String login = readFile("/webUI/login.html"); \
                                FAIL_ON_EMPTY(login); \
                                SEND_LOG_RETURN(401, "text/html", login); \
                            }

String active_session_token = ""; // currently only one session at a time

void handle_login(void) {
    String username = server.arg("username");
    String password = server.arg("password");
    logger.log(LL_INFO, "Login request for: `" + username + "`");
    if (username != admin_username || password != admin_password) SEND_LOG_RETURN(403, "text/plain", "Invalid credentials.");

    active_session_token = randomHex();
    server.sendHeader("Set-Cookie", "session=" + active_session_token + "; Path=/; SameSite=Strict; HttpOnly; Secure");
    SEND_LOG_RETURN(200, "text/plain", "Logged in.");
}

bool is_authenticated() {
    String cookie = server.header("Cookie");

    int start = cookie.indexOf("session=");
    if (start == -1) return false;
    start += 8;

    int end = cookie.indexOf(';', start);

    return cookie.substring(start, end == -1 ? cookie.length() : end) == active_session_token;
}

void handle_root(void) {
    FAIL_ON_AUTH_MISS();
    String index_template = readFile("/webUI/index.html");
    FAIL_ON_EMPTY(index_template);
    index_template.replace("_REPLACE_VERSION_", VERSION);
    SEND_LOG_RETURN(200, "text/html", index_template);
}

void handle_log(void) {
    FAIL_ON_AUTH_MISS();
    STREAM_LOG_RETURN("/log.txt", "text/plain; charset=utf-8");
}

void init_web_ui(void) {
    logger.log(LL_INFO, "starting webUI");

    String private_key = readFile("/private/private.key");
    if (!private_key.length()) esp_exit("failed to load private key");
    String server_cert = readFile("/private/server.crt");
    if (!private_key.length()) esp_exit("failed to load server certificate");

    server.getServer().setRSACert(new BearSSL::X509List(server_cert.c_str()), new BearSSL::PrivateKey(private_key.c_str()));
    server.getServer().setCache(&serverCache);

    server.on("/", HTTP_GET, handle_root);
    server.on("/login", HTTP_POST, handle_login);
    server.on("/log", HTTP_GET, handle_log);
    server.onNotFound([]() SEND_LOG_RETURN(404, "text/plain", "Not found."));

    const char* headerkeys[] = { "Cookie" };
    server.collectHeaders(headerkeys, sizeof(headerkeys) / sizeof(*headerkeys));
    server.begin();
    logger.log(LL_INFO, "webUI ready!");
}
