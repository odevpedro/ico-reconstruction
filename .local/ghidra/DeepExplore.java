// Deep exploration: dispatch table usage, _Clip xrefs, named system map

import ghidra.app.script.GhidraScript;
import ghidra.program.model.symbol.*;
import ghidra.program.model.address.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.mem.*;
import ghidra.program.model.data.*;
import ghidra.program.model.util.*;
import ghidra.util.task.TaskMonitor;
import ghidra.program.model.lang.OperandType;

import java.io.*;
import java.util.*;

public class DeepExplore extends GhidraScript {

    @Override
    protected void run() throws Exception {
        Program program = getCurrentProgram();
        Listing listing = program.getListing();
        SymbolTable symTable = program.getSymbolTable();
        AddressFactory addrFactory = program.getAddressFactory();
        Memory memory = program.getMemory();

        String outPath = "/home/peter/Documentos/repos/ico-reconstruction/research/ghidra-deep-exploration-2026-05-21.md";
        PrintWriter w = new PrintWriter(new FileWriter(outPath));

        w.println("# Ghidra Deep Exploration — dispatch table, _Clip xrefs, system map");
        w.println("**Date:** 2026-05-21");
        w.println();

        // ─── 1. DISPATCH TABLE FIELD ANALYSIS ──────────────────────
        w.println("## 1. Dispatch table 0x282690 — field decoding");
        w.println();

        Address tableAddr = addrFactory.getAddress("282690");
        int numEntries = 17;
        int entrySize = 16;

        w.println("### Raw decoded entries");
        w.println();
        w.println("| N | +0x00 flag | +0x04 mode | +0x08 tier | +0x0C callback | Name |");
        w.println("|---|-----------|------------|------------|----------------|------|");

        for (int i = 0; i < numEntries; i++) {
            Address ea = tableAddr.add(i * entrySize);
            int f0 = memory.getInt(ea);
            int f4 = memory.getInt(ea.add(4));
            int f8 = memory.getInt(ea.add(8));
            int cb = memory.getInt(ea.add(12));
            long cbVa = cb & 0xFFFFFFFFL;

            String name = "-";
            if (cbVa > 0x00100000 && cbVa < 0x01700000) {
                Address a = addrFactory.getAddress(Long.toHexString(cbVa));
                if (a != null && memory.getBlock(a) != null) {
                    Function func = listing.getFunctionContaining(a);
                    if (func != null) name = func.getName();
                }
            }

            w.println(String.format("| %d | 0x%08X | 0x%08X | 0x%08X | 0x%08X | `%s` |",
                i, f0 & 0xFFFFFFFFL, f4 & 0xFFFFFFFFL, f8 & 0xFFFFFFFFL, cbVa, name));
        }
        w.println();

        // Group callbacks by type
        w.println("### Callback group summary");
        w.println();
        w.println("**Wall clipping (`_clipW*`):**");
        w.println("- `_clipWDebug` (entry 0) — debug variant");
        w.println("- `_clipW` (entries 1, 8) — base wall clip");
        w.println("- `_clipWR` (entry 2) — wall-ref");
        w.println("- `_clipWField` (entries 3, 9) — wall field");
        w.println("- `_clipWE` (entry 4) — wall edge");
        w.println("- `_clipWEField` (entry 5) — wall edge field");
        w.println("- `_clipWWaveForce` (entry 6) — wave force");
        w.println("- `_clipWDitchHangWalkStop` (entry 7) — ditch/hang/walk stop");
        w.println("- `_clipWBoxStop` (entry 10) — box stop");
        w.println("- FUN_00169968 (entry 11) — unknown wall variant");
        w.println();

        w.println("**Floor clipping (`_clipF*`):**");
        w.println("- `_clipF` (entries 12, 16) — base floor clip");
        w.println("- `_clipFE` (entry 13) — floor edge");
        w.println("- `_clipFR` (entry 14) — floor ref");
        w.println("- `_clipFIH` (entry 15) — floor ???");
        w.println();

        w.println("**Field analysis:**");
        w.println("- `+0x04` = 1 on entries 4-5: `_clipWE`, `_clipWEField` — edge-related variants");
        w.println("- `+0x08` = 1 on entries 8-9: `_clipW`, `_clipWField` — possibly a second layer/ref pass");
        w.println("- `+0x08` = 1 on entry 16: `_clipF` — duplicata de ref");
        w.println("- When flag=0 (entries 12-16): floor clipping, possibly inactive by default");
        w.println();

        // ─── 2. DATA XREFS TO DISPATCH TABLE ──────────────────────
        w.println("## 2. Code references to dispatch table 0x282690");
        w.println();
        w.println("| Address | Context | Function |");
        w.println("|---------|---------|----------|");

        ReferenceIterator refs = program.getReferenceManager().getReferencesTo(tableAddr);
        if (!refs.hasNext()) {
            w.println("(no direct xrefs found — may be accessed via GP-relative addressing)");
        }
        while (refs.hasNext()) {
            Reference ref = refs.next();
            Address fromAddr = ref.getFromAddress();
            Function func = listing.getFunctionContaining(fromAddr);
            String name = (func != null) ? func.getName() : "FUN_" + Long.toHexString(fromAddr.getOffset());
            w.println(String.format("| `0x%08X` | %s | `%s` |", fromAddr.getOffset(), ref.getReferenceType().getName(), name));
        }
        w.println();

        // ─── 3. _Clip XREFS ────────────────────────────────────────
        w.println("## 3. Callers of `_Clip` (0x00166E10)");
        w.println();
        w.println("| Caller Address | Caller Name |");
        w.println("|----------------|-------------|");

        Address clipAddr = addrFactory.getAddress("00166E10");
        refs = program.getReferenceManager().getReferencesTo(clipAddr);
        if (!refs.hasNext()) w.println("(no xrefs found)");
        while (refs.hasNext()) {
            Reference ref = refs.next();
            Address fromAddr = ref.getFromAddress();
            Function func = listing.getFunctionContaining(fromAddr);
            String name = (func != null) ? func.getName() : "FUN_" + Long.toHexString(fromAddr.getOffset());
            w.println(String.format("| `0x%08X` | `%s` |", fromAddr.getOffset(), name));
        }
        w.println();

        // ─── 4. ALL _clip* FUNCTIONS ───────────────────────────────
        w.println("## 4. All `_clip*` functions");
        w.println();
        w.println("| VA | Name |");
        w.println("|----|------|");

        List<String> clipFuncs = new ArrayList<>();
        for (Symbol s : symTable.getAllSymbols(true)) {
            String n = s.getName();
            if (n.startsWith("_clip") || n.startsWith("__Clip")) {
                clipFuncs.add(String.format("| `0x%08X` | `%s` |", s.getAddress().getOffset(), n));
            }
        }
        Collections.sort(clipFuncs);
        for (String f : clipFuncs) w.println(f);
        w.println();

        // ─── 5. CLOTH/PHYSICS FUNCTIONS ──────────────────────────
        w.println("## 5. Cloth/physics functions (getCloth*, cloth*, InitCloth*)");
        w.println();
        w.println("| VA | Name |");
        w.println("|----|------|");

        List<String> clothFuncs = new ArrayList<>();
        for (Symbol s : symTable.getAllSymbols(true)) {
            String n = s.getName();
            if (n.toLowerCase().contains("cloth")) {
                clothFuncs.add(String.format("| `0x%08X` | `%s` |", s.getAddress().getOffset(), n));
            }
        }
        Collections.sort(clothFuncs);
        for (String f : clothFuncs) w.println(f);
        w.println();

        // ─── 6. ENTITY GEOMETRY FUNCTIONS ──────────────────────────
        w.println("## 6. Entity geometry functions (*Geo, *DL)");
        w.println();
        w.println("| VA | Name |");
        w.println("|----|------|");

        List<String> geoFuncs = new ArrayList<>();
        for (Symbol s : symTable.getAllSymbols(true)) {
            String n = s.getName();
            if (n.endsWith("Geo") || n.endsWith("DL")) {
                geoFuncs.add(String.format("| `0x%08X` | `%s` |", s.getAddress().getOffset(), n));
            }
        }
        Collections.sort(geoFuncs);
        for (String f : geoFuncs) w.println(f);
        w.println();

        // ─── 7. ALL FUNCTIONS IN clothAnimation.c RANGE ──────────
        w.println("## 7. Functions in clothAnimation.c range (0x001D27A8 - 0x001D3B28)");
        w.println();
        w.println("| VA | Name |");
        w.println("|----|------|");

        for (Symbol s : symTable.getAllSymbols(true)) {
            long va = s.getAddress().getOffset();
            if (va >= 0x001D27A8 && va <= 0x001D3B28 && s.getSymbolType() == SymbolType.FUNCTION) {
                w.println(String.format("| `0x%08X` | `%s` |", va, s.getName()));
            }
        }
        w.println();

        // ─── 8. BOY/GIRL/ENEMY FUNCTIONS ──────────────────────────
        w.println("## 8. Boy/Girl/Enemy named functions");
        w.println();
        w.println("| VA | Name |");
        w.println("|----|------|");

        for (Symbol s : symTable.getAllSymbols(true)) {
            String n = s.getName();
            if (n.toLowerCase().startsWith("boy") || n.toLowerCase().startsWith("girl")
                || n.toLowerCase().startsWith("enemy") || n.toLowerCase().startsWith("queen")
                || n.toLowerCase().startsWith("bird")) {
                w.println(String.format("| `0x%08X` | `%s` |", s.getAddress().getOffset(), n));
            }
        }
        w.println();

        // ─── 9. BARREL/ROPE/WOODBOX FUNCTIONS ──────────────────────
        w.println("## 9. Barrel/Rope/Woodbox named functions");
        w.println();
        w.println("| VA | Name |");
        w.println("|----|------|");

        for (Symbol s : symTable.getAllSymbols(true)) {
            String n = s.getName();
            if (n.toLowerCase().startsWith("barrel") || n.toLowerCase().startsWith("rope")
                || n.toLowerCase().startsWith("woodbox") || n.toLowerCase().startsWith("attackch")) {
                w.println(String.format("| `0x%08X` | `%s` |", s.getAddress().getOffset(), n));
            }
        }
        w.println();

        // ─── 10. FUNCTIONS CALLING BOTH _Clip AND execBombGeo ────
        w.println("## 10. Functions that reference both _Clip and execBombGeo");
        w.println();

        Set<String> clipCallers = new HashSet<>();
        refs = program.getReferenceManager().getReferencesTo(clipAddr);
        while (refs.hasNext()) {
            Reference ref = refs.next();
            Function func = listing.getFunctionContaining(ref.getFromAddress());
            if (func != null) clipCallers.add(func.getName());
        }

        Address bombAddr = addrFactory.getAddress("001d37c8");
        Set<String> bombCallers = new HashSet<>();
        refs = program.getReferenceManager().getReferencesTo(bombAddr);
        while (refs.hasNext()) {
            Reference ref = refs.next();
            Function func = listing.getFunctionContaining(ref.getFromAddress());
            if (func != null) bombCallers.add(func.getName());
        }

        Set<String> common = new HashSet<>(clipCallers);
        common.retainAll(bombCallers);
        if (common.isEmpty()) {
            w.println("(no function calls both _Clip and execBombGeo)");
        } else {
            for (String name : common) w.println("- `" + name + "`");
        }
        w.println();

        // ─── 11. _Clip internal structure (inline blocks) ──────────
        w.println("## 11. `_Clip` function internals");
        w.println();

        Function clipFunc = listing.getFunctionAt(clipAddr);
        if (clipFunc != null) {
            w.println(String.format("- Body range: `0x%08X` - `0x%08X`",
                clipFunc.getBody().getMinAddress().getOffset(),
                clipFunc.getBody().getMaxAddress().getOffset()));
            w.println("- Number of instructions: " + countInstructions(listing, clipFunc));
            w.println();

            // List named labels/entry points inside _Clip
            w.println("### Internal labels / sub-blocks in _Clip range");
            w.println();
            w.println("| VA | Label |");
            w.println("|----|-------|");

            long clipStart = clipFunc.getBody().getMinAddress().getOffset();
            long clipEnd = clipFunc.getBody().getMaxAddress().getOffset();

            for (Symbol s : symTable.getAllSymbols(true)) {
                long va = s.getAddress().getOffset();
                if (va > clipStart && va < clipEnd) {
                    w.println(String.format("| `0x%08X` | `%s` |", va, s.getName()));
                }
            }
            w.println();
        } else {
            w.println("(function _Clip not found)");
            w.println();
        }

        w.flush();
        w.close();
        println("Deep exploration written to: " + outPath);
    }

    private int countInstructions(Listing listing, Function func) {
        int count = 0;
        Instruction instr = listing.getInstructionAt(func.getBody().getMinAddress());
        while (instr != null && func.getBody().contains(instr.getAddress())) {
            count++;
            instr = instr.getNext();
        }
        return count;
    }
}
