#include "libs/webCfg.h"
#include "libs/display.h"

IRAM_ATTR void loadSD()
{
    if (hasSD && stage == IDLE)
    {
        SD.end();
        hasSD = false;
        SDsize = "";
    }
    else if (!hasSD)
    {
        hasSD = SD.begin(D8, SPI_FULL_SPEED);
        SDsize = humanReadableSize(SD.size64());
    }
}

void setup()
{
    loadSD();
    attachInterrupt(digitalPinToInterrupt(3), loadSD, FALLING);
    initOLED();
    stage = IDLE;

    WiFi.mode(WIFI_AP_STA);
    String appsk;
    for (short i = 0; i < 8; ++i)
        appsk += char(random(32, 126));

    WiFi.softAP(apssid, appsk);

    APdnsServer.setTTL(300);
    APdnsServer.setErrorReplyCode(DNSReplyCode::ServerFailure);
    APdnsServer.start(DNS_PORT, dnsDom, WiFi.softAPIP());

    server.on("/listfiles", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(200, "text/plain", listFiles()); });
    server.onFileUpload(handleUpload);
    server.on("/file", HTTP_GET, handleFile);
    server.on("/postSTA", HTTP_GET, handleSTA);
    server.onNotFound(handleNotFound);
    server.begin();
}

void loop()
{
    printStr();
    updateTimer();
    APdnsServer.processNextRequest();
}