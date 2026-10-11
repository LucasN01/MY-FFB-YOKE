#pragma once
#include "../SimConnectBridge/SerialLink.h"

static void linkChecks() {
    using namespace YokeLink;
    CHECK(classifyWrite(true,113,113,0)==WriteOutcome::Ok);
    CHECK(classifyWrite(true,40,113,0)==WriteOutcome::Timeout);
    CHECK(classifyWrite(false,0,113,121)==WriteOutcome::Timeout);
    CHECK(classifyWrite(false,40,113,121)==WriteOutcome::Timeout);
    CHECK(classifyWrite(false,0,113,31)==WriteOutcome::DeviceError);
    CHECK(classifyWrite(false,0,113,22)==WriteOutcome::DeviceError);
    CHECK(classifyWrite(false,0,113,995)==WriteOutcome::DeviceError);

    LinkMonitor monitor;
    CHECK(!monitor.onWrite(WriteOutcome::Timeout,0));
    CHECK(!monitor.onWrite(WriteOutcome::Timeout,20));
    CHECK(!monitor.onWrite(WriteOutcome::Timeout,40));
    CHECK(monitor.onWrite(WriteOutcome::Timeout,1000));
    monitor.reset();
    for(unsigned i=0;i<50;++i) CHECK(!monitor.onWrite(WriteOutcome::Timeout,i*400/49));
    CHECK(!monitor.onWrite(WriteOutcome::Ok,500));
    CHECK(monitor.consecutiveTimeouts==0);
    CHECK(!monitor.onWrite(WriteOutcome::Timeout,600));
    CHECK(monitor.firstTimeoutMs==600);
    CHECK(!monitor.onWrite(WriteOutcome::Timeout,1000));
    CHECK(!monitor.onWrite(WriteOutcome::Timeout,1599));
    CHECK(monitor.onWrite(WriteOutcome::Timeout,1600));
    monitor.reset();
    CHECK(monitor.onWrite(WriteOutcome::DeviceError,0));

    Reconnector reconnect;
    reconnect.onLost(100);
    CHECK(reconnect.shouldAttempt(100));
    CHECK(!reconnect.armAllowed(100));
    reconnect.onAttemptFailed(100);
    CHECK(!reconnect.shouldAttempt(599));
    CHECK(reconnect.shouldAttempt(600));
    reconnect.onLost(600);
    CHECK(reconnect.reconnections==1 && reconnect.lostAtMs==100);
    CHECK(reconnect.shouldLogRetry(100));
    CHECK(!reconnect.shouldLogRetry(5099));
    CHECK(reconnect.shouldLogRetry(5100));
    CHECK(!reconnect.shouldLogRetry(5101));
    reconnect.onReopened(1000);
    CHECK(!reconnect.shouldAttempt(1000));
    CHECK(!reconnect.armAllowed(1499));
    CHECK(reconnect.armAllowed(1500));
    reconnect.onLost(1600);
    CHECK(reconnect.reconnections==2 && reconnect.lostAtMs==1600);

    BootLineScanner scanner;
    unsigned boots=0;
    char last[160]={0};
    const auto onBoot=[&](const char* line) { ++boots; std::strcpy(last,line); };
    const char single[]="BOOT first\n";
    scanner.feed(single,sizeof(single)-1,onBoot);
    CHECK(boots==1 && std::strcmp(last,"BOOT first")==0);
    scanner.feed("BO",2,onBoot);
    scanner.feed("OT par",6,onBoot);
    CHECK(boots==1);
    scanner.feed("tial\n",5,onBoot);
    CHECK(boots==2 && std::strcmp(last,"BOOT partial")==0);
    const char crlf[]="BOOT crlf\r\n";
    scanner.feed(crlf,sizeof(crlf)-1,onBoot);
    CHECK(boots==3 && std::strcmp(last,"BOOT crlf")==0);
    const char other[]="INFO BOOT ignored\n";
    scanner.feed(other,sizeof(other)-1,onBoot);
    CHECK(boots==3);
    char oversized[161];
    std::memset(oversized,'x',sizeof(oversized));
    std::memcpy(oversized,"BOOT ",5);
    oversized[160]='\n';
    scanner.feed(oversized,sizeof(oversized),onBoot);
    CHECK(boots==3);
    const char next[]="BOOT recovered\n";
    scanner.feed(next,sizeof(next)-1,onBoot);
    CHECK(boots==4 && std::strcmp(last,"BOOT recovered")==0);
}
