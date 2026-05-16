# Cloth Initializer Spatial Observations

## Resumo executivo

Os 15 hits runtime de `0x001D27A8` (cloth payload init) revelam um padrao espacial
consistente: 3 entidades de cloth sao inicializadas por area, com posicoes que
formam uma linha em Y constante. Duas areas distintas foram observadas durante a
sessao de gameplay, cada uma com seu proprio valor de `[a1+0x30]`. As mesmas
entidades sao reinicializadas em revisitas, sugerindo recriacao do estado do
cloth ao carregar/revisitar uma zona.

## Fonte dos dados

Sessao PCSX2 com build instrumentada (EE Recompiler) em 2026-05-16.

Patch: `research/runtime/pcsx2-ico-recompiler-probe.patch`
Log: `.local/runtime-captures/ico-probe/events.jsonl`
Alvo: `0x001D27A8` (cloth_payload_init)
15 eventos capturados, filtro automatico por `pc == 0x001d27a8`.

Dados extraidos de `a1_words_32` (stack initializer de 32 words em `a1 = sp`):
- Words 0, 1, 2: float32 X, Y, Z da posicao espacial.
- Word 3: `0x3f800000` (1.0f) presente em todos os eventos.
- Word 12: varia por area (0 ou 1).
- Words 14-15: valores fixos por area.
- Words 4-11: `0x3f800000` ou `0x00000000` (provaveis escalares/padding).
- Words 16-31: copia identica das words 0-15 (mesmo struct de 16 words duplicado).

## Relacao com Rev.046

A Rev.046 confirmou que `a1 == sp` em `0x001D27A8` (o initializer recebe um
bloco na stack, nao um ponteiro de heap). Os dados capturados agora confirmam
que esse bloco de stack contem pelo menos posicao espacial e um campo de
variant/area. O initializer copia ou preenche 32 words (128 bytes) no total,
com os primeiros 16 words repetidos nos segundos 16.

## Area A — entrada do castelo

`[a1+0x30] = 0`
`word[12] = 0`

| Entity | Position | Revisits |
|---|---|---|
| `0x008320b4` | `(411, -175, 1333)` | 3x |
| `0x00832228` | `(350, -175, 1389)` | 3x |
| `0x0083239c` | `(311, -175, 1441)` | 3x |

Y constante em `-175`. X diminui e Z aumenta, formando uma linha no mesmo piso.
Provavel grupo de cloth initializers da entrada do castelo.

## Area B — area baixa

`[a1+0x30] = 1`
`word[12] = 1`

| Entity | Position | Revisits |
|---|---|---|
| `0x00831c58` | `(1342, -1245, -523)` | 2x |
| `0x00831f40` | `(1394, -1245, -530)` | 2x |
| `0x00832228` | `(1452, -1245, -525)` | 2x |

Y constante em `-1245` (nivel vertical mais baixo que area A). Z negativo.
A distribuicao espacial e compativel com uma ponte, dungeon ou passagem inferior.

## Comparacao entre areas

| Aspecto | Area A | Area B |
|---|---|---|
| `[a1+0x30]` | `0` | `1` |
| `word[12]` | `0` | `1` |
| Y medio | `-175` | `-1245` |
| Direcao Z | positivo (1333 a 1441) | negativo (-530 a -523) |
| Entidades | `0x008320b4`, `0x00832228`, `0x0083239c` | `0x00831c58`, `0x00831f40`, `0x00832228` |
| Revisitas | 3 ciclos | 2 ciclos |
| word[14-15] | `0x00ebe1d2` | `0x149a99a0` |
| word[3] | `1.0f` | `1.0f` |

Ambas as areas tem exatamente 3 entidades de cloth, Y constante por area,
e um valor fixo de word[14-15] que difere entre areas.

## Interpretacao de `[a1+0x30]`

O campo `[a1+0x30]` (word[12] do array de 32) varia entre 0 e 1 e se correlaciona
perfeitamente com o grupo espacial observado. E provavel que seja um campo de
variant/mode ou area ID, consistente com o campo `variant` em `[state_block + 0x04]`
observado nas funcoes de cloth (Rev.041-042).

Hipotese: este valor e copiado ou deriva do `[initializer_arg + 0x30]` descrito
na Rev.043 (cloth initializer arg source).

## Entidade `0x00832228` aparecendo em duas areas

O endereco de entidade `0x00832228` aparece nos dois grupos com posicoes
diferentes: `(350, -175, 1389)` na Area A e `(1452, -1245, -525)` na Area B.
Isso requer cautela:

- Pode ser a mesma entidade de jogo que contem dois cloths diferentes,
  cada um inicializado em areas diferentes.
- Pode ser reutilizacao de slot/object pool: o mesmo endereco de memoria
  e reaproveitado para uma entidade diferente em outra area.
- Pode ser coincidencia de alocacao (heap reuse).

Nao concluir persistencia sem evidencia de ciclo de vida.

## Unknown repeated words

**Area A** — word[14] e word[15] repetem `0x00ebe1d2` (nas duas metades do
struct de 32 words, entao aparece 4 vezes por evento). Value fixo em todos
os 9 eventos da Area A.

**Area B** — word[14] e word[15] repetem `0x149a99a0` (4 vezes por evento).
Valor fixo em todos os 6 eventos da Area B.

Possiveis interpretacoes (nenhuma confirmada):
- IDs de asset ou textura.
- Configuracao de rendering ou material.
- Dados de transform adicionais.
- Residuo de stack nao inicializado (menos provavel dado que o valor e
  consistente entre eventos e entre revisitas em ciclos diferentes).

Nao concluir asset ID sem evidencia cruzada com tabelas de asset ou
extracao de DATA.DF.

## O que fica confirmado

1. A stack initializer em `0x001D27A8` (`a1 = sp`) contem posicao espacial
   (float32 X, Y, Z nas words 0-2).
2. Word[3] e sempre `1.0f` (escala ou flag booleana padrao).
3. `[a1+0x30]` (word[12]) varia por area/grupo e se correlaciona com
   observacoes de variant em Rev.041-042.
4. Existem grupos de 3 cloth initializers por area, com Y constante e
   distribuicao espacial linear.
5. Os mesmos initializers sao recriados em revisitas a mesma area
   (ciclos diferentes, mesmas posicoes, mesmos valores de word[14-15]).
6. O struct de 32 words e simetrico: words 0-15 sao copia de words 16-31.

## O que fica provavel

1. `[a1+0x30]` e o campo variant/mode/area ID do cloth payload.
2. Area A corresponde a regiao de entrada do castelo.
3. Area B corresponde a uma regiao inferior (ponte, dungeon, passagem).
4. Word[14-15] e um campo de configuracao nao aleatorio, possivelmente
   um ID ou parametro de material.
5. Word[3] (`1.0f`) e um escalar de configuracao (escala uniforme ou
   flag ativada).

## O que ainda e especulativo

1. Se `0x00832228` e a mesma entidade persistente nas duas areas.
2. Se `0x00ebe1d2` ou `0x149a99a0` sao IDs de asset/textura.
3. Se esses tres cloths correspondem aos objetos "ROPE" documentados no
   modelo descriptor/callback.
4. Se existem mais de 3 cloths por area (a sessao so capturou a primeira
   leva de initializacao).
5. Se o padrao de 3 cloths se mantem em outras areas do castelo.
6. Qual a semantica exata de word[4-11] (alternam entre `0x3f800000` e `0x0`).

## Proximas capturas recomendadas

Na proxima sessao runtime, capturar adicionalmente:

1. `[a1+0x00]` a `[a1+0x3C]` em `0x001D27A8` (todas as 16 words) para
   confirmar a estrutura completa do initializer.
2. `[state_block + 0x04]` no dispatcher `0x001D37C8` para correlacionar
   variant com area.
3. Se possivel, capturar o nome da sala/area atual via algum descritor
   de cena ou string proxima no binario.
4. Explorar outras areas do castelo para ver se o padrao de 3 cloths
   por area se mantem (ex: salao do trono, torres, ponte levadica).

## Veredito

Os dados espaciais da stack initializer sao limpos e consistentes.
O padrao de 3 cloths por area com Y constante e reinicializacao em
revisitas e forte evidencia de que `0x001D27A8` e o initializer de
um grupo de objetos cloth ligados a zona atual do jogador.

O campo `[a1+0x30]` (refletido em word[12]) e o candidato mais provavel
para variant/area ID, alinhado com o modelo de Rev.041-042.

Dois valores fixos de word[14-15] foram observados, um por area.
Sua semantica exata requer mais evidencia.

A correlacao entre runtime, estrutura interna e espaco do jogo comeca
a se tornar viavel. Este e o tipo de dado que pode transformar a
decompilacao em compreensao real do sistema.
