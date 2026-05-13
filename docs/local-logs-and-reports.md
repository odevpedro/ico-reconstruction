# Local Logs and Reports

> Where AI agents and contributors should look after local tool runs or manual PCSX2 tests.

## Project Reports

Project-generated metadata reports are written to:

```text
.local/reports/
```

This directory is ignored by git. It is local working state, not public project data.

Important current reports for the `Continue / Yes / No` investigation:

```text
.local/reports/20260513T133625Z-SCUS_971.13-exe-ref-index.json
.local/reports/20260513T133643Z-SCUS_971.13-mips-immediate-scan.json
.local/reports/20260513T133657Z-function-ref-correlator.json
.local/reports/20260513T134921Z-SCUS_971.13-mips-call-graph.json
```

## PCSX2 Emulator Logs

PCSX2 is installed as a Flatpak on this machine.

The emulator log directory found here is:

```text
/home/peter/.var/app/net.pcsx2.PCSX2/config/PCSX2/logs/
```

The current emulator log file found here is:

```text
/home/peter/.var/app/net.pcsx2.PCSX2/config/PCSX2/logs/emulog.txt
```

After the user performs a manual PCSX2 test, inspect `emulog.txt` first for emulator/runtime messages.

## Current Manual-Test Workflow

For the current `Continue / Yes / No` investigation:

1. User runs the manual PCSX2 test.
2. AI checks:

```text
/home/peter/.var/app/net.pcsx2.PCSX2/config/PCSX2/logs/emulog.txt
.local/reports/
.local/key-concepts.md
```

3. AI records any relevant validated result in `.local/key-concepts.md`.
4. AI reports the progress percentage in chat, not in `key-concepts.md`.

## PCSX2 Breakpoint Workflow

PCSX2 version found locally:

```text
Flatpak net.pcsx2.PCSX2 v2.6.3
```

To open the debugger in PCSX2 2.x:

1. In PCSX2, enable `Tools -> Show Advanced Settings`.
2. Open `Debug -> Open Debugger`.
3. Use the `R5900` layout. This is the PS2 Emotion Engine CPU and is the target for main-game MIPS addresses.
4. Find the `Breakpoints` dock/tab.
5. Add an execute breakpoint at the address being tested.

Current addresses to test for the `Continue / Yes / No` investigation:

```text
0x0011a520
0x0011a794
0x0012d218
0x0012fd58
```

If a breakpoint triggers when the `Continue` menu appears, record:

```text
a0 a1 a2 a3 v0 v1 sp ra
```

If the debugger UI is unavailable or unclear, use the simple log-only workflow first and then document that no breakpoint evidence was collected.

Observed Portuguese UI on this machine:

1. Open `Configuracoes do PCSX2`.
2. Click `Depurar` in the left sidebar.
3. In tab `Interface do Usuario`, section `Janela do Debugger`, enable `Mostrar na Inicializacao`.
4. Close settings.
5. Restart PCSX2 or restart the emulation. The debugger window should open automatically.
6. If a top menu named `Depurar` appears after closing settings, use it to open the debugger window directly.

The screenshots in `prints/` show the settings panel, not the debugger window itself. The useful checkbox visible there is `Mostrar na Inicializacao`.

If this only opens `Janela do Registro` instead of the debugger, launch PCSX2 from a terminal with the official debugger flag:

```bash
flatpak run net.pcsx2.PCSX2 -debugger -- "/home/peter/Imagens/Ico (USA)/Ico (USA)-mod6.bin"
```

For the original BIN:

```bash
flatpak run net.pcsx2.PCSX2 -debugger -- "/home/peter/Imagens/Ico (USA)/Ico (USA).bin"
```

The `-debugger` option opens the debugger and breaks on the entry point.

## Notes For New AI Agents

Do not assume PCSX2 logs are inside the repository. They are outside the repo under the Flatpak config directory above.

Do not commit `.local/reports/`, extracted ELF files, emulator logs, modified BIN files, save states, memory cards, or proprietary outputs.
