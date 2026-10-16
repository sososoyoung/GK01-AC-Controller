#include <algorithm>
#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <functional>
#include <map>
#include <string>
#include "core/live_log.h"

uint32_t now = 100;
uint32_t millis() { return now; }
SerialPort Serial;
class String : public std::string {
public:
    using std::string::string;
    String(const std::string& value) : std::string(value) {}
    String(uint32_t value) : std::string(std::to_string(value)) {}
    char charAt(size_t i) const { return at(i); }
    bool reserve(size_t n) { std::string::reserve(n); return true; }
};
String operator+(const String& a, uint32_t b) { return std::string(a) + std::to_string(b); }
namespace str {
#include "escape.inc"
}
struct FakeServer {
    bool authorized = true;
    int code = 0;
    std::string body;
    std::map<std::string, String> args;
    std::map<std::string, std::function<void()>> routes;
    bool authenticate(const char* user, const char* password) { assert(std::string(user)=="admin");assert(std::string(password)=="secret");return authorized; }
    void sendHeader(const char*, const char*) {}
    void send(int c, const char*, const String& b) { code = c; body = b; }
    String arg(const char* key) { return args[key]; }
    void on(const char* path, int, std::function<void()> handler) { routes[path]=handler; }
} server_;
struct { FakeServer& raw() { return server_; } } web;
struct { bool busy = false; bool isUploading() const { return busy; } } ota;
const char* getApPass() { return "secret"; }
uint32_t os_random() { static uint32_t id=40;return ++id; }
constexpr int HTTP_POST = 1, HTTP_GET = 0;
using std::min;
#include "routes.inc"

void call(const char* route, uint32_t session=0, uint32_t after=0) {
    server_.args["session"]=String(session);server_.args["after"]=String(after);
    server_.routes[route]();
}
int main() {
    static_assert(sizeof(LiveLog) <= 4096);
    LiveLogQueue q;
    q.append("old", now);assert(q.size()==0);
    assert(q.start(1, now)==0);q.append("first",now);
    assert(q.size()==1 && q.at(0).seq==1);
    assert(q.acknowledge(0)==0 && q.size()==1);
    assert(q.acknowledge(1)==0 && q.size()==0);
    for(int i=0;i<25;++i)q.append("line",now);
    assert(q.size()==20 && q.acknowledge(1)==5);
    auto cursor=q.start(2,now);assert(q.size()==0 && cursor==26);
    assert(!q.stop(1) && q.session()==2);
    assert(!q.renew(1,now));assert(q.renew(2,now));
    now+=LiveLogQueue::LEASE_MS;assert(!q.active(now) && q.size()==0);
    now=UINT32_MAX-100;q.start(3,now);q.append("wrap",now);
    now=50;assert(q.renew(3,now));now+=LiveLogQueue::LEASE_MS;assert(!q.active(now));
    now=100;appLog.start(4);
    std::string longLine(170,'x');longLine+="中文中文中文";
    appLog.println(longLine.c_str());
    assert(std::string(appLog.queue.at(0).text)==std::string(170,'x')+"...");
    assert(Serial.output.find(longLine)!=std::string::npos);
    appLog.println("quotes \" tab\t control\x01 <script> & \\ 中文");
    auto escaped=str::jsonEscape(appLog.queue.at(1).text);
    assert(escaped.find("\\t")==std::string::npos && escaped.find("\\u0009")!=std::string::npos);
    assert(escaped.find("\\u0001")!=std::string::npos && escaped.find("\\\"")!=std::string::npos);
    appLog.print("partial");appLog.start(5);appLog.println("fresh");
    assert(std::string(appLog.queue.at(0).text)=="fresh");
    appLog.print("secret partial");now+=LiveLogQueue::LEASE_MS;appLog.tick();
    assert(appLog.queue.session()==0 && appLog.queue.size()==0);
    appLog.start(6);appLog.println("next");assert(std::string(appLog.queue.at(0).text)=="next");
    registerLogs();
    server_.authorized=false;call("/api/logs/start");assert(server_.code==403);
    server_.authorized=true;ota.busy=true;call("/api/logs/start");assert(server_.code==503 && !appLog.queue.session());ota.busy=false;
    call("/api/logs/start");assert(server_.code==200);auto first=appLog.queue.session();auto initial=appLog.queue.cursor();
    assert(appLog.queue.size()==0);
    call("/api/logs/start");auto session=appLog.queue.session();assert(session!=first);
    call("/api/logs/stop",first);assert(server_.code==409 && appLog.queue.session()==session);
    call("/api/logs",session,initial+1);assert(server_.code==400);
    server_.args["session"]="4294967296";server_.routes["/api/logs"]();assert(server_.code==400);
    std::string controls(170,'\x01');
    for(int i=0;i<20;++i)appLog.println(controls.c_str());
    call("/api/logs",session,initial);assert(server_.code==200 && server_.body.size()<=2400);
    assert(server_.body.find("\\u0001")!=std::string::npos);
    // Budget must limit this batch to two lines; cursor leaves the rest pending.
    assert(server_.body.find("\"next\":"+std::to_string(initial+2))!=std::string::npos);
    auto next=initial+2;call("/api/logs",session,next);assert(server_.code==200 && appLog.queue.size()==18);
    now+=LiveLogQueue::LEASE_MS;call("/api/logs",session,next);assert(server_.code==409 && appLog.queue.size()==0);
    puts("PASS live logs: no history, cursor/overflow, session replacement, lease/wrap, actual Print UTF-8 truncation, JSON controls, auth/OTA, bounded response");
    printf("LiveLog RAM: %zu bytes; queue: %zu bytes\n",sizeof(LiveLog),sizeof(LiveLogQueue));
}
