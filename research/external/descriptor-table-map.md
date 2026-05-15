# Descriptor Table Map

## Date

2026-05-15

## Resumo executivo

Mapeamento da tabela de descritores na `.data` do ICO USA, baseado no
dump capturado em runtime (Rev.046) e nas funções do cluster cloth.

## Dados crus observados

Endereço base dos descritores observados: `0x002A3920`-`0x002A3AC0`.

### BARREL (endereço: 0x002A3924)

```hex
002A3920: 00 00 00 00 42 41 52 52 45 4C 00 00 00 00 00 00  ....BARREL......
002A3930: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
002A3940: 00 00 00 00 F3 00 00 00 00 00 80 BF 00 00 80 3F
002A3950: 0A D7 23 3C 01 00 00 00 00 00 00 00 00 00 00 00
002A3960: 00 00 00 00 00 00 00 00 01 00 00 00 28 3B 1D 00
002A3970: 00 00 00 00 30 3A 1D 00 00 00 00 00 A8 27 1D 00
002A3980: 00 00 00 00 00 00 00 00 52 4F 50 45 00 00 00 00
```

**Stride**: 0x60 bytes (96 bytes) até o próximo descritor.

### CHAIN (endereço: 0x002A3984, stride 0x60 de BARREL)

```hex
002A39A0: 00 00 00 00 00 00 00 00 00 00 00 00
002A39B0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
002A39C0: 00 00 00 00 00 00 00 00 01 00 00 00 00 00 00 00
002A39D0: 30 96 1E 00 00 00 00 00 10 98 1E 00 00 00 00 00
002A39E0: 38 8F 1E 00 00 00 00 00 00 00 00 00 43 48 41 49
002A39F0: 4E 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
```

Nome: **CHAIN** em `+0x04`.

### FLEVER (endereço: 0x002A39E8 + 0x60 = 0x002A3A48? OU stride diferente)

Nome: **FLEVER** em `0x002A3A54`.

### FLEVER_TRISTATE (endereço: 0x002A3AB4)

Nome: **FLEVER_TRISTATE** em `0x002A3AB4`.

## Tabela de descritores

| # | Endereço | Nome | Slots +0x48/50/58 | Observações |
|---|---|---|---|---|
| 1 | `0x002A3924` | BARREL | `0x1D3B28` / `0x1D3A30` / `0x1D27A8` | **Confirmado em runtime** (s0) |
| 2 | `0x002A3984` | CHAIN | `0x1E9630` / `0x1E9810` / `0x1E8F38` | Slots preenchidos |
| 3 | `0x002A39E4` | (não lido) | — | Entre CHAIN e FLEVER |
| 4 | `0x002A3A44` | FLEVER | — | Nome em holandês (alavanca/seta) |
| 5 | `0x002A3AA4` | FLEVER_TRISTATE | — | Variante do FLEVER |
| 6+ | ? | ? | — | Fora do dump |

### Funções nos slots do CHAIN

| Slot | Endereço | Símbolo | Possível papel |
|---|---|---|---|
| +0x48 | `0x001E9630` | func_001E9630 | Auxiliar CHAIN |
| +0x50 | `0x001E9810` | func_001E9810 | Update CHAIN |
| +0x58 | `0x001E8F38` | func_001E8F38 | Init CHAIN |

Estas funções estão no `main_text_postcloth_001d45b0_1705d4.s` e não
foram promovidas — pertencem a outro sistema (corrente física).

## Observações

1. **Stride**: parece ser 0x60 bytes entre descritores (BARREL → CHAIN
   = 0x60). Confirmar com FLEVER e FLEVER_TRISTATE.
2. **Nome "ROPE"**: está em `+0x60` do BARREL (pós-slots). Pode ser
   label extra ou dado de outro campo. Não é um descritor separado.
3. **FLEVER**: palavra holandesa que pode significar "seta" ou "alavanca".
   Nome de codigo interno Team Ico para sistemas físicos.
4. **FLEVER_TRISTATE**: variante com três estados do FLEVER.
5. **BARREL** é o descritor ativo no runtime capturado (s0). CHAIN e
   FLEVER não foram verificados em runtime.

## Próximo passo

1. Confirmar stride de 0x60 para todos os descritores
2. Verificar se FLEVER tem os mesmos slots (+0x48/50/58) preenchidos
3. Breakpoint em `0x001B7A74` para ver se o mesmo caller itera todos
   os descritores ou apenas BARREL
