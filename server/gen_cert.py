"""
Генерация самоподписанного сертификата для HTTPS.
Запуск: python gen_cert.py
Создаёт cert.pem и key.pem в текущей папке (нужен openssl в PATH).
"""

import subprocess
import sys
import os

def main():
    cmd = [
        "openssl", "req", "-x509", "-newkey", "rsa:2048",
        "-keyout", "key.pem", "-out", "cert.pem", "-days", "365", "-nodes",
        "-subj", "/CN=localhost/O=SmartOsmos",
    ]
    try:
        subprocess.run(cmd, check=True, cwd=os.path.dirname(os.path.abspath(__file__)))
        print("Created cert.pem and key.pem")
    except FileNotFoundError:
        print("Install OpenSSL and add it to PATH, or run without cert — app will use adhoc SSL.")
    except subprocess.CalledProcessError as e:
        print("OpenSSL error:", e)

if __name__ == "__main__":
    main()
