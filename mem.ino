#include <WiFi.h>
#include <DNSServer.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

// ===== CONFIGURATION ===== //
const char *apSSID = "Free_WiFi";
const char *staSSID = "Aeiou";
const char *staPassword = "helloworld";
const IPAddress local_IP(192, 168, 4, 1);
const IPAddress gateway(192, 168, 4, 1);
const IPAddress subnet(255, 255, 255, 0);

DNSServer dnsServer;
AsyncWebServer server(80);

// ========== Connect to Upstream WiFi (STA mode) ========== //
void connectToSTA() {
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(staSSID, staPassword);
  Serial.print("Connecting to upstream WiFi");
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected to STA!");
    Serial.print("STA IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFailed to connect to STA.");
  }
}

// ========== Start Access Point ========== //
void startAP() {
  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP(apSSID);
  delay(100);
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  // DNS: redirect all domains to ESP32's IP
  dnsServer.start(53, "*", local_IP);
}

// ========== URL Filter Logic ========== //
bool allowRequest(String host) {
  return host.indexOf("204") >= 0;
}

// ========== Start Web Server ========== //
void startWebServer() {
  server.onNotFound([](AsyncWebServerRequest *request) {
    String hostHeader = request->host();
    Serial.println("Incoming request: " + hostHeader);

    // Allow only if URL contains "204"
    if (allowRequest(hostHeader)) {
      request->send(204); // Return no content
      return;
    }

    // All others get redirected page with notification polling
    request->send(200, "text/html",
      "<html><head><title>Notification</title></head>"
      "<body><h1>Welcome to Free WiFi</h1>"
      "<p>You are being monitored.</p>"
      "<div id='note'></div>"
      "<script>"
      "setInterval(()=>{"
      "  fetch('/notify').then(r=>r.text()).then(t=>{"
      "    document.getElementById('note').innerText = '📢 Notification: ' + t;"
      "  });"
      "}, 5000);"
      "</script>"
      "</body></html>"
    );
  });

  // Notification endpoint
  server.on("/notify", HTTP_GET, [](AsyncWebServerRequest *request) {
    String notification = "Stay safe online! [" + String(random(1000, 9999)) + "]";
    request->send(200, "text/plain", notification);
  });

  server.begin();
}

// ========== Setup ========== //
void setup() {
  Serial.begin(115200);
  delay(1000);
  connectToSTA();
  startAP();
  startWebServer();
}

// ========== Main Loop ========== //
void loop() {
  dnsServer.processNextRequest();
}
