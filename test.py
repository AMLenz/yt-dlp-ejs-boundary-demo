import sys
import solver_boundary_py


def print_usage(prog: str) -> None:
    print("Usage:")
    print(f"  python3 {prog} local <player_path> <sig_or_dash> <challenge> [<type> <challenge> ...]")
    print(f"  python3 {prog} watch <watch_url> <sig_or_n> <challenge> [<type> <challenge> ...]")
    print()
    print("Optional per environment or by editing defaults below:")
    print("  solver_dir     = '.'")
    print("  meriyah_path   = 'meriyah.umd.js'")
    print("  astring_path   = 'astring.min.js'")
    print()
    print("Examples:")
    print(f"  python3 {prog} local base.js sig AHEq...")
    print(f"  python3 {prog} local base.js sig AHEq... n llfPRf5kII7KWfGhoJg")
    print(f"  python3 {prog} watch https://www.youtube.com/watch?v=tjSnrDikc4M n llfPRf5kII7KWfGhoJg")
    print(f"  python3 {prog} watch https://www.youtube.com/watch?v=tjSnrDikc4M sig AHEq... n llfPRf5kII7KWfGhoJg")


def parse_requests(args: list[str]) -> list[dict]:
    if len(args) == 0 or len(args) % 2 != 0:
        raise ValueError("requests must be given as pairs: <type> <challenge>")

    out: list[dict] = []
    for i in range(0, len(args), 2):
        req_type = args[i].strip()
        challenge = args[i + 1].strip()

        if req_type not in ("sig", "n"):
            raise ValueError(f"unsupported request type: {req_type}")
        if not challenge:
            raise ValueError(f"empty challenge for request type: {req_type}")

        out.append({
            "type": req_type,
            "challenge": challenge,
        })
    return out


def print_responses(responses: list[dict]) -> None:
    print(f"\nResponses: {len(responses)}")
    for i, r in enumerate(responses, start=1):
        print(f"\n{i}:")
        print(f"  type : {r.get('type', '')}")
        print(f"  ok   : {r.get('ok', False)}")
        print(f"  data : {r.get('data', '')}")
        print(f"  error: {r.get('error', '')}")


def main() -> int:
    if len(sys.argv) < 5:
        print_usage(sys.argv[0])
        return 1

    mode = sys.argv[1].strip().lower()
    target = sys.argv[2].strip()

    solver_dir = "."
    meriyah_path = "meriyah.umd.js"
    astring_path = "astring.min.js"

    try:
        requests = parse_requests(sys.argv[3:])
    except ValueError as e:
        print(f"Error: {e}")
        print()
        print_usage(sys.argv[0])
        return 2

    try:
        if mode == "local":
            responses = solver_boundary_py.solve_requests(
                player_path=target,
                requests=requests,
                solver_dir=solver_dir,
                meriyah_path=meriyah_path,
                astring_path=astring_path,
            )
        elif mode == "watch":
            responses = solver_boundary_py.solve_requests_from_watch(
                watch_url=target,
                requests=requests,
                solver_dir=solver_dir,
                meriyah_path=meriyah_path,
                astring_path=astring_path,
            )
        else:
            print(f"Error: unsupported mode: {mode}")
            print()
            print_usage(sys.argv[0])
            return 3

        print_responses(responses)
        return 0

    except Exception as e:
        print(f"Runtime error: {e}")
        return 4


if __name__ == "__main__":
    raise SystemExit(main())
