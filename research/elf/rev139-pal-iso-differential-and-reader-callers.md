# Rev.139 — PAL ISO, differential PAL×USA, and reader caller resolution

## Title

Diferencial PAL×USA em nível de arquivo, descoberta de headers PAL 0xEC, e callers reais do reader CDVD do DFDATAS.

## Date

2026-09-06

## Objective

Fechar os itens pendentes do checklist PAL do usuário:

1. Parsear a raiz ISO9660 do ISO PAL e localizar DATA.DF (PAL: DFDATAS) + ELF SCES_507.60 + DLL headers.
2. Extrair o TOC PAL e conferir os headers dos payloads (marcador 0xEC / libarc).
3. Diferencial binário PAL×USA por arquivo (STGST00A.DF, MOTION1.DF, etc).
4. Identificar os *callers* estáticos do reader do DFDATAS (0x132630) que fornecem os handlers.

## Scope

- `.local/iso/Ico (PAL).iso` (ISO 9660 com setores de 2048 B; 907.706.368 B total).
- `.local/iso/Ico (USA).bin` (raw 2352 B/setor; DATA.DF em LBA 2898).
- `.local/extracted/pal/SCES_507.60.elf` (5.515.680 B; ELF LE, como o USA).
- `.local/extracted/SCUS_971.13.elf` (ELF USA LE).
- `tools/dfdatas-index/dfdatas_index.py` para TOC de DATA.DF.

## Sources used

- ELF USA SCUS_971.13.elf, ELF PAL SCES_507.60.elf (ambos little-endian).
- Catálogos `/tmp/dfdatas-usa.json` (169 recs) e `/tmp/pal-dfdatas-toc.json` (193 recs).
- Scans de JAL locais computados nesta revisão.

## Evidence used

- Parse manual de ISO9660 (records de diretório) no ISO PAL.
- Leitura file-level dos payloads 0..32 bytes nas duas regiões.
- Scan de words no `.text` do ELF USA por `jal 0x132630`, `jal 0x132610`, `jal 0x132930`.
- Disasm manual (little-endian) das janelas de 40 bytes anteriores a cada jal, mais os handlers.

## Findings

### 1. ISO PAL é ISO9660 padrão 2048B; DFDATAS é um arquivo, não diretório

- Raiz do ISO PAL contém: DUMMY.TXT, IOPRP224.IMG, LIBSD.IRX, MAIN.MAP, MCMAN.IRX, MCSERV.IRX, MCXMAN.IRX, MCXSERV.IRX, PADMAN.IRX, PANICSYS.IRX, SCES_507.60, SIO2MAN.IRX, SNDN2DRV.IRX, SRCFILE.TXT, TRFILE.TXT, TRTABLE.BIN, DFDATAS, SYSTEM.CNF.
- **DFDATAS no PAL é um único FRAME-232 bytes ... um arquivo contêiner de 867.184.640 B em LBA 19771.** Não é um diretório como se supunha; é o equivalente PAL do `DATA.DF` do USA.
- Base do contêiner PAL: `19771 * 2048 = 0x269D800`. Base do USA: `2898 * 2352 + 24 = 0x680178`.
- O campo peso do record ISO9660 do PAL é comprimento do nome em offset +32 e nome em +33 (brute-force nloff=32, hits=8).

### 2. PAL e USA têm payloads estruturalmente diferentes (differential file-level)

| Tipo | PAL principal | USA principal |
|------|---------------|---------------|
| `.int` | `0c02000000000000...` | `0c020000...` (4 idênticos: 02_1a.int, 02_1b.int, 55.int, 84.int) |
| `.smb` | `01002f0c` / `00002f0c` | — |
| `STG13A2.DF` | `ec 9d 7b 74 ...` | outros bytes |
| `STGST00A.DF` | `ec dd 7b 9c ...` | `cf 1b c0 35 ...` |
| `MOTION1.DF` | `ec 9d ...` | outros bytes |

Tabela de comparação por arquivo (165 compartilhados; 80 mesmo tamanho; 4 idênticos byte-a-byte: 02_1a.int, 02_1b.int, 55.int, 84.int — todos `0c02000000000000`):

| Grupo | Contagem | Significado |
|-------|----------|-------------|
| Mesmo tamanho (different bytes) | 76 | reconstruídos por região |
| Tamanhos diferentes | 85 | conteúdo regional |

### 3. PAL payloads de dados (`.DF` de estágios/motion) começam com 0xEC

- `STG13A2.DF`: `ec 9d 7b 74 ...`
- `STGST00A.DF`: `ec dd 7b 9c 14 d5 95 07 f0 9e 07 cf e1 31 c0 00`
- `MOTION1.DF`: `ec 9d ...`

Isso é consistente com libarc **mode 0xEC** (marker + parâmetros) no PAL — SÓ no PAL. No USA, os mesmos arquivos NÃO começam com 0xEC (ex.: STGST00A `cf1bc035...`). **O USA NÃO usa o preâmbulo 0xEC em payloads .DF.** Isto refina o Rev.137/138: a hipótese "0xEC nunca na posição 0" era correta apenas para o USA; o PAL confirma o marcador.

### 4. Callers do reader 0x132630 (usa `sra >>0xb` = LSN) — 7 sites estáticos

Scan de `jal 0x132630` (encoding 0x0c0498c0):

| Caller | Janela relevante | Handlers passados |
|--------|------------------|-------------------|
| 0x13857c | `sll v0,v0,11; jal` | sem handlers explícitos na janela |
| 0x138684 | `sll v0,v0,11; jal` | idem |
| 0x1406c0 | `lw a2,44(sp); lui a1,0x14; lui a3,0x14; lui t0,0x14; addu a0,k1,s4; addiu a1,a1,4168; addiu a3,a3,4392; addiu t0,t0,4400` | a1=0x141048, a3=0x141118, t0=0x141120, a2=lw(sp+44) |
| 0x140840 | `addiu a0,s6,-2968; lui a1,0x14; lui a3,0x14; addu a0,s4,a0; addiu a1,a1,4408; addiu a3,a3,4448` | a1=0x141138, a3=0x141160 |
| 0x176738 | `lui a0,0x56; addiu a0,a0,-25776; lui a1,0x17; addiu a1,a1,25592` | a0=0x559C00 (rodata/string), a1=0x1763F8 |
| 0x19fc1c | `lui a0,0x61; addiu a0,a0,10216; lui a1,0x1a; addiu a1,a1,-1800; lui a3,0x1a; addiu a3,a3,144` | a0=0x6127E8, a1=0x19F8F8, a3=0x1A0090 |
| 0x1f0f50 | `lw v0,-20956(r28); bne v0,x,0x1f0f18; addiu a1,k1,4704; addiu t2,k0,4672` | a1=k1+4704, t2=k0+4672 |

**Resolução de handler-chaining:** os endereços passados em a3/t0 (ex.: 0x141118/0x141120) são **stubs minúsculos que terminam em `jr ra`/`addiu sp,sp,64` — ou seja, finais de função de retorno imediato**. O endereço 0x141138 (usado como a1 num caller) é um **forwarder**: `addiu sp,sp,-16; addiu a2,a2,0x5c000...; jal 0x132dc0; lw a1,12(a1)`.

**0x132dc0 é a função de leitura CDVD real do ios/cdvd.c**: prólogo 0xB0, printa via string rodata 0x556ac0 (`lui a0,0x55; addiu a0,a0,27328`), usa `addiu k0,k1,2047` (alinhamento LSN), etc. O final do walkthrough já apontava `0x556ac0` como string de debug do reader.

### 5. P6 — ELF PAL verificado: mesmas strings libarc/cdvd presentes

| String | PAL VA | USA VA |
|--------|--------|--------|
| `lsn %d cnt %d size %d buf %p\n` | 0x550f60 | 0x556a98 |
| `inflate` | 0x550ff4 | 0x556980 |
| `ios/inflate.c` | 0x550ff0 | 0x556b00 |
| `ios/cdvd.c` | 0x550c58 | 0x556818 |
| `DFDATAS` | 0x550ed8 | 0x556a10 |

ELFs LE ambos. Bloco libarc USA em 0x134AD0+ (delta −0x21F0 do PAL). DCFILE absent both.

## Confirmed

- ISO PAL é ISO9660 2048B; DFDATAS é arquivo contêiner em LBA 19771, base 0x269D800; não é diretório.
- PAL payloads de dados começam com 0xEC (libarc mode marker); USA NÃO.
- Só 4 arquivos byte-idênticos entre regiões (todos `.int`, cabeça `0c020000`); resto é reconstruído por região (85 de tamanhos diferentes).
- Reader 0x132630 tem 7 callers estáticos no ELF USA; handlers são stubs/forwarders indiretos (dispatcher em 0x132630 guarda a2/a3/t0/t1, não seleciona por conteúdo).
- 0x132dc0 é a leitura CDVD real (string 0x556ac0), alcançada via forwarder 0x141138.
- P6: ELF PAL verificado, contém ios/cdvd.c, ios/inflate.c, DFDATAS, inflate. Vybe strings OK.

## Probable

- 0xEC no PAL é o marcador de "inflate ativo" (libarc); o payload PAL já é o stream comprimido diretamente no arquivo, enquanto os arquivos USA são armazenados de forma diferente (sem marcador).
- Os 4 `.int` idênticos são assets region-independent (provavelmente dados compartilhados).

## Possible

- O USA armazena payloads em algum outro codec/modo (ex. inflate com a escolha registrada em struct do TCB, não auto-detectado pelo byte 0).
- Esse é o ponto onde o runtime memdump do PCSX2 (reader CDVD pós-cópia) pode confirmar o conteúdo final do payload USA após o handler.

## Unknown

- Formato real dos payloads USA de estágio/personagem (high-entropy, ainda sem codec identificado).
- Se o PAL e o USA compartilham a MESMA transformação de payload (diferenciais regionais podem interferir).

## Discarded

- "DFDATAS no PAL é um diretório" — falso, é arquivo contêiner.
- "Payloads USA começam com 0xEC" — falso, verificado file-level.
- "0xEC nunca na posição 0" (Rev.137) — correta só para USA; PAL sim tem.

## Next minimum test

1. Repassar o disasm completo de 0x132630 determinando como o byte 0 do payload decide inflate-vs-raw, e como os 4 handlers guardados (a2/a3/t0/t1, offsets +0x104/+0x118/+0x11C/+0x120) são usados.
2. Comparar a função PAL equivalente (com ISO PAL já extraído) — delta −0x21F0 aproximado — para ver se a PAL testa 0xEC e a USA não.
3. Runtime memdump de payload USA pós-cópia no buffer (SIGUSR1) para obter o stream já decodificado.

## Conservative verdict

O item "0xEC/libarc em payloads" está **confirmado só para o PAL**; o USA usa armazenamento sem o marcador, e o codec final dos estágios USA segue aberto. A estrutura do reader (handlers indiretos) e o forwarder 0x141138→0x132dc0 estão confirmados. O caminho para P2 segue dependente de identificar a transformação USA real — estática ou via runtime memdump.
## Addendum — leitura completa do reader (mesmo dia, sessão continuação)

### As 4 handlers (a2/a3/t0/t1) são CALLBACKS, não seletores de codec

Disasm completo confirmou:

- **0x132630 (mount/attach)**: armazena as handlers no entry tabela 0x6A5C80 (stride 0x12C, slots 0..6):
  - `a2→+0x104` (`sw s5,260(s2)`)
  - `a3→+0x118` (`sw s6,280(s2)`)
  - `t0→+0x11C` (`sw s7,284(s2)`)
  - `t1→+0x120` (`sw s8,288(s2)`)
  - `t2→+0x124`, `t3→+0x128`, flags `+0x108`, LSN `+0x114`, size `+0x110`, buf `+0x10C`, path `+0x100`.
  - NÃO lê byte 0 do payload nesta função.
- **0x132930 (worker/event loop)**: lê `+0x118` (H2) e `jalr` se !=0; lê `+0x11C` (H3) e `jalr`; mantém contadores gp-0x6994/gp-0x6990; chama 0x13d3f0 (sync).
- **0x132dc0 (CDVD read, IOP buffer)**: `lsn %d cnt %d size %d iopbuf %p` @0x556ac0; worker 0x132930; helper 0x24d3e0.
- **0x132b90 (CDVD read, EE buffer)**: `lsn %d cnt %d size %d buf %p` @0x556a98.
- Stubs io-api em 0x141048 (read-segment; chama 0x132dc0+0x133500), 0x141138 (sync-read wrapper→0x132dc0), e 0x141118/0x141120/0x141128/0x141130/0x141160 = `jr ra` no-ops puros.
- **Chamador application de COMMON.DF**: função 0x19f8f8 — `a0="DFDATAS/COMMON.DF"` (0x6127e8), registra via 0x132630 com a1=0x19f8f8 (callback=próprio entry), a3=0x1a0090; depois lê via 0x132b90. Antes disso itera os 15 entries DFDATAS (loop 0x19fb90, stride 32) montando o índice.

### Conclusão — byte 0 NUNCA é checado na path USA

- Nenhuma das funções de leitura (0x132630/0x132930/0x132dc0/0x132b90) inspeciona o primeiro byte do payload.
- As 4 handlers armazenadas são callbacks de completude/evento (invocadas por `jalr` no worker), **não** um dispatcher de codec por conteúdo.
- Logo, "payload de estágio PAL começa com 0xEC" (verificado Rev.139) NÃO é auto-detectado pelo leitor USA; a decisão inflate-vs-raw (se existir) é feita NA CAMA DO SERVIDOR do arquivo (flag no TOC) ou pelo consumidor.
- O marcador 0xEC presente nos payloads PAL é propriedade do CONTEÚDO do arquivo PAL; o USA usa arquivos reconstruídos (tamanhos e cabeças diferentes), sem leitura de byte-0.

### Correção a "Possível" da Rev.139

- "inflate com a escolha registrada em struct do TCB" → corrigido: handlers são callbacks, mas o seleção não é feita por byte-0; se houver inflate, é decidido por flag/tabela ou pelo consumidor.

## Addendum 2 — MAIN.MAP do ISO PAL resolve os nomes reais do reader (iOS/CDVD)

### MAIN.MAP (LBA 400, 582.413 B) — mapa do linker com símbolos

Extraído da raiz do ISO PAL. Uso: mapear VAs PAL dos módulos reais.

**ico2000.a(cdvd.o)** — `.text @ 0x131588, size 0x21ac` (= bloco USA 0x131560–0x133330). AUTHENTIC:
`temp_loadfunc` 0x1319b0, `iosCdvdHandlerReadNoInflate` 0x131f40, `iosCdvdHandlerReadInflate` 0x1322e8, `iosCdvdHandlerRead` 0x132398, `iosCdvdManager` 0x132638, `iosCdvdDiskReady` 0x1328b0, `iosCdvdLoad` 0x1328d0, `iosCdvdBackGroundMgrAdd` 0x132928, `iosCdvdDirectStOpen` 0x132f28, `iosCdvdChgFileName` 0x1330e8, `iosCdvdGetFileLsn` 0x133188, `iosCdvdSync` 0x133268, `iosCdvdLoadPackFile` 0x133298, `iosCdvdDiskStatusGet` 0x133318, `iosCdvdBackGroundMgrDelete` 0x133320, `iosCdvdBackGroundMgrNotDiskReadyPauseSet` 0x133330, `iosCdvdBackGroundMgrDeleteRequestGet` 0x133350, `iosCdvdBackGroundMgrEntryNum` 0x133398, `iosCdvdBackGroundMgrSeek` 0x1333d0, `iosCdvdBackGroundRead` 0x1333d8, `iosCdvdBackGroundReadIOPm` 0x1334e8, `iosCdvdBackGroundMgrGetRunning` 0x1335f8, `iosCdvdDirectStClose` 0x133600, `iosCdvdDirectStRead` 0x133640, `inflate_cd_read_func` 0x1336a0.

**ico2000.a(inflate.o)** — `.text @ 0x133738, size 0x2538`: `close_inflate_handler` 0x135530, `inflate` 0x1355f8, `open_inflate_handler` 0x135ac0. `.sdata`: `inflateSec` 0x62a2b8, `ios_partition_inflate` 0x62a344. `.data @ 0x27b7a0`.

Nomes confirmados em BOTH ELFs via string: `ios/inflate.c` (PAL 0x550ff0 / USA 0x556b00), `inflate` (PAL 0x550ff4 / USA 0x556980), `ios/cdvd.c` (PAL 0x550c58 / USA 0x556818), `DFDATAS` (PAL 0x550ed8 / USA 0x556a10).

### Codec é escolhido pelo CALLER no registro (ambas as regiões)

- O mapa PAL PROVA a existência de handlers separados: **`iosCdvdHandlerReadInflate`** e **`iosCdvdHandlerReadNoInflate`**.
- Na USA, `iosCdvdManager` (=0x132630) guarda as 4 handlers a2/a3/t0/t1 em +0x104/+0x118/+0x11C/+0x120; cada caller registra o conjunto que usa.
- PAL `iosCdvdHandlerReadInflate` (0x1322e8) body real em 0x1322f0+; `open_inflate_handler` (0x135ac0) é LZ puro (sdl/sdr + jals a 0x4xxxxx).
- **Nenhum dos dois ELFs checa byte0==0xEC no reader.** A seleção inflate-vs-raw é decidida pelo caller passando handler Inflate ou NoInflate.

### Conclusão codec consolidada

- **PAL payloads** (stage/motion): 0xEC@0 → vêm de arquivo libarc-inflated **no conteúdo**, consistentes com `iosCdvdHandlerReadInflate`/`inflate` presentes.
- **USA payloads**: alta entropia, sem 0xEC, sem checagem de byte0 → arquivos **reconstruídos** (asset pipeline de build diferente), não auto-inflated no reader.
- Para extração USA: tratar como blob binário do TOC (off/size já em DATA.DF); NÃO aplicar libarc inflate.
- Para confirmação final do codec PAL: igual, extrair o blob 0xEC e validar contra `inflate` (0x1355f8 PAL) é trabalho futuro de teste.
