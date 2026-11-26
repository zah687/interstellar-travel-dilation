import json
import os
import subprocess
import time
from http.server import BaseHTTPRequestHandler, HTTPServer
from urllib.parse import parse_qs, urlparse

IMAGE = "travel:1.0"
PORT = 8000
HOST = os.environ.get("HOST", "localhost")

stats = {"ok_count": 0, "error_count": 0, "seconds_total": 0.0}


class Handler(BaseHTTPRequestHandler):

    def do_GET(self):
        parsed = urlparse(self.path)

        if parsed.path == "/":
            self.serve_page()
        elif parsed.path == "/calculate":
            self.handle_calculate(parse_qs(parsed.query))
        elif parsed.path == "/metrics":
            self.serve_metrics()
        else:
            self.send_error(404)

    def serve_page(self):
        try:
            with open("index.html", "rb") as f:
                content = f.read()
        except OSError:
            self.send_error(500, "Could not read index.html")
            return

        self.send_response(200)
        self.send_header("Content-Type", "text/html; charset=utf-8")
        self.end_headers()
        self.wfile.write(content)

    def handle_calculate(self, query):
        destination = query.get("destination", [""])[0]
        speed = query.get("speed", [""])[0]

        started = time.time()
        try:
            proc = subprocess.run(
                ["docker", "run", "--rm", IMAGE, destination, speed],
                capture_output=True,
                text=True,
                timeout=20,
            )
        except subprocess.TimeoutExpired:
            stats["error_count"] += 1
            stats["seconds_total"] += time.time() - started
            self.send_json({"ok": False, "output": "Calculation timed out."})
            return
        except OSError as exc:
            stats["error_count"] += 1
            stats["seconds_total"] += time.time() - started
            self.send_json({"ok": False, "output": f"Docker error: {exc}"})
            return

        stats["seconds_total"] += time.time() - started

        if proc.returncode == 0:
            stats["ok_count"] += 1
            payload = {"ok": True, "output": proc.stdout.strip()}
        else:
            stats["error_count"] += 1
            payload = {
                "ok": False,
                "output": proc.stderr.strip() or proc.stdout.strip() or "Calculation failed.",
            }

        self.send_json(payload)

    def serve_metrics(self):
        total = stats["ok_count"] + stats["error_count"]
        body = (
            "# HELP travel_calculations_total Calculations done, by result.\n"
            "# TYPE travel_calculations_total counter\n"
            'travel_calculations_total{result="ok"} %d\n'
            'travel_calculations_total{result="error"} %d\n'
            "# HELP travel_calculation_seconds Time spent on one calculation.\n"
            "# TYPE travel_calculation_seconds summary\n"
            "travel_calculation_seconds_sum %f\n"
            "travel_calculation_seconds_count %d\n"
        ) % (stats["ok_count"], stats["error_count"], stats["seconds_total"], total)

        self.send_response(200)
        self.send_header("Content-Type", "text/plain; version=0.0.4")
        self.end_headers()
        self.wfile.write(body.encode("utf-8"))

    def send_json(self, data):
        encoded = json.dumps(data).encode("utf-8")
        self.send_response(200)
        self.send_header("Content-Type", "application/json")
        self.end_headers()
        self.wfile.write(encoded)


if __name__ == "__main__":
    print(f"Open http://{HOST}:{PORT} in your browser (Ctrl+C to stop)")
    HTTPServer((HOST, PORT), Handler).serve_forever()