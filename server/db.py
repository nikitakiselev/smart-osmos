"""
SQLite-хранилище телеметрии Умный Осмос.
"""

import sqlite3
import os
from datetime import datetime, timedelta
from contextlib import contextmanager

DB_PATH = os.environ.get("OSMOS_DB", "osmos_telemetry.db")


@contextmanager
def get_db():
    conn = sqlite3.connect(DB_PATH)
    conn.row_factory = sqlite3.Row
    try:
        yield conn
        conn.commit()
    finally:
        conn.close()


def init_db():
    with get_db() as conn:
        conn.execute("""
            CREATE TABLE IF NOT EXISTS readings (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                ts INTEGER NOT NULL,
                tds_ppm REAL,
                flow_in_lpm REAL,
                flow_out_lpm REAL,
                volume_in_l REAL,
                volume_out_l REAL
            )
        """)
        conn.execute("CREATE INDEX IF NOT EXISTS idx_readings_ts ON readings(ts)")


def insert_reading(ts: int, tds_ppm: float, flow_in_lpm: float, flow_out_lpm: float,
                   volume_in_l: float, volume_out_l: float):
    with get_db() as conn:
        conn.execute(
            """INSERT INTO readings (ts, tds_ppm, flow_in_lpm, flow_out_lpm, volume_in_l, volume_out_l)
               VALUES (?, ?, ?, ?, ?, ?)""",
            (ts, tds_ppm, flow_in_lpm, flow_out_lpm, volume_in_l, volume_out_l),
        )


def get_readings_aggregated(period: str):
    """
    period: 'minute' (последние 60 мин по минутам), 'hour' (24 ч по часам), 'day' (7 дней по дням), 'week' (12 недель).
    Возвращает список dict с полями: ts, tds_ppm_avg, flow_in_sum, flow_out_sum, volume_in_delta, volume_out_delta, count.
    """
    now = datetime.utcnow()
    with get_db() as conn:
        if period == "minute":
            since = int((now - timedelta(minutes=60)).timestamp())
            rows = conn.execute("""
                SELECT (ts / 60) * 60 AS bucket,
                       AVG(tds_ppm) AS tds_ppm_avg,
                       SUM(flow_in_lpm) AS flow_in_sum,
                       SUM(flow_out_lpm) AS flow_out_sum,
                       MAX(volume_in_l) - MIN(volume_in_l) AS volume_in_delta,
                       MAX(volume_out_l) - MIN(volume_out_l) AS volume_out_delta,
                       COUNT(*) AS count
                FROM readings
                WHERE ts >= ?
                GROUP BY bucket
                ORDER BY bucket
            """, (since,)).fetchall()
        elif period == "hour":
            since = int((now - timedelta(hours=24)).timestamp())
            rows = conn.execute("""
                SELECT (ts / 3600) * 3600 AS bucket,
                       AVG(tds_ppm) AS tds_ppm_avg,
                       SUM(flow_in_lpm) AS flow_in_sum,
                       SUM(flow_out_lpm) AS flow_out_sum,
                       MAX(volume_in_l) - MIN(volume_in_l) AS volume_in_delta,
                       MAX(volume_out_l) - MIN(volume_out_l) AS volume_out_delta,
                       COUNT(*) AS count
                FROM readings
                WHERE ts >= ?
                GROUP BY bucket
                ORDER BY bucket
            """, (since,)).fetchall()
        elif period == "day":
            since = int((now - timedelta(days=7)).timestamp())
            rows = conn.execute("""
                SELECT (ts / 86400) * 86400 AS bucket,
                       AVG(tds_ppm) AS tds_ppm_avg,
                       SUM(flow_in_lpm) AS flow_in_sum,
                       SUM(flow_out_lpm) AS flow_out_sum,
                       MAX(volume_in_l) - MIN(volume_in_l) AS volume_in_delta,
                       MAX(volume_out_l) - MIN(volume_out_l) AS volume_out_delta,
                       COUNT(*) AS count
                FROM readings
                WHERE ts >= ?
                GROUP BY bucket
                ORDER BY bucket
            """, (since,)).fetchall()
        elif period == "week":
            since = int((now - timedelta(weeks=12)).timestamp())
            rows = conn.execute("""
                SELECT (ts / 604800) * 604800 AS bucket,
                       AVG(tds_ppm) AS tds_ppm_avg,
                       SUM(flow_in_lpm) AS flow_in_sum,
                       SUM(flow_out_lpm) AS flow_out_sum,
                       MAX(volume_in_l) - MIN(volume_in_l) AS volume_in_delta,
                       MAX(volume_out_l) - MIN(volume_out_l) AS volume_out_delta,
                       COUNT(*) AS count
                FROM readings
                WHERE ts >= ?
                GROUP BY bucket
                ORDER BY bucket
            """, (since,)).fetchall()
        else:
            return []

    return [
        {
            "ts": r["bucket"],
            "tds_ppm_avg": round(r["tds_ppm_avg"] or 0, 1),
            "flow_in_sum": round(r["flow_in_sum"] or 0, 2),
            "flow_out_sum": round(r["flow_out_sum"] or 0, 2),
            "volume_in_delta": round(r["volume_in_delta"] or 0, 2),
            "volume_out_delta": round(r["volume_out_delta"] or 0, 2),
            "count": r["count"],
        }
        for r in rows
    ]
