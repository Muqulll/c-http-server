# C HTTP Server

A low-level HTTP web server written in C using POSIX socket programming.

## Features
- Handles incoming HTTP GET requests using socket programming.
- Serves static assets (`index.html`, `style.css`, `logo.png`).
- Parses and maps dynamic MIME types for proper `Content-Type` headers.

## Build & Run

1. **Compile:**
   ```bash
   gcc mime_server.c -o server

```

2. **Execute:**
```bash
./server

```


3. **Test:**
Open `http://localhost:8080` in your browser or test via `curl`:
```bash
curl -I http://localhost:8080

```



```bash
git init
git add .
git status   # Verify that executables (server, mime, modes) are ignored!
git commit -m "feat: initial C HTTP server with socket programming and MIME handling"

```
