// Final offline exploration: FUN_00166FD0, GirlBrain, slot distribution mapping

import ghidra.app.script.GhidraScript;
import ghidra.program.model.symbol.*;
import ghidra.program.model.address.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.mem.*;
import ghidra.program.model.lang.*;
import ghidra.util.task.TaskMonitor;

import java.io.*;
import java.util.*;

public class FinalExploration extends GhidraScript {

    @Override
    protected void run() throws Exception {
        Program program = getCurrentProgram();
        SymbolTable symTable = program.getSymbolTable();
        AddressFactory addrFactory = program.getAddressFactory();
        Memory memory = program.getMemory();
        Listing listing = program.getListing();

        String outPath = "/home/peter/Documentos/repos/ico-reconstruction/research/ghidra-final-exploration-2026-05-21.md";
        PrintWriter w = new PrintWriter(new FileWriter(outPath));

        w.println("# Ghidra Final Exploration — FUN_00166FD0, GirlBrain, slot mapping");
        w.println("**Date:** 2026-05-21");
        w.println();

        // ═══════════════════════════════════════════════════════════════
        // 1. FUN_00166FD0 analysis
        // ═══════════════════════════════════════════════════════════════
        w.println("## 1. FUN_00166FD0 — halfword writer + dispatch_point host");
        w.println();

        Address fnAddr = addrFactory.getAddress("00166FD0");
        Function fn = listing.getFunctionAt(fnAddr);

        if (fn != null) {
            w.println("- Name: " + fn.getName());
            w.println("- Body: `0x%08X` - `0x%08X`".formatted(
                fn.getBody().getMinAddress().getOffset(),
                fn.getBody().getMaxAddress().getOffset()));
            w.println("- Return type: " + fn.getReturnType());
            w.println("- Parameters:");
            for (Parameter p : fn.getParameters()) {
                w.println("  - " + p.getName() + ": " + p.getDataType() + " (" + p.getComment() + ")");
            }
            w.println();

            // Disassemble key range
            w.println("### Disassembly (key region)");
            w.println("```");
            Instruction instr = listing.getInstructionAt(fn.getBody().getMinAddress());
            while (instr != null && fn.getBody().contains(instr.getAddress())) {
                long va = instr.getAddress().getOffset();
                String mnem = instr.getMnemonicString();
                String ops = instr.toString();
                if (ops.indexOf(":") > 0) ops = ops.substring(ops.indexOf(":") + 1).trim();

                // Highlight known probe points
                String marker = "";
                if (va == 0x166BB0) marker = "  <-- halfword_entry (0x166BB0)";
                if (va == 0x166DFC) marker = "  <-- halfword_fast_path (0x166DFC)";
                if (va == 0x167020) marker = "  <-- dispatch_point (0x167020)";

                w.println(String.format("  0x%08X: %-8s %-20s%s", va, mnem, ops, marker));

                Address nextAddr = instr.getAddress().add(instr.getLength());
                instr = listing.getInstructionAt(nextAddr);

                if (va >= 0x167030) break; // stop after dispatch_point area
            }
            w.println("  ...");
            w.println("```");
            w.println();

            // Callers
            w.println("### Callers of FUN_00166FD0");
            w.println();
            w.println("| Caller Address | Caller Name |");
            w.println("|----------------|-------------|");

            ReferenceIterator refs = program.getReferenceManager().getReferencesTo(fnAddr);
            if (!refs.hasNext()) w.println("(no xrefs)");
            while (refs.hasNext()) {
                Reference ref = refs.next();
                Address fromAddr = ref.getFromAddress();
                Function caller = listing.getFunctionContaining(fromAddr);
                String name = (caller != null) ? caller.getName() : "FUN_" + Long.toHexString(fromAddr.getOffset());
                w.println(String.format("| `0x%08X` | `%s` |", fromAddr.getOffset(), name));
            }
            w.println();

            // Calls made by this function
            w.println("### Calls made by FUN_00166FD0");
            w.println();
            w.println("| Call Target | Function Name |");
            w.println("|-------------|---------------|");

            Set<Long> seenTargets = new HashSet<>();
            instr = listing.getInstructionAt(fn.getBody().getMinAddress());
            while (instr != null && fn.getBody().contains(instr.getAddress())) {
                if (instr.getMnemonicString().equals("jal") || instr.getMnemonicString().equals("jalr")) {
                    Reference[] refsFrom = instr.getReferencesFrom();
                    for (Reference ref : refsFrom) {
                        Address toAddr = ref.getToAddress();
                        long toVa = toAddr.getOffset();
                        if (!seenTargets.contains(toVa)) {
                            seenTargets.add(toVa);
                            Function callee = listing.getFunctionAt(toAddr);
                            String name = (callee != null) ? callee.getName() : "not_a_function";
                            w.println(String.format("| `0x%08X` | `%s` |", toVa, name));
                        }
                    }
                }
                Address nextAddr = instr.getAddress().add(instr.getLength());
                instr = listing.getInstructionAt(nextAddr);
            }
            w.println();
        } else {
            w.println("(FUN_00166FD0 not found as a function entry)");
            w.println();
        }

        // ═══════════════════════════════════════════════════════════════
        // 2. GirlBrain state machine
        // ═══════════════════════════════════════════════════════════════
        w.println("## 2. GirlBrain state machine");
        w.println();

        w.println("### All Girl* named functions");
        w.println();
        w.println("| VA | Name |");
        w.println("|----|------|");

        List<String> girlFuncs = new ArrayList<>();
        for (Symbol s : symTable.getAllSymbols(true)) {
            String n = s.getName();
            if (n.toLowerCase().contains("girl")) {
                girlFuncs.add(String.format("| `0x%08X` | `%s` |", s.getAddress().getOffset(), n));
            }
        }
        Collections.sort(girlFuncs);
        for (String f : girlFuncs) w.println(f);
        w.println();

        // Xref chain between Girl* functions
        w.println("### GirlBrain call graph");
        w.println();
        w.println("| Caller | → | Callee |");
        w.println("|--------|---|--------|");

        String[] girlNames = {"GirlBrainClearTarget", "girlBrainMain_PositionUpdate",
            "girlBrainHide_GoalTurn", "girlBrainRunawaySearchPoint",
            "girlBrainRunawayMoveByWay", "GirlAct_BoyAndMeCollisionMail",
            "GirlGeo", "GirlForceFieldDL"};

        for (String name : girlNames) {
            Symbol s = getUniqueSymbol(symTable, name);
            if (s == null) continue;
            Function gf = listing.getFunctionAt(s.getAddress());
            if (gf == null) continue;

            Set<String> myCalls = new HashSet<>();
            Instruction gi = listing.getInstructionAt(gf.getBody().getMinAddress());
            while (gi != null && gf.getBody().contains(gi.getAddress())) {
                if (gi.getMnemonicString().equals("jal") || gi.getMnemonicString().equals("jalr")) {
                    Reference[] refsFrom = gi.getReferencesFrom();
                    for (Reference ref : refsFrom) {
                        Address toAddr = ref.getToAddress();
                        Function callee = listing.getFunctionAt(toAddr);
                        if (callee != null && callee.getName().toLowerCase().contains("girl")) {
                            myCalls.add(callee.getName());
                        }
                    }
                }
                Address nextAddr = gi.getAddress().add(gi.getLength());
                gi = listing.getInstructionAt(nextAddr);
            }
            for (String callee : myCalls) {
                w.println(String.format("| `%s` | → | `%s` |", name, callee));
            }
        }
        w.println();

        // ═══════════════════════════════════════════════════════════════
        // 3. Slot distribution mapping
        // ═══════════════════════════════════════════════════════════════
        w.println("## 3. Slot distribution — finding the right dispatch system");
        w.println();
        w.println("The runtime probe identified slot activity counts from PCSX2.");
        w.println("The _Clip dispatch table at 0x282690 uses 17 slots for clipping variants,");
        w.println("but the runtime slot distribution may belong to a different system.");
        w.println();

        // Find other dispatch-like structures
        w.println("### Functions with 'slot', 'dispatch' or 'dispatch' in name");
        w.println();
        w.println("| VA | Name |");
        w.println("|----|------|");

        for (Symbol s : symTable.getAllSymbols(true)) {
            String n = s.getName().toLowerCase();
            if (n.contains("slot") || n.contains("dispatch") || n.contains("handler")) {
                w.println(String.format("| `0x%08X` | `%s` |", s.getAddress().getOffset(), s.getName()));
            }
        }
        w.println();

        // Find descriptor table functions
        w.println("### Functions near descriptor table (0x2A31B8)");
        w.println();
        w.println("| VA | Name |");
        w.println("|----|------|");

        for (Symbol s : symTable.getAllSymbols(true)) {
            long va = s.getAddress().getOffset();
            if (va >= 0x0013E000 && va <= 0x00140000) {
                String n = s.getName();
                if (!n.startsWith("FUN_") && !n.startsWith("DAT_") && !n.startsWith("s_")) {
                    w.println(String.format("| `0x%08X` | `%s` |", va, n));
                }
            }
        }
        w.println();

        // Find callback_register-like functions
        w.println("### Functions around callback_register (0x0013f7a8)");
        w.println();
        w.println("| VA | Name |");
        w.println("|----|------|");

        for (Symbol s : symTable.getAllSymbols(true)) {
            long va = s.getAddress().getOffset();
            if (va >= 0x0013E800 && va <= 0x00140000) {
                String n = s.getName();
                w.println(String.format("| `0x%08X` | `%s` |", va, n));
            }
        }
        w.println();

        // Halfword table xrefs
        w.println("### Xrefs to halfword table 0x006AB080");
        w.println();
        w.println("| Caller | Function |");
        w.println("|--------|----------|");

        Address hwAddr = addrFactory.getAddress("006AB080");
        ReferenceIterator hwRefs = program.getReferenceManager().getReferencesTo(hwAddr);
        if (!hwRefs.hasNext()) w.println("(no direct xrefs — GP-relative)");
        while (hwRefs.hasNext()) {
            Reference ref = hwRefs.next();
            Function caller = listing.getFunctionContaining(ref.getFromAddress());
            String name = (caller != null) ? caller.getName() : "FUN_...";
            w.println(String.format("| `0x%08X` | `%s` |", ref.getFromAddress().getOffset(), name));
        }
        w.println();

        // All functions between 0x166000 and 0x16A000 (the Clip system range)
        w.println("### All named functions in _Clip system range (0x166000-0x16A000)");
        w.println();
        w.println("| VA | Name |");
        w.println("|----|------|");

        for (Symbol s : symTable.getAllSymbols(true)) {
            long va = s.getAddress().getOffset();
            if (va >= 0x166000 && va <= 0x16A000 && !s.getName().startsWith("DAT_")) {
                w.println(String.format("| `0x%08X` | `%s` |", va, s.getName()));
            }
        }
        w.println();

        w.flush();
        w.close();
        println("Final exploration written to: " + outPath);
    }

    private Symbol getUniqueSymbol(SymbolTable symTable, String name) {
        List<Symbol> syms = symTable.getSymbols(name, null);
        return (!syms.isEmpty()) ? syms.get(0) : null;
    }
}
