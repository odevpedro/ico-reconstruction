// Apply 2886 verified symbols from symbol reconciliation pipeline

import ghidra.app.script.GhidraScript;
import ghidra.program.model.symbol.SourceType;
import ghidra.program.model.symbol.SymbolTable;
import ghidra.program.model.address.Address;
import ghidra.program.model.address.AddressFactory;
import ghidra.program.model.listing.Program;
import ghidra.program.model.mem.Memory;

import java.io.*;
import java.nio.file.*;

public class ApplySymbols extends GhidraScript {

    @Override
    protected void run() throws Exception {
        Program program = getCurrentProgram();
        SymbolTable symTable = program.getSymbolTable();
        AddressFactory addrFactory = program.getAddressFactory();
        Memory memory = program.getMemory();

        Path labelsPath = Paths.get(
            "/home/peter/Documentos/repos/ico-reconstruction",
            "docs", "symbols", "ghidra_labels.txt"
        );

        if (!Files.exists(labelsPath)) {
            println("ERROR: labels file not found: " + labelsPath);
            return;
        }

        int count = 0;
        int errors = 0;

        try (BufferedReader reader = Files.newBufferedReader(labelsPath)) {
            String line;
            while ((line = reader.readLine()) != null) {
                line = line.trim();
                if (line.isEmpty() || line.startsWith("#")) {
                    continue;
                }
                String[] parts = line.split("\\s+", 2);
                if (parts.length < 2) {
                    continue;
                }
                String addrStr = parts[0].replace("0x", "");
                String name = parts[1];

                Address addr = addrFactory.getAddress(addrStr);
                if (addr != null && memory.getBlock(addr) != null) {
                    symTable.createLabel(addr, name, SourceType.IMPORTED);
                    count++;
                } else {
                    errors++;
                }
            }
        }

        println("Applied " + count + " symbols (" + errors + " errors/skipped)");
    }
}
