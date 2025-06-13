# MyEngineGame Server

Simple HTTP server for serving MyEngineGame static files.

## Quick Start

```bash
# Run with default settings (serves x64/Release on port 8080)
python server.py

# Run with custom port
python server.py --port 3000

# Serve debug build
python server.py --directory x64/Debug

# Bind to all interfaces (for network access)
python server.py --host 0.0.0.0
```

## Command Line Options

- `--port`: Server port (default: 8080)
- `--host`: Host to bind to (default: localhost)
- `--directory`: Directory to serve (default: x64/Release)
- `--debug`: Enable debug mode

## Features

- CORS headers for game resources
- Caching headers for static assets
- Custom MIME types for game files
- Clean logging output

## Configuration

Edit `server_config.json` to customize:
- Default port and host
- MIME type mappings
- CORS settings

## Requirements

- Python 3.6+
- No external dependencies required