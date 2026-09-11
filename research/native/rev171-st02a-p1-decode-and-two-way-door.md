# Rev.171 — st02a_p1.p2o decode (room-family variant) + two-way door transition

- **Data**: 2026-09-11
- **Objetivo**: fazer `st02a_p1.p2o` (interior real da sala st02a) parsear e ser usada
  como malha de colisão/render do st02a, destravando a transição de porta de volta
  st02a → st00a (Rev.170 era one-way porque a porta st02a não "snapava" para o chão).
- **Escopo**: `native/src/engine/Ps2oMesh.cpp` (`loadCharacterMesh`), `native/src/main.cpp`
  (raio de snap da porta), verificação two-way headless.
- **Fontes/evidência**: `.local/iso/Ico (USA).bin` (derivados), arquivos em
  `native/assets/scene/rooms/st02a/pieces/*.p2o`, logs `/tmp/rev171-*.log` e shots
  `/tmp/rev171-final-*.ppm`, script de réplica `/tmp/probe_p1e.py`.

## Resultado (bytes)

`st02a_p1.p2o` é da **família room** mas o roteador `findObjs` a encaminha ao
`loadCharacterMesh` (caracter/family p2c). Ele tem **67 regiões OBJH**:

| Situação | Regiões | Observação |
|---|---|---|
| Parse limpo (strips canônicos Rev.151) | 48 | 8.156 strips / 19.652 tris / 15.552 verts |
| `nv == 0` (região vazia / metadata) | 7 | `[18, 24, 30, 36, 43, 52, 62]` — região 52 = só nomes de material (`47a_block3`, `sabi01`) |
| "strstop" (`a >= nvLocal`) | 12 | quads planos 2-D com posições `w==0`; guarda antiga os parava (não-fatal) |

O bug do loader: o primeiro encontro de `nvLocal == 0` (ou de região sem header
de strip) fazia `loadCharacterMesh` retornar `false` e **abortava o arquivo
inteiro** — st02a_p1 e st02a_p3 nunca carregavam.

### Mudanças em `Ps2oMesh.cpp`

1. `if (nvLocal == 0) return false;` → `continue;` — módulos OBJH sem dado de
   posição (registros de nome de material / footprints 2-D) são pulados, não
   abortam o load.
2. `if (i + kVertexStrideBytes > e) return false;` → `continue;` — região sem
   header de strip (metadata-only) é pulada.

### Resultado verificado (headless)

```
piece .../st02a_p1.p2o: 15552 verts, 19652 tris, 67 submeshes, 5 material names, fmax=4
piece .../st02a_p3.p2o: 2488 verts, 2851 tris, 16 submeshes
room st02a ClipBridge ok grid=54x65 blocked=2033 (from .../st02a_p1.p2o)
Render: 57 draw calls / 56698 triangles  (Rev.170: 54 calls / 34195 tris com fallback p2)
```

- **Colisão do st02a agora vem do interior real (p1)**, não do fallback p2.
- Render salta de 34.195 → 56.698 tris (+ interior p1 + p3).
- Texturas ausentes (`ba_2`, `ba_4`, `block3`, `abe4`, `kabe1`) usam fallback
  compartilhado `shared/gray` (não são encontradas no diretório st02a nem no
  shared; problema de extração de assets, não de decode).

## Diagnóstico do snap da porta st02a (Rev.170 era one-way)

Com p1 como malha de colisão, a porta `2a_door1.p2o` tem centro AABB xz = **(-210, 25)**
mas o chão caminhável mais próximo está a **~300 unidades** (`(-297.085,-262.082)`,
h=1917 vs nível do spawn 2150 — terraços de penhasco diferentes). O snap antigo
procurava num raio ≤ 150; não achava ponto standable5 → zona não instalada → o
menino nunca "fechava" na zona → sem `DOOR OPEN st02a -> st00a`.

### Mudança em `main.cpp`

Raio do spiral de snap da porta: `150.0f` → **`400.0f`** (heurística host documentada:
centro de AABB de peça de porta pode ficar a centenas de unidades do primeiro
ponto standable5 do piso; só conta com o predicado 5-cantos, não com floor simples).

## Verificação two-way (direção A e B)

**A — st00a → st02a** (`--room st00a --pair st02a --teleport 16.6671,15.2856`):
```
door st00a snapped to standable floor at (16.6671,15.2856)
DOOR OPEN st00a -> st02a
transition execute: ok (scene 0x2B, 23 host GObjs)
render 56698 tris sustained (st02a)
```

**B — st02a → st00a** (`--room st02a --pair st00a --teleport -210,-265`):
```
door st02a snapped to standable floor at (-210,-265)   (300 u do centro AABB)
door zone st02a at (-210,-265) -> scene 0x2B
DOOR OPEN st02a -> st00a  (frame ~33, após openDelay 1 s)
transition execute: ok (scene 0x2B, 23 host GObjs)
door zone st00a reinstalled at (16.6671,15.2856) -> scene 0x0F
```

CTest: **29/30** (única falha pré-existente: `opengl_backend` headless SEGFAULT).

## Confirmado
- st02a_p1 decode via skip de regiões `nv==0` + skip de região sem face header;
- números exatos de bytes/regiões por réplica python independente
  (`/tmp/probe_p1e.py`);
- porta st02a agora snapável (raio 400) e transição **bidirecional** verificada
  nos dois sentidos headless.

## Provável
- `nv==0` = módulos de "footprint"/metadata (nome de material) sem geometria;
- os 12 "strstops" são quads planos 2-D (`w==0` nas posições) que o guarda
  `a>=nvLocal` corta precocemente — realçável depois, secundário.

## Possível
- piso do st02a tem 2 terraços (h≈2150 no spawn vs h≈1917 perto da porta);
  snap do snap = 300 u é um artefato do ClipBridge por-peça (colisão = só p1).

## Desconhecido
- vínculo GObj↔mesh por sala continua heurística host (round-robin), não
  reconstrução byte-verificada; captura PCSX2 real do st02a ainda pendente.

## Descartado
- Grid coarseness como causa do não-snap (ClipBridge cell=50, grid 54x65 = normal).

## Próximo teste mínimo
- `--room st02a --pair st00a --teleport -210,-265` confirma `DOOR OPEN st02a -> st00a`
  (feito); re-testar interativo com WASD para sentir a distância do trigger.

## Veredito conservador
- Rev.171 fecha o conversor da família room-variant p2 (st02a_p1 + st02a_p3) para
  render/colisão e torna o demo de duas salas bidirecional. A distância de snap
  de 400 u é heurística host documentada, não afirmação sobre o jogo original.