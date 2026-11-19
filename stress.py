#!/usr/bin/env python3
"""
Stress tester: generator vs. solution vs. brute force.

Usage:
    python stress.py                                   # 1000 iterations, defaults below
    python stress.py -n 5000 --gen-args "type=tree nmin=2 n=10 shape=random"
    python stress.py --sol fast.cpp --brute slow.cpp --tl 2
    python stress.py --no-compile --start 500          # reuse binaries, begin at seed 500
    python stress.py --sanitize                        # ASan/UBSan (Linux/macOS, not MinGW)
    python stress.py --exact                           # compare output byte-for-byte

The seed is appended to the generator arguments as `seed=<i>`.
On failure the input is saved to fail_in.txt (plus fail_sol.txt / fail_brute.txt).
"""
import argparse
import os
import shlex
import subprocess
import sys
import time

DEFAULT_GEN_ARGS = "type=graph nmin=2 n=8 mmin=1 m=12 weighted wl=1 wr=10"


def exe(name):
    return os.path.abspath(name + (".exe" if os.name == "nt" else ""))


def compile_cpp(src, out, sanitize):
    cmd = ["g++", "-O2", "-std=c++17", "-o", out, src]
    if sanitize:
        cmd[1:1] = ["-fsanitize=address,undefined", "-g"]
    print("compiling", src, "...", flush=True)
    if subprocess.run(cmd).returncode != 0:
        sys.exit("compilation failed: " + src)


def run(binary, stdin_path, stdout_path, timeout, args=()):
    """Run binary; returns (status, returncode) where status is 'ok', 'tle' or 'rte'."""
    with open(stdout_path, "wb") as fout:
        fin = open(stdin_path, "rb") if stdin_path else subprocess.DEVNULL
        try:
            p = subprocess.run([binary, *args], stdin=fin, stdout=fout,
                               stderr=subprocess.PIPE, timeout=timeout)
        except subprocess.TimeoutExpired:
            return "tle", None, b""
        finally:
            if stdin_path:
                fin.close()
    status = "ok" if p.returncode == 0 else "rte"
    return status, p.returncode, p.stderr


def read(path):
    with open(path, "rb") as f:
        return f.read()


def same(a, b, exact):
    if exact:
        return a == b
    return a.split() == b.split()  # whitespace-insensitive token comparison


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("-n", "--iters", type=int, default=1000)
    ap.add_argument("--start", type=int, default=1, help="first seed")
    ap.add_argument("--gen", default="gen.cpp")
    ap.add_argument("--sol", default="sol.cpp")
    ap.add_argument("--brute", default="brute.cpp")
    ap.add_argument("--gen-args", default=DEFAULT_GEN_ARGS)
    ap.add_argument("--tl", type=float, default=5.0, help="time limit for sol (seconds)")
    ap.add_argument("--no-compile", action="store_true")
    ap.add_argument("--sanitize", action="store_true", help="build sol with ASan/UBSan")
    ap.add_argument("--exact", action="store_true", help="exact output comparison")
    a = ap.parse_args()

    g, s, b = exe("gen"), exe("sol"), exe("brute")
    if not a.no_compile:
        compile_cpp(a.gen, g, False)
        compile_cpp(a.sol, s, a.sanitize)
        compile_cpp(a.brute, b, False)

    gen_args = shlex.split(a.gen_args)
    t0 = time.time()
    for i in range(a.start, a.start + a.iters):
        st, rc, err = run(g, None, "in.txt", 10, gen_args + [f"seed={i}"])
        if st != "ok":
            sys.exit(f"generator failed on seed {i}: {err.decode(errors='replace')}")

        st, rc, err = run(s, "in.txt", "out.txt", a.tl)
        if st != "ok":
            label = "Time limit exceeded" if st == "tle" else f"Runtime error (exit code {rc})"
            print(f"{label} on seed {i}")
            if err:
                print(err.decode(errors="replace"))
            os.replace("in.txt", "fail_in.txt")
            print("input saved to fail_in.txt")
            sys.exit(1)

        st, rc, err = run(b, "in.txt", "ans.txt", 60)
        if st != "ok":
            sys.exit(f"brute failed ({st}, code {rc}) on seed {i} - fix the brute force first")

        out, ans = read("out.txt"), read("ans.txt")
        if not same(out, ans, a.exact):
            print(f"Wrong answer on seed {i}")
            os.replace("in.txt", "fail_in.txt")
            os.replace("out.txt", "fail_sol.txt")
            os.replace("ans.txt", "fail_brute.txt")
            data = read("fail_in.txt")
            if len(data) < 2000:
                print("--- input ---\n" + data.decode(errors="replace"))
                print("--- yours ---\n" + read("fail_sol.txt").decode(errors="replace"))
                print("--- brute ---\n" + read("fail_brute.txt").decode(errors="replace"))
            print("saved fail_in.txt, fail_sol.txt, fail_brute.txt")
            sys.exit(1)

        if (i - a.start + 1) % 100 == 0:
            print(f"{i - a.start + 1} tests passed ({time.time() - t0:.1f}s)", flush=True)

    print(f"All {a.iters} tests passed")


if __name__ == "__main__":
    main()
