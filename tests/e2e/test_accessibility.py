"""End-to-end accessibility test: launches a GUI application and asserts that
its controls are exposed on the AT-SPI bus (and, optionally, that a given set
of ``Accessible.name`` values are present).

The test is intentionally application-agnostic: every path, binary name and
expectation is provided through environment variables (see README.md in this
directory for the full list and for manual run instructions). Nothing in this
file is specific to any single project — point it at any built binary that
uses an accessibility-aware toolkit (Qt, GTK, ...) and it will work the same
way.

Requires a running AT-SPI/D-Bus session (see tests/e2e/README.md).
"""
import logging
import os
import shlex
import subprocess
import time
from pathlib import Path

import pyatspi
import pytest

logger = logging.getLogger(__name__)

# --- Configuration (all overridable via environment variables) -------------

APP_BINARY = os.environ.get("E2E_APP_BINARY")
APP_ARGS = shlex.split(os.environ.get("E2E_APP_ARGS", ""))
APP_CWD = os.environ.get("E2E_APP_CWD")
APP_NAME = os.environ.get(
    "E2E_APP_NAME",
    Path(APP_BINARY).stem if APP_BINARY else "",
)
EXTRA_LD_LIBRARY_PATH = os.environ.get("E2E_LD_LIBRARY_PATH", "")
QT_PLUGIN_PATH = os.environ.get("E2E_QT_PLUGIN_PATH", "")
QPA_PLATFORM = os.environ.get("E2E_QPA_PLATFORM", "xcb")
STARTUP_TIMEOUT = float(os.environ.get("E2E_STARTUP_TIMEOUT", "30"))

# Comma-separated list of Accessible.name values expected to be present.
# Left empty by default: the test then only checks that the app registers on
# the AT-SPI bus and logs the names it found, without asserting specific ones.
_expected_raw = os.environ.get("E2E_EXPECTED_ACCESSIBLE_NAMES", "")
EXPECTED_NAMES = {name.strip() for name in _expected_raw.split(",") if name.strip()}


def _collect_names(accessible, names):
    if accessible.name:
        names.add(accessible.name)
    for i in range(accessible.childCount):
        _collect_names(accessible.getChildAtIndex(i), names)


def _find_app(process, app_name, timeout):
    print(f"[DEBUG] _find_app: waiting up to {timeout}s for app_name={app_name!r} pid={process.pid}")
    deadline = time.monotonic() + timeout
    target_pid = process.pid

    while time.monotonic() < deadline:
        returncode = process.poll()
        if returncode is not None:
            print(f"[DEBUG] _find_app: process exited with returncode={returncode}")
            pytest.fail(f"Application exited during startup: {returncode}")

        desktop_count = pyatspi.Registry.getDesktopCount()
        print(f"[DEBUG] _find_app: desktop_count={desktop_count}")
        for di in range(desktop_count):
            desktop = pyatspi.Registry.getDesktop(di)
            if desktop is None:
                continue

            for app in desktop:
                name = ""
                pid = None
                try:
                    name = app.name or ""
                except Exception:
                    pass
                try:
                    pid = app.get_process_id()
                except Exception:
                    pass

                print(f"[DEBUG] _find_app: seen app name={name!r} pid={pid}")

                if pid == target_pid:
                    print(f"[DEBUG] _find_app: matched by pid={pid}")
                    return app

                if app_name and app_name.lower() in name.lower():
                    logger.info("Found app %r on the AT-SPI bus", app.name)
                    print(f"[DEBUG] _find_app: matched by name={name!r}")
                    return app

        time.sleep(0.5)

    logger.warning(
        "Timed out after %.1fs waiting for %r on the AT-SPI bus", timeout, app_name
    )
    print(f"[DEBUG] _find_app: timed out after {timeout}s, app not found")
    return None


@pytest.fixture
def running_app():
    if not APP_BINARY:
        pytest.skip("E2E_APP_BINARY is not set")

    binary = Path(APP_BINARY)
    if not binary.is_file():
        pytest.skip(f"binary not built: {binary}")

    cwd = Path(APP_CWD) if APP_CWD else binary.parent

    env = os.environ.copy()
    if QT_PLUGIN_PATH:
        env["QT_PLUGIN_PATH"] = QT_PLUGIN_PATH
    if EXTRA_LD_LIBRARY_PATH:
        existing = env.get("LD_LIBRARY_PATH", "")
        env["LD_LIBRARY_PATH"] = EXTRA_LD_LIBRARY_PATH + (
            os.pathsep + existing if existing else ""
        )
    # Force the accessibility backend on and keep it on even headless/offscreen.
    env["QT_QPA_PLATFORM"] = QPA_PLATFORM
    env["QT_LINUX_ACCESSIBILITY_ALWAYS_ON"] = "1"

    logger.info("Launching %s %s (cwd=%s)", binary, APP_ARGS, cwd)
    logger.info("DISPLAY=%r DBUS_SESSION_BUS_ADDRESS=%r",
                env.get("DISPLAY"), env.get("DBUS_SESSION_BUS_ADDRESS"))
    print(f"[DEBUG] running_app: binary={binary} args={APP_ARGS} cwd={cwd}")
    print(f"[DEBUG] running_app: QT_QPA_PLATFORM={env.get('QT_QPA_PLATFORM')!r} "
          f"QT_PLUGIN_PATH={env.get('QT_PLUGIN_PATH')!r} "
          f"LD_LIBRARY_PATH={env.get('LD_LIBRARY_PATH')!r}")
    print(f"[DEBUG] running_app: DISPLAY={env.get('DISPLAY')!r} "
          f"DBUS_SESSION_BUS_ADDRESS={env.get('DBUS_SESSION_BUS_ADDRESS')!r}")

    process = subprocess.Popen(
        [str(binary), *APP_ARGS],
        cwd=cwd,
        env=env,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
    )
    print(f"[DEBUG] running_app: started pid={process.pid}")
    try:
        yield process
    finally:
        if process.poll() is None:
            process.terminate()
            try:
                process.wait(timeout=5)
            except subprocess.TimeoutExpired:
                logger.warning("pid=%s did not exit in time, killing", process.pid)
                print(f"[DEBUG] running_app: pid={process.pid} did not exit in time, killing")
                process.kill()
                process.wait()
        print(f"[DEBUG] running_app: final returncode={process.returncode}")
        if process.stdout:
            app_logs = process.stdout.read().decode("utf-8", errors="replace")
            logger.info(
                "Binary output (returncode %s):\n%s",
                process.returncode,
                app_logs or "(none)",
            )
            print(f"[DEBUG] running_app: binary output:\n{app_logs or '(none)'}")


def test_app_exposes_accessible_controls(running_app):
    print(f"[DEBUG] test: APP_BINARY={APP_BINARY!r} APP_NAME={APP_NAME!r} "
          f"APP_ARGS={APP_ARGS!r} QPA_PLATFORM={QPA_PLATFORM!r} "
          f"STARTUP_TIMEOUT={STARTUP_TIMEOUT!r}")
    app = _find_app(running_app, APP_NAME, STARTUP_TIMEOUT)
    print(f"[DEBUG] test: running_app.poll()={running_app.poll()!r}")
    assert running_app.poll() is None, (
        f"Application exited early: returncode {running_app.returncode}"
    )
    assert app is not None, f"{APP_NAME!r} never registered on the AT-SPI bus"

    names = set()
    _collect_names(app, names)
    logger.info("Collected AT-SPI accessible names: %s", sorted(names))
    print(f"[DEBUG] test: collected names={sorted(names)}")

    if EXPECTED_NAMES:
        missing = EXPECTED_NAMES - names
        print(f"[DEBUG] test: expected={sorted(EXPECTED_NAMES)} missing={sorted(missing)}")
        assert not missing, f"Controls missing from the AT-SPI tree: {missing}"
