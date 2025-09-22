#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import subprocess
import json
import argparse
import shutil

SILENT_OUTPUT = False # Set to True to suppress command line output

def run_command(command):
    subprocess.run(command, shell=True, check=True, capture_output=SILENT_OUTPUT, text=True)

# replace slashes in branch names with dashes for file naming, otherwise it will create directories
def process_branch_name(branch_name):
    return branch_name.replace('/', '-')

def get_current_git_state():
    try:
        # try branch name
        result = subprocess.run("git symbolic-ref --short HEAD", shell=True,
                                capture_output=True, text=True, check=True)
        return result.stdout.strip()
    except subprocess.CalledProcessError:
        # else try commit hash (for detached state)
        result = subprocess.run("git rev-parse HEAD", shell=True, check=True, capture_output=True, text=True)
        return result.stdout.strip()

def restore_git_state(original_git_state):
    print(f"\nRestoring original git state: {original_git_state}")
    run_command(f"git checkout {original_git_state}")

# Locate Google Benchmark's compare.py script
def find_google_benchmark_compare(custom_path=None):
    if custom_path:
        if os.path.exists(custom_path):
            return custom_path
        else:
            raise FileNotFoundError(f"Custom compare.py path does not exist: {custom_path}")
    compare = shutil.which("compare.py")
    if compare:
        return compare
    try:
        import google_benchmark  # type: ignore
        gb_path = os.path.dirname(google_benchmark.__file__)
        script = os.path.join(gb_path, "tools", "compare.py")
        if os.path.exists(script):
            return script
    except ImportError:
        pass
    raise FileNotFoundError("Google Benchmark's compare.py not found.")


def configure_and_build(commit, cmake_variables, benchmark_target, build_dir):
    print(f"\nConfiguring and building commit: {commit}\n")

    run_command(f"git checkout {commit}")

    cmake_vars_str = " ".join(cmake_variables)
    command = f"cmake -S . -B {build_dir} -DCMAKE_BUILD_TYPE=Release {cmake_vars_str}"
    run_command(command)

    command = f"cmake --build {build_dir} --target {benchmark_target} --clean-first"
    run_command(command)

def run_benchmarks(commit, benchmark_target, build_dir):
    print(f"Running benchmarks for commit: {commit}\n")
    output = process_branch_name(commit) + "-results.json"
    command = f"./{build_dir}/benchmarks/{benchmark_target} --benchmark_out_format=json --benchmark_time_unit=us --benchmark_out={output}"
    run_command(command)
    print(f"Benchmarks for commit {commit} completed and results saved to {output}\n")

def run_and_compare_benchmarks(base_commit, head_commit, benchmark_target, build_dir, compare_path=None, output_file=None):
    # Run benchmarks for both commits
    run_benchmarks(base_commit, benchmark_target, build_dir)
    run_benchmarks(head_commit, benchmark_target, build_dir)
    base_file = process_branch_name(base_commit) + "-results.json"
    new_file = process_branch_name(head_commit) + "-results.json"

    # Find compare.py
    compare_script = find_google_benchmark_compare(compare_path)

    # Run compare.py
    cmd = [
        sys.executable,
        compare_script,
        "benchmarks",
        base_file,
        new_file,
        "--json",
    ]
    try:
        result = subprocess.run(
            cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
            text=True, check=True
        )
    except subprocess.CalledProcessError as e:
        print("Error running compare.py:\n", e.stderr, file=sys.stderr)
        sys.exit(1)

    try:
        data = json.loads(result.stdout)
    except json.JSONDecodeError:
        print("Failed to parse JSON output from compare.py", file=sys.stderr)
        sys.exit(1)

    # Format output
    benchmarks = data.get("benchmarks", [])
    if not benchmarks:
        result_str = "No benchmarks found in comparison."
        print(result_str)
        if output_file:
            with open(output_file, "w") as f:
                f.write(result_str + "\n")
        return

    time_unit = data["benchmarks"][0].get("time_unit", "ns")
    header = f"\nBenchmark Comparison: {base_file} vs {new_file}\nTime Unit: {time_unit}\n"
    table_header = "{:<60} {:>15} {:>15} {:>15}".format(
        "Benchmark", f"Base ({time_unit})", f"New ({time_unit})", "Δ %"
    )
    separator = "-" * 110
    lines = [header, table_header, separator]

    for bm in benchmarks:
        name = bm.get("name", "")
        base = bm.get("cpu_time_baseline", None)
        cont = bm.get("cpu_time_contender", None)

        if base is None or cont is None:
            continue

        try:
            diff_pct = ((cont - base) / base) * 100.0
        except ZeroDivisionError:
            diff_pct = float("inf")

        lines.append("{:<60} {:>15.2f} {:>15.2f} {:>14.2f}%".format(
            name, base, cont, diff_pct
        ))

    lines.append("\nComparison completed successfully.")
    result_str = "\n".join(lines)
    print(result_str)
    if output_file:
        with open(output_file, "w") as f:
            f.write(result_str + "\n")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Compare SeQuant benchmarks between two commits",
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )

    parser.add_argument("base_commit", help="Base commit SHA to compare against")
    parser.add_argument("head_commit", help="Head commit SHA to compare")
    parser.add_argument("--benchmark-target", "-t",
                        default="sequant_benchmarks",
                        help="Benchmark target to build and run (default: sequant_benchmarks)")
    parser.add_argument("--build-dir", "-b",
                        default="build",
                        help="Build directory for CMake (default: build)")
    parser.add_argument("--compare-path", "-c",
                        default=None,
                        help="Path to Google Benchmark's compare.py script (optional)")
    parser.add_argument("--output-file", "-o",
                        default=None,
                        help="File to write comparison results (optional)")

    args = parser.parse_args()

    # print info
    print("**" * 50)
    print("SeQuant Benchmark Comparison Script\n")
    print(f"Base commit: {args.base_commit}")
    print(f"Head commit: {args.head_commit}")
    print(f"Benchmark target: {args.benchmark_target}")
    print(f"Build directory: {args.build_dir}")
    print(f"Compare.py path: {args.compare_path}")
    print(f"Output file: {args.output_file}")
    print("**" * 50)

    original_ref = get_current_git_state()
    print(f"Original git reference: {original_ref}")

    # Define CMake variables
    cmake_variables = ["-G Ninja",
                       "-DCMAKE_BUILD_TYPE=Release",
                       "-DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON",
                       "-DSEQUANT_TESTS=OFF",
                       "-DSEQUANT_EVAL_TESTS=OFF",
                       "-DSEQUANT_BENCHMARKS=ON",
                       "-DSEQUANT_MIMALLOC=ON",
                       "-DSEQUANT_CONTEXT_MANIPULATION_THREADSAFE=ON"]

    print("\nCMake variables:")
    for var in cmake_variables:
        print(f"{var}")

    try:
        # base commit
        configure_and_build(args.base_commit, cmake_variables, args.benchmark_target, args.build_dir)
        # head commit
        configure_and_build(args.head_commit, cmake_variables, args.benchmark_target, args.build_dir)
        run_and_compare_benchmarks(
            args.base_commit,
            args.head_commit,
            args.benchmark_target,
            args.build_dir,
            args.compare_path,
            args.output_file
        )
    except Exception as e:
        print(f"Error during benchmark execution: {e}")
        sys.exit(1)
    finally:
        # restore original git state even if script fails
        restore_git_state(original_ref)
