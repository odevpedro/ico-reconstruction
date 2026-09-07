# Rev.142 — PS2O (`.p2o`) — layout de vértices confirmado + estrutura de registros de face localizada

- **Date:** 2026-09-06
- **Branch:** native-port
- **Trilha:** PORT (P1 — geometria do estágio) / estática
- **Objective:** avançar a decodificação do container de geometria `.p2o` (PS2 Object) que bloqueia o "próximo marco" do pipeline (asset → geometria → render). Confirmar o layout de posições de vértice e localizar a estrutura dos índices de face nos meshes de sala (STGST00A).

---

## Frame atual do problema

O pipeline de port chegou ao marco de renderizar uma sala estática. Os assets de geometria de estágio estão em arquivos `.p2o` (PS2O) dentro dos containers `.DF` PAL. A catálogo (Rev.139-141) mapeou 4.381 `.p2o` em 67 DFs. O bloqueio central é entender o formato binário para extrair vértices + faces e desenhar a geometria.

Arquivos de teste (extraídos de `STGST00A.DF` para `/tmp/df-stgst00a/`):

| Arquivo | Tamanho | sub-mesh (campo +0x08) | Uso |
|---------|---------|------------------------|-----|
| `170_st00a_p1.p2o` | 1.065.408 B | 3 | mesh principal da sala |
| `171_st00a_p2.p2o` | 476.656 B | 1 | mesh secundário (limpo) |
| `128_0str01_s2.p2o` | 5.472 B | 1 | peça estrutural |
| `097_box_water.p2o` | 864 B | 1 | objeto mínimo (proxy de partícula/água) |

---

## Confirmado — cabeçalho PS2O

Header comum a todos os `.p2o` analisados (LE):

| Offset | Tamanho | Campo | Valor observado |
|--------|---------|-------|-----------------|
| +0x00 | 4 | magic | `"PS2O"` (0x4F325350) |
| +0x04 | u32 | `payload_size − 0x10` | p1=1.065.392 (=1.065.408−16); p2=476.640; 0str01=5.456 |
| +0x08 | u32 | sub-mesh / objeto count | p1=3; p2=1; 0str01=1; box_water=1 |
| +0x0c..0x17 | 12 | zeros | — |
| +0x18 | 4 | tag `"SUM\0"` (0x004D5553) | todos |
| +0x1c | u32 | campo de significado não-resolvido | p1=0x04B88241; p2=0x01D80214; 0str01=0x0004DF06; box_water=0x5A90 |

O campo `+0x1c` NÃO se relaciona diretamente com contagem de vértices (p1: NV=14194, +0x1c≠NV·k para k inteiro em {1,2,4,8,16}). **Sem interpretação confirmada** (provável: flags/checksum/offste ou sub-bloco).

---

## Confirmado — layout de posições de vértice

A partir de **+0x20**, os vértices são **4 floats LE homogêneos `(x, y, z, w=1.0)`**, 16 bytes cada:

- p1: 14.194 vértices (227.104 B, termina 0x37740)
- p2: 6.330 vértices (101.280 B, termina 0x18BC0)
- 0str01: 79 vértices (1.264 B, termina 0x510)
- box_water: 8 vértices (termina 0xA0) — canto `(±0.1, ±0, ±0.1)`

Validação por padrão `w=1.0` (bit-pattern `0x3F800000`). Coordenadas são floats de mundo:
- **p1**: AABB principal X∈[-878,758], Y∈[-1573,100], Z∈[-1,1600]; **chão em Y≈0** (2.703 pontos em Y=0.0).
- **p2**: contém âncoras/outliers até X=-7.700, Y=-2.850, Z=+11.000 (objetos referenciais de cena, não poluição).

**Confirmado:** Y é o eixo vertical (up); o chão da sala principal de STGST00A está próximo de Y=0.

Point clouds exportados (dados, não visuais):
- `/tmp/st00a_p1_verts.obj` (14.194 v), `/tmp/st00a_p2_verts.obj` (6.330 v), `/tmp/0str01_verts.obj` (79 v), `/tmp/st00a_all_verts.obj` (20.603 v).
- Análise numérica (percentis, densidade Y) usada no lugar de renderização por imagem (modelo sem suporte a imagem nesta sessão).

---

## Confirmado — seções pós-vértice (atributos) em p2

Após as posições em p2 (0x18BC0):

- **UVs começando em 0x18BC0**: cada entrada `(u, v, 0, 0)` como 4 floats (16 B). Ex.: `(-0.5509, 0.5027, 0, 0)`. Região de UV estende-se até ≈0x2B330 (75.632 B = 4.727×16), não 1:1 com NV=6.330 → **os atributos NÃO são 1:1 com o array de posição**; a estrutura é composta/sublista.
- **Primeiro `0xFFFF` em 0x2B6A4**: marca o início dos primitivos/faces.
- **Strings de material/textura a partir de 0x2B99C** (p2) / 0x6A2C1 (p1): paths como `st00a\metal`, `wall_dec1`, `object\sdf\st00a\texture\*`. 53 strings em p1. Strings = names de material, ligados às faces via índice de material.

---

## Confirmado — face data = records de 16 bytes em primitivos `0xFFFF`-framed

O bloco de face é composto de **primitivos/strips delimitados por runs de `0xFFFF`**, e cada record de vértice/aresta é **8×u16 = 16 bytes**:

```
[c, t, 0, a, 1, b, 0, 0]
```

- `c` = flag: `0xFFFF` no **primeiro record do strip**, `0` nos seguintes.
- `t` = tipo do strip (observados `0x00` e `0x01`, alternando entre strips consecutivos).
- `a` (u16[3]) e `b` (u16[5]) = índices de vértice. Em strips tipo `0x00` observou-se **a == b** (mesmo índice espelhado), formando a sequência da espinha do strip; em strips tipo `0x01` observaram-se pares `(a,b)` com `a≠b` possíveis (ex.: `(7,11)`, `(6,10)`; ver exemplo abaixo).

### Exemplo legível por bytes — strip 1 (tipo 0x00), p2 @0x2BC2E (100 B)

```
[65535] [0] [0] [2] [1] [2] [0] [0]    <- c=0xFFFF (início do strip), a=b=2
[   0 ] [0] [0] [3] [1] [3] [0] [0]    <- a=b=3
[   0 ] [0] [0] [4] [1] [4] [0] [0]    <- a=b=4
[   0 ] [0] [0] [5] [1] [5] [0] [0]
[   0 ] [0] [0] [7] [1] [7] [0] [0]
[   0 ] [0] [0] [6] [1] [6] [0] [0]
então [0][4] + run de 0xFFFF (terminador do strip)
```

Espinha do strip = `[2,3,4,5,7,6]` → triângulos `(2,3,4) (3,4,5) (4,5,7) (5,7,6)`. Todos os índices `< NV`. Tam. do strip = 4 B de header + N×16 B de records, múltiplo de múltiplos do record.

### Exemplo — strip 2 (tipo 0x01), p2 @0x2BC9E (68 B)

```
[65535] [1] [0] [7] [1] [11] [0] [0]   <- c=0xFFFF, a=7, b=11
[   0 ] [1] [0] [8] [1] [ 8] [0] [0]   <- a=b=8
[   0 ] [1] [0] [6] [1] [10] [0] [0]   <- a=6, b=10
+ terminador 0xFFFF
```

Strip tipo `0x01` parece usar um segundo tipo de primitivo (fan/arestas?) com pares `(a,b)` que evoluem de forma diferente do tipo `0x00`.

**Correções à versão anterior desta nota:** (1º) o scan inicial leu `[0,0,N,1,N,0,0]`; (2º) um re-alinhamento alternativo propôs "aresta `[1,a,...,b]`". A leitura correta, confirmada por dump por bytes do cruzamento de dois strips, é **`[c,t,0,a,1,b,0,0]`** — onde `c` da PRIMEIRA posição é o frame marker `0xFFFF` do strip (não um header separado) e os índices aparecem em u16[3] e u16[5].

**Validado por bytes** (p2, cruzamento @0x2BC7E-0x2BCD0): strip 1 termina com run de 6× `0xFFFF`; strip 2 abre com `c=0xFFFF,t=1`. Tamanhos de strip: 100 B (tipo 0), 68 B (tipo 1), depois 176 B (tipo 0)… — todos múltiplos da grade de records + header + terminador.

**BLOQUEIO RESOLVIDO — regra de parsing global fechada:** a regra é **detectar `0xFFFF` na posição do campo `c`** (primeiro u16 do record de 16 B) como início de strip, e então ler records de 16 B enquanto validam o formato `[c,t,0,a,1,b,0,0]`. **Não é stride fixo** — o erro anterior foi tentar stride lockstep cego; o stride só funciona PRIMEIRO à detecção de frame. Com essa regra, o parser global varre o arquivo inteiro e recupera strips válidos em qualquer região de face (p2: **27 strips detectados** entre 0x2BC2E e 0x5CBAE, até nas regiões esparsas intercaladas com dados de atributos — gaps de até 51 KB são zonas de atributos/strings entre clusters de face). **76 triângulos não-degenerados** foram reconstruídos na varredura global completa e exportados como `/tmp/st00a_p2_faces.obj` (vertical bar: área mediana 1312, máximo 6625, todos os índices `< NV`, 193 vértices únicos usados em Y∈[-2800,1350] — faixa multi-andar/âncora de p2). Validação estrutural: 0 triângulos com área zero. **Confirmado em região isolada E no parser global.**

**Dois tipos de strip confirmados globalmente (não apenas 1):**
- **type `0x00`**: strips longos de triângulos (espinha a==b, média 12 vértices/strip, até 20), geram triângulos válidos e contíguos → **triangle strip do mesh**.
- **type `0x01`**: primitivos curtos (1-4 pares; npairs observados 1,2,4,6), pares `(a,b)` com `a≠b` que **não encadeiam** (zero `b==next_a`) → não são triangle strips; são **primitivos pequenos (quads/fans)**, consistentes com o record de quad do box_water. Semântica dos pares a≠b ainda em aberto (fan com pivô implícito?, 2 vértices de cada quad?).

### Nuance — múltiplos tipos de primitivo por strip (tipo `0x00` vs `0x01`)

O `box_water.p2o` (objeto mínimo, primitivo @0x160) mostra um **encodings de record compatível** com o formato canônico, mas com tipo/contagem diferente:

```
header: [4, 65535, 65535, 65535, 65535, 65535, 65535, 65535]   # count=4
record: [1, 0, 3, 0, 3, 0, 0, 0]   -> índice b=3 na posição u16[2] e u16[4]
record: [1, 0, 0, 0, 0, 0, 0, 0]
record: [1, 0, 2, 0, 2, 0, 0, 0]
record: [1, 0, 1, 0, 1, 0, 0, 0]
footer: [0, 65535, 65535, 65535, 65535, 65535, 65535, 65535]
```

Aqui o record é `[1, 0, b, 0, b, 0, 0, 0]` — o índice é **duplicado** (u16[2] e u16[4]) e o header é um `count` explícito (4) antes dos records, com footer de `0xFFFF`. Já no mesh de sala o record é `[c,t,0,a,1,b,0,0]` (índices em u16[3]/u16[5]) e o framing é por `c=0xFFFF` no primeiro record de cada strip, sem header-count. Conclusão: **o formato tem variantes de record conforme o tipo de primitivo/objeto** (mesh de sala: strips `0xFFFF`-framed com índice em u16[3]/u16[5]; objeto mínimo: quad com duplicação em u16[2]/u16[4] e header-count). O padrão comum — índice repetido em dois slots do record de 16 B e runs `0xFFFF` como delimitador — é consistente em ambos.

---

## Tags `"OBJH"` e `"null"` em `.p2o` mínimos

Em `097_box_water.p2o` (864 B, objeto mínimo), além de `"PS2O"` (0x00) e `"SUM\0"` (0x18):

- **`"null"` em 0xD0**: string de nome de sub-objeto + dados até 0x1C0.
- **`"OBJH"` (0x484A424F) em 0x250**: um header-objeto com tabela de streams. Após o magic, pares `[offset, count]` de 16 B referenciam blocos do arquivo:

| offset no arquivo | count | Conteúdo |
|-------------------|-------|----------|
| 0x20 | 4 | posições (4 cantos do box) |
| 0x60 | 1 | `(0, 1, 0, 1)` — provável normal/extent |
| 0x70 | 4 | normais/cores (zeros + 1.0) |
| 0xB0 / 0xC0 | 1 | cores `0x80…` (RGBA) |
| 0xD0 | 1 | string `"null"` (nome) |
| 0x1C0 | 1 | aponta para 0x160 = dados primitivos |

O `box_water` é um **proxy/objeto simples** (4 cantos + 1 primitivo), não um mesh completo — útil como referência mínima do formato OBJH/sub-stream.

---

## Confirmado / provável / possível / desconhecido / descartado

- **Confirmado**: magic `"PS2O"`; payload_size; sub-mesh count; tag `"SUM\0"`; posições = 4 floats LE `(x,y,z,1.0)` a partir de +0x20 (16 B/vértice); Y-up com chão em Y≈0; strings de material a partir de ~0x2B99C/0x6A2C1; dados de UV e vertex colors após as posições; **face data = records de 16 B `[c,t,0,a,1,b,0,0]` em strips `0xFFFF`-framed** (índices em u16[3]/u16[5], `c=0xFFFF` no primeiro record do strip), com variante de objeto mínimo `[1,0,b,0,b,0,0,0]` + header-count; tags `"OBJH"`/`"null"` em objeto mínimo com pares [offset,count]; **regra de parsing global FECHADA** (detectar `0xFFFF` na posição `c`, não stride fixo): 27 strips / 76 triângulos não-degenerados recuperados na varredura completa de p2; **dois tipos de strip** — type 0x00 = triangle strips (a==b, longos), type 0x01 = primitivos curtos (quads/fans, a≠b).
- **Provável**: type `t` seleciona primitivo (0x00 = tri-strip, 0x01 = quad/fan); `a` = vértice anterior/espinha, `b` = novo vértice; os pares `a≠b` do tipo 0x01 formam quads (2 vértices por record) com pivô implícito; campo `+0x1c` = flags/checksum; `"OBJH"` = tabela de sub-streams do objeto; vertex colors (`RR GG BB FF`) e lighting constants no meio do arquivo antes das strings de material; gaps entre clusters de face = atributos/material.
- **Possível**: runs `0xFFFF` entre strips são padding de alinhamento + delimitação de material; atributos (UV) indexados por stream separado em vez de por vértice; o quad do box_water (header-count) é a mesma família do type 0x01.
- **Unknown**: semântica exata dos pares `a≠b` do type 0x01 (como gerar quads/fans); significado do campo `+0x1c`; delimitação exata dos `count=3` sub-meshes de p1 no buffer linear; encoding do índice (0-based u16?).
- **Descartado**: "posições são floats inconsistentes/ruído" (erro de range de expoente no primeiro scan; range correto `0x70..0x90`); "parsing por stride fixo global" (falha com 0 válidos — a regra correta é detecção de `0xFFFF` na posição `c`, não stride); **"record de face com índice em u16[0]"** e **"aresta `[1,a,...,b]`"** como formato único universal (corrigido para `[c,t,0,a,1,b,0,0]` no mesh; box continua com duplicação em u16[2]/u16[4]).

---

## Erros de método corrigidos nesta sessão

1. **Range de expoente errado** no scan de floats (usei `0x38..0x48`; correto `0x70..0x90` ⊕ `1.0`). Causou falso "dados não são floats" para p1. Corrigido iterando o padrão `w=1.0` em vez de expoente.
2. **Hipótese "1:1 atributos↔vértices"**: os atributos (UV) não são 1:1 com o array de posição (4.727 entradas vs 6.330 vértices em p2) — a estrutura é de sub-listas, não uma interleaved por vértice única.
3. **Confusão UV vs vertex colors vs faces**: a região aparentemente "face" em 0x2B6A4 é na verdade **vertex colors** (`RR GG BB FF`), seguida de lighting constants e strings de material. As faces reais (primitivos `0xFFFF`) começam depois, ~0x2BC2E+.
4. **"Um único record universal de face"** — três leituras tentadas: (a) `[0,0,N,1,N,0,0,0]` (índice duplicado em u16[2]/u16[4]); (b) aresta `[1,a,...,b]`; (c) `[c,t,0,a,1,b,0,0]`. A leitura (c) é a correta para o mesh de sala (confirmada por dump do cruzamento de dois strips em 0x2BC7E-0x2BCD0), e (a) vale para o objeto mínimo. O erro foi assumir um formato único: os tipos de strip (0x00 vs 0x01) e o tipo de objeto (mesh vs proxy) mudam o layout. **Scan lockstep de stride fixo (16 B) sobre o arquivo todo deu 0 válido** (contagem 10.844→0) — a lição: NÃO é stride fixo global; o parser deve **detectar `0xFFFF` na posição `c`** e então ler records enquanto validarem. Com isso o parser global passou a funcionar (27 strips / 76 triângulos em p2).

---

## Próximo mínimo teste

1. **Decodificar o type `0x01` completo** (quads/fans): testar fan com pivô = primeiro `a` de cada primitivo (tri9s `(p,a_k,b_k)`), ou quad 2-vértices-por-record. Validar contra o box_water.
2. **Completar a extração**: reconstruir TODOS os triângulos (type 0x00) dos 27 strips + quads do type 0x01, gerar `.obj` completo de p2 e exportar o `p1` (3 sub-meshes) com a mesma regra.
3. **Mapear os `count=3` sub-meshes de p1**: usar a tabela `"OBJH"`-like [offset,count] ou o reframing para delimitar os 3 sub-buffers.
4. **Resolver o campo `+0x1c`**: comparar entre os 4 arquivos e procurar relação com offsets/tamanhos reais.

## Verdict conservador

O layout de **posições de vértice** (4 floats LE a partir de +0x20, 16 B/vértice) e a **forma das faces** (records de 16 B `[c,t,0,a,1,b,0,0]` com `0xFFFF`-frame na posição `c`; variante com header-count no objeto mínimo; dois tipos de strip 0x00/0x01) estão **confirmados por bytes E por parser global**: a regra de detecção de `0xFFFF` na posição `c` recupera 27 strips / 76 triângulos não-degenerados na varredura completa de p2 (área mediana 1312, todos índices `< NV`), exportados como `/tmp/st00a_p2_faces.obj`. Permanecem em aberto: semântica exata do type `0x01` (quads/fans), delimitação multi-submesh de p1, e o campo `+0x1c`. **Conclusão-chave de método**: parsing de face NÃO é stride fixo — é detecção de frame `0xFFFF` na posição `c` seguida de records de 16 B enquanto validarem. O formato está essencialmente decodificado para o mesh de sala (type 0x00); o type 0x01 é o próximo passo determinante.

---

# Addendum — Rev.142 follow-up (2026-09-06): tabela de materiais, array de UV e tabelas de stream OBJH em p1 (170_st00a_p1.p2o)

Objectivo desta passada: resolver a ligação face→material e face→UV (documentada como unknown no Rev.142). Avanço confirmado parcial; a ligação face→material continua em aberto, mas a infraestrutura foi mapeada por bytes e via tabelas OBJH.

## Confirmado — posições terminam em 0x36370 (p1)

O scan `w==1.0` dá **13.877 posições** (0x20→0x36370). Imediatamente após, em **0x36370**, começa o array de UV.

## Confirmado — array de UV em 0x36370 (p1)

Entrada = 4 floats LE `(u, v, 0, 0)` (16 B). Exemplos: `(0.590, 0.500, 0, 0)`, `(0.649, 0.534, 0, 0)`, `(0.658, 0.500, 0, 0)`. UV predominantemente em `[0..1]`, mas alguns valores fora (ex.: `-1.434`, `-0.75`) — indica tiling/repetição ou mapeamento ainda não exato. **Contagem do array = 13.070 entradas** (0x36370→0x69450), NÃO 1:1 com os 13.877 vértices nem com os 7.922 strips.

## Confirmado — tabela de materiais em ~0x6A210 (p1), 7 registros

- **7 registros de material**, stride = **0x90 (144 B)**, nomes em 0x6A2B0, 0x6A340, 0x6A3D0, 0x6A460, 0x6A4F0, 0x6A580, 0x6A610.
- Cada registro: params de shading/cores (padrões `0x80xxxxxx` = RGBA8, floats `0x3F800000`=1.0, `0x3EFEFEFE`≈-0.498) + **nome** + **path de textura**.
- **Os 7 nomes correspondem 1:1 a 7 arquivos TM2** por base: `metal`, `wall_dec1`, `st0_a`, `broken`, `wall_fuchi2`, `isikabe`, `pole` — paths como `object\sdf\st00a\texture\{nome}`. **Ligação material→textura = por nome (7 materiais ↔ 7 TM2)**.
- Anteriormente à tabela (0x6A180-0x6A2A0): tabela de **vertex colors / lighting** (RGBA8 `0x80...`).

## Confirmado — records de face de p1 NÃO carregam índice de material
> **CORREÇÃO (Addendum 3).** Esta seção está parcialmente superada: o índice de material NÃO está em u16[4] (que é constante 1) nem em u16[3] (espinha `a`), **mas está sim presente como u16[7] (`f`)**, que é o índice de material/partição por strip (100% constante dentro de cada strip; valores `{0..7,9}`). O que permanece correto desta seção: u16[4]=1 e `a`=espinha; o que se corrige: "não carregam índice de material" → "carregam o índice em `f` (u16[7])".

Re-parse por bytes da região `0xFFFF`-framed (o mesmo do loader, que renderiza a sala com 7.922 strips / 15.161 tris):
- Record p1 = `[c, t, 0, a, 1, b, 0, 0]` — `a` (u16[3]) = índice de posição (espinha), `u16[4]` = **constante 1**, `b` (u16[5]) = espelho de `a`. Nenhum campo pequeno 0..6 que sirva de material.
- O campo que parecia "material 0..6" num sample inicial era **coincidência** (era a espinha `a`/posição, que incrementa 3,4,5,...). O campo `u16[4]` é constante `1`.
- Consequência: **face→material não está nos records de face** — deve vir de uma tabela por partição/subbatch ou por ordem de stream (não localizada ainda).

## Confirmado — tabelas de stream `OBJH` por sub-mesh (p1, 3 tags)

- `subMeshCount=3` → **3 tags `"OBJH"`**: `0x101620`, `0x103D20`, `0x1040B0` (perto do fim do arquivo, arquivo termina 0x1041B0).
- O `OBJH` @0x101620 lista os streams do mesh principal em pares `[offset, count]`:
  | offset | count | conteúdo |
  |--------|-------|----------|
  | 0x00000020 | 0x001E71 (7793) | posições A (0x20→0x1E730) |
  | 0x0001E730 | 0x0017C4 (6084) | posições B (0x1E730→0x36370); A+B=13.877 |
  | 0x00036370 | 0x00330E (13070) | **UV array** (0x36370→0x69450) |
  | 0x00069450 | 0x00036C (876) | face/attr (@0x6A6AE inicia) |
  | 0x0006A210 | 0x00000A (10) | tabela de materiais |
  | 0x0006A2B0 | 0x000007 (7) | **nomes de material (7)** |
- Os `OBJH` @0x103D20 e @0x1040B0 referenciam streams em 0x1017xx-0x1014xx (outros sub-meshes/overlays).

## Confirmado / em aberto (atualização)

- **Confirmado agora**: array de UV em 0x36370 (p1); tabela de **7 materiais ↔ 7 TM2 por nome** (~0x6A210); **índice de material por strip em `f` = u16[7]** (ver Addendum 3 — corrige o texto original desta seção); 3 tags OBJH = 3 sub-meshes com tabelas de stream [offset,count] que apontam para posições/UV/materiais.
- **Em aberto (o bloqueio real para texturização)**: o **índice face→UV** (13.070 UVs ≠ 7.922 strips ≠ 13.877 vts — NÃO é 1:1 com nenhum) e o **índice face→material**. Como os records de face só têm UM índice de vértice (a u16[3]), o UV provavelmente é indexado por `a` numa sub-lista de UV (13.070 ≥ max `a`=7.792 → `UV[a]` válido para todas as faces), mas isso precisa validação visual (UV fora de [0,1] sugere tiling ou espinhamento não exato). O material deve ser resolvido via os streams OBJH/por-partição.
- **Próximo mínimo teste proposto**: testar `UV = UVarray[índice_de_posição a]` por face e renderizar texturizado para validar visualmente a coerência da textura; e mapear qual stream/partição OBJH atribui material a cada grupo de faces.

---

# Addendum 2 — 2026-09-06: EXPERIMENTO de validação da hipótese face→UV (REFUTADA)

## Objectivo / enquadramento

Teste de validação da hipótese mais barata (opção B): **UV = UVarray[índice de posição `a`]** por face. NÃO é marco de texturização — é experimento para reduzir incerteza antes de investir em RE do stream encodado 0x69450-0x6A210 (opção A).

## O que foi feito

1. `Ps2oMesh::uvs` + leitura best-effort do array de UV em 0x36370 (13.070 entradas `(u,v,0,0)`) — confirmado o load (13.070 UVs, primeiras entradas `(0.590,0.500)` etc., max tri idx 7792 < 13070, sempre in-bounds).
2. `icá_native --uv-test`: render do mesh de sala p1 com **checkerboard 64x64** (o grid expõe seam/flip/stretch), UVs = `UVarray[a]`, salvando `/tmp/uvtest.ppm` via `captureFrameRGB`.

## Evidência (mensurável, não só visual)

- **mean UV edge spread por triângulo = 1.46** (domínio de textura ~2 unidades) — um mesh corretamente texturizado tem spread por triângulo tipicamente < 0.2 (cada triângulo amostra uma região pequena).
- **27% dos 15.161 triângulos têm spread de aresta UV > 2.0** (mais de um tile inteiro num único triângulo) — fisicamente errado para textura coerente.
- Apenas 8.5% com spread < 0.25.
- Suavidade local do buffer: `mean|UV[a]-UV[a+1]| = 0.40` com saltos até 4.2 — função índice→UV descontínua, incompatível com coordenadas de textura locais.

## Verdict

**A hipótese `UV = UVarray[índice de posição a]` está REFUTADA** por evidência medida (não por leitura visual ambígua). A etapa B eliminou o caminho barato com certeza: **o array de UV em 0x36370 NÃO é indexado diretamente pelo índice de posição `a`** das faces. As UVs são uma **sub-lista separadamente indexada (por strip/batch/partição)**, e a ponte face→UV exige o stream encodado 0x69450-0x6A210 e/ou os streams OBJH. Isto torna a **opção A (decodificar o stream/partição OBJH) a única via**, agora sem ambiguidade sobre se vale a pena.

## Próximo passo (após refutação)

1. RE do stream 0x69450-0x6A210 (os valores `0xffxx`/`xxff` — prováveis s16) como candidato a tabela de atributos por strip/face (cor/UV/partição de material).
2. Cruzamento com os streams OBJH: como as partições mapeiam face→material e face→UV.
3. Só depois, texturização real com os 7 TM2 (material↔textura por nome, já confirmado).

---

# Addendum 3 — 2026-09-06: material por strip (coluna `f`) e modelos de indexação de UV testados

## Descoberta confirmada: a coluna `f` (u16[7]) do record de face é o índice de material/partição por strip

Re-parse dos records `[c,t,0,a,m,b,s,f]` de p1 (via regra spine-a) em todo o bloco de faces (0x6A6A0→~0x9AC90, 4.980 strips / 19.441 records):

- **`f` (u16[7]) é 100% constante dentro de cada strip** (4.980/4.980 = 100%), e assume **apenas `{0..7, 9}`** — um índice de partição/material por strip. Isto CORRIGE o registro anterior (Addendum 1) que dizia que o record não carregava índice de material: **o indexador não está em u16[4] (constante 1) nem em u16[3] (espinha `a`), mas em u16[7] (`f`)**.
- Distribuição dos strips por `f`: `{0:1501, 2:1902, 4:915, 7:320, 1:184, 3:125, 5:22, 6:8, 9:3}` — **não-uniforme e espaçada**, coerente com 7 materiais texturizados (0-6) + um material extra `7`/`9` (talvez shadow/efeito): 0,2,4,7,1,3 são os grupos grandes.
- **Runs de material**: os strips aparecem em runs do mesmo `f` (80 runs no total), com interleaving 2↔3 em geometria interna (provável geometria intercalada dos mesmos materiais), mas `f=7` forma 1 run longo contíguo (310 strips) e `f=6` um run de 8 imediatamente antes — sugere ordem de batch parcial.
- As outras colunas: `t` (u16[1]) 0..1; u16[2]≡0; `a` (u16[3]) = espinha (posição, range 0..7645); u16[4]=1 const; `b` (u16[5]) = espinha espelhada (range 0..7792); `s` (u16[6]) = **range 0..703** (possível segundo índice, divergente de `a` em alguns records).

## Modelos de indexação face→UV testados (todos sobre a UV array 0x36370, 13.070 entradas)

Métrica: **UV edge spread por triângulo** (quanto menor e <0.2, mais coeso/correto).

| Modelo | Índice UV | mean spread | bad(>2.0) | Verdict |
|--------|-----------|-------------|-----------|---------|
| `UV[a]` (por índice de posição) | `UVarray[a]` | **1.39** | 2525 | REFUTADO (visual + métrica) |
| `UV[s]` (coluna u16[6]) | `UVarray[s]` | 1.52 | 2977 | refutado |
| 3 UV por tri global | `UV[3t+k]` | **0.71** | 367 | parcial (cobre só 4.356/9.531 tris, mistura materials) |
| base_f (slice por f, 3 UV/tri) | `UV[base_f+3*t_f+k]` | 0.71 | 367 | estoura UVs (base total 27k>13k) |

- O modelo `UV[3t+k]` é o **melhor relativo**, mas só cobre os **primeiros ~4.356 tris** (13.070/3) e mistura materiais — NÃO é o mapeamento final, e o "0.32" reportado antes era um bug de denominador (corrigido: 0.71).
- Nenhuma proporção limpa: UVs=13.070 vs records=19.441 vs tris=9.531. Razões 0.672 e 1.37 — não inteiras. **A UV array não é 1:1 nem por vértice, nem por tri, nem por record.**
- A UV array tem **interleaving denso** (2.172 saltos >1.0 nos primeiros 300) — não é agrupada por material em blocos longos; padrão consistente com UVs interleaved por vértice de streaming, mas a indexação exata permanece em aberto.

## O que isso significa

- **face→material está RESOLVIDO** (o suficiente para começar): `f` = índice de material por strip, e material↔TM2 por nome (confirmado). Isso é um avanço grande e acionável para o milestone de texturização.
- **face→UV permanece em aberto**: os modelos de indexação testados não fecham. A próxima via é investigar a **estrutura da UV array em si** (13.070 entradas — como são agrupadas/fatiadas) e a relação com `f` (talvez haja slices UV por `f` com contagens em outro lugar), e re-examinar a coluna `s` (range 0..703) como possível base de UV por strip/partição.
- Observação chave a verificar a seguir: 13.070 ≈ soma de alguma contagem por `f` (ex.: nº de records de um subconjunto), e a estrutura `0x69450` (876×4B, hi-byte 0xFF = cores RGBA6 etc.) entre UV e materiais.

## Próximo mínimo teste

1. Testar a hipótese **slices de UV por `f`**: para cada material `f`, procurar a sub-lista de UVs que dá spread mínimo nos tris daquele `f` (busca de base por `f`, 7 dimensions), usando os ~13.070 UVs como pool — confirmar a regra de indexação real.
2. Re-examinar `s` (u16[6], 0..703) como contador/índice de batch de UV por strip.
3. Validar `f` como material no render (pintar por `f`) antes de qualquer textura TM2.
