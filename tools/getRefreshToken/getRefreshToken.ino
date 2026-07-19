// ─────────────────────────────────────────────────────────────────────────────
//  getRefreshToken.ino — run this ONCE to obtain your Spotify refresh token.
//
//  You do not flash this as part of the main firmware. Open it in the Arduino
//  IDE (or a throwaway PlatformIO sketch), install the "spotify-api-arduino"
//  library by Brian Lough, fill in the four values below, and upload.
//
//  Steps:
//    1. Create an app at https://developer.spotify.com/dashboard and note the
//       Client ID + Client Secret.
//    2. Fill ssid/password/clientId/clientSecret below and upload.
//    3. Open Serial Monitor @ 115200. It prints the board's IP, e.g. 192.168.1.42
//    4. In the Spotify dashboard, add this EXACT Redirect URI:
//           http://192.168.1.42/callback
//       (use your board's IP). Save.
//    5. In a browser on the same network, open   http://192.168.1.42/
//       Log in / authorise. The page + Serial Monitor print your REFRESH TOKEN.
//    6. Copy that token into src/config.h  →  SPOTIFY_REFRESH_TOKEN.
//
//  You never need to run this again unless you revoke access.
// ─────────────────────────────────────────────────────────────────────────────
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WebServer.h>
#include <SpotifyArduino.h>

// ── FILL THESE IN ────────────────────────────────────────────────────────────
char ssid[]         = "YOUR_WIFI_SSID";
char password[]     = "YOUR_WIFI_PASSWORD";
char clientId[]     = "your_spotify_client_id";
char clientSecret[] = "your_spotify_client_secret";
// ─────────────────────────────────────────────────────────────────────────────

// Scopes we need for the turntable.
#define SCOPES "user-read-currently-playing%20user-read-playback-state"

WiFiClientSecure client;
SpotifyArduino   spotify(client, clientId, clientSecret);
WebServer        server(80);

char callbackURI[64];   // built at runtime from the board's IP

void handleRoot() {
  char authUrl[512];
  snprintf(authUrl, sizeof(authUrl),
    "https://accounts.spotify.com/authorize?client_id=%s&response_type=code"
    "&redirect_uri=%s&scope=%s",
    clientId, callbackURI, SCOPES);
  server.sendHeader("Location", authUrl);
  server.send(302, "text/plain", "Redirecting to Spotify...");
}

void handleCallback() {
  String code = server.arg("code");
  if (code.length() == 0) {
    server.send(400, "text/plain", "No 'code' in callback. Start again at http://<ip>/");
    return;
  }
  Serial.println("[auth] exchanging code for tokens...");
  const char *refreshToken = spotify.requestAccessTokens(code.c_str(), callbackURI);
  if (refreshToken != nullptr) {
    Serial.println("\n==================  YOUR REFRESH TOKEN  ==================");
    Serial.println(refreshToken);
    Serial.println("=========================================================");
    Serial.println("Paste it into src/config.h -> SPOTIFY_REFRESH_TOKEN\n");

    String page = "<h2>Success!</h2><p>Refresh token (also printed to Serial):</p><code>";
    page += refreshToken;
    page += "</code><p>Copy it into src/config.h. You can close this page.</p>";
    server.send(200, "text/html", page);
  } else {
    server.send(500, "text/plain", "Token exchange failed. Check Client ID/Secret and Redirect URI.");
  }
}

void setup() {
  Serial.begin(115200);
  delay(200);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) { delay(300); Serial.print('.'); }

  IPAddress ip = WiFi.localIP();
  snprintf(callbackURI, sizeof(callbackURI), "http://%s/callback", ip.toString().c_str());

  Serial.println();
  Serial.printf("Board IP: %s\n", ip.toString().c_str());
  Serial.printf("1) Add this EXACT Redirect URI in the Spotify dashboard:\n     %s\n", callbackURI);
  Serial.printf("2) Then open  http://%s/  in your browser.\n", ip.toString().c_str());

  client.setInsecure();   // skip cert pinning for this one-time flow
  server.on("/", handleRoot);
  server.on("/callback", handleCallback);
  server.begin();
}

void loop() {
  server.handleClient();
}
