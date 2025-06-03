#ifdef TEST_BUILD

#include <algorithm>
#include <cstdio>
#include <random>
#include <signal.h>
#include <unistd.h>


#include "base64.h"
#include "config.h"
#include "fmt/format.h"
#include "logic/endpoints.h"
#include "sha256.h"
#include "gtest/gtest.h"
#include <jsoncpp/json/json.h>

extern void __start_func__();

class EndPointsTest : public testing::Test {
public:
  EndPointsTest()
      : testing::Test(), cli("127.0.0.1", config_t::get_config()->http_port()) {
  }

  httplib::Client cli;

protected:
  static void SetUpTestSuite() {
    std::remove(config_t::get_config()->db_filename().c_str());
    uid();
    pid_t srv_pid = fork();

    if (srv_pid == -1) {
      ASSERT_TRUE(false && "Fork fails");
    }

    if (srv_pid == 0) {
      signal(SIGALRM, (void (*)(int))kill_child);
      __start_func__();
    } else {
      sleep(2);
    }
  }

  static void kill_child(int) { exit(0); }

  static void TearDownTestSuite() { alarm(0); }

  static std::array<uint8_t, 32> *uid() {
    if (_uid == nullptr) {
      sha256_t hash;
      hash.update((const uint8_t *)"intx82@gmail.com"
                                   "\x00"
                                   "12345678"
                                   "\x00",
                  26);
      std::array<uint8_t, 32> __uid = hash.digest();
      sqlite_db_t db(config_t::get_config()->db_filename().c_str());
      user_tbl_t users(&db);
      user_t *user = users.insert("Danil Ruban", "intx82@gmail.com", __uid, 1);

      assert(user != nullptr);
      assert(user->name() == "Danil Ruban");
      assert(user->id() == 1);
      assert(user->mail() == "intx82@gmail.com");
      assert(__uid[0] == 0x55 && __uid[1] == 0x88);

      card_tbl_t cards(&db);
      cards.insert(user->id(), 1234567890);

      _uid = new std::array<uint8_t, 32>(__uid);
    }
    return _uid;
  }

  static uint64_t login() {
    httplib::Client cli("127.0.0.1", config_t::get_config()->http_port());
    std::string b64uid = fmt::format(
        "{{ \"user\": \"{}\" }}", base64_encode(uid()->data(), uid()->size()));
    httplib::Result res = cli.Post("/api/login", b64uid, "application/json");
    assert(res);
    assert(res->status == 200);

    Json::Reader reader;
    Json::Value _res_json;
    reader.parse(res->body, _res_json);

    assert(_res_json.isMember("redirect"));
    assert(_res_json.isMember("session"));
    assert(_res_json.isMember("status"));
    assert(_res_json["status"].asUInt() == 200);
    assert(_res_json["session"].asUInt64() > 0);
    return _res_json["session"].asUInt64();
  }

private:
  static std::array<uint8_t, 32> *_uid;
};

std::array<uint8_t, 32> *EndPointsTest::_uid = nullptr;

TEST_F(EndPointsTest, PingTest) {
  httplib::Result res = cli.Get("/api/ping");
  ASSERT_TRUE(res);
  ASSERT_TRUE(res->status == 200);
  ASSERT_TRUE(res->body == "\"pong\"");
}

TEST_F(EndPointsTest, OnUnknownUserLoginTest) {
  httplib::Result res =
      cli.Post("/api/login",
               "{ \"user\": \"dGhpcnR5IHR3byBjaGFyYWN0ZXJzIGxpbmUgYmFzZTY=\" }",
               "application/json");
  ASSERT_TRUE(res);
  ASSERT_TRUE(res->status == 403);
}

TEST_F(EndPointsTest, OnKnownUserLoginTest) {
  std::string b64uid = fmt::format("{{ \"user\": \"{}\" }}",
                                   base64_encode(uid()->data(), uid()->size()));
  spdlog::info("--- REQ: {}", b64uid);
  httplib::Result res = cli.Post("/api/login", b64uid, "application/json");

  ASSERT_TRUE(res);
  ASSERT_TRUE(res->status == 200);
  spdlog::info("--- RESP: {}", res->body);

  Json::Reader reader;
  Json::Value _res_json;
  reader.parse(res->body, _res_json);

  ASSERT_TRUE(_res_json.isMember("redirect"));
  ASSERT_TRUE(_res_json.isMember("session"));
  ASSERT_TRUE(_res_json.isMember("status"));

  ASSERT_TRUE(_res_json["status"].asUInt() == 200);
  ASSERT_TRUE(_res_json["session"].asUInt64() > 0);
}

TEST_F(EndPointsTest, OnUnknownUserCheckCookieTest) {
  httplib::Headers headers = {{"Cookie", "session_id=1234567890"}};

  httplib::Result res = cli.Get("/api/activity", headers);
  ASSERT_TRUE(res);
  ASSERT_TRUE(res->status == 403);
}

TEST_F(EndPointsTest, OnKnownUserCheckCookieTest) {
  uint64_t session_id = EndPointsTest::login();
  ASSERT_TRUE(session_id > 0);

  std::string cookie = fmt::format("session_id={}", session_id);
  httplib::Headers headers = {{"Cookie", cookie.c_str()}};

  spdlog::info("Session-ID: {}", session_id);

  httplib::Result res = cli.Get("/api/activity", headers);
  ASSERT_TRUE(res);
  spdlog::info("Status: {}", res->status);
  ASSERT_TRUE(res->status == 200);
}


TEST_F(EndPointsTest, GetActivityFullTest) {
  uint64_t session_id = EndPointsTest::login();
  ASSERT_TRUE(session_id > 0);

  std::string cookie = fmt::format("session_id={}", session_id);
  httplib::Headers headers = {{"Cookie", cookie.c_str()}};

  httplib::Result res = cli.Get("/api/activity", headers);
  ASSERT_TRUE(res);
  ASSERT_TRUE(res->status == 200);
  Json::Reader reader;
  Json::Value _res_json;
  reader.parse(res->body, _res_json);

  ASSERT_TRUE(_res_json.isMember("ts"));
  ASSERT_TRUE(_res_json.isMember("from"));
  ASSERT_TRUE(_res_json.isMember("to"));
  ASSERT_TRUE(_res_json.isMember("items"));
}


TEST_F(EndPointsTest, OnEventKnownCardReadTest) {

  std::string evtdata = "{ \"t\": 0, \"u\": 1234567890 }";

  httplib::Result res = cli.Post("/api/event", evtdata, "application/json");
  ASSERT_TRUE(res);
  ASSERT_TRUE(res->status == 200);
}


TEST_F(EndPointsTest, OnEventUnknownCardReadTest) {

  std::string evtdata = "{ \"t\": 0, \"u\": 1111 }";

  httplib::Result res = cli.Post("/api/event", evtdata, "application/json");
  ASSERT_TRUE(res);
  ASSERT_TRUE(res->status == 403);
}

TEST_F(EndPointsTest, OnEventMissingCardReadTest) {

  std::string evtdata = "{ \"t\": 0 }";

  httplib::Result res = cli.Post("/api/event", evtdata, "application/json");
  ASSERT_TRUE(res);
  ASSERT_TRUE(res->status == 500);
}

TEST_F(EndPointsTest, OnEventDoorStateChangedTest) {

  std::string evtdata = "{ \"t\": 1 }";
  
  httplib::Result res = cli.Post("/api/event", evtdata, "application/json");
  ASSERT_TRUE(res);
  ASSERT_TRUE(res->status == 200);
}

TEST_F(EndPointsTest, OnEventUnknownTest) {

  std::string evtdata = "{ \"t\": 1234567 }";
  
  httplib::Result res = cli.Post("/api/event", evtdata, "application/json");
  ASSERT_TRUE(res);
  ASSERT_TRUE(res->status == 500);
}

#endif