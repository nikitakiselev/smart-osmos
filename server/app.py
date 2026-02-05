"""
HTTPS-сервер приёма телеметрии Умный Осмос.
Принимает POST /api/ingest с JSON и X-API-Key, сохраняет в SQLite.
UI: графики по часам, дням, неделям.
"""

import os
import ssl
from flask import Flask, request, jsonify, send_from_directory
from db import init_db, insert_reading, get_readings_aggregated

API_KEY = os.environ.get("OSMOS_API_KEY", "your-secret-api-key")
HOST = os.environ.get("OSMOS_HOST", "0.0.0.0")
PORT = int(os.environ.get("OSMOS_PORT", "8443"))

app = Flask(__name__, static_folder="static")
init_db()


@app.after_request
def log_request(response):
    """Вывод входящих HTTP-запросов в stdout."""
    print(f"{request.method} {request.path} — {request.remote_addr} — {response.status_code}", flush=True)
    return response


@app.route("/api/ingest", methods=["POST"])
def ingest():
    key = request.headers.get("X-API-Key")
    if key != API_KEY:
        return jsonify({"error": "Invalid API key"}), 401
    try:
        data = request.get_json(force=True)
    except Exception:
        return jsonify({"error": "Invalid JSON"}), 400

    ts = data.get("ts")
    if ts is None:
        import time
        ts = int(time.time())
    tds_ppm = float(data.get("tds_ppm", 0))
    flow_in_lpm = float(data.get("flow_in_lpm", 0))
    flow_out_lpm = float(data.get("flow_out_lpm", 0))
    volume_in_l = float(data.get("volume_in_l", 0))
    volume_out_l = float(data.get("volume_out_l", 0))

    insert_reading(ts, tds_ppm, flow_in_lpm, flow_out_lpm, volume_in_l, volume_out_l)
    return jsonify({"ok": True}), 201


@app.route("/api/chart")
def chart():
    period = request.args.get("period", "hour")
    if period not in ("minute", "hour", "day", "week"):
        period = "hour"
    rows = get_readings_aggregated(period)
    return jsonify({"period": period, "data": rows})


@app.route("/")
def index():
    return send_from_directory(app.static_folder, "index.html")


@app.route("/<path:path>")
def static_files(path):
    return send_from_directory(app.static_folder, path)


if __name__ == "__main__":
    cert = os.environ.get("OSMOS_CERT", "cert.pem")
    key = os.environ.get("OSMOS_KEY", "key.pem")
    if os.path.isfile(cert) and os.path.isfile(key):
        ssl_ctx = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
        ssl_ctx.load_cert_chain(cert, key)
        app.run(host=HOST, port=PORT, ssl_context=ssl_ctx, threaded=True)
    else:
        # Самоподписанный сертификат на лету (pyopenssl)
        app.run(host=HOST, port=PORT, ssl_context="adhoc", threaded=True)
