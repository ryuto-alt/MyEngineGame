#!/usr/bin/env python3
"""
Simple HTTP server for MyEngineGame
Serves static files from x64/Release directory
"""

import http.server
import socketserver
import os
import sys
import argparse
from pathlib import Path

class GameHTTPRequestHandler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=self.server.serve_directory, **kwargs)
    
    def end_headers(self):
        # Add CORS headers for game resources
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'GET, POST, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', 'Content-Type')
        
        # Cache control for game assets
        if self.path.endswith(('.png', '.jpg', '.jpeg', '.obj', '.mtl', '.fbx')):
            self.send_header('Cache-Control', 'public, max-age=3600')
        
        super().end_headers()
    
    def log_message(self, format, *args):
        # Custom logging format
        sys.stderr.write(f"[{self.log_date_time_string()}] {format % args}\n")

def main():
    parser = argparse.ArgumentParser(description='MyEngineGame HTTP Server')
    parser.add_argument('--port', type=int, default=8080, help='Port to serve on (default: 8080)')
    parser.add_argument('--host', default='localhost', help='Host to bind to (default: localhost)')
    parser.add_argument('--directory', default='x64/Release', help='Directory to serve (default: x64/Release)')
    parser.add_argument('--debug', action='store_true', help='Enable debug mode')
    
    args = parser.parse_args()
    
    # Validate directory exists
    serve_dir = Path(args.directory)
    if not serve_dir.exists():
        print(f"Error: Directory '{args.directory}' does not exist.")
        print("Available directories:")
        for d in ['x64/Release', 'x64/Debug']:
            if Path(d).exists():
                print(f"  - {d}")
        sys.exit(1)
    
    # Set up the server
    handler = GameHTTPRequestHandler
    
    try:
        with socketserver.TCPServer((args.host, args.port), handler) as httpd:
            httpd.serve_directory = args.directory
            
            print(f"MyEngineGame Server")
            print(f"{'=' * 40}")
            print(f"Serving directory: {os.path.abspath(args.directory)}")
            print(f"Server running at: http://{args.host}:{args.port}")
            print(f"{'=' * 40}")
            print("Press Ctrl+C to stop the server\n")
            
            httpd.serve_forever()
            
    except KeyboardInterrupt:
        print("\nServer stopped.")
    except OSError as e:
        if e.errno == 98:  # Address already in use
            print(f"Error: Port {args.port} is already in use. Try a different port with --port option.")
        else:
            print(f"Error: {e}")
        sys.exit(1)

if __name__ == '__main__':
    main()