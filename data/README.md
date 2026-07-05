# Word frequency data

Lexis word popularity labels (`core`, `common`, `intermediate`, `advanced`, `rare`) come from
a bundled SQLite database generated with the [wordfreq](https://github.com/rspeer/wordfreq)
Python package.

## Generate `data/frequency.db`

```bash
python3 -m venv .venv-wordfreq
.venv-wordfreq/bin/pip install -r scripts/requirements.txt
.venv-wordfreq/bin/python scripts/generate_frequency_db.py
```

Or via CMake (requires Python 3 + wordfreq installed for the interpreter CMake finds):

```bash
cmake -B build
cmake --build build --target generate_frequency_db
```

The database covers all nine Lexis learning languages:

`en`, `es`, `de`, `fr`, `ru`, `it`, `pl`, `uk`, `tr`

## Tier bands (by corpus rank)

| Rank | Tier |
|------|------|
| 1–5,000 | core |
| 5,001–10,000 | common |
| 10,001–15,000 | intermediate |
| 15,001–25,000 | advanced |
| 25,001+ | rare |

Words are stored case-insensitively. Lookup is implemented in `FrequencyLookup` (`src/db/frequency_lookup.cpp`).

The generated `data/frequency.db` is gitignored (~200 MiB). Copy it next to the Lexis binary
automatically on build when present.
