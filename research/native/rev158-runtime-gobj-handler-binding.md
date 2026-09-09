# Rev.158 — Runtime GObj↔processCallback binding por sala (PCSX2, gaiola da Yorda)

- **Data:** 2026-09-09
- **Trilha:** [TRILHA: PORT] — Passo 2 do vínculo GObj↔modelo (ver Rev.154/155)
- **Objetivo:** substituir a heurística host round-robin `m_sceneGObjs[i % size]`
  (Rev.155) por um vínculo GObj↔init_fn/processCallback verificado em runtime
  (PCSX2 instrumentado), usando `isys_gobj_proc_add` (0x13F3F0).
- **Escopo:** análise de log runtime apenas; nenhuma mudança de código host.

## Fonte

- Captura do dia: sessão "gaiola da Yorda" (New Game → perseguição/gaiola).
  Extraída do arquivo antigo `ico-runtime-20260825-152452.jsonl` (a gameplay foi
  gravada lá por corrida de timing no atalho; nada foi perdido).
- Fatia: último `elf_entry_sentinel` (linha 3657809) → fim = boot contínuo único.
- Sessão validada: `session-extracted-20260909-gaiola.jsonl`, `status=valid`.
- Fluxo validado por `verify_runtime_probe_log.py`.

## Volume

| Evento | Contagem |
|--------|---------|
| Total | 94.266 |
| `world_state_load` | 13 |
| `world_state_room_init_fn` | 5 |
| `init_scene_gobj` | 574 |
| `isys_gobj_add` | 2.018 |
| `isys_gobj_proc_add` | 14.207 |
| `ios_om_main` | 49.668 |
| `ios_om_create_dl` | 3.164 |
| `halfword_fast_path` | 23.243 |
| `elf_entry_sentinel` | 1 |

## Semântica do probe `isys_gobj_proc_add` (0x13F3F0) — observada

Regs capturados (a1=a1hi||a1lo, etc., montados no fork):

| Reg | Significado observado |
|-----|----------------------|
| `a1` | endereço do GObj (range 0x00830230..0x008471D4) |
| `a2` | endereço do processCallback registrado; `0` = não-callback (slots auxiliares) |
| `a3` | `0x00000001` = handler primário (`*_hB`/`*_hD`); `0x00000000` = sub-callback/layer (`*_init`, `subGirlBrain_*`, callbacks de estado) |
| `v0` | `0x00000013` nos registros com sub-callback; pointer (ex. 0x00712CC0) ou 0 nos demais — não interpretado |

**Correção de suposição anterior:** `a3` NÃO é add/remove. `a3=1` marca o handler
principal; `a3=0` marca callbacks de camada/sub-estado (init e estados). Isso
explica por que cada GObj do BOY re-registra `boy_init` (a3=0) e `boy_hB` (a3=1)
em toda sala, e por que `subGirlBrain_*` aparecem como layers.

Distribuição a3 (14.207 eventos):
- a3=1 com nome: 277 eventos (type60_hB 70, flag_hB 35, torch_hB 33, type6_hB 19,
  boy_hB 13, type36_hB 9, seffect_hB 7, girl_hB 5, enemy1_hD 5, ...)
- a3=0 com nome: boy_init 13, subGirlBrain_Idle 8, subGirlBrain_Hesitate 7, ...
- Total callbacks únicos `a2!=0`: 205 endereços; 29 resolvidos contra `TARGET_FUNCTIONS`.

## Callbacks nomeados resolvidos (via `TARGET_FUNCTIONS`, 612 funções)

| Callback | Endereço | Função observada |
|----------|----------|------------------|
| `boy_hB` | 0x001C1DD8 | handler principal do BOY |
| `boy_init` | 0x00153478 | init/sub-callback do BOY (layer) |
| `girl_hB` | 0x001D17F8 | handler principal GIRL |
| `girl_hD` | 0x001D1AD0 | handler secundário GIRL |
| `enemy1_hB` | 0x001CE3C0 | handler ENEMY1 |
| `enemy1_hD` | 0x001CE760 | handler secundário ENEMY1 |
| `enemy1_init` | 0x00164440 | init/enemy1 (layer) |
| `generator_hB` | 0x001930B0 | gerador (shadows) |
| `bird_hB` | 0x00197078 | pássaro |
| `torch_hB` | 0x001F1CF0 | tocha |
| `flag_hB` | 0x001D00F8 | bandeira |
| `chain_hB` | 0x0018ECC8 | corrente |
| `seffect_hB` | 0x001EF980 | efeito especial |
| `ap1_hB` | 0x001BA330 | "ap1" (asset/portal?) |
| `woodbox0_hB` | 0x001C0538 | caixa de madeira |
| `ItemGeo` | 0x001D3A30 | geometria de item |
| `type6_hB` | 0x001CE6F0 | tipo 6 |
| `type15_hB` | 0x001EA5E8 | tipo 15 |
| `type22_hB` | 0x001BC1A8 | tipo 22 |
| `type24_hB` | 0x001BC530 | tipo 24 |
| `type36_hB` | 0x001F44C8 | tipo 36 |
| `type39_hB` | 0x0010D070 | tipo 39 |
| `type55_hB` | 0x001D1CF8 | tipo 55 |
| `type60_hB` | 0x0023D518 | tipo 60 |
| `subGirlBrain_Idle` | 0x00175CB0 | estado Idle GirlBrain (layer) |
| `subGirlBrain_Hesitate` | 0x00175CE0 | estado Hesitate (layer) |
| `subGirlBrain_Busy` | 0x00175DC0 | estado Busy (layer) |
| `subGirlBrain_PulledUp` | 0x0016CED0 | pulled-up (layer) |

## Ligação GObj↔handler por sala (núcleo deste revisão)

Cada `world_state_load` inicia um segmento. A tabela abaixo mostra, por segmento,
os GObjs com handler nomeado (`a3=1`) e os sub-callbacks (`a3=0`) anexados. A
mesma casa de GObj (`0x00830xxx`) é reutilizada entre salas: piscina estável do
sistema `isysGObj`, conteúdo re-ligado por sala.

| Seg | GObj | Handler(s) a3=1 | Sub a3=0 |
|-----|------|-----------------|----------|
| 00 | 0x831970 | seffect_hB | — |
| 00 | 0x831AE4 | type6_hB | — |
| 00 | 0x831C58 | type6_hB | — |
| 00 | 0x831DCC | type6_hB | — |
| 00 | 0x831F40 | type6_hB | — |
| 00 | 0x83350C | **boy_hB** | boy_init |
| 00 | 0x83A4F0..0x83AAC0 (5) | type60_hB | — |

| Seg | GObj | Handler(s) | Sub |
|-----|------|------------|-----|
| 01 | 0x831514, 0x831688, 0x8317FC | type6_hB | — |
| 01 | 0x831C58 | torch_hB | — |
| 01 | 0x831DCC | flag_hB | — |
| 01 | 0x83350C | boy_hB | boy_init |
| 01 | 0x83B204..0x83B7D4 (5) | type60_hB | — |

| Seg | GObj | Handler(s) | Sub |
|-----|------|------------|-----|
| 02 | 0x8313A0 | type22_hB | — |
| 02 | 0x831514, 0x831688, 0x8317FC | type6_hB | — |
| 02 | 0x831AE4 | type39_hB | — |
| 02 | 0x832F3C | **boy_hB** | boy_init |
| 02 | 0x839668, 0x839C38, 0x83A208, 0x83A7D8, 0x83ADA8 | type36_hB | — |
| 02 | 0x83B378..0x83B948 (5) | type60_hB | — |
| 02 | 0x83BABC, 0x83BDA4, 0x83C08C, 0x83C374, 0x83C65C | torch_hB | — |
| 02 | 0x83BC30, 0x83BF18, 0x83C200, 0x83C4E8, 0x83C7D0 | flag_hB | — |

| Seg | GObj | Handler(s) | Sub |
|-----|------|------------|-----|
| 03 | 0x831514, 0x831688, 0x8317FC | type6_hB | — |
| 03 | 0x831AE4 | type24_hB | — |
| 03 | 0x835DBC | **boy_hB** | boy_init |
| 03 | 0x83B660, 0x83BC30, 0x83C200, 0x83C7D0 | type36_hB | — |
| 03 | 0x846064..0x846634 (5) | type60_hB | — |
| 03 | 0x8467A8, 0x846A90, 0x846D78, 0x847060 | torch_hB | — |
| 03 | 0x84691C, 0x846C04, 0x846EEC, 0x8471D4 | flag_hB | — |

| Seg | GObj | Handler(s) | Sub |
|-----|------|------------|-----|
| 04 | 0x830F44 | **girl_hB** | — |
| 04 | 0x8310B8 | enemy1_hB, enemy1_hD | enemy1_init |
| 04 | 0x833224..0x8337F4 (5) | flag_hB | — |
| 04 | 0x833968 | **boy_hB** | boy_init |
| 04 | 0x83D658..0x83DC28 (5) | type60_hB | — |

| Seg | GObj | Handler(s) | Sub |
|-----|------|------------|-----|
| 05 | 0x832684 | type24_hB | — |
| 05 | 0x832AE0 | **boy_hB** | boy_init |
| 05 | 0x83CF14..0x83D4E4 (5) | type60_hB | — |

| Seg | GObj | Handler(s) | Sub |
|-----|------|------------|-----|
| 06 | 0x83122C, 0x831514, 0x8317FC | torch_hB | — |
| 06 | 0x831AE4 | chain_hB | — |
| 06 | 0x831C58 | seffect_hB | — |
| 06 | 0x83296C..0x832F3C (5) | flag_hB | — |
| 06 | 0x8330B0 | **boy_hB** | boy_init |
| 06 | 0x8397DC..0x839DAC (5) | type60_hB | — |

| Seg | GObj | Handler(s) | Sub |
|-----|------|------------|-----|
| 07 | 0x83122C, 0x831F40, 0x8320B4, 0x83239C, 0x8327F8, 0x83296C | torch_hB | — |
| 07 | 0x832AE0 | type22_hB | — |
| 07 | 0x832C54 | chain_hB | — |
| 07 | 0x832DC8 | seffect_hB | — |
| 07 | 0x832F3C | **girl_hB**, girl_hD | — |
| 07 | 0x833224 | enemy1_hB, enemy1_hD | enemy1_init |
| 07 | 0x833398, 0x83350C | generator_hB | — |
| 07 | 0x837388..0x837958 (5) | flag_hB | — |
| 07 | 0x837C40 | **boy_hB** | boy_init |
| 07 | 0x84007C..0x84064C (5) | type60_hB | — |
| 07 | 0x8407C0 | torch_hB | — |

| Seg | GObj | Handler(s) | Sub |
|-----|------|------------|-----|
| 08 | 0x83122C, 0x831514, 0x8317FC | torch_hB | — |
| 08 | 0x831AE4 | chain_hB | — |
| 08 | 0x831C58 | seffect_hB | — |
| 08 | 0x83296C..0x832F3C (5) | flag_hB | — |
| 08 | 0x832F3C | flag_hB | subGirlBrain_Idle |
| 08 | 0x8330B0 | **boy_hB** | boy_init |
| 08 | 0x8397DC..0x839DAC (5) | type60_hB | — |

| Seg | GObj | Handler(s) | Sub |
|-----|------|------------|-----|
| 09 | (igual a seg07, mesmos endereços) | idem | idem |

| Seg | GObj | Handler(s) | Sub |
|-----|------|------------|-----|
| 10 | 0x831AE4 | type55_hB | — |
| 10 | 0x832F3C | **boy_hB** | boy_init, subGirlBrain_Idle/Hesitate/Busy/PulledUp |
| 10 | 0x8330B0 | **girl_hB**, girl_hD | — |
| 10 | 0x83CC2C..0x83D370 (6) | type60_hB | — |

| Seg | GObj | Handler(s) | Sub |
|-----|------|------------|-----|
| 11 | 0x831970 | woodbox0_hB | — |
| 11 | 0x831C58, 0x831DCC, 0x831F40 | torch_hB | — |
| 11 | 0x8320B4, 0x832228, 0x83239C | ItemGeo | — |
| 11 | 0x832510 | seffect_hB | — |
| 11 | 0x832684, 0x8327F8 | enemy1_hB, enemy1_hD | enemy1_init |
| 11 | 0x83296C | type15_hB | — |
| 11 | 0x832AE0..0x8330B0 (4) | ap1_hB | — |
| 11 | 0x8330B0 também | ap1_hB | subGirlBrain_Idle |
| 11 | 0x833224 | generator_hB | — |
| 11 | 0x83350C, 0x833680, 0x8337F4 | bird_hB | — |
| 11 | 0x834F34 | **boy_hB** | boy_init |
| 11 | 0x8350A8 | **girl_hB**, girl_hD | — |
| 11 | 0x83FD94..0x840934 (9) | type60_hB | — |

Seg 12 (final): 0 GObjs com handler nomeado — sem cena ativa (fim do log).

## Padrões confirmados

1. **Piscina/reuso:** as mesmas casas de GObj (ex. 0x831C58 = type6_hB no seg00 →
   torch_hB no seg01 → seffect_hB no seg06) são reaproveitadas entre salas. O
   mapa casa→handler é re-escrito a cada `world_state_load`. Isso explica o
   round-robin: o host vê a mesma lista de handles, mas com DONOS diferentes.
2. **1 GObj = 1 handler primário + 0..N layers:** `*_hB`/`*_hD` como `a3=1`;
   `*_init`, `subGirlBrain_*` e callbacks de estado como `a3=0`. Ex.: BOY é sempre
   `boy_hB` (a3=1) + `boy_init` (a3=0). No seg10, o BOY acumula ainda os quatro
   estados GirlBrain como layers.
3. **`type60_hB` (0x0023D518) é o item mais frequente:** 5 por sala na maioria
   (9 no seg11). Candidato núcleo para a geometria de sala estática/objetos de
   cena de tipo 60 (piso/paredes) — provável "room props" que o host deve ligar
   aos meshes.
4. **Chave de reuso vs conteúdo:** a mesma casa pode mudar de papel entre salas;
   portanto o vínculo host GObj↔mesh NÃO pode ser estático por handle — deve ser
   (re)ligado a cada `world_state_load` de acordo com o handler observado.

## Confirmed / Probable / Possible / Unknown

- **Confirmed:** 29 callbacks nomeados resolvidos; ligação GObj↔handler por sala
  reproduzível do log; `a3` distingue handler principal vs layer; piscina GObj é
  reutilizada entre salas; `type60_hB` é o handler mais comum.
- **Probable:** `type60_hB` ↔ geometria/props de cena (não verificado contra
  meshes); `boy_init`/`enemy1_init` = callbacks de inicialização de entidade.
- **Possible:** os 176 callbacks não-resolvidos (não nomeados em `TARGET_FUNCTIONS`)
  incluem estados e handlers de entidades sem símbolo (ex. GirlBrain states adicionais).
- **Unknown:** qual mesh (P2O/TM2) cada handler corresponde no host; a ordem
  cronológica exata entre `init_scene_gobj` e `proc_add`; se `type60_hB` cobre
  todas as 5 geometrias de sala da cena 0x0F.
- **Discarded:** `a3=0` como "remove" — não é; é layer/init.

## Implicação para o Passo 2 (native-port)

O host hoje usa `m_sceneGObjs[i % size]` (round-robin) para ligar GObj→mesh.
Este revisão fornece a conexão runtime: para cada GObj criado em uma sala, o
handler `a3=1` após o `world_state_load` identifica o papel do GObj
(`boy_hB`→BOY/BoxMarker, `girl_hB`→GIRL, `type60_hB`→props de cena, etc.).
O re-link deve ocorrer por transição de sala, não em init único.

## Próximo teste mínimo

No host (`native-port`), em `KanbanSceneLoader`, após `applyVerifiedSceneTables()`
criar os 25 host GObjs: correlacionar as casas de GObj do log (seg02, 11 GObjs
nominais + 25 host) com os handlers capturados e substituir o round-robin por
um mapa `GObjHandle → handler esperado` derivado da tabela Rev.158, validando
com `gobj_attachment_test` que cada GObj recebe o mesh do papel correto.

## Veredito conservador

A ligação GObj↔processCallback por sala é fato verificado em runtime. A ligação
GObj↔mesh (nome do asset) permanece uma hipótese a confirmar por captura de
render ou por correlação com a entry table (0x2A4C48) + descritores (0x2A31B8).