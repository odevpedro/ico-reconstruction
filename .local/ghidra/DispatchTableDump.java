// Dump dispatch table at 0x282690 in raw hex, decode full structure

import ghidra.app.script.GhidraScript;
import ghidra.program.model.symbol.*;
import ghidra.program.model.address.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.mem.*;

import java.io.*;

public class DispatchTableDump extends GhidraScript {

    @Override
    protected void run() throws Exception {
        Program program = getCurrentProgram();
        SymbolTable symTable = program.getSymbolTable();
        AddressFactory addrFactory = program.getAddressFactory();
        Memory memory = program.getMemory();
        Listing listing = program.getListing();

        String outPath = "/home/peter/Documentos/repos/ico-reconstruction/research/ghidra-dispatch-table-2026-05-21.md";
        PrintWriter w = new PrintWriter(new FileWriter(outPath));

        w.println("# Dispatch table analysis — 0x282690");
        w.println("**Date:** 2026-05-21");
        w.println();

        // ─── Dump 256 bytes from 0x282690 ──────────────────────────
        w.println("## Raw bytes (first 256 bytes from 0x282690)");
        w.println();
        w.println("```");
        Address tableAddr = addrFactory.getAddress("282690");
        for (int row = 0; row < 16; row++) {
            Address rowAddr = tableAddr.add(row * 16);
            String hex = "";
            for (int col = 0; col < 16; col++) {
                int b = memory.getByte(rowAddr.add(col)) & 0xFF;
                hex += String.format("%02X ", b);
            }
            w.println(String.format("0x%08X  %s", rowAddr.getOffset(), hex));
        }
        w.println("```");
        w.println();

        // ─── Decode as repeating 16-byte entries ───────────────────
        w.println("## Decoded 16-byte entries");
        w.println();
        w.println("16-byte repeating structure. Offset +0x00: flag, +0x04..+0x0B: padding/unknown, +0x0C: callback");
        w.println();
        w.println("| Entry | +0x00 (flag) | +0x04 | +0x08 | +0x0C (callback) | Function name |");
        w.println("|-------|-------------|-------|-------|------------------|---------------|");

        for (int i = 0; i < 24; i++) {  // dump 24 entries to be safe
            Address entryAddr = tableAddr.add(i * 16);
            int flag = memory.getInt(entryAddr);
            int u1 = memory.getInt(entryAddr.add(4));
            int u2 = memory.getInt(entryAddr.add(8));
            int cb = memory.getInt(entryAddr.add(12));

            long flagVal = flag & 0xFFFFFFFFL;
            long cbVa = cb & 0xFFFFFFFFL;

            String cbName = "-";
            if (cbVa > 0x00100000 && cbVa < 0x01700000) {
                Address cbAddr = addrFactory.getAddress(Long.toHexString(cbVa));
                if (cbAddr != null && memory.getBlock(cbAddr) != null) {
                    Function func = listing.getFunctionContaining(cbAddr);
                    if (func != null) {
                        cbName = func.getName();
                    } else {
                        Symbol[] syms = symTable.getSymbols(cbAddr);
                        if (syms.length > 0) cbName = syms[0].getName();
                        else cbName = "FUN_" + Long.toHexString(cbVa);
                    }
                }
            }

            if (flagVal == 1 || cbVa > 0x00100000) {
                w.println(String.format("| %d | `0x%08X` | `0x%08X` | `0x%08X` | `0x%08X` | `%s` |",
                    i, flagVal, u1 & 0xFFFFFFFFL, u2 & 0xFFFFFFFFL, cbVa, cbName));
            }
        }
        w.println();
        w.println("(Entries with flag=0 and cb=0x00000000 omitted for brevity)");
        w.println();

        // ─── Also dump surrounding data to check bounds ────────────
        w.println("## Boundary check — 32 bytes before and after");
        w.println();
        w.println("```");
        long[] bounds = {0x282670, 0x282690, 0x282800};
        for (long va : bounds) {
            Address a = addrFactory.getAddress(Long.toHexString(va));
            w.println(String.format("--- 0x%08X ---", va));
            for (int row = 0; row < 4; row++) {
                Address rowAddr = a.add(row * 16);
                String hex = "";
                for (int col = 0; col < 16; col++) {
                    int b = memory.getByte(rowAddr.add(col)) & 0xFF;
                    hex += String.format("%02X ", b);
                }
                w.println(String.format("0x%08X  %s", rowAddr.getOffset(), hex));
            }
        }
        w.println("```");
        w.println();

        // ─── Search for other similar tables near this address ──────
        w.println("## Nearby data that may be related");
        w.println();
        w.println("Scanning 0x282000-0x283000 for patterns matching 16-byte structure...");
        w.println();

        w.flush();
        w.close();
        println("Table dump written to: " + outPath);
    }
}
