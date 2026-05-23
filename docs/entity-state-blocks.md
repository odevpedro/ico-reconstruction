# Entity State Block Layout

> Documentacao gerada por analise estatica dos .s dos handlers (Rev.100).
> Cada tipo de entidade tem um state block de layout diferente em +0x800.

## Estrutura comum

```
entity_context (struct, stride variavel por entidade):
  +0x000  ...             (cabecalho/campos comuns)
  +0x15C  entity_state    (ptr para state block)
  +0x160-0x16B            (flags/contadores)
  +0x16C  avail_flag      (flag de disponibilidade)
  +0x170  active_flag     (flag de ativo)
  +0x174  ...             (fim do cabecalho comum — stride 0x174 = GObj!)
  +0x800  state_block     (offset a partir de entity_state)
```

## BARREL / ROPE (desc idx 0x14)

| Offset | Campos | Usado por |
|--------|--------|-----------|
| +0x070 | colisao/estado | barrel_init |
| +0x074 | fisica/pos | barrel_init, fn_1D2550, sub_1D2650 |
| +0x078 | frame atual | barrel_hC |
| +0x088 | velocidade | sub_1D2650 |
| +0x15C | entity_state | barrel_hC, barrel_init, fn_1D2550, sub_1D2650 |
| +0x16C | disponibilidade | barrel_init |
| +0x800 | state block base | barrel_hC, fn_1D2550, sub_1D2650 |

## WOODBOX0 (desc idx 17)

| Offset | Campos | Usado por |
|--------|--------|-----------|
| +0x05C | contador/estado | woodbox0_hC |
| +0x070 | tipo/subtipo | woodbox0_hC |
| +0x074 | parent ptr | woodbox0_hC |
| +0x0F0 | escala | woodbox0_hC |
| +0x0F4 | rotacao | woodbox0_hC |
| +0x134 | pos Y | woodbox0_hC |
| +0x138 | pos X/Z | woodbox0_hA |
| +0x15C | entity_state | woodbox0_hA/B/C |
| +0x160 | contador frames | woodbox0_hC |
| +0x16C | disponibilidade | woodbox0_hC |
| +0x180 | child ptr | woodbox0_hC |
| +0x7EC | unk | woodbox0_hC |
| +0x800 | state block base | woodbox0_hA/B/C |
| +0x814 | offset child data | woodbox0_hC |
| +0x840 | unk (matriz?) | woodbox0_hC |

## BIRD (desc idx 32)

| Offset | Campos | Usado por |
|--------|--------|-----------|
| +0x15C | entity_state | bird_hC |
| +0x4AC | timer/estado | bird_hC |
| +0x4B0 | contador | bird_hC |
| +0x4C4 | pos alvo | bird_hC |
| +0x544-0x550 | bounding box | bird_hC |

## BOY (desc idx 0)

| Offset | Campos | Usado por |
|--------|--------|-----------|
| +0x15C | entity_state | boy_hC, boy_hA, boy_activate, boy_dispCrown |
| +0x554 | estado animacao | boy_hC |
| +0x644-0x648 | flags | boy_hA |
| +0x67C | unk | boy_init |
| +0x800 | state block base | boy_hC, boy_hA, boy_activate, boy_dispCrown |
| +0x4EC0 | VBlank counter ref | boy_init |
| +0x4ED4 | unk counter | boy_hA |

## ENEMY1 (desc idx 4)

| Offset | Campos | Usado por |
|--------|--------|-----------|
| +0x088 | estado | enemy1_hC |
| +0x0C0-0x0C4 | flags | enemy1_init |
| +0x15C | entity_state | enemy1_hB/C, enemy1_init |
| +0x164 | connect/status | enemy1_hB, enemy1_init |
| +0x1D0-0x1FC | array de slots (14 x 4B) | enemy1_init |
| +0x204 | slot count | enemy1_init |
| +0x220 | child array ptr | enemy1_hB |
| +0x340 | unk ptr | enemy1_init |
| +0x378 | unk ptr | enemy1_init |
| +0x45C-0x468 | bounding box/size | enemy1_init |
| +0x548-0x558 | estado/transform | enemy1_hB/C |
| +0x63C | estado ataque | enemy1_hB |
| +0x670-0x678 | flags/estado | enemy1_init |
| +0x800 | state block base | enemy1_hB/C |
| +0x840 | child list head | enemy1_hB, enemy1_init |

## GIRL (desc idx 1)

| Offset | Campos | Usado por |
|--------|--------|-----------|
| +0x15C | entity_state | sub_1C1C48, sub_1C1EA8, sync... |
| +0x4A0 | destino/quaternion | sub_1C1C48, sync... |
| +0x800 | state block base | sub_1C1EA8 |

## ATTACKCH idx 62/63

| Offset | Campos | Usado por |
|--------|--------|-----------|
| +0x15C | entity_state | attackch62_hC |
| +0x16C | disponibilidade | attackch62_hC |
| +0x800 | state block base | attackch62_hC |
| +0x4EC0 | VBlank counter | (indireto) |

## Notas

- O stride do entity_context varia por tipo de entidade. O GObj (stride 0x174)
  contem o cabecalho comum, e os campos especificos ficam depois (ate o
  state block em +0x800).
- Muitos campos sao acessiveis via offsets no contexto, nao via ponteiros:
  a funcao carrega um ponteiro base (ex: $16 = context) e acessa
  offsets fixos (ex: 0x74($16)).
- O state block em +0x800 e acessado como: `lw $n, 0x15C($ctx)` -> $n = state_ptr,
  depois `lw $m, OFFSET($n)` para campos do state block.
  Ou as vezes diretamente: `lw $n, 0x800($ctx)`.
- +0x4EC0 e +0x4ED4 sao GP-relative offsets (nao campos do contexto).
