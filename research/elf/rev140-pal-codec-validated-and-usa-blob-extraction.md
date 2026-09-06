# Rev.140 — Codec PAL validado: `0xEC` = deflate puro → container DF com assets nomeados

- **Date:** 2026-09-06
- **Branch:** native-port
- **Trilha:** PORT (P2 — acesso a DATA.DF) / estática
- **Objective:** fechar a pergunta em aberto da Rev.139 ("para confirmação final do codec PAL: extrair o blob 0xEC e validar contra `inflate` é trabalho futuro de teste") executando o teste real sobre payloads PAL, e enfim extrair os blobs USA do TOC para análise.

---

## Resultado principal — codec PAL confirmado byte-exact por decodificação

**Os payloads `.DF` do PAL que começam com `0xEC` são streams deflate (RFC 1951) puros, sem wrapper zlib, decodificáveis a partir do byte 0.** O primeiro byte `0xEC` (1100 1110) é o próprio primeiro byte do stream: BFINAL=0, BTYPE=10 (`dynamic Huffman`). Não é um "marcador mágico" separado — é header deflate válido.

Evidência (blobs lidos do ISO PAL, decodificados com `zlib.decompressobj(-15)` em Python stdlib):

| Arquivo | Tamanho armazenado | Tamanho decodificado | Resultado |
|---|---|---|---|
| `STGST00A.DF` | 3.229.179 B | 7.887.012 B | deflate OK, container 449 subfiles |
| `COMMON.DF` | 2.365.389 B | 3.955.140 B | deflate OK, count 0xF9=249 |

Cross-check: `df-read2.cpp` (majo33, ferramenta de extração de ICO; `buffer[0]==0xEC → zip_inflate` com `open_inflate_handler`/`inflate_codes`) usa exatamente este algoritmo — Mark Adler deflate (libarc/zip), o mesmo `inflate.c` da libarc de `ios/inflate.c` localizada nas Rev.138-139.

## Container DF (formato revelado pelo decodificado)

O PAL `.DF` decodificado é um **container de subarquivos** com layout (caso com o `unpack_df` do df-read2):

- `file_count u32` @+0 (ex.: 0x1C1 = 449)
- Tabela de `file_count` entradas de **0x224 B** logo após o header (0x10):
  | offset | campo | exemplo (entry 0, STGST00A) |
  |---|---|---|
  | +0x00 | `unknown_value1 u32` | 10 |
  | +0x04 | `unknown_value2 u32` | 11 |
  | +0x08 | `unknown_header[4]` | `01 00 00 00` / `00 00 00 00` |
  | +0x0C | `size u32` | 210.672 |
  | +0x10 | `filename[0x200]` (path ASCII, resto 0xFF) | `sound/ICO_SE/share_amb1.bd` |
  | +0x210 | `global_id u32` | 17.395 |
  | +0x214 | `offset u32` (relativo ao início do container) | 0x3C134 |
  | +0x218 | `unknown[12]` | 0xFFFFFFFF x3 |
- Blobs dos subfiles seguem a tabela; `off` é relativo ao início do container decodificado.

Invariantes verificados em STGST00A (449 entries):
- `min(off)=0x3C134` == exatamente o fim da tabela (`0x10 + 449*0x224`);
- `max(off+size)=0x7858A4` <= tamanho decodificado (7.887.012) com folga;
- `sum(size)=7.640.944` < total decodificado (≈246 KB de folga/slack).

**Nomes reais dos assets dentro de STGST00A.DF (PAL):**

```
sound/ICO_SE/share_amb1.bd
sound/ICO_SE/share_amb1.hd
sound/ICO_SE/stage/st00a_b-g.hd
sound/ICO_SE/stage/st00a_obj.hd
object/sdf/bird/model/../texture/lb_all.tm2
object/sdf/bird/model/../texture/lb_all_l.tm2
```

Isto responde uma pergunta antiga: **texturas TM2 NÃO estão na cabeça dos `.DF`** (Rev.137/138 confirmaram que as cabeças não têm magic) — elas vivem **dentro** do container DF como subfiles nomeados com path (ex.: `.../texture/lb_all.tm2`). O container PAL (estágio) inclui também submix de som (`sound/ICO_SE/...`).

## USA continua diferente — e agora com mais evidência

Blobs USA extraídos do TOC (DATAS de `/tmp/dfdatas-usa.json`; base `2898*2352+24`), localizados diante de `Ico (USA).bin` para `/tmp/df-usa/`:

| Arquivo | tamanho | primeira fatia (head) |
|---|---|---|
| `COMMON.DF` | 2.275.935 B | `00000000 … 00000000` (zeros até ~0x390) |
| `STGST00A.DF` | 2.874.005 B | `cf1bc035809d29b779d94c1ef8a8aa94` |
| `MOTION1.DF` | 2.010.533 B | `b764c5a6…` |
| `MOTION2.DF` | 1.256.222 B | `4afc43ae…` |
| `STGLOG.DF` | 3.670.408 B | `a5bfa0a7…` |
| `boy_blk.ico` | 95.624 B | alta entropia |
| `ICON.DF` | 51.922 B | `5d2173f5…` |

Testes negativos USA (fechados nesta revisão):
- **deflate raw (−15)** em offset 0: falha (invalid block type / stored length);
- **deflate em varredura de offset 0..4096** com 4 KB de saída: melhor hit (2040, 277 B) e (1216, 4096 B) = acidentes, não stream;
- **byte-swap 32 / byte-swap 16 / reversão** + deflate: falham;
- count de container `u32 LE/BE` @0: não é plausível (0x35C01BCF) — USA não é container `.DF` no mesmo layout.

Histograma usa (topo): `0xFF`, `0x7F`, `0xFE`/`0xFD` dominam; fração de zeros ~0.6-0.7%. `0x7F/0xFF`-heavy é padrão de dados assinados/estrutura, não de deflate.

## Leitura conservadora

- **PAL**: confirmado — `.DF` de estágios/motion são `deflate → container DF (DfHeader + tabela 0x224 + blobs com paths)`. Pipeline de asset: `DATA.DF` (container raiz, TOC stride 40) → `.DF` (container PAL, deflate) → subfiles nomeados (TM2, hd/bd, etc.).
- **USA**: `.DF` de estágios **não são nem deflate nem container DF plaintext**. Alta entropia com `0xFF/0x7F`-heavy. Hhípótese em aberto: armazenamento com outra transformação (potencialmente regional), ou os payloads USA são pré-descomprimidos por pipeline de build diferente (raw no disco, consumer usa direto). Resolver exige runtime (memdump do buffer pós-handler) ou o disasm do consumidor que indexa por nome.

## Confirmado / provável / possível / desconhecido / descartado

- **Confirmado**: codec PAL = deflate puro (2 arquivos, byte-exact por decodificação); estrutura do container DF (0x224/entry) e nomes internos; TM2 dentro dos containers; USA rejeita deflate em 0/4 testes (raw, offsets, swap, rev).
- **Provável**: `buffer[0]==0xEC` no code de extração da comunidade = convenção "este arquivo está deflate"; o `iosCdvdHandlerReadInflate` (PAL) aplica o mesmo deflate no runtime.
- **Possível**: USA armazena estágios já descomprimidos mas com layout próprio (não .DF); OU usa inflate com flag de TOC/registro (não byte0).
- **Unknown**: transformação USA; VA do consumidor que lê os paths nomeados.
- **Descartado**: "0xEC é um marcador mágico extra" (é header deflate válido); "payload USA = .DF container" (count impossível).

## Próximo mínimo teste

1. Decodificar TODOS os `.DF` PAL do TOC (193 recs) e catalogar subfiles por extensão (`.tm2`, `.bd`, `.hd`, `.isa`…) para inventariar o asset pipeline PAL completo.
2. Para o USA: procurar no ELF USA o consumidor que faz lookup de path (strncmp `jal 0x265024`) para ver se o payload USA já é consumido direto ou passa por transformação.
3. Runtime: memdump do buffer de dados USA pós-handler de leitura, comparar com blob on-disc para medir se o reader transforma ou serve cru.

## Verdict conservador

O codec PAL está **fechado por teste de decodificação real** (3.229.179 → 7.887.012 B, sem erro, container válido com nomes 100% legíveis). O USA permanece em aberto com transformação desconhecida; o teste mínimo agendado são os passos 1-3 acima. Evidência completa: blobs em `/tmp/pal-stgst00a.raw`, `/tmp/pal-share_amb1.bd`, `/tmp/df-usa/`.