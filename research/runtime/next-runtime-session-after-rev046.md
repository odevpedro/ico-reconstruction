# Next Runtime Session — After Rev.046

## Date

2026-05-15

## Objetivo

Aproveitar o momento pós-Rev.046 para capturar dados complementares que
fecham o modelo de descritores.

## Breakpoints

### 1. Breakpoint em 0x001B7A74 — CALLER (PRIORIDADE)

```
Endereço: 0x001B7A74
Descrição: "descriptor_caller_iter"
```

Sabemos que este código chama `[descriptor+0x58]`. Queremos saber:

- **Quem chama este código?** (`ra` quando chegar aqui)
- **Qual descriptor está ativo?** (`s0`)
- **Qual entity context?** (`s3`)
- **Este código itera todos os descritores ou só BARREL?**

O que capturar:
- `a0`, `s0`, `s3`, `ra`
- Se possível, dar `step over` algumas vezes para ver se `s0` muda

### 2. Breakpoint em 0x0013F7A8 — REGISTRO (CONFIRMAÇÃO)

```
Endereço: 0x0013F7A8
Condição: a3 == 0x13
Descrição: "rope_callback_register"
```

Para confirmar o registro do callback `0x001D3A30` no slot +0x50 do
descritor BARREL. O que capturar:

- `a0` — handle
- `a1` — **callback registrado** (deve ser 0x001D3A30)
- `a2` — slot (deve ser algo que leva a +0x50)
- `ra` — quem chamou o registro?

### 3. Breakpoint em 0x001D27A8 — CONTINUAR MONITORANDO

```
Endereço: 0x001D27A8
Descrição: "init_callback_hits"
```

O mesmo breakpoint da Rev.046, mas agora para verificar:

- O `variant` muda em outros hits? (`[a1+0x30]`)
- O caller é sempre o mesmo? (`ra`)
- O `a1 == sp` sempre? (confirmar)

## Dados complementares

### Dump do descritor FLEVER

Se o breakpoint 1 disparar, capturar dump de `s0` (64 bytes).
Se o descritor for FLEVER, verificar slots +0x48/50/58.

### Dump do initializer em outros hits

Se o breakpoint 3 disparar de novo, capturar `[a1+0x30]` e verificar
se o valor é diferente de 0.

## Script de captura

Quando cada breakpoint disparar, copiar manualmente:

```
BP1 (0x001B7A74): a0= s0= s3= ra= sp=
BP2 (0x0013F7A8): a0= a1= a2= a3= ra=
BP3 (0x001D27A8): a0= a1= ra= [a1+0x30]=
```

## Checklist

- [ ] Carregar `splat/SCUS_971.13.sym` (191 símbolos)
- [ ] Breakpoint 1 ativo: `0x001B7A74`
- [ ] Breakpoint 2 ativo: `0x0013F7A8` cond: `a3 == 0x13`
- [ ] Breakpoint 3 ativo: `0x001D27A8`
- [ ] Aba Memória aberta para dump rápido

## Próximo passo após captura

Com os dados dos 3 breakpoints, será possível:
1. Confirmar se o caller itera múltiplos descritores
2. Ver o registro do callback ROPE no slot +0x50
3. Ver se variant muda em outros contexts
