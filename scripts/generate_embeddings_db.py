#!/usr/bin/env python3
"""Generate Lexis word-embedding SQLite database from fastText crawl vectors."""

from __future__ import annotations

import argparse
import gzip
import sqlite3
import struct
import sys
import urllib.request
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
BATCH_SIZE = 2_000
FASTTEXT_VEC_URL = (
    "https://dl.fbaipublicfiles.com/fasttext/vectors-crawl/cc.{lang}.300.vec.gz"
)
EMBEDDING_DIMENSIONS = 300


def normalize_word(word: str) -> str:
    return word.casefold()


def ensure_schema(connection: sqlite3.Connection) -> None:
    connection.executescript(
        """
        CREATE TABLE IF NOT EXISTS metadata (
          key TEXT PRIMARY KEY,
          value TEXT NOT NULL
        );

        CREATE TABLE IF NOT EXISTS word_embeddings (
          language_code TEXT NOT NULL,
          word TEXT NOT NULL,
          vector BLOB NOT NULL,
          PRIMARY KEY (language_code, word)
        );

        CREATE INDEX IF NOT EXISTS idx_word_embeddings_language
          ON word_embeddings(language_code);
        """
    )


def write_metadata(
    connection: sqlite3.Connection,
    languages: tuple[str, ...],
    max_words: int,
) -> None:
    connection.execute("DELETE FROM metadata")
    rows = [
        ("generated_at", datetime.now(timezone.utc).isoformat()),
        ("model", "fasttext-crawl"),
        ("dimensions", str(EMBEDDING_DIMENSIONS)),
        ("wordlist", WORDLIST),
        ("max_words_per_language", str(max_words)),
        ("languages", ",".join(languages)),
        ("source_url", FASTTEXT_VEC_URL),
    ]
    connection.executemany("INSERT INTO metadata(key, value) VALUES (?, ?)", rows)


def wanted_words(language_code: str, max_words: int) -> set[str]:
    available = wordfreq.available_languages(WORDLIST)
    if language_code not in available:
        raise ValueError(f"Language {language_code!r} is not supported by wordfreq")

    frequency_dict = wordfreq.get_frequency_dict(language_code, wordlist=WORDLIST)
    ranked_words = sorted(frequency_dict.items(), key=lambda item: -item[1])
    return {normalize_word(word) for word, _ in ranked_words[:max_words]}


def download_vector_file(language_code: str, cache_dir: Path) -> Path:
    cache_dir.mkdir(parents=True, exist_ok=True)
    destination = cache_dir / f"cc.{language_code}.300.vec.gz"
    if destination.exists() and destination.stat().st_size > 0:
        return destination

    url = FASTTEXT_VEC_URL.format(lang=language_code)
    print(f"  downloading {url}...", flush=True)
    tmp_path = destination.with_suffix(".part")
    with urllib.request.urlopen(url) as response, tmp_path.open("wb") as handle:
        while True:
            chunk = response.read(1024 * 1024)
            if not chunk:
                break
            handle.write(chunk)
    tmp_path.replace(destination)
    return destination


def vector_to_blob(values: list[float]) -> bytes:
    if len(values) != EMBEDDING_DIMENSIONS:
        raise ValueError(f"Expected {EMBEDDING_DIMENSIONS} dimensions, got {len(values)}")
    return struct.pack(f"<{EMBEDDING_DIMENSIONS}f", *values)


def populate_language(
    connection: sqlite3.Connection,
    language_code: str,
    max_words: int,
    cache_dir: Path,
) -> int:
    targets = wanted_words(language_code, max_words)
    if not targets:
        return 0

    vector_path = download_vector_file(language_code, cache_dir)
    connection.execute(
        "DELETE FROM word_embeddings WHERE language_code = ?",
        (language_code,),
    )

    batch: list[tuple[str, str, bytes]] = []
    inserted = 0
    remaining = len(targets)

    with gzip.open(vector_path, "rt", encoding="utf-8", errors="ignore") as handle:
        header = handle.readline().split()
        if len(header) != 2:
            raise ValueError(f"Invalid fastText header in {vector_path}")
        dimensions = int(header[1])
        if dimensions != EMBEDDING_DIMENSIONS:
            raise ValueError(
                f"Expected {EMBEDDING_DIMENSIONS} dimensions, got {dimensions} in {vector_path}"
            )

        for line in handle:
            if remaining <= 0:
                break
            parts = line.rstrip().split(" ")
            if len(parts) != EMBEDDING_DIMENSIONS + 1:
                continue
            word = normalize_word(parts[0])
            if word not in targets:
                continue
            values = [float(value) for value in parts[1:]]
            batch.append((language_code, word, vector_to_blob(values)))
            targets.remove(word)
            remaining -= 1
            if len(batch) >= BATCH_SIZE:
                connection.executemany(
                    "INSERT INTO word_embeddings(language_code, word, vector)"
                    "VALUES (?, ?, ?)",
                    batch,
                )
                inserted += len(batch)
                batch.clear()

    if batch:
        connection.executemany(
            "INSERT INTO word_embeddings(language_code, word, vector)"
            "VALUES (?, ?, ?)",
            batch,
        )
        inserted += len(batch)

    if remaining > 0:
        print(
            f"  warning: {remaining} requested words were missing from fastText vectors",
            flush=True,
        )

    return inserted


def generate_database(
    output_path: Path,
    languages: tuple[str, ...],
    max_words: int,
    cache_dir: Path,
) -> None:
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
            total_rows += populate_language(connection, language_code, max_words, cache_dir)
            connection.commit()
            print(f"  inserted {total_rows} rows so far", flush=True)

        write_metadata(connection, languages, max_words)
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
        default=Path(__file__).resolve().parent.parent / "data" / "embeddings.db",
        help="Output SQLite database path (default: data/embeddings.db)",
    )
    parser.add_argument(
        "--cache-dir",
        type=Path,
        default=Path(__file__).resolve().parent.parent / "data" / "fasttext-cache",
        help="Directory for downloaded fastText vector files",
    )
    parser.add_argument(
        "--max-words",
        type=int,
        default=25_000,
        help="Maximum number of frequent words to store per language",
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
    generate_database(
        args.output.resolve(),
        languages,
        args.max_words,
        args.cache_dir.resolve(),
    )


if __name__ == "__main__":
    main()
