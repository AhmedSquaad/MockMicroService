# E2E accessibility test

`test_accessibility.py` launches a GUI binary and checks that it registers on
the **AT-SPI** accessibility bus, and (optionally) that a given set of
`Accessible.name` values are exposed by its controls.

The test is generic: it does not hardcode any project, binary name or path.
Everything is configured through environment variables, so it can be pointed
at any built application (this project's `ecommerce` binary, or any other
Qt/GTK app).

## Environment variables

| Variable                          | Required | Description                                                                 |
| ---------------------------------- | -------- | ---------------------------------------------------------------------------- |
| `E2E_APP_BINARY`                   | yes      | Absolute path to the executable to launch.                                   |
| `E2E_APP_NAME`                     | no       | Substring used to find the app on the AT-SPI bus. Defaults to the binary's filename. |
| `E2E_APP_ARGS`                     | no       | Extra CLI arguments passed to the binary (shell-quoted string).              |
| `E2E_APP_CWD`                      | no       | Working directory for the process. Defaults to the binary's directory.       |
| `E2E_LD_LIBRARY_PATH`              | no       | Extra entries prepended to `LD_LIBRARY_PATH` (e.g. a bundled `lib/` folder).  |
| `E2E_QT_PLUGIN_PATH`               | no       | Sets `QT_PLUGIN_PATH` for Qt apps with non-standard plugin locations.         |
| `E2E_QPA_PLATFORM`                 | no       | Qt platform plugin to use. Defaults to `xcb` (needs a real/virtual X server). |
| `E2E_STARTUP_TIMEOUT`              | no       | Seconds to wait for the app to appear on the AT-SPI bus. Defaults to `30`.    |
| `E2E_EXPECTED_ACCESSIBLE_NAMES`    | no       | Comma-separated `Accessible.name` values expected to be present. If unset, the test only checks that the app registers on the bus and logs whatever names it found. |

## Running manually

You need a running X server (or Xvfb) and a D-Bus session with AT-SPI
enabled. On a Debian/Ubuntu machine:

```bash
sudo apt-get update && sudo apt-get install -y \
    at-spi2-core python3-pyatspi xvfb dbus-x11

python3 -m venv .venv && source .venv/bin/activate
pip install pytest
# pyatspi comes from the system package above (not available on PyPI);
# make sure the venv can see system site-packages, e.g.:
#   python3 -m venv --system-site-packages .venv

# Build the app first (see scripts/build.sh), then run the test under a
# virtual display + its own D-Bus/AT-SPI session:
xvfb-run -a dbus-run-session -- env \
    E2E_APP_BINARY="$PWD/build/ecommerce" \
    E2E_LD_LIBRARY_PATH="$PWD/build/lib" \
    E2E_APP_NAME="ecommerce" \
    E2E_EXPECTED_ACCESSIBLE_NAMES="Commandes,Liste des commandes,Rafraîchir" \
    pytest tests/e2e/test_accessibility.py -v
```

If you already have a desktop session with AT-SPI running (e.g. a regular
Linux desktop), you can drop `xvfb-run -a dbus-run-session --` and run the
`pytest` command directly.

## Running in CI

See the `test:e2e:accessibility` job in `.gitlab-ci.yml`. It reuses the
`build/` artifact produced by the `build` job and runs the test inside
`xvfb-run`/`dbus-run-session` on a headless Ubuntu image.
