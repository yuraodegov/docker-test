import json
import sys

def analyze(filepath):
    with open(filepath) as f:
        logs = json.load(f)

    total   = len(logs)
    passed  = sum(1 for e in logs if e["ok"])
    failed  = total - passed

    # Считаем по case
    cases = {}
    for e in logs:
        case = e["case"]
        if case not in cases:
            cases[case] = {"ok": 0, "fail": 0}
        if e["ok"]:
            cases[case]["ok"] += 1
        else:
            cases[case]["fail"] += 1

    # Отчёт
    print("=" * 40)
    print(f"  MONITORING REPORT")
    print("=" * 40)
    print(f"  Total steps : {total}")
    print(f"  Passed      : {passed} ✅")
    print(f"  Failed      : {failed} {'✅' if failed == 0 else '❌'}")
    print("-" * 40)
    print("  Results by case:")
    for case, stat in cases.items():
        status = "✅" if stat["fail"] == 0 else "❌"
        print(f"  {status} {case:10} ok={stat['ok']} fail={stat['fail']}")
    print("-" * 40)

    # Показываем ошибки если есть
    errors = [e for e in logs if not e["ok"]]
    if errors:
        print("  FAILURES:")
        for e in errors:
            print(f"  ❌ case={e['case']} step={e['step_index']}")
            print(f"     cmd: {e['command']}")
            print(f"     expected={e['expect_value']} got={e['got_value']}")
    else:
        print("  No failures!")
    print("=" * 40)

    # Возвращаем код ошибки для CI
    return 1 if failed > 0 else 0

if __name__ == "__main__":
    path = sys.argv[1] if len(sys.argv) > 1 else "log.json"
    exit(analyze(path))
