# verify-local-copy

Metadata-only verifier for a local user-owned ICO input.

The tool records file or directory metadata and SHA-256 hashes into a local JSON report. It does not copy, extract, patch, decode, or redistribute game data.

## Usage

```bash
python3 tools/verify-local-copy/verify_local_copy.py /path/to/local/input
```

Default reports are written under:

```text
.local/reports/
```

That directory is ignored by git.

## Options

```bash
python3 tools/verify-local-copy/verify_local_copy.py /path/to/input --output-dir .local/reports
python3 tools/verify-local-copy/verify_local_copy.py /path/to/input --no-hash
python3 tools/verify-local-copy/verify_local_copy.py /path/to/input --max-files 500
```

## Safe Output

Reports may include:

- input path on the local machine
- file names
- sizes
- modification timestamps
- SHA-256 hashes
- aggregate totals

Do not commit reports generated from a real game copy unless they have been reviewed and contain metadata only.
