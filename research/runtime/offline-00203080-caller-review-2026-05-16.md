# Offline Review of Caller 0x00203080

## Date

2026-05-16

## Objective

Revisar offline o caller `0x00203080` de `0x00201e70`, sem nova sessao de
emulador, para entender melhor a origem do `a1` nesse caminho e decidir se ele
deve ser priorizado no proximo probe runtime.

## Scope

Incluido:

- disassembly local com Capstone sobre `.local/extracted/SCUS_971.13.elf`;
- revisao de `0x00203080`, `0x00203ea0` e `0x00201e70`;
- busca byte-level por ponteiros/literais relevantes no segmento carregado;
- comparacao com Rev.037 e com a sessao runtime focada.

Excluido:

- nova execucao do PCSX2;
- inferencia semantica de gameplay;
- dump amplo de tabelas ou dados proprietarios;
- prova negativa definitiva sobre valores escritos dinamicamente.

## Sources Used

| Fonte | Uso |
|---|---|
| `.local/extracted/SCUS_971.13.elf` | bytes locais para disassembly e busca de words |
| `research/elf/ghidra-rev037-remaining-callers-and-rope-gap.md` | mapa estatico original dos callers de `0x00201e70` |
| `research/runtime/pcsx2-recompiler-focused-logpoints-session-2026-05-16.md` | runtime recente que observou `0x00203ea0`, mas nao `0x00203080` |
| `tools/runtime-probe-analyzer/runtime_probe_analyzer.py` | classificacao de callsites observados |

## Evidence Used

| Evidencia | Resultado |
|---|---|
| Disassembly local de `0x00202a60..0x0020312c` | localiza o callsite `0x00203080` dentro de uma funcao maior |
| Disassembly local de `0x00203d90..0x00203ee0` | confirma o caller `0x00203ea0` para comparacao |
| Disassembly local de `0x00201e70..0x00201f30` | confirma que `0x00201e70` usa o `a1` recebido do caller |
| Busca por word `0x001d3a30` no segmento carregado | encontrou apenas `0x002a3974` |
| Busca por word `0x001d3a30` na regiao/tabela iniciada em `0x0055cfd8` | nenhum match em stride `0x50` |

## Byte-Level Or Instruction-Level Findings

Trecho relevante do caller `0x00203080`:

```asm
0x00202ee8: lw       s0,0x0(v0)
...
0x0020301c: lw       a0,-0x6714(gp)
0x00203028: daddu    a1,s0,zero
0x0020302c: daddu    a2,t8,zero
...
0x00203080: jal      0x00201e70
0x00203084: sd       v1,0x18(s2)
```

Interpretação instrucional:

- `a0` vem de um global via `gp[-0x6714]`;
- `a1` e exatamente o valor salvo em `s0`;
- `a2` vem de `t8`, inicializado antes como `s2 + 4`;
- `s0` foi carregado de memoria em `0x00202ee8`.

O carregamento de `s0` ocorre depois de computar um endereco com base em uma
regiao apontada por:

```asm
0x00202c90: lui      s6,0x56
...
0x00202ecc: addiu    a0,s6,-0x3028
...
0x00202ee4: addu     v0,v0,a0
0x00202ee8: lw       s0,0x0(v0)
```

O endereco base literal e:

```txt
0x0055cfd8
```

Algumas instrucoes R5900 de multiply/move-from-accumulator nao foram
decodificadas pelo Capstone usado nesta revisao e aparecem como `.word` no
rascunho local. Por isso, esta nota nao afirma a formula completa do indice.

Comparacao com o caller observado em runtime:

```asm
0x00203e90: lw       a1,0x4(s0)
0x00203e94: beqz     a1,0x00203ea8
0x00203e9c: lw       a0,-0x6714(gp)
0x00203ea0: jal      0x00201e70
0x00203ea4: addiu    a2,s1,0x4
```

Diferença principal:

| Caller | Fonte de `a1` para `0x00201e70` |
|---|---|
| `0x00203080` | `s0`, carregado de uma regiao/tabela indexada |
| `0x00203ea0` | `[s0+4]`, lido de uma estrutura iterada em runtime |

## Literal Search Results

Busca por words little-endian no segmento carregado local:

| Word procurada | Matches | Endereco(s) |
|---|---:|---|
| `0x001d3a30` | 1 | `0x002a3974` |
| `0x001d27a8` | 1 | `0x002a397c` |
| `0x00201e70` | 0 | nenhum literal direto |
| `0x00203080` | 0 | nenhum literal direto |
| `0x00203ea0` | 0 | nenhum literal direto |

Busca especifica por `0x001d3a30` em entries de stride `0x50` a partir de:

```txt
0x0055cfd8
```

Resultado:

```txt
0 matches
```

## Confirmed

1. `0x00203080` chama `0x00201e70`.
2. No callsite `0x00203080`, `a1` e `s0`.
3. `s0` vem de um carregamento de memoria anterior, nao de um literal imediato.
4. A busca byte-level no segmento carregado encontrou `0x001d3a30` apenas em
   `0x002a3974`, o descriptor ja conhecido.
5. A sessao runtime focada observou `0x00203ea0`, mas nao observou
   `0x00203080`.

## Probable

1. `0x00203080` depende de uma tabela/regiao indexada para escolher o callback
   passado como `a1`.
2. Como `0x001d3a30` nao aparece nessa regiao em dados estaticos locais, o
   caminho `0x00203080` nao parece registrar o callback alvo por literal
   estatico.
3. Se `0x00203080` algum dia passar `a1 == 0x001d3a30`, isso provavelmente
   exigiria dado escrito/carregado em runtime, nao apenas o estado estatico do
   ELF.

## Possible

1. Uma rota de jogo diferente pode ativar `0x00203080`.
2. Dados de sala/layout podem alterar a regiao lida por `0x00203080` antes da
   chamada.
3. O indice que escolhe `s0` pode estar ligado a campos de entrada/objeto ainda
   nao nomeados.

## Unknown

1. A formula completa do indice usado para selecionar o `s0` em `0x00202ee8`,
   porque algumas instrucoes R5900 nao foram decodificadas por Capstone.
2. Se `0x00203080` ocorre em algum trecho de gameplay ainda nao testado.
3. Se algum dado dinamico pode fazer `s0 == 0x001d3a30`.

## Discarded

1. Tratar `0x00203080` como caminho estatico direto por literal
   `0x001d3a30`: a busca byte-level nao encontrou esse literal na regiao
   indexada observada.
2. Priorizar outro teste amplo em `0x001b7a74`: a sessao focada ja mostrou que
   probes diretos produzem evidencia melhor.

## Next Minimum Test

| Prioridade | Teste | Motivo |
|---|---|---|
| 1 | adicionar probe direto em `0x00203080` com `a0`, `a1`, `a2`, `s0`, `s2`, `s3`, `s4`, `t8` e word dump pequeno ao redor de `s0` | confirmar se esse caller ocorre em runtime e qual callback passa |
| 2 | manter o filtro em `0x0013f7a8` para `a1 == 0x001d3a30` | detectar o alvo final sem ruido |
| 3 | se `0x00203080` nao aparecer apos outra sessao longa, rebaixar sua prioridade e voltar ao caminho `entry[+0x24]` | evitar perseguir caller nao ativado no trecho testado |

## Conservative Verdict

Sem o emulador, foi possivel melhorar a priorizacao: `0x00203080` permanece
um caller tecnicamente possivel de `0x00201e70`, mas a evidencia estatica local
nao mostra `0x001d3a30` como literal ou entrada direta na regiao/tabela que
alimenta `s0`.

O proximo probe deve ser especifico e barato: logar `0x00203080` diretamente,
em vez de voltar para breakpoints amplos. Se ele nao aparecer em runtime, o
caminho deve perder prioridade frente ao outro candidato vivo,
`entry[+0x24]`.
