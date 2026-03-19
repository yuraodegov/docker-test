#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from __future__ import annotations
import argparse
import dataclasses
import json
import os
import re
import sys
import time
from typing import Dict, Optional, Tuple, List

DEFAULT_BAUD = 115200
READ_TIMEOUT_S = 1.5
RETRIES = 2

GREEN = "\033[92m"
RED = "\033[91m"
RESET = "\033[0m"


# ===================== Транспорт =====================
class Transport:
    def exec(self, cmd: str) -> str:
        raise NotImplementedError


class SerialTransport(Transport):
    def __init__(self, port: str, baud: int = DEFAULT_BAUD, timeout: float = READ_TIMEOUT_S):
        import serial
        self._ser = serial.Serial(port=port, baudrate=baud, timeout=timeout)

    def exec(self, cmd: str) -> str:
        self._ser.write((cmd + "\n").encode())
        return self._ser.readline().decode().strip()


class DummyTransport(Transport):
    def __init__(self):
        self.state: Dict[str, Dict[str, float]] = {"get_outputs_1": {}}

    def exec(self, cmd: str) -> str:
        m = re.match(r"^(set_[^:]+):([A-Za-z0-9_]+)=(.+)$", cmd)
        if m:
            _, key, val = m.groups()
            self.state["get_outputs_1"][key] = int(val)
            return "OK"

        m = re.match(r"^(get_[A-Za-z0-9_]+)$", cmd)
        if m:
            opcode = m.group(1)
            payload = ",".join(f"{k}={v}" for k, v in self.state.get(opcode, {}).items())
            return f"{opcode}:{payload};"

        return "ERR"


# ===================== Парсинг =====================
KV_RE = re.compile(r"^(?P<opcode>[^:]+):(?P<body>.*?);?$")
PAIR_RE = re.compile(r"([A-Za-z0-9_]+)=([^,;]+)")


def parse_kv(s: str):
    m = KV_RE.match(s.strip())
    opcode = m.group("opcode")
    body = m.group("body")

    out = {}
    for k, v in PAIR_RE.findall(body):
        out[k] = int(v)
    return opcode, out


# ===================== Модель =====================
@dataclasses.dataclass
class Step:
    send: str
    expect_get: str
    expect_field: str
    expect_value: int


@dataclasses.dataclass
class Case:
    name: str
    steps: List[Step]


@dataclasses.dataclass
class Result:
    case: str
    step_index: int
    ok: bool
    command: str
    expect_get: str
    expect_field: str
    expect_value: int
    got_value: Optional[int]
    message: str


# ===================== Тесты =====================
def build_cases():
    cases = []
    for f in ["SHABBAT", "MENU", "EXH", "HJ", "HC"]:
        cases.append(Case(
            name=f,
            steps=[
                Step(f"set_output_1:{f}=1", "get_outputs_1", f, 1),
                Step(f"set_output_1:{f}=0", "get_outputs_1", f, 0),
            ]
        ))
    return cases


# ===================== Исполнение =====================
def run_case(tp: Transport, case: Case) -> List[Result]:
    results = []

    for i, step in enumerate(case.steps):
        tp.exec(step.send)

        raw = tp.exec(step.expect_get)
        _, kv = parse_kv(raw)

        got = kv.get(step.expect_field)
        ok = (got == step.expect_value)

        msg = "OK" if ok else f"expected {step.expect_value}, got {got}"

        results.append(Result(
            case=case.name,
            step_index=i,
            ok=ok,
            command=step.send,
            expect_get=step.expect_get,
            expect_field=step.expect_field,
            expect_value=step.expect_value,
            got_value=got,
            message=msg
        ))

    return results


# ===================== Summary =====================
def print_summary(results: List[Result], txt_path: str):
    with open(txt_path, "w", encoding="utf-8") as f:
        for r in results:
            status = "OK" if r.ok else "FAIL"
            f.write(f"[{status}] {r.case} step#{r.step_index}: {r.message}\n")


# ===================== main =====================
def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--dry-run", action="store_true")
    args = parser.parse_args()

    tp = DummyTransport() if args.dry_run else None

    json_path, txt_path = ("/app/output/log.json", "/app/output/summary.txt")##json_path, txt_path = ("log.json", "summary.txt")

    cases = build_cases()
    all_results = []

    print("START\n")

    for case in cases:
        results = run_case(tp, case)

        for r in results:
            status = "OK" if r.ok else "FAIL"
            color = GREEN if r.ok else RED

            print(f"{color}{status}{RESET} {r.case} step#{r.step_index}")

            all_results.append(r)

    with open(json_path, "w", encoding="utf-8") as f:
        json.dump([dataclasses.asdict(r) for r in all_results], f, indent=2)

    print_summary(all_results, txt_path)

    print("\nDONE")


if __name__ == "__main__":
    main()
