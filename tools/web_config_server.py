#!/usr/bin/env python3

from __future__ import annotations

import json
import os
import posixpath
import sys
from http import HTTPStatus
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from urllib.error import HTTPError, URLError
from urllib.parse import parse_qs, quote, urlparse
from urllib.request import Request, urlopen

WEB_DIR = Path(__file__).resolve().parent.parent / "web"
SEARCH_LIMIT = 20
ALLOWED_CLASSIFY = {
    "AStock": "stock",
    "Index": "index",
}


def normalize_market_query(raw_value: str) -> str:
    return "".join(raw_value.split()).lower()


def request_symbol_from_quote_id(quote_id: str, code: str) -> str:
    if not quote_id or "." not in quote_id:
        return ""
    market_prefix, raw_code = quote_id.split(".", 1)
    final_code = (raw_code or code).strip()
    if not final_code:
        return ""
    if market_prefix == "1":
        return f"sh{final_code}"
    if market_prefix == "0":
        return f"sz{final_code}"
    return ""


def search_markets(query: str) -> list[dict[str, str]]:
    normalized_query = normalize_market_query(query)
    if not normalized_query:
        return []
    upstream_query = normalized_query
    if (
        len(normalized_query) > 2
        and normalized_query[:2] in {"sh", "sz"}
        and normalized_query[2:].isdigit()
    ):
        upstream_query = normalized_query[2:]

    url = (
        "https://searchapi.eastmoney.com/api/suggest/get"
        f"?input={quote(upstream_query)}&type=14&count={SEARCH_LIMIT}"
    )
    request = Request(
        url,
        headers={
            "Accept": "application/json",
            "Referer": "https://quote.eastmoney.com/",
            "User-Agent": "M5PaperClock/1.0",
        },
    )

    with urlopen(request, timeout=8) as response:
        payload = json.load(response)

    rows = (
        payload.get("QuotationCodeTable", {})
        .get("Data", [])
        or []
    )
    results: list[dict[str, str]] = []
    seen: set[str] = set()
    for row in rows:
        kind = ALLOWED_CLASSIFY.get(str(row.get("Classify", "")).strip())
        if not kind:
            continue

        code = str(row.get("Code", "")).strip()
        request_symbol = request_symbol_from_quote_id(
            str(row.get("QuoteID", "")).strip(),
            code,
        )
        if not code or not request_symbol:
            continue

        if not (
            code.lower().startswith(normalized_query)
            or request_symbol.lower().startswith(normalized_query)
        ):
            continue

        dedupe_key = request_symbol.lower()
        if dedupe_key in seen:
            continue
        seen.add(dedupe_key)

        results.append(
            {
                "requestSymbol": request_symbol,
                "code": code,
                "displayName": str(row.get("Name", "")).strip() or code,
                "kind": kind,
            }
        )
    return results


class WebConfigHandler(SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=str(WEB_DIR), **kwargs)

    def end_headers(self) -> None:
        self.send_header("Cache-Control", "no-store")
        super().end_headers()

    def do_GET(self) -> None:
        parsed = urlparse(self.path)
        if parsed.path == "/api/market-search":
            self.handle_market_search(parsed)
            return
        super().do_GET()

    def translate_path(self, path: str) -> str:
        parsed = urlparse(path)
        clean_path = parsed.path
        clean_path = posixpath.normpath(clean_path)
        parts = [part for part in clean_path.split("/") if part and part not in {".", ".."}]
        resolved = WEB_DIR
        for part in parts:
            resolved /= part
        return str(resolved)

    def handle_market_search(self, parsed) -> None:
        query = parse_qs(parsed.query).get("q", [""])[0]
        try:
            payload = {
                "ok": True,
                "query": normalize_market_query(query),
                "results": search_markets(query),
            }
            self.write_json(HTTPStatus.OK, payload)
        except HTTPError as error:
            self.write_json(
                HTTPStatus.BAD_GATEWAY,
                {
                    "ok": False,
                    "error": f"行情搜索失败：HTTP {error.code}",
                },
            )
        except URLError as error:
            self.write_json(
                HTTPStatus.BAD_GATEWAY,
                {
                    "ok": False,
                    "error": f"行情搜索失败：{error.reason}",
                },
            )
        except Exception as error:  # noqa: BLE001
            self.write_json(
                HTTPStatus.INTERNAL_SERVER_ERROR,
                {
                    "ok": False,
                    "error": f"行情搜索失败：{error}",
                },
            )

    def log_message(self, format: str, *args) -> None:
        sys.stdout.write("%s - - [%s] %s\n" % (self.address_string(), self.log_date_time_string(), format % args))
        sys.stdout.flush()

    def write_json(self, status: HTTPStatus, payload: dict) -> None:
        body = json.dumps(payload, ensure_ascii=False).encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Content-Length", str(len(body)))
        self.send_header("Access-Control-Allow-Origin", "*")
        self.end_headers()
        self.wfile.write(body)


def main() -> int:
    if not WEB_DIR.is_dir():
        print(f"web directory not found: {WEB_DIR}", file=sys.stderr)
        return 1

    port = int(sys.argv[1]) if len(sys.argv) > 1 else 4173
    server = ThreadingHTTPServer(("127.0.0.1", port), WebConfigHandler)
    print(f"Serving Web Serial config UI at http://localhost:{port}")
    print("Press Ctrl+C to stop.")
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nStopping server.")
    finally:
        server.server_close()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
