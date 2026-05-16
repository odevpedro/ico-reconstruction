# Runtime Probe Analyzer

Reads JSONL emitted by the patched PCSX2 runtime probe and prints compact
runtime evidence for decompilation work.

The first POC log path is controlled by the PCSX2-side environment variable:

```sh
ICO_PCSX2_PROBE_LOG=/home/peter/Documentos/repos/ico-reconstruction/.local/runtime-captures/ico-probe/events.jsonl
```

Run a live monitor:

```sh
python3 tools/runtime-probe-analyzer/runtime_probe_analyzer.py \
  --input .local/runtime-captures/ico-probe/events.jsonl \
  --follow
```

Run a summary after a session:

```sh
python3 tools/runtime-probe-analyzer/runtime_probe_analyzer.py \
  --input .local/runtime-captures/ico-probe/events.jsonl \
  --summary
```

This tool reads only local generated logs. It does not require proprietary game
data in the repository.
