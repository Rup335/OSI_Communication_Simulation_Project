#!/usr/bin/env python3
"""
Lightweight local web server to launch the live OSI 7-Layer visualizer in the default browser.
"""

import http.server
import socketserver
import webbrowser
import os
import sys

PORT = 8000

class QuietHandler(http.server.SimpleHTTPRequestHandler):
    def log_message(self, format, *args):
        # Keep console clean
        pass

def start_server():
    # Ensure current directory is repository root
    project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
    os.chdir(project_root)

    Handler = QuietHandler
    try:
        with socketserver.TCPServer(("", PORT), Handler) as httpd:
            url = f"http://localhost:{PORT}/index.html"
            print("=" * 65)
            print("  OSI 7-LAYER SIMULATION - LOCAL LIVE WEB VISUALIZER")
            print("=" * 65)
            print(f" Live Web URL: {url}")
            print(" Press Ctrl+C to stop the server anytime.")
            print("=" * 65)
            try:
                webbrowser.open(url)
            except Exception:
                pass
            httpd.serve_forever()
    except OSError as e:
        if e.errno == 98 or e.errno == 10048:
            alt_port = 8081
            print(f"Port {PORT} in use, trying http://localhost:{alt_port}/index.html")
            with socketserver.TCPServer(("", alt_port), Handler) as httpd:
                webbrowser.open(f"http://localhost:{alt_port}/index.html")
                httpd.serve_forever()
        else:
            raise

if __name__ == "__main__":
    try:
        start_server()
    except KeyboardInterrupt:
        print("\n[Server stopped]")
