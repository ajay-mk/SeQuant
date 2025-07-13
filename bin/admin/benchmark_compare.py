#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import subprocess
import json

SILENT_OUTPUT = True # Set to True to suppress command line output

def run_command(command):
    subprocess.run(command, shell=True, check=True, capture_output=SILENT_OUTPUT, text=True)

def configure_and_build(commit, cmake_variables, benchmark_target):
    print(f"\nConfiguring and building commit: {commit}\n")

    run_command(f"git checkout {commit}")

    cmake_vars_str = " ".join(cmake_variables)
    command = f"cmake -S . -B build -DCMAKE_BUILD_TYPE=Release {cmake_vars_str}"
    run_command(command)

    command = f"cmake --build build --target {benchmark_target} --clean-first"
    run_command(command)

def run_benchmarks(commit, benchmark_target):
    print(f"Running benchmarks for commit: {commit}\n")
    command = f"./build/benchmarks/{benchmark_target} --benchmark_out_format=json --benchmark_out={commit}-results.json"
    run_command(command)
    print(f"Benchmarks for commit {commit} completed and results saved to {commit}-results.json\n")

def compare_benchmarks(base_commit, head_commit, metric="cpu_time"):
    base_file = f"{base_commit}-results.json"
    new_file = f"{head_commit}-results.json"
    output_file = f"{base_commit}-{head_commit}-comparison.txt"

    print(f"\nComparing benchmarks between {base_commit} and {head_commit} using metric: {metric}\n")

    # Validate files exist and load JSON data
    try:
        if not os.path.exists(base_file):
            raise FileNotFoundError(f"Base file not found: {base_file}")
        if not os.path.exists(new_file):
            raise FileNotFoundError(f"New file not found: {new_file}")

        with open(base_file, 'r') as f:
            base_data = json.load(f)
        with open(new_file, 'r') as f:
            new_data = json.load(f)

    except json.JSONDecodeError as e:
        raise ValueError(f"Invalid JSON format: {e}")
    except Exception as e:
        raise RuntimeError(f"Error reading files: {e}")

    # index benchmarks by name
    base_benchmarks = {b['name']: b for b in base_data['benchmarks']}
    new_benchmarks = {b['name']: b for b in new_data['benchmarks']}

    # get time unit
    time_unit = base_data['benchmarks'][0].get('time_unit', 'ns')

    # only compare benchmarks that are present in both files
    common_names = []
    new_benchmark_names = set(new_benchmarks.keys())
    for benchmark in base_data['benchmarks']:
        if benchmark['name'] in new_benchmark_names:
            common_names.append(benchmark['name'])

    # output details
    output_lines = []
    output_lines.append(f"Benchmark Comparison: {base_commit} vs {head_commit}")
    output_lines.append(f"Metric: {metric}")
    output_lines.append(f"Time Unit: {time_unit}")
    output_lines.append(f"Date: {os.popen('date').read().strip()}")
    output_lines.append("")

    if 'context' in base_data:
        output_lines.append("Base Benchmark Context:")
        output_lines.append(json.dumps(base_data['context'], indent=2))
        output_lines.append("")

    if 'context' in new_data:
        output_lines.append("New Benchmark Context:")
        output_lines.append(json.dumps(new_data['context'], indent=2))
        output_lines.append("")

    output_lines.append(f"{'Name':<60} {f'Base ({time_unit})':<15} {f'New ({time_unit})':<15} {f'Diff ({time_unit})':<15} {'% Diff':<10}")
    output_lines.append("-" * 125)

    # header for console
    # print(f"{'Name':<50} {f'Base ({time_unit})':<15} {f'New ({time_unit})':<15} {f'Diff ({time_unit})':<15} {'% Diff':<10}")
    # print("-" * 100)

    # compare each benchmark
    for name in common_names:
        base_value = base_benchmarks[name][metric]
        new_value = new_benchmarks[name][metric]

        diff = new_value - base_value
        percentage_diff = ((new_value - base_value) / base_value) * 100.0 if base_value != 0 else 0.0
        sign = "+" if percentage_diff > 0 else ""

        line = f"{name:<60} {base_value:<15.2f} {new_value:<15.2f} {diff:<15.2f} {sign}{percentage_diff:<9.2f}"
        # print(line)
        output_lines.append(line)

    # Write to file
    with open(output_file, 'w') as f:
        f.write('\n'.join(output_lines))

    print(f"\nComparison results written to: {output_file}")


if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python3 benchmark_compare.py <base_commit> <head_commit> [benchmark_target]")
        sys.exit(1)

    base_commit = sys.argv[1]
    head_commit = sys.argv[2]
    benchmark_target = sys.argv[3] if len(sys.argv) > 3 else "sequant_benchmarks"

    # print info
    print("**" * 50)
    print("SeQuant Benchmark Comparison Script\n")
    print(f"Base commit: {base_commit}")
    print(f"Head commit: {head_commit}")
    print(f"Benchmark target: {benchmark_target}")
    print("**" * 50)

    # Define CMake variables
    cmake_variables = ["-G Ninja",
                       "-DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON",
                       "-DSEQUANT_TESTS=OFF",
                       "-DSEQUANT_EVAL_TESTS=OFF",
                       "-DSEQUANT_BENCHMARKS=ON",
                       "-DSEQUANT_MIMALLOC=ON",
                       "-DSEQUANT_CONTEXT_MANIPULATION_THREADSAFE=ON"]

    print("\nCMake variables:")
    for var in cmake_variables:
        print(f"{var}")

    # base commit
    configure_and_build(base_commit, cmake_variables, benchmark_target)
    run_benchmarks(base_commit, benchmark_target)

    # head commit
    configure_and_build(head_commit, cmake_variables, benchmark_target)
    run_benchmarks(head_commit, benchmark_target)

    # Compare benchmarks
    compare_benchmarks(base_commit, head_commit)
    print("Benchmark comparison completed successfully.")
