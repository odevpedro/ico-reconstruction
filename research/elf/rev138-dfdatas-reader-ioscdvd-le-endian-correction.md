# Rev.138 — DFDATAS reader localizado no USA: `ios/cdvd.c`, correção de endianness, libarc no .text

- **Date:** 2026-09-05
- **Branch:** native-port
- **Trilha:** PORT (P2 — acesso a DATA.DF) / estática
- **Objective:** localizar o código que lê `DATA.DF`/`DFDATAS` no ELF USA, validar a relevância do conteúdo do astrange, e fornecer a base estática para o P2 (extração de assets originais).

---

## Correção metodológica crítica

**O ELF USA (`SCUS_971.13.elf`) armazena instruções MIPS em little-endian** (`e_ident[EI_DATA] = 1`; verificado em bytes crus). A sessão anterior (Rev.037) disassemblava em big-endian e concluiu que "o reader de DATA.DF não está no ELF" e que "as strings do astrange nunca são referenciadas". **Ambas as conclusões estavam erradas por causa da endianness.**

- Disassembly big-endian gerava lixo tipo `scd $fp,-0x42d9($a3)`, `bbit132`, `ld.b $w0` — artefatos de decodificação errada, não de ausência de código.
- Busca de padrão `lui rt,0x0055` em big-endian: **0 ocorrências**. Em little-endian: **1558 ocorrências** no ELF inteiro.
- `tools/disasm_local_range.py` já usa `CS_MODE_LITTLE_ENDIAN` (correto); capstone com `skipdata=True` em bytes desconhecidos do R5900 entra em loop infinito — usar chunks de 16B com reset por bloco.

## Verificação da página do astrange (evidência fraca, cross-check)

| Afirmação do astrange | Verificação em bytes USA |
|---|---|
| Strings `" incomplete literal tree"` @ 0x556B10, `" incomplete distance tree"` @ 0x556B30, `"ios/inflate.c"` @ 0x556B00, `"DFDATAS/%s"` @ 0x556A10 | **CONFIRMADO** — presentes no .rodata USA nas mesmas VAs |
| Walkthrough de código `lui a0,0x55 + addiu 0x6B10 + jal 0x1A6E28` | **NÃO é do USA** — zero ocorrências desses bytes LE no ELF USA; é do PAL (release europeu, "Update!") |
| VAs das funções libarc | **Não servem para o USA** — o layout USA difere (bloco deslocado); servem como **chave de fingerprint** para localizar o bloqueio |

**Verdict:** o conteúdo do astrange é *naming hint* + *string match* (evidência fraca, índice 9-10 na hierarquia). Confirmado apenas onde bytes USA comprovam.

## libarc (inflate/mblock) localizado no .text USA

Fingerprint por prefixo (primeiros 16-108 bytes idênticos) usando os VAs TRFILE do PAL:

| Função libarc | VA PAL (astrange/TRFILE) | VA USA (delta -0x21F0) | Bytes idênticos |
|---|---|---|---|
| huft_build | 0x136CC0 | 0x134AD0 | 88 |
| inflate_codes | 0x137488 | 0x135298 | 36 |
| inflate_stored | 0x137BD0 | 0x1359E0 | 108 |
| init_mblock | 0x139690 | 0x1374A0 | 16 |
| new_mblock_node | 0x139568 | 0x137378 | prologue diverge (relink) |
| inflate_fixed | 0x137EF0 | 0x135D00 | prologue diverge |
| inflate_dynamic | 0x138150 | 0x135F60 | prologue diverge |
| inflate_start | 0x138A68 | 0x136878 | prologue diverge |
| close_inflate_handler | 0x138AB8 | 0x1368C8 | prologue diverge |
| inflate | 0x138B80 | 0x136990 | prologue diverge |
| open_inflate_handler | 0x139048 | 0x136E58 | prologue diverge |
| fill_inbuf | 0x1390D8 | 0x136EE8 | prologue diverge |
| huft_free | 0x1391B8 | 0x136FC8 | prologue diverge |
| reuse_mblock1 | 0x139668 | 0x137478 | prologue diverge |
| new_segment | 0x1396A0 | 0x1374B0 | prologue diverge |
| reuse_mblock | 0x139748 | 0x137558 | prologue diverge |
| strdup_mblock | 0x1397A0 | 0x1375B0 | prologue diverge |

Região comparada PAL 0x136CC0-0x1397A0 vs USA 0x134AD0: 11.488 bytes, 5.342 idênticos (46,5%). Divergências **todas de 1-2 bytes** (728 spans) = imediatos de endereço de saltos/strings re-linkados entre builds. **O bloco libarc está contiguamente no .text USA, deslocado por -0x21F0.**

## Reader do DFDATAS: `ios/cdvd.c` no USA

Refs à string `ios/cdvd.c` (0x556818): 8 sites, dos quais 0x1326F8/0x132874/0x133154 dentro das funções que usam `DFDATAS/%s`.

**Funções identificadas (delimitadores por prologue/epilogue jr-ra):**

| Função | Range | Padrão observado |
|---|---|---|
| leitura de bloco CDVD | 0x131560-0x131780 | assert sites 0x1314BC/0x1317A4 |
| (manager) | 0x132038-0x1320E8 | flags de manager |
| **open/cmd do DFDATAS (A)** | ~0x132384-0x132930 | `DFDATAS/%s` (0x1327A8), `\%s;1` (0x1327C0), loop TOC stride 48B, count em gp-0x699C, duas tabelas BSS base 0x281320 e 0x281328; asserts 0x1326F8/0x132874; strings `iosMcManager*` e `%s lsn:%d handler:%p` |
| **read loop (B)** | 0x132930-0x132B8C | `sra >> 0xb` (LSN/LBA), retry, contadores gp-0x6944/0x6948, `addiu a3,gp,-0x6990` (struct de leitura) |
| **open/cmd (C, 2º site)** | ~0x132FF0-0x1331D8 | `DFDATAS/%s` @0x133034, `\%s;1` @0x133058, loop TOC stride 48B |

Strings de apoio (todas presentes e referenciadas): `DFDATAS/%s`, `DATA.DF` (0x556A28), `\%s;1` (0x631F70), `ios/inflate.c`, `ios/mblock.c`, `ios/cdvd.c`, `lsn %d cnt %d size %d buf %p`, `lsn %d cnt %d size %d iopbuf %p`, `%s`, `ios/cdvd.c`.

Símbolo reconcilhado `iosCdvdHandlerReadInflate`/`NoInflate` aponta para endereços errados (0x130448/0x104478 sem callers); os VAs verdadeiros ficam na região 0x131560-0x133330.

## Correção sobre o Rev.137

O Rev.137 ("instruções do reader não estão no ELF USA"; "reader + decompressor vivem num overlay DVP") está **refutado**. Causa raiz: os scans do Rev.137 (`lui 0x55`/refs absolutos/gp-relative) foram feitos em **big-endian**, e instruções LE não casam. O reader está no main .text USA; a hipótese de overlay fica descartada.

## Validação de payload (adicionada nesta revisão)

Scan sobre os 169 arquivos do TOC USA (`/tmp/dfdatas-usa.json`, offsets byte-granulares do Rev.136):

- **Sem TIM2/PS2D/ATRS/ICNM em nenhum arquivo — nem em native, nem byte-swapped** (magics invertidas `2MIT`/`D2SP` etc.). Texturas não estão como blobs TIM2 na cabeça dos `.DF`.
- **Sem zlib-wrapped** (`78 9C/01/5E/DA`, native ou reverso) em MOTION1/boy_blk/data.jim/MOTION2.
- **Raw deflate (wbits=-15)** não decodifica desses arquivos senão em matches minúsculos acidentais (<300 B). MOTION1.DF (2,010,533 B, entropia 7.92) e boy_blk.ico (95,624 B) permanecem **não-descomprimíveis por deflate**.
- Classificação por histograma (rel-mean-dev):
  | Arquivo | valor | leitura |
  |---|---|---|
  | MOTION1.DF | 0.068 | alto-entropia, codec custom ou cifrado |
  | boy_blk.ico | 0.249 | idem |
  | data.jim | 1.031 | estruturado/plaintext |
  | MOTION2.DF | 1.748 | maioria um valor (peaks 204529/20417 = floats/estrutura) |
  | STGLOG.DF | 1.945 | plaintext com zeros |
  | COMMON.DF | 1.090 | 0xCD-fill + resíduos (≥45% de 0xCD) |
- STG13B3.DF começa com `0xFFFFFF00` (padrão de preenchimento, igual ao bloco `.int` do Rev.137). STGs em geral: alto-entropia, não-deflate.

**Leitura:** os `.DF` do DFDATAS são *formatos por-arquivo* (MOTION2=floats puros; STGLOG=índice; COMMON.DF=estruturas com fill 0xCD), não um container TM2 simples. O caminho de decodificação real (handler:+0x118 do TCB de CDVD, dispatcher indireto) será resolvido lendo a função reader completa, não por scan de magic.

## Verificação do TOC e do índice em memória (adicionada nesta revisão)

**On-disc TOC de `DATA.DF` (stride 40 B, verificado byte-exact):** `count u32` @+0 (0xAC=172 USA / 0xC1=193 PAL), depois por recordo: `name[12]` @+0 (ex.: `COMMON.DF\0`), **bloco de ponteiros constante de 20 B** @+12 (5×u32: `0x0804C438 0x08052488 0x0805279C 0x7FFFF2D8 0x7FFFF2C4` — os mesmos 5 valores do PAL, apenas relocados), `off u32` @+0x20 (relativo ao início de `DATA.DF`; rec0=0x2000 ✓), `size u32` @+0x24. Correção da suposição anterior ("name[32]" do Rev.137): o campo de nome tem só 12 B; o resto dos 20 B do recordo é ponteiro constante. O layout USA == layout PAL.

**Índice em memória (o loop do reader):** stride **48 B** (0x30), base `lui 0x28` → `0x27E520`/`0x27E528`, contador em `gp-0x699C`. O reader não usa direto os records on-disc de 40 B para busca; ele constrói um índice 48 B em RAM (name+off+size+LSN+handler provável) e faz lookup de nome via `jal 0x265024` (equivalente strncmp). Isso reconcilia os dois strides observados (40 no disco, 48 em RAM).

## Contrato de chamada do reader (adicionada nesta revisão)

Disassembly completo do prelúdio (0x132630, frame 0x2C0) em LE mostra que os ponteiros de handler **não** vêm de uma tabela interna — vêm dos **registradores de argumento do chamador**:

| arg | salvo em | re-escrito em `s` | destino no entry (stride 0x12c, base 0x6A5C80) |
|---|---|---|---|
| a2 | t9 | s5 | `+0x104` |
| a3 | k0 | s6 | `+0x118` |
| t0 | k1 | s7 | `+0x11C` |
| t1 | r30 | fp | `+0x120` |
| stk+0x204 | — | (v0→`lw 520(sp)`) | `+0x100` |
| stk+0x208 | — | — | `+0x124` |
| stk+0x20C | — | — | `+0x128` |
| (ret de jal 0x265730) | — | v0 | `+0x10C` |

Ou seja, `a2/a3/t0/t1` são **4 handlers fornecidos pelo chamador** (prováveis `read_inflate`, `read_noinflate`, `read_async`, `read_callback`). Isto confirma que a seleção inflate-vs-plaintext **não** é feita por conteúdo aqui — o chamador já decide. O entry+0x104/+0x118/+0x11C/+0x120 é exatamente o que a string `%s lsn:%d handler:%p` imprime (0x1328B4). A busca do índice 48B (0x27E528) usa `jal 0x265024` = strncmp, e o flagno +0x108 tem bit 1/4/5 (compressed?, em uso, recendo dados).

## Confirmado

- Instruções no ELF USA são **little-endian**; qualquer análise anterior em big-endian é inválida nesse registro.
- Bloco libarc (inflate/mblock do ios) está contiguamente no .text USA (base 0x134AD0, delta -0x21F0).
- **Reader do DFDATAS está no ELF USA**, não em overlay: monta `DFDATAS/<nome>;1`, itera TOC de `DATA.DF` stride 48B, com count em gp-0x699C e tabelas BSS.
- Strings do astrange `ios/inflate.c` e afins estão presentes no .rodata USA (confirmado).
- `ios/cdvd.c` é o módulo do file-system DFDATAS.
- Handlers de inflate são despachados **indiretamente** (registrados em structs de TCB/CDVD, não via JAL direto): scan de words LE no load segment por VAs do libarc retorna só **3 falsos positivos** (encodings de instruções que coincidem com endereços libarc, ex. `sll $t3,$s3,2`=0x135880 em .text) e 0 ponteiros reais de dados.
- Payload `.DF`: sem TIM2/PS2D byte-swapped; MOTION2=data.jim/STGLOG plaintext; demais alto-entropia não-deflate.

## Provável

- A tabela BSS base 0x281320 (stride 48B) saindo de `lui 0x28` é a estrutura do TOC de `DATA.DF` em memória (ingestão do índice; layout 48B ainda não resolvido — nome provavelmente herdado dos 12 B on-disc + off + size + LSN + handler + flags).
- `jal 0x264DF8` (sprintf) e `jal 0x265168` (worker/semáforo) são helpers internos de `ios/` usados pelo path-build; 164 callers de `0x264DF8` = sistema de path-building geral.
- Funções de leitura do CDVD (usa `sra >>0xb` = LSN, retry, `addiu a3,gp,-0x6990`) correspondem às camadas `iosCdvdHandlerReadInflate`/`NoInflate` reais — os VAs reconciliados (0x130448/0x104478) estão errados, as reais ficam em ~0x131560-0x133330.

## Possível

- `DATA.DF` no USA é servido por `ios/cdvd.c` com lógica de retry/LSN igual ao PAL (o walkthrough do astrange é PAL-específico e não auto-aplicável).
- Texturas TM2 reais vivem **dentro** dos arquivos de estágio/personagem em sub-posições (não na cabeça do `.DF`), ou são geradas em runtime.

## Descartado

- "Reader não está no ELF USA" (Rev.037/137) — falso, causa raiz = endianness.
- "Sem nenhuma ref às strings do astrange no USA" — falso; refs existem às VAs de .rodata USA, só não às do PAL.
- Walkthrough de instruções do astrange como descrição direta do USA.
- "Reader/decompressor em overlay DVP" (Rev.137 probable) — falso: main .text.
- "Payload de `.DF` é deflate/zlib/TIM2/PS2D" — não, em nenhuma ordem de bytes na cabeça dos arquivos.

## Próximo mínimo teste

1. Achar os **callers** do reader (scan de JAL para ~0x132630) para ver quem fornece os 4 handlers em a2/a3/t0/t1 — é aí que a seleção inflate-vs-plaintext é decidida. Os VAs reconciliados `iosCdvdHandlerReadInflate`/`NoInflate` (0x130448/0x104478) estão errados; as reais ficam na faixa ~0x131560-0x133330.
2. Identificar os tratadores reais apontados nesses callers (refs look-up de ponteiros de função nas tabelas de registro do ios).
3. Confirmar se o handler de leitura aplica libarc inflate a um `.DF` de estágio, ou se payloads alto-entropia têm codec próprio (ex.: filtro de byte-add/base-128 tipo o `compress_stream` do libarc). Esse é o passo que decide a decodificação de assets.
4. Depois de resolvida a transformação: escrever leitor semântico C `dfdatas_open(name)`/`dfdatas_read()` no `native/src/` e começar a extração real de assets de `DATA.DF` (P2).

## Verdict conservador

O reader do DFDATAS está **confirmado** no ELF USA em `ios/cdvd.c` (~0x131560-0x133330), e o Rev.137 está superado (endianness). O astrange é apenas naming hint + fingerprint-hook; a evidência direta é o byte-diff PAL→USA (11.488 B, 46,5% idênticos, span de divergência só em relinks). Payloads `.DF` **não** são TM2/deflate/zlib na cabeça dos arquivos (verificado em ambas as ordens de byte); MOTION2, data.jim e STGLOG são plaintext estruturado, enquanto estágios/personagens são alto-entropia com codec próprio ainda não identificado. Caminho para P2: dissecar o dispatcher de handler no reader, identificar o codec de payload real, então reconstruir leitor semântico para extrair assets de `DATA.DF`.