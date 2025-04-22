#include <BearSSLHelpers.h>
#include <ESP8266WebServerSecure.h>
#include "webUI.hpp"
#include "utils.hpp"
#include "logger.hpp"
#include "settings.hpp"

BearSSL::ESP8266WebServerSecure server(443);
BearSSL::ServerSessions serverCache(4);

// TODO: client IP
#define SEND_LOG_RETURN(code, contentType, content) { \
                                                        server.send(code, contentType, content); \
                                                        logger.log(LL_INFO, "`" + server.uri() + "` requested, responded with " + String(code)); \
                                                        return; \
                                                    }
#define FAIL_ON_EMPTY(data) if (!data.length()) SEND_LOG_RETURN(500, "text/plain", "Something went wrong.");
#define AUTH_AND_RETURN(string) if (!is_authenticated()) { \
                                    FAIL_ON_EMPTY(string); \
                                    SEND_LOG_RETURN(401, "text/html", string); \
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

String login;

void handle_root(void) {
    AUTH_AND_RETURN(login);
    SEND_LOG_RETURN(200, "text/plain", "WIP");
}

void handle_log(void) {
    AUTH_AND_RETURN(login);
    File file = LittleFS.open("/log.txt", "r");
    if (!file) SEND_LOG_RETURN(404, "text/plain", "File not found.");

    server.streamFile(file, "text/plain; charset=utf-8");
    file.close();
    logger.log(LL_INFO, "`/log` requested, responded with 200");
}

void init_web_ui(void) {
    logger.log(LL_INFO, "starting webUI");

    String private_key = readFile("/private/private.key");
    if (!private_key.length()) esp_exit("failed to load private key");
    String server_cert = readFile("/private/server.crt");
    if (!private_key.length()) esp_exit("failed to load server certificate");

    login = readFile("/webUI/login.html");

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
