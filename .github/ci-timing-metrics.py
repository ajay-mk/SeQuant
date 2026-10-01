"""Measure experiment commands with standard-library wall time and host resources."""
import ctypes
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import threading
import time


def used_memory():
    if sys.platform.startswith('linux'):
        values = dict(line.split(':', 1) for line in Path('/proc/meminfo').read_text().splitlines())
        return (int(values['MemTotal'].split()[0]) - int(values['MemAvailable'].split()[0])) * 1024
    if sys.platform == 'win32':
        class MemoryStatus(ctypes.Structure):
            _fields_ = [('length', ctypes.c_ulong), ('load', ctypes.c_ulong)] + [
                (name, ctypes.c_ulonglong) for name in
                ('total', 'available', 'total_page', 'available_page', 'total_virtual', 'available_virtual', 'extended')]
        status = MemoryStatus()
        status.length = ctypes.sizeof(status)
        if ctypes.windll.kernel32.GlobalMemoryStatusEx(ctypes.byref(status)):
            return status.total - status.available
    return None


def main():
    stage, separator, *command = sys.argv[1:]
    if separator != '--' or not command:
        raise SystemExit('usage: metrics.py STAGE -- COMMAND ...')
    output = Path(os.environ['RUNNER_TEMP']) / 'ci-timing-results'
    output.mkdir(exist_ok=True)
    initial_memory = used_memory()
    initial_free = shutil.disk_usage('.').free
    samples = []
    stop = threading.Event()

    def sample():
        while True:
            samples.append((used_memory(), shutil.disk_usage('.').free))
            if stop.wait(2):
                break

    thread = threading.Thread(target=sample, daemon=True)
    thread.start()
    started = time.monotonic()
    try:
        result = subprocess.run(command, check=False).returncode
    finally:
        stop.set()
        thread.join()
    measured_memory = [memory for memory, _ in samples if memory is not None]
    report = dict(stage=stage, command=command, exit_code=result,
                  elapsed_seconds=time.monotonic() - started,
                  source_sha=os.environ.get('SOURCE_SHA'), phase=os.environ.get('EXPERIMENT_PHASE'),
                  variant=os.environ.get('EXPERIMENT_VARIANT'),
                  initial_host_used_memory_bytes=initial_memory,
                  peak_host_used_memory_bytes=max(measured_memory, default=None),
                  initial_disk_free_bytes=initial_free,
                  minimum_disk_free_bytes=min(free for _, free in samples),
                  final_disk_free_bytes=shutil.disk_usage('.').free)
    (output / (stage + '.json')).write_text(json.dumps(report, indent=2))
    print('CI_TIMING ' + json.dumps(report), flush=True)
    summary = os.environ.get('GITHUB_STEP_SUMMARY')
    if summary:
        with open(summary, 'a') as handle:
            handle.write(f'| {stage} | {report["elapsed_seconds"]:.1f} seconds | exit {result} |\n')
    raise SystemExit(result)


if __name__ == '__main__':
    main()
