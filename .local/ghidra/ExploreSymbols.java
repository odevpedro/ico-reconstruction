// Explore named functions: dispatch table, cross-references, module map

import ghidra.app.script.GhidraScript;
import ghidra.program.model.symbol.*;
import ghidra.program.model.address.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.mem.*;

import java.io.*;
import java.util.*;

public class ExploreSymbols extends GhidraScript {

    @Override
    protected void run() throws Exception {
        Program program = getCurrentProgram();
        Listing listing = program.getListing();
        SymbolTable symTable = program.getSymbolTable();
        AddressFactory addrFactory = program.getAddressFactory();
        Memory memory = program.getMemory();

        String outPath = "/home/peter/Documentos/repos/ico-reconstruction/research/ghidra-exploration-2026-05-21.md";
        PrintWriter w = new PrintWriter(new FileWriter(outPath));

        w.println("# Ghidra Exploration — 2886 named symbols");
        w.println("**Date:** 2026-05-21");
        w.println();

        int totalNamed = 0;
        List<String> funcs = new ArrayList<>();
        for (Symbol s : symTable.getAllSymbols(true)) {
            totalNamed++;
            String name = s.getName();
            if (!name.startsWith("FUN_") && !name.startsWith("DAT_") && !name.startsWith("LAB_")) {
                funcs.add(String.format("| `0x%08X` | %s |", s.getAddress().getOffset(), name));
            }
        }
        Collections.sort(funcs);

        w.println("## 1. All named functions (" + totalNamed + " total symbols, " + funcs.size() + " named)");
        w.println();
        w.println("| VA | Name |");
        w.println("|----|------|");
        for (String f : funcs) {
            w.println(f);
        }
        w.println();

        // ─── 2. Dispatch table at 0x282690 (17 slots) ──────────────
        w.println("## 2. Dispatch table at 0x282690 (17 slots)");
        w.println();
        w.println("| Slot | Raw Address | Function Name |");
        w.println("|------|-------------|---------------|");

        Address tableAddr = addrFactory.getAddress("282690");
        if (memory.getBlock(tableAddr) != null) {
            MemoryBlock block = memory.getBlock(tableAddr);
            for (int i = 0; i < 17; i++) {
                Address entryAddr = tableAddr.add(i * 4);
                int raw = 0;
                try {
                    raw = memory.getInt(entryAddr);
                } catch (Exception e) {
                    raw = 0;
                }
                long targetVa = raw & 0xFFFFFFFFL;
                Address targetAddr = addrFactory.getAddress(Long.toHexString(targetVa));
                String name = "null";
                if (targetAddr != null && memory.getBlock(targetAddr) != null) {
                    Function func = listing.getFunctionContaining(targetAddr);
                    if (func != null) {
                        name = func.getName();
                    } else {
                        Symbol[] symbols = symTable.getSymbols(targetAddr);
                        if (symbols.length > 0) name = symbols[0].getName();
                        else name = "FUN_" + Long.toHexString(targetVa);
                    }
                }
                w.println(String.format("| %d | `0x%08X` | `%s` |", i, targetVa, name));
            }
        } else {
            w.println("(table address 0x282690 not in memory)");
        }
        w.println();

        // ─── 3. Cross-references to main_dispatcher 0x00166E10 ─────
        w.println("## 3. Callers of main_dispatcher (0x00166E10)");
        w.println();
        w.println("| Caller Address | Caller Name |");
        w.println("|----------------|-------------|");

        Address dispatchAddr = addrFactory.getAddress("00166E10");
        ReferenceIterator refs = program.getReferenceManager().getReferencesTo(dispatchAddr);
        if (!refs.hasNext()) w.println("(no xrefs found)");
        while (refs.hasNext()) {
            Reference ref = refs.next();
            Address fromAddr = ref.getFromAddress();
            Function func = listing.getFunctionContaining(fromAddr);
            String name = "FUN_" + Long.toHexString(fromAddr.getOffset());
            if (func != null) name = func.getName();
            w.println(String.format("| `0x%08X` | `%s` |", fromAddr.getOffset(), name));
        }
        w.println();

        // ─── 4. Functions by module prefix ─────────────────────────
        w.println("## 4. Functions by module prefix");
        w.println();
        String[][] prefixes = {
            {"sugipon_", "sugipon"}, {"omori_", "omori"}, {"fumi_", "fumi"},
            {"ito_", "ito"}, {"seki_", "seki"}, {"cloth", "cloth"},
            {"enemy", "enemy"}, {"boy_", "boy"}, {"barrel", "barrel"},
            {"rope", "rope"}, {"woodbox", "woodbox"}, {"bird", "bird"},
            {"girl", "girl"}, {"queen", "queen"}, {"attackch", "attackch"}
        };
        for (String[] p : prefixes) {
            List<String> matches = new ArrayList<>();
            for (Symbol s : symTable.getAllSymbols(true)) {
                String n = s.getName().toLowerCase();
                if (n.startsWith(p[0])) {
                    matches.add(String.format("  - `0x%08X` %s", s.getAddress().getOffset(), s.getName()));
                }
            }
            if (!matches.isEmpty()) {
                w.println("### " + p[1] + "* (" + matches.size() + ")");
                w.println();
                for (String m : matches) w.println(m);
                w.println();
            }
        }

        // ─── 5. Cloth dispatcher 0x001d37c8 callers ────────────────
        w.println("## 5. Callers of cloth_dispatcher (0x001d37c8)");
        w.println();
        w.println("| Caller Address | Caller Name |");
        w.println("|----------------|-------------|");

        Address clothAddr = addrFactory.getAddress("001d37c8");
        refs = program.getReferenceManager().getReferencesTo(clothAddr);
        if (!refs.hasNext()) w.println("(no xrefs found)");
        while (refs.hasNext()) {
            Reference ref = refs.next();
            Address fromAddr = ref.getFromAddress();
            Function func = listing.getFunctionContaining(fromAddr);
            String name = "FUN_" + Long.toHexString(fromAddr.getOffset());
            if (func != null) name = func.getName();
            w.println(String.format("| `0x%08X` | `%s` |", fromAddr.getOffset(), name));
        }
        w.println();

        // ─── 6. Halfword system named functions ────────────────────
        w.println("## 6. Halfword system — resolved names");
        w.println();
        w.println("| Address | Name |");
        w.println("|---------|------|");
        long[] addrs = {0x166BB0, 0x166DFC, 0x166E10, 0x167020, 0x167230, 0x167258,
                        0x16828C, 0x168294, 0x1AF948, 0x166600, 0x166E00, 0x1D37C8,
                        0x1D3A30, 0x13F7A8, 0x13ED40};
        for (long va : addrs) {
            Address a = addrFactory.getAddress(Long.toHexString(va));
            String name = "(not in memory)";
            if (a != null && memory.getBlock(a) != null) {
                Function func = listing.getFunctionContaining(a);
                name = (func != null) ? func.getName() : "no function entry";
            }
            w.println(String.format("| `0x%08X` | `%s` |", va, name));
        }
        w.println();

        w.flush();
        w.close();
        println("Exploration written to: " + outPath);
    }
}
