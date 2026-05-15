# rev.046 — Runtime Capture: a1 Source Resolved

## Date

2026-05-15

## Objective

Realizar a captura runtime do argumento `a1` em `0x001d27a8` para resolver
o gap aberto desde Rev.037 (origem do `a1`).

## Scope

- Breakpoint em `0x001d27a8` via PCSX2 debugger
- Captura de registradores (`a0`, `a1`, `ra`, `sp`)
- Dump de memória do initializer struct (`a1/sp`)
- Identificação do caller em `0x001B7A88`
- Dump do descriptor/record (`s0 = 0x002A3924`)

## Resultados

### Pergunta principal respondida

```
De onde vem a1 em 0x001D27A8(a0, a1)?
→ a1 = sp (stack pointer do caller)

Qual estrutura ele representa?
→ Initializer struct temporário na stack com dados de transform/pose

Qual variante é configurada?
→ [a1 + 0x30] = 0 (variant inicial = 0, neste hit)
```

### Cadeia de chamada completa

```
s0 = 0x002A3924  (descriptor/record na .data — ROPE?)
  └─ [s0 + 0x58] = 0x001D27A8  (callback de inicialização cloth)
       └─ jalr v0 em 0x001B7A80
            └─ a0 = s3 = 0x008320B4 (entity/context)
            └─ a1 = sp = 0x00798E40 (initializer na stack)
                 └─ 0x001D27A8 executa
                      └─ [a1 + 0x30] = 0 → copiado para payload.variant
                      └─ retorno v0 = payload cloth
                           └─ [entity + 0x800] = v0 (Rev.043 confirmado)
```

### Caller identificado

```asm
001B7A74  lw    v0, 0x58(s0)         ; carrega callback do descriptor
001B7A78  beq   v0, zero, 0x001B7A90  ; se null, skip
001B7A7C  daddu a0, s3, zero          ; a0 = entity/context
001B7A80  jalr  v0                    ; chama callback (0x001D27A8)
001B7A84  daddu a1, sp, zero          ; delay slot: a1 = sp
001B7A88  lw    v1, 0x15C(s3)         ; entity ptr
001B7A8C  sw    v0, 0x800(v1)         ; salva payload em [entity+0x800]
```

### Dump de a1/sp (initializer struct)

```
00798E40: 00 80 CD 43 00 00 2F C3 00 A0 A6 44 00 00 80 3F  410.5, -175.0, 1333.0, 1.0f
00798E50: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  (zeros)
00798E60: 00 00 80 3F 00 00 80 3F 00 00 80 3F 00 00 80 3F  1.0, 1.0, 1.0, 1.0
00798E70: 00 00 00 00 00 00 00 00 D2 E1 EB 00 D2 E1 EB 00  [a1+0x30]=0, sizes
00798E80: 00 80 CD 43 00 00 2F C3 00 A0 A6 44 00 00 80 3F  (bloco duplicado!)
00798E90: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00798EA0: 00 00 80 3F 00 00 80 3F 00 00 80 3F 00 00 80 3F
00798EB0: 00 00 00 00 00 00 00 00 D2 E1 EB 00 D2 E1 EB 00
```

A struct tem duas metades de 0x40 bytes cada, possivelmente representando
"pose inicial" e "pose alvo". Contém dados de transform (posição X/Y/Z:
410.5, -175.0, 1333.0) e floats 1.0 para escala/peso.

## What Is Confirmed

1. `a1` em `0x001D27A8` = `sp` do caller — estrutura temporária na stack
2. Caller está em `0x001B7A74`-`0x001B7A8C`
3. O callback é carregado de `[descriptor + 0x58]` onde descriptor = `0x002A3924`
4. `[a1 + 0x30] = 0` neste hit — variant inicial é 0
5. `[a1 + 0x58] = 0` — NÃO contém o callback (callback está no descriptor)
6. Retorno de `0x001D27A8` é salvo em `[entity + 0x800]`
7. Initializer struct contém dados de transform/pose, não só IDs

## What Is Probable

1. `0x002A3924` é o ROPE descriptor (próximo a `0x002A3974` documentado)
2. A struct de 0x80 bytes com duas metades representa "pose inicial → pose alvo"
3. O variant = 0 é o modo padrão de simulação cloth
4. O caller em `0x001B7A74` é parte do sistema de iteração de descritores
   (state resolver)

## What Remains Unknown

1. Estrutura exata do descriptor em `0x002A3924` (dump pendente)
2. Se este é o único callsite ou há outros contexts que passam a1 diferente
3. Qual fase do jogo disparou este breakpoint exatamente
4. Se o variant muda em outros hits (o breakpoint pode disparar múltiplas vezes)

## Next Minimum Tests

1. Capturar dump do descriptor `0x002A3924` (campos +0x00 a +0x68)
2. Investigar o caller `0x001B7A74` — qual contexto chama este código?
3. Verificar se há outros callsites de `0x001D27A8` (breakpoint continuar)
4. Atualizar structs hipotéticas com `[descriptor + 0x58]` como callback slot

## Conservative Verdict

O gap de runtime principal — a origem do `a1` em `0x001D27A8` — foi
**resolvido**. `a1` aponta para um initializer struct na **stack do caller**,
montado temporariamente com dados de transform/pose da entidade. O callback
(`0x001D27A8`) é carregado de `[descriptor + 0x58]`, não do initializer.

Isto encerra a investigação que estava aberta desde Rev.037 (estática) e
confirma o padrão suspeito na Rev.044 (constructor-like com `a1 = sp + 0x20`).

O descriptor `0x002A3924` e seu campo `+0x58` são os próximos alvos para
completar o entendimento do sistema de registro/callback cloth.
