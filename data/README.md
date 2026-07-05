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

## Tier bands (by Zipf score)

Tiers use [Zipf frequency](https://en.wikipedia.org/wiki/Zipf%27s_law#Use_in_measures_of_diversity) (log scale), not corpus rank. Rank alone mislabels common words when the dictionary contains hundreds of thousands of lemmas.

| Zipf | Tier |
|------|------|
| ≥ 4.0 | core |
| ≥ 3.0 | common |
| ≥ 2.0 | intermediate |
| ≥ 1.0 | advanced |
| < 1.0 | rare |

Examples (English): *thing* → core (5.7), *frog* → common (3.9), *freckle* → intermediate (2.4).

Words are stored case-insensitively. Lookup is implemented in `FrequencyLookup` (`src/db/frequency_lookup.cpp`).

The generated `data/frequency.db` is gitignored (~200 MiB). Copy it next to the Lexis binary
automatically on build when present.

# Word embeddings (group suggestions)

Lexis group suggestions can use pre-trained [fastText crawl vectors](https://fasttext.cc/docs/en/crawl-vectors.html)
stored in a bundled SQLite database. At runtime, `TextEmbedding` averages token vectors and compares
groups by cosine similarity to a group centroid (`src/ml/text_embedding.cpp`).

## Generate `data/embeddings.db`

```bash
python3 -m venv .venv-wordfreq
.venv-wordfreq/bin/pip install -r scripts/requirements.txt
.venv-wordfreq/bin/python scripts/generate_embeddings_db.py --languages en --max-words 5000
```

The first run downloads the fastText `.vec.gz` file for each language into `data/fasttext-cache/`.
Generating all nine Lexis languages at 25,000 words each requires several GiB of downloads and
produces a database on the order of hundreds of MiB.

Or via CMake:

```bash
cmake --build build --target generate_embeddings_db
```

When `data/embeddings.db` is present, it is copied next to the Lexis executable on build. Without
it, group suggestions fall back to the earlier token-overlap heuristic.
