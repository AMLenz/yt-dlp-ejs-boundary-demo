# yt-dlp-ejs-boundary-demo

Small Python-facing example showing a possible solver boundary around the existing `yt-dlp-ejs` assets.

## What this is

This is **not** a downloader and not a larger workflow example.

It only demonstrates a very small integration seam around the solver layer:

- reuse the existing solver assets (`lib.min.js`, `core.min.js`)
- optionally fetch the current `base.js` dynamically from a YouTube watch URL
- submit explicit `sig` / `n` requests
- return structured results / errors to Python

The point is **not** to replace `yt-dlp-ejs`, but to show how small and explicit the boundary around it can be.

## Files

- `solver_boundary_py.cpp`
  Python module entry point
- `solver_boundary_py_backend.*`
  small backend for local `base.js` and dynamic watch-page loading
- `solver_boundary_py_methods.*`
  Python method wrappers
- `solver_engine.*`
  request/response solver execution against existing ejs assets
- `quickjs_json_utils.*`, `quickjs_raii.h`
  QuickJS helpers
- `http_utils.*`
  minimal HTTPS fetch for dynamic `base.js`
- `shared_cli_json.*`, `url_query_utils.*`, `yoube_model.h`
  small supporting utilities / types
- `test.py`
  minimal test script

## Requirements

- Python 3.12
- QuickJS (headers, library, and the included `quickjs/` sources in this repository)
- jsoncpp
- OpenSSL
- existing solver assets available on disk:
  - `meriyah.umd.js`
  - `astring.min.js`
  - `lib.min.js`
  - `core.min.js`

## Solver assets

This demo expects the existing solver assets to be available separately:

- `meriyah.umd.js`
- `astring.min.js`
- `lib.min.js`
- `core.min.js`

They are intentionally not included here. The point of this repository is only to demonstrate the Python-facing boundary around the existing solver layer, not to redistribute the assets themselves.

## Note on QuickJS

This demo uses QuickJS as the embedded JS runtime for executing the existing solver assets.

The `quickjs/` directory is included in this repository because QuickJS is an essential runtime dependency for building and running the example. The main point of this repository is still the small Python-facing solver boundary around the existing solver layer, not the JS runtime itself.

The relevant QuickJS license is included in this repository.

## Build

```bash
g++ -O3 -Wall -Wextra -shared -std=c++17 -fPIC \
  $(python3-config --cflags) \
  $(pkg-config --cflags jsoncpp) \
  solver_boundary_py.cpp \
  solver_boundary_py_backend.cpp \
  solver_boundary_py_methods.cpp \
  solver_engine.cpp \
  quickjs_json_utils.cpp \
  http_utils.cpp \
  shared_cli_json.cpp \
  url_query_utils.cpp \
  -o solver_boundary_py$(python3-config --extension-suffix) \
  $(python3-config --ldflags) \
  $(pkg-config --libs jsoncpp) \
  -lssl -lcrypto \
  -I. -I quickjs -L quickjs -lquickjs
