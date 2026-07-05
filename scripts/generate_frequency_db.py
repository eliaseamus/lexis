#!/usr/bin/env python3
"""Generate Lexis word-frequency SQLite database from wordfreq."""

from __future__ import annotations

import argparse
import math
import sqlite3
import sys
from datetime import datetime, timezone
from pathlib import Path

try:
    import wordfreq
except ImportError as exc:
    print(
        "wordfreq is required. Install with:\n"
        "  pip install -r scripts/requirements.txt",
        file=sys.stderr,
    )
    raise SystemExit(1) from exc

LEXIS_LANGUAGES = ("en", "es", "de", "fr", "ru", "it", "pl", "uk", "tr")
WORDLIST = "best"
BATCH_SIZE = 10_000

TIER_BANDS = (
    (4.0, "core"),
    (3.0, "common"),
    (2.0, "intermediate"),
    (1.0, "advanced"),
)


def tier_from_zipf(zipf: float) -> str:
    for minimum_zipf, label in TIER_BANDS:
        if zipf >= minimum_zipf:
            return label
    return "rare"


def zipf_from_frequency(frequency: float) -> float:
    if frequency <= 0.0:
        return 0.0
    return math.log10(frequency * 1e9)


def normalize_word(word: str) -> str:
    return word.casefold()


def ensure_schema(connection: sqlite3.Connection) -> None:
    connection.executescript(
        """
        CREATE TABLE IF NOT EXISTS metadata (
          key TEXT PRIMARY KEY,
          value TEXT NOT NULL
        );

        CREATE TABLE IF NOT EXISTS word_frequencies (
          language_code TEXT NOT NULL,
          word TEXT NOT NULL,
          rank INTEGER NOT NULL,
          zipf REAL NOT NULL,
          tier TEXT NOT NULL,
          PRIMARY KEY (language_code, word)
        );

        CREATE INDEX IF NOT EXISTS idx_word_frequencies_tier
          ON word_frequencies(language_code, tier);
        """
    )


def write_metadata(connection: sqlite3.Connection, languages: tuple[str, ...]) -> None:
    connection.execute("DELETE FROM metadata")
    rows = [
        ("generated_at", datetime.now(timezone.utc).isoformat()),
        ("wordfreq_version", getattr(wordfreq, "__version__", "unknown")),
        ("wordlist", WORDLIST),
        ("languages", ",".join(languages)),
        (
            "tier_bands",
            "zipf>=4.0:core,zipf>=3.0:common,zipf>=2.0:intermediate,"
            "zipf>=1.0:advanced,zipf<1.0:rare",
        ),
    ]
    connection.executemany("INSERT INTO metadata(key, value) VALUES (?, ?)", rows)


def populate_language(connection: sqlite3.Connection, language_code: str) -> int:
    available = wordfreq.available_languages(WORDLIST)
    if language_code not in available:
        raise ValueError(f"Language {language_code!r} is not supported by wordfreq")

    frequency_dict = wordfreq.get_frequency_dict(language_code, wordlist=WORDLIST)
    ranked_words = sorted(frequency_dict.items(), key=lambda item: -item[1])

    connection.execute(
        "DELETE FROM word_frequencies WHERE language_code = ?",
        (language_code,),
    )

    batch: list[tuple[str, str, int, float, str]] = []
    inserted = 0

    for rank, (word, frequency) in enumerate(ranked_words, start=1):
        zipf = zipf_from_frequency(frequency)
        batch.append(
            (
                language_code,
                normalize_word(word),
                rank,
                zipf,
                tier_from_zipf(zipf),
            )
        )
        if len(batch) >= BATCH_SIZE:
            connection.executemany(
                "INSERT INTO word_frequencies"
                "(language_code, word, rank, zipf, tier)"
                "VALUES (?, ?, ?, ?, ?)",
                batch,
            )
            inserted += len(batch)
            batch.clear()

    if batch:
        connection.executemany(
            "INSERT INTO word_frequencies"
            "(language_code, word, rank, zipf, tier)"
            "VALUES (?, ?, ?, ?, ?)",
            batch,
        )
        inserted += len(batch)

    return inserted


def generate_database(output_path: Path, languages: tuple[str, ...]) -> None:
    output_path.parent.mkdir(parents=True, exist_ok=True)
    if output_path.exists():
        output_path.unlink()

    connection = sqlite3.connect(output_path)
    try:
        connection.execute("PRAGMA journal_mode = OFF")
        connection.execute("PRAGMA synchronous = OFF")
        ensure_schema(connection)

        total_rows = 0
        for language_code in languages:
            print(f"Generating {language_code}...", flush=True)
            total_rows += populate_language(connection, language_code)
            connection.commit()
            print(f"  inserted {total_rows} rows so far", flush=True)

        write_metadata(connection, languages)
        connection.commit()
        connection.execute("VACUUM")
    finally:
        connection.close()

    size_mb = output_path.stat().st_size / (1024 * 1024)
    print(f"Wrote {output_path} ({total_rows} rows, {size_mb:.1f} MiB)")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--output",
        type=Path,
        default=Path(__file__).resolve().parent.parent / "data" / "frequency.db",
        help="Output SQLite database path (default: data/frequency.db)",
    )
    parser.add_argument(
        "--languages",
        nargs="+",
        default=list(LEXIS_LANGUAGES),
        help="Language codes to include (default: all Lexis learning languages)",
    )
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    languages = tuple(dict.fromkeys(args.languages))
    generate_database(args.output.resolve(), languages)


if __name__ == "__main__":
    main()
