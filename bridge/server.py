"""Local-only HTTP bridge. The C executable remains the data/business-logic owner."""
from http.server import ThreadingHTTPServer, SimpleHTTPRequestHandler
from pathlib import Path
import json
import subprocess
import urllib.parse

ROOT = Path(__file__).resolve().parents[1]
CLI = ROOT / "bin" / "bb_cli"
FRONTEND = ROOT / "frontend"


def run_cli(*args):
    result = subprocess.run([str(CLI), *[str(x) for x in args]], cwd=ROOT,
                            capture_output=True, text=True, timeout=10)
    try:
        data = json.loads(result.stdout or "{}")
    except json.JSONDecodeError:
        data = {"ok": False, "error": result.stderr or "C program returned invalid output"}
    if result.returncode != 0:
        data["ok"] = False
    return data


class Handler(SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=str(FRONTEND), **kwargs)

    def send_json(self, payload, status=200):
        body = json.dumps(payload).encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Content-Length", str(len(body)))
        self.send_header("Cache-Control", "no-store")
        self.end_headers()
        self.wfile.write(body)

    def read_json(self):
        length = int(self.headers.get("Content-Length", "0"))
        if length > 20000:
            raise ValueError("Request too large")
        return json.loads(self.rfile.read(length).decode("utf-8"))

    def do_GET(self):
        path = urllib.parse.urlparse(self.path).path
        if path == "/api/donors":
            self.send_json(run_cli("list")); return
        if path == "/api/inventory":
            self.send_json(run_cli("inventory")); return
        if path.startswith("/api/donors/"):
            donor_id = path.rsplit("/", 1)[-1]
            self.send_json(run_cli("get", donor_id)); return
        if path == "/":
            self.path = "/index.html"
        super().do_GET()

    def do_POST(self):
        path = urllib.parse.urlparse(self.path).path
        try:
            data = self.read_json()
            if path == "/api/donors":
                args = ["add", data["name"], data["age"], data.get("gender", "O"),
                        data["weight"], data["blood_group"], data.get("phone", ""),
                        int(bool(data.get("has_donated_before", False))),
                        data.get("day", 0), data.get("month", 0), data.get("year", 0)]
                result = run_cli(*args)
            elif path == "/api/inventory/restock":
                result = run_cli("stock-add", data["blood_group"], data["units"])
            elif path == "/api/inventory/dispense":
                result = run_cli("stock-deduct", data["blood_group"], data["units"])
            else:
                self.send_json({"ok": False, "error": "Route not found"}, 404); return
            self.send_json(result, 200 if result.get("ok") else 400)
        except (ValueError, KeyError, TypeError, json.JSONDecodeError) as exc:
            self.send_json({"ok": False, "error": str(exc)}, 400)

    def do_PUT(self):
        path = urllib.parse.urlparse(self.path).path
        if not path.startswith("/api/donors/"):
            self.send_json({"ok": False, "error": "Route not found"}, 404); return
        try:
            donor_id = path.rsplit("/", 1)[-1]
            data = self.read_json()
            args = ["update", donor_id, data["name"], data["age"], data.get("gender", "O"),
                    data["weight"], data["blood_group"], data.get("phone", ""),
                    int(bool(data.get("has_donated_before", False))), data.get("day", 0),
                    data.get("month", 0), data.get("year", 0)]
            result = run_cli(*args)
            self.send_json(result, 200 if result.get("ok") else 400)
        except (ValueError, KeyError, TypeError) as exc:
            self.send_json({"ok": False, "error": str(exc)}, 400)

    def do_DELETE(self):
        path = urllib.parse.urlparse(self.path).path
        if path.startswith("/api/donors/"):
            result = run_cli("delete", path.rsplit("/", 1)[-1])
            self.send_json(result, 200 if result.get("ok") else 400)
        else:
            self.send_json({"ok": False, "error": "Route not found"}, 404)

    def log_message(self, fmt, *args):
        print("[local server] " + fmt % args)


if __name__ == "__main__":
    if not CLI.exists():
        raise SystemExit("C CLI not built. Run 'make' from the project folder first.")
    print("Blood Bank local app: http://127.0.0.1:8000")
    print("Local machine only. Press Ctrl+C to stop.")
    ThreadingHTTPServer(("127.0.0.1", 8000), Handler).serve_forever()
