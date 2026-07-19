#!/usr/bin/env python3
"""
get_refresh_token_laptop.py — get your Spotify refresh token on your LAPTOP,
no ESP32 required. This fills SPOTIFY_REFRESH_TOKEN in src/config.h so your
firmware is ready before the hardware even arrives.

Stdlib only — no pip installs.

Steps:
  1. https://developer.spotify.com/dashboard  → Create app (tick "Web API").
     Add this EXACT Redirect URI:   http://127.0.0.1:8888/callback
     Copy the Client ID + Client Secret.
  2. Set them below (or as env vars CLIENT_ID / CLIENT_SECRET).
  3. Run:   python tools/get_refresh_token_laptop.py
  4. A browser opens; log in / authorise. The refresh token prints here and on
     the page. Paste it into src/config.h -> SPOTIFY_REFRESH_TOKEN.
"""
import base64, http.server, json, os, secrets, urllib.parse, urllib.request, webbrowser

CLIENT_ID     = os.environ.get("CLIENT_ID",     "your_spotify_client_id")
CLIENT_SECRET = os.environ.get("CLIENT_SECRET", "your_spotify_client_secret")
REDIRECT_URI  = "http://127.0.0.1:8888/callback"
SCOPES        = "user-read-currently-playing user-read-playback-state"
PORT          = 8888

state  = secrets.token_urlsafe(16)
result = {}


def exchange_code_for_tokens(code):
    body = urllib.parse.urlencode({
        "grant_type":   "authorization_code",
        "code":         code,
        "redirect_uri": REDIRECT_URI,
    }).encode()
    auth = base64.b64encode(f"{CLIENT_ID}:{CLIENT_SECRET}".encode()).decode()
    req = urllib.request.Request(
        "https://accounts.spotify.com/api/token",
        data=body,
        headers={
            "Authorization": f"Basic {auth}",
            "Content-Type":  "application/x-www-form-urlencoded",
        },
    )
    with urllib.request.urlopen(req) as resp:
        return json.load(resp)


class Handler(http.server.BaseHTTPRequestHandler):
    def do_GET(self):
        parsed = urllib.parse.urlparse(self.path)
        if parsed.path != "/callback":
            self.send_response(404); self.end_headers(); return
        params = urllib.parse.parse_qs(parsed.query)

        if params.get("state", [""])[0] != state:
            return self._page(400, "State mismatch — please re-run the script.")
        if "error" in params:
            return self._page(400, "Authorisation denied: " + params["error"][0])
        try:
            tokens = exchange_code_for_tokens(params["code"][0])
            rt = tokens.get("refresh_token", "")
            result["refresh_token"] = rt
            self._page(200,
                "<h2>Success!</h2><p>Your refresh token (also printed in the "
                "terminal):</p><code style='word-break:break-all'>" + rt +
                "</code><p>Paste it into <b>src/config.h</b> &rarr; "
                "SPOTIFY_REFRESH_TOKEN. You can close this tab.</p>")
        except Exception as e:  # noqa: BLE001 — surface any exchange error to the page
            self._page(500, "Token exchange failed: " + str(e) +
                            "<br>Check the Client ID/Secret and Redirect URI.")

    def _page(self, code, html):
        self.send_response(code)
        self.send_header("Content-Type", "text/html")
        self.end_headers()
        self.wfile.write(("<html><body style='font-family:sans-serif;max-width:40em;"
                          "margin:3em auto'>" + html + "</body></html>").encode())

    def log_message(self, *_):  # silence default request logging
        pass


def main():
    if "your_spotify" in CLIENT_ID or "your_spotify" in CLIENT_SECRET:
        print("!! Set CLIENT_ID and CLIENT_SECRET first — edit this file or set env vars.")
        return

    auth_url = "https://accounts.spotify.com/authorize?" + urllib.parse.urlencode({
        "client_id":     CLIENT_ID,
        "response_type": "code",
        "redirect_uri":  REDIRECT_URI,
        "scope":         SCOPES,
        "state":         state,
    })
    print("Opening your browser to authorise...")
    print("If it doesn't open, paste this URL:\n" + auth_url + "\n")
    webbrowser.open(auth_url)

    server = http.server.HTTPServer(("127.0.0.1", PORT), Handler)
    while "refresh_token" not in result:
        server.handle_request()

    print("\n==================  YOUR REFRESH TOKEN  ==================")
    print(result["refresh_token"])
    print("=========================================================")
    print("Paste it into src/config.h -> SPOTIFY_REFRESH_TOKEN\n")


if __name__ == "__main__":
    main()
