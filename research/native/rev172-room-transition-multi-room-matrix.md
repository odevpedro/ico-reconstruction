# Rev.172 — Multi-room transition matrix: the two-room door pipeline generalizes (12/12 verified)

- **Data**: 2026-09-11
- **Objetivo**: verificar que a transição de porta com troca REAL de resident set
  (Rev.170/171) generaliza para qualquer par de salas cuja porta "snapa" no piso
  standable5 — e mapear quais das 40 salas extraídas têm porta transicionável.
- **Escopo**: análise/verificação headless apenas (`--room <primary> --pair <companion>`);
  nenhuma mudança de código (o pipeline já era data-driven desde Rev.170).
- **Fontes/evidência**: binário `native/build/ico_native`, assets em
  `native/assets/scene/rooms/`, scripts de réplica `/tmp/verify_pairs.sh` e
  `/tmp/capture_evidence.sh`.

## Resultado (matriz)

**12/12 transições direcionais verificadas** entre os 4 rooms com porta snapável,
dentro de `--frames 90` headless (teleport no ponto snapado da porta):

| primary \ pair | st00a | st02a | st13b | st18a |
|---|---|---|---|---|
| **st00a** | — | OK | OK | OK |
| **st02a** | OK | — | OK | OK |
| **st13b** | OK | OK | — | OK |
| **st18a** | OK | OK | OK | — |

Cada célula = headless run que logou `DOOR OPEN <primary> -> <pair>` **e**
`transition execute: ok` no mesmo run (critério: ambos os greps ≥ 1). O par
original Rev.170/171 (st00a↔st02a) permanece válido; st13b e st18a passam a
funcionar nos dois sentidos com qualquer companheiro do grupo.

### Pontos de snap das portas (4 salas funcionais)

| Room | Piece de porta | Centro AABB xz | Snap (standable5, raio 400) |
|---|---|---|---|
| st00a | `door.p2o` | (0,-9.6585) | (16.6671, 15.2856) |
| st02a | `2a_door1.p2o` | (-210, 25) | (291.085?, -265) — Rev.171: (-210,-265) |
| st13b | `elev_door1.p2o` | (710,-494) | (1287.83, -958.462) |
| st18a | `st18door.p2o` | (0, 70.34) | (-3.49691e-06, 70.3415) |

> Nota Rev.172: o snap do st02a em Rev.171 foi logado como `(-210,-265)`;
> os snaps st13b/st18a acima são os valores logados pelo mesmo código.

## Salas com peça de porta, sem snap (5)

| Room | Piece | Centro AABB xz | ClipBridge (p1) | Causa provável do não-snap |
|---|---|---|---|---|
| st03t | `3_door.p2o` | (-0.5, 0) | grid 41x55, blocked 1533 | centro dentro dos bounds globais P1, mas sem chão standable5 em raio 400 |
| st04b | `door_monyo.p2o` | (0, -1.125) | grid 225x134, blocked 18085 | centro no limite/fora da área interior |
| st06a | `p3_door.p2o` | (0, 0) | grid 76 OBJH (p1 6 verts header), extent máx | centro fora do interior |
| st13c | `cagedoor.p2o` | (67, -4.78735) | 12 OBJH | centro dentro bounds globais, sem piso standable5 no raio |
| st22a | `cagedoor_01.p2o` | (-12.5, -237.461) | grid 289x297, blocked 53635 | centro dentro bounds globais, sem piso standable5 no raio |

Todas sem `DOOR OPEN`/`transition execute` em teleport no centro da porta.
`st05b` também tem peça de porta no inventário (não testada — vetor/sala
grande, mesma família dos casos sem snap).

## Confirmado
- O pipeline de transição **é genuinamente genérico**: 12/12 pares direcionais
  entre st00a/st02a/st13b/st18a atravessam porta e trocam o resident set
  (`requestScene(0x2B)`+`execute()`) no mesmo run headless.
- O único pré-requisito para uma sala participar do demo de 2 salas é a porta
  snapar (zona instalada em ponto standable5 do piso do próprio p1) — herança
  do critério Rev.170/171, não um hardcode de pares.
- Nenhuma mudança de código necessária para o resultado; a generalização já
  vinha do CLI `--room/--pair` (Rev.170) + snap 400 (Rev.171).

## Provável
- A porta decora o vão de transição; para 5 das 10 salas com porta o centro do
  AABB da peça fica longe do primeiro chão standable5 do interior (mais de 400 u,
  ou em "terraço"/borda sem piso de 5 cantos) — mesma classe do gap st02a que o
  raio 150→400 resolveu em Rev.171, só que em magnitude maior.

## Possível
- Os casos st03t/st13c/st22a (centro dentro dos bounds globais) podem ter piso
  standable5 num raio > 400 ou num terraço conectado por rampa; aumentar o raio
  de novo testaria isso, mas degrada a heurística (falso-snap longe da porta).

## Desconhecido
- Vínculo GObj↔mesh continua round-robin host para a sala 0x0F (Item 1 depende
  de sessão PCSX2 do usuário).
- Ordem/encaixe mundo-real entre salas (st00a→st02a→st03t...) não é modelada:
  o demo usa salas em espaços locais independentes, não um mundo global.

## Descartado
- Necessidade de código novo para generalizar (o CLI já cobria; análise apenas).
- Dependência do par: st13b↔st18a funciona sem st00a presente, provando que o
  swap é por `sceneId`, não por pares "aprendidos".

## Próximo teste mínimo
- `--room st13b --pair st18a` e `--room st18a --pair st13b` (feito, OK); nada
  bloqueado neste eixo. Próximo passo de PORT real continua sendo o Item 1
  (captura PCSX2 sala 0x0F) ou Item 3 (purga de assets do histórico git).

## Veredito conservador
- Rev.172 é uma revisão de **verificação e mapeamento**: documenta que a
  transição de sala por porta é genérica (12/12) e cataloga as 10 salas com
  porta em 2 grupos (4 funcionais, 5 sem snap + 1 sem teste). Não há claim novo
  sobre o binário original; o snap de 400 u segue heurística host.