# Rev.162 — PAL→USA reconciliation: byte-fingerprint validation (conclusive)

## Title
Rev.162 — PAL→USA: pipe de fingerprint validado contra ELF PAL; mapa MAIN.MAP já continha 100% dos matches recuperáveis

## Date
2026-09-10

## Objective
Fechar a pendência histórica "Matching PAL→USA byte-a-byto aguarda o ELF PAL"
(AGENTS.md) e determinar se o pipeline `tools/symbol_reconcile/reconcile.py`
consegue recuperar **novos** símbolos além do mapa já derivado de MAIN.MAP.

## Scope
- Fonte PAL disponível localmente: `.local/extracted/pal/SCES_507.60.elf` (5.515.680 B) + `.local/extracted/pal/MAIN.MAP` + `SRCFILE.TXT`.
- Pipeline fingerprint: `tools/symbol_reconcile/reconcile.py --step candidates`.
- Artefatos de comparação:
  - Mapa de referência (pré-existente): `docs/symbols/pal_usa_symbol_map.csv` (4536 linhas).
  - Crawl PAL symbols: `docs/symbols/raw_symbols_pal.csv` (4536 linhas).
  - Fingerprints USA: `docs/symbols/usa_fingerprints.json`.
  - Fingerprints PAL (14782 funções): `docs/symbols/pal_fingerprints.json`.

## Evidence / method
1. Gerado `/tmp/pal-symbols-symbol_addrs.txt` no formato splat
   (`sym = 0xVA; // type:func`) a partir de `raw_symbols_pal.csv`.
2. Executado com `PAL_ELF=.../.local/extracted/pal/SCES_507.60.elf`:
   ```
   [INFO] PAL fingerprints loaded: 14782 functions
   [MATCH] exact=224 opseq=138 norm=0 sameva=5 fuzzy=37 unmatched=4132 total=4536
   ```
   → 404 matches (367 verified + 37 candidate), 4132 unmatched.
3. Comparação símbolo-a-símbolo com o mapa MAIN.MAP pré-existente.

## Results (byte-level / instruction-level findings)

| Métrica | Valor |
|---------|-------|
| PAL symbols no pipeline | 4536 |
| PAL fingerprints (do ELF PAL) | 14782 |
| Matches do pipeline fingerprint | 404 (exact=224, opseq=138, sameva=5, fuzzy=37) |
| Já presentes no mapa antigo | **404 / 404** (367 verified + 37 candidate) |
| Novos símbolos além do mapa antigo | **0** |
| Old-map "unmatched" que fingerprint recuperaria | 0 / 1332 |
| Símbolos fingerprint-verified com usa_va no mapa antigo | 367 |
| Discordância de USA VA (fingerprint vs mapa antigo) | **0** |

Conclusão de cobertura:
- **Tudo que é recuperável por fingerprint já está no mapa MAIN.MAP.** O pipeline
  de fingerprint não adiciona cobertura nova.
- **Corroboração forte:** 367 símbolos verificados por fingerprint (raw_sha1 /
  op_seq_hash byte-exact) têm o MESMO USA VA do mapa antigo em 100% dos casos
  (0 conflitos). O mapa antigo é byte-correcto no conjunto verificado.
- Os 1332 "unmatched" do mapa antigo não têm correspondência binária no ELF USA:
  são símbolos só-PAL (dados, símbolos de debug/Main, features PAL-only) ou que
  o scanner de prologue não segmenta. Não há evidência adicional local para
  recuperá-los sem o diferencial completo PAL ISO.

## What is confirmed
- O ELF PAL está disponível em `.local/extracted/pal/` desde 2026-09-05 e os
  fingerprints PAL reais existem (14782 funções).
- Com o bytes do ELF PAL, o pipeline `reconcile.py` confirma **367 símbolos
  byte-exact** (224 raw_sha1 + 138 op_seq_hash + 5 same_va) todos coincidindo
  com o mapa MAIN.MAP pré-existente.
- Nenhum símbolo novo é recuperável via fingerprint além do que a reconciliação
  MAIN.MAP já produziu.

## What is probable
- O conjunto "unmatched" (1332) contém majoritariamente símbolos de dados ou
  entradas de symbol table PAL sem contraparte binária USA; diferenciá-los
  exigiria diff PAL ISO vs USA ISO (complexo, fora do escopo atual).

## What is possible
- Rodar o mesmo pipeline contra o mapa candidates antigo (2575 candidatos fuzzy)
  para tentar "promover" candidatos por fingerprint — plausível, mas zero ganhos
  esperados dado o resultado de 404/404 já presentes.

## What is unknown
- Quantos dos 1332 unmatched são funções genuínas PAL-only (dados/strings)
  vs funções USA correspondentes que o scanner de prologue não identificou.

## What is discarded
- Regerar `pal_usa_symbol_map.csv` a partir de `reconcile.py` sozinho: DIMINUI
  a cobertura (404 vs 3191 com usa_va). O mapa antigo permanece o de referência.

## Next minimum test
- Nenhum obrigatório para este item. Se houver retrabalho futuro deste pipeline,
  primeiro confirmar que os novos matches não contradizem o mapa (0 conflitos
  atuais) e que cobertura ≥ 3191 com usa_va.

## Conservative verdict
Item "PAL→USA byte-a-byto" **fechado**: o método de fingerprint, agora com o ELF
PAL real, valida byte-exact 367 símbolos já mapeados por MAIN.MAP (0 conflitos) e
não recupera símbolo novo algum. O mapa de referência permanece
`docs/symbols/pal_usa_symbol_map.csv` (4536 linhas; 616 verified, 2575 candidate,
1332 unmatched, 13 rejected). Backups: `/tmp/pal_usa_symbol_map.backup.csv` e
`/tmp/pal_usa_symbol_map.reconcile-candidates.csv`.