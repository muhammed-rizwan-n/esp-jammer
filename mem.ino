
#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>

// ===== CONFIGURATION =====
const char* apSSID = "MyESP32_AP";
const char* apPassword = "12345678";
const char* staSSID = "Aeiou";
const char* staPassword = "helloworld";
IPAddress apIP(192, 168, 4, 1);
IPAddress netMsk(255, 255, 255, 0);
const byte DNS_PORT = 53;

DNSServer dnsServer;
WebServer server(80);

// Whitelisted domains (add more if needed)
const char* whitelist[] = {
  "example.com",
  "openai.com"
};
const int whitelistSize = sizeof(whitelist) / sizeof(whitelist[0]);

bool isWhitelisted(const String& query) {
  Serial.println(query);
  for (int i = 0; i < whitelistSize; i++) {
    if (query.indexOf(whitelist[i]) >= 0) return true;
  }
  return false;
}

void handleRoot() {
  server.send(200, "text/html", "<html><body><h1>Access Restricted</h1><p>You are being redirected because the domain is not whitelisted.</p><script>setInterval(()=>alert('ESP32 Notification'), 15000);</script></body></html>");
}

void setup() {
  Serial.begin(115200);

  // Connect to upstream WiFi (STA mode)
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(staSSID, staPassword);
  Serial.print("Connecting to STA");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to STA");

  // Start AP
  WiFi.softAP(apSSID, apPassword);
  WiFi.softAPConfig(apIP, apIP, netMsk);
  Serial.println("AP Started");

  // Start DNS spoofing server
  dnsServer.start(DNS_PORT, "*", apIP);

  // DNS request handler (check whitelist)
  dnsServer.setTTL(60);
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);

  // Start captive web server
  server.onNotFound(handleRoot);
  server.begin();
  Serial.println("Web server started");
}

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();
}
