#!/usr/bin/env python3
"""Generate Lexis word-embedding SQLite database.

Vectors come from a pre-trained gensim model (GloVe by default) and are
restricted to the most frequent words of the language (via wordfreq) to keep
the database small. Each vector is L2-normalized and quantized to int8, so a
100-dimensional embedding costs 104 bytes: a float32 scale followed by the
quantized components.
"""

from __future__ import annotations

import argparse
import sqlite3
import struct
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

DEFAULT_MODEL = "glove-wiki-gigaword-100"
DEFAULT_LANGUAGE = "en"
DEFAULT_MAX_WORDS = 60_000
WORDLIST = "best"
BATCH_SIZE = 5_000


def quantize(vector) -> bytes:
    """Serialize a vector as float32 scale + int8 components."""
    norm = sum(component * component for component in vector) ** 0.5
    if norm == 0.0:
        return b""
    unit = [component / norm for component in vector]
    peak = max(abs(component) for component in unit)
    if peak == 0.0:
        return b""
    scale = peak / 127.0
    quantized = [max(-127, min(127, round(component / scale))) for component in unit]
    return struct.pack("<f", scale) + struct.pack(f"{len(quantized)}b", *quantized)


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
          rank INTEGER NOT NULL,
          vector BLOB NOT NULL,
          PRIMARY KEY (language_code, word)
        );

        CREATE INDEX IF NOT EXISTS idx_word_embeddings_rank
          ON word_embeddings(language_code, rank);
        """
    )


def load_model(model_name: str):
    try:
        import gensim.downloader
    except ImportError as exc:
        print(
            "gensim is required. Install with:\n"
            "  pip install -r scripts/requirements.txt",
            file=sys.stderr,
        )
        raise SystemExit(1) from exc

    print(f"Loading model {model_name} (this may download it once)...", flush=True)
    return gensim.downloader.load(model_name)


def populate_language(
    connection: sqlite3.Connection,
    model,
    language_code: str,
    max_words: int,
) -> int:
    frequency_dict = wordfreq.get_frequency_dict(language_code, wordlist=WORDLIST)
    ranked_words = sorted(frequency_dict.items(), key=lambda item: -item[1])

    connection.execute(
        "DELETE FROM word_embeddings WHERE language_code = ?",
        (language_code,),
    )

    batch: list[tuple[str, str, int, bytes]] = []
    inserted = 0
    rank = 0

    for word, _frequency in ranked_words:
        if inserted >= max_words:
            break
        normalized = word.casefold()
        if normalized not in model:
            continue
        blob = quantize(model[normalized])
        if not blob:
            continue
        rank += 1
        batch.append((language_code, normalized, rank, blob))
        inserted += 1
        if len(batch) >= BATCH_SIZE:
            connection.executemany(
                "INSERT OR IGNORE INTO word_embeddings"
                "(language_code, word, rank, vector) VALUES (?, ?, ?, ?)",
                batch,
            )
            batch.clear()

    if batch:
        connection.executemany(
            "INSERT OR IGNORE INTO word_embeddings"
            "(language_code, word, rank, vector) VALUES (?, ?, ?, ?)",
            batch,
        )

    return inserted


def write_metadata(
    connection: sqlite3.Connection, model_name: str, dimensions: int, languages: list[str]
) -> None:
    connection.execute("DELETE FROM metadata")
    rows = [
        ("generated_at", datetime.now(timezone.utc).isoformat()),
        ("model", model_name),
        ("dimensions", str(dimensions)),
        ("languages", ",".join(languages)),
        ("format", "float32 scale + int8 components, unit-normalized"),
    ]
    connection.executemany("INSERT INTO metadata(key, value) VALUES (?, ?)", rows)


def generate_database(
    output_path: Path, model_name: str, languages: list[str], max_words: int
) -> None:
    model = load_model(model_name)
    dimensions = int(model.vector_size)

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
            total_rows += populate_language(connection, model, language_code, max_words)
            connection.commit()
            print(f"  inserted {total_rows} rows so far", flush=True)

        write_metadata(connection, model_name, dimensions, languages)
        connection.commit()
        connection.execute("VACUUM")
    finally:
        connection.close()

    size_mb = output_path.stat().st_size / (1024 * 1024)
    print(f"Wrote {output_path} ({total_rows} rows, {dimensions} dims, {size_mb:.1f} MiB)")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--output",
        type=Path,
        default=Path(__file__).resolve().parent.parent / "data" / "embeddings.db",
        help="Output SQLite database path (default: data/embeddings.db)",
    )
    parser.add_argument(
        "--model",
        default=DEFAULT_MODEL,
        help=f"gensim model name (default: {DEFAULT_MODEL})",
    )
    parser.add_argument(
        "--languages",
        nargs="+",
        default=[DEFAULT_LANGUAGE],
        help="Language codes to include; the model must match the language "
        f"(default: {DEFAULT_LANGUAGE})",
    )
    parser.add_argument(
        "--max-words",
        type=int,
        default=DEFAULT_MAX_WORDS,
        help=f"Maximum words per language (default: {DEFAULT_MAX_WORDS})",
    )
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    generate_database(args.output.resolve(), args.model, args.languages, args.max_words)


if __name__ == "__main__":
    main()
