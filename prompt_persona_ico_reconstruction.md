# A Ruína Digital Chamada ICO

## Como aprender engenharia reversa escavando um dos jogos mais silenciosos do PlayStation 2

---

### Prólogo — O silêncio do ELF

Quando abri o ELF pela primeira vez, tive a sensação estranha de estar diante de uma ruína. Não havia nomes de funções. Não havia comentários. Não havia mapas. Apenas endereços, registradores e pedaços de intenção fossilizados em MIPS.

O arquivo se chamava `SCUS_971.13`. Era o executável principal de **ICO**, um dos jogos mais misteriosos do PlayStation 2. E estava completamente stripped — sem tabela de símbolos, sem nomes de funções, sem debug symbols. Como uma biblioteca onde todos os livros tiveram suas capas arrancadas.

Eu não era um engenheiro reverso experiente. Era apenas um estudante de Ciência da Computação fascinado pelos jogos da Team Ico, tentando entender como aquela obra havia sido construída. E ali estava eu, diante de 5.48 megabytes de código MIPS anônimo, sem saber por onde começar.

Este texto é o relato dessa jornada. Não é um tutorial. Não é uma documentação formal. É o diário de uma escavação digital.

---

## 1. O primeiro contato com o binário

Tudo começa com um disco. Não um disco qualquer — uma imagem BIN/CUE de ICO, versão americana. O primeiro passo foi entender o que eu tinha em mãos.

O arquivo `.cue` descrevia uma trilha única:

```
TRACK 01 MODE2/2352
```

Isso significa que não era um CD-ROM comum. Era um disco no formato `MODE2/2352`, onde cada setor físico tem 2352 bytes, e os dados úteis ISO9660 começam 24 bytes depois do início do setor. Um detalhe técnico pequeno, mas que já mostrava uma verdade importante: engenharia reversa de console não é como abrir um executável no Windows. Cada plataforma tem suas peculiaridades, e o PS2 é uma das mais complexas.

Dentro do ISO, encontrei o executável principal: `SCUS_971.13`. Um ELF32 little-endian MIPS, com entry point em `0x00100008`. Também encontrei `DFDATAS/DATA.DF`, um arquivo de mais de 500 MB que claramente continha os assets do jogo — texturas, modelos, animações, cenas. Mas esse era um problema para depois.

O executável, por enquanto, era o foco.

### A descoberta dos primeiros metadados

Sem poder (e sem querer) distribuir o binário do jogo, criei ferramentas que liam apenas metadados: nomes de seções, tamanhos, endereços, flags. Nada do conteúdo interno.

O ELF tinha 27 seções. As mais importantes:

- `.text`: código principal, começando em `0x00100000`
- `.data`: dados inicializados
- `.rodata`: dados somente leitura
- `.bss`: dados zerados em memória
- `.DVP.ovlytab` e `.DVP.ovlystrtab`: tabela de overlays
- 12 seções `.DVP.overlay...`: possivelmente código carregado sob demanda

E, claro, nada de `.symtab`, nada de `.dynsym`. O binário estava limpo. Nenhuma função teria nome. Cada uma das ~4000 funções que eu viria a encontrar seria um número até que eu pudesse dar a ela um significado.

Foi quando eu percebi: isso não é engenharia reversa. Isso é arqueologia.

---

## 2. O que significa reconstruir um jogo

Antes de continuar, preciso fazer uma distinção importante.

**Emular** é fazer o jogo rodar em outro hardware. **Modificar** é alterar bytes para mudar comportamento. **Decompilar** é traduzir assembly de volta para C. **Reconstruir** é criar uma base de código que produza o mesmo binário quando compilada. **Documentar** é registrar o comportamento observado sem necessariamente produzir código.

Este projeto começou como documentação e, aos poucos, está se movendo em direção à reconstrução. Mas não existe um botão mágico que transforma um binário em código fonte legível. Existe um processo lento de:

1. Identificar funções por seus prologues (o `addiu $sp, $sp, -N` que marca o início de cada função MIPS)
2. Agrupar funções por contexto (strings que referenciam, dados que acessam, funções que chamam)
3. Entender o comportamento observando instruções, não nomes
4. Validar hipóteses com breakpoints no emulador
5. Repetir

Eu encontrei 3991 prologues no `.text` do ICO. 71 tamanhos diferentes de stack frame. Os mais comuns eram frames pequenos (16-64 bytes), o que sugeria muitas funções auxiliares simples. Mas alguns frames chegavam a 2784 bytes — funções grandes, complexas, provavelmente centrais para o funcionamento do jogo.

Cada uma delas era uma sala fechada, esperando para ser aberta.

---

## 3. O ELF stripped e o silêncio dos nomes perdidos

Trabalhar com um ELF stripped é como explorar uma cidade soterrada onde todas as placas de rua foram arrancadas. Você sabe que há estrutura — ruas, praças, edifícios — mas nada tem nome. Cada endereço é apenas um número.

No meu caso, os endereços eram como `0x001d37c8`. Números hexadecimais de 32 bits que, isoladamente, não dizem nada. Mas quando você começa a ver padrões — "esta função lê deste offset, chama esta outra função, salva o resultado naquele campo" — os números começam a ganhar significado.

Ferramentas como Ghidra ajudam. Elas decompilam o assembly para um pseudocódigo C que é mais fácil de ler. Mas o Ghidra não é autoridade absoluta. Ele pode nomear funções erradas, inferir tipos errados, pular instruções que não reconhece. Aprendi isso da maneira mais difícil quando uma instrução `sll $3, $0, 2` (shift left logical) no Ghidra era na verdade `sll $3, $3, 2` — e essa diferença mudava completamente a interpretação de um dispatcher inteiro.

A regra que adotei: **confie nos bytes, não no Ghidra**. Byte-level verification primeiro. Instrução por instrução. Ghidra é uma lupa, não um oráculo.

---

## 4. O território MIPS

MIPS é uma arquitetura RISC (Reduced Instruction Set Computer). Cada instrução tem 32 bits. Não há instruções de tamanho variável como no x86. Isso torna o disassembly mais previsível, mas também mais limitado.

Algumas coisas que aprendi na prática:

- `jal` (jump and link) chama uma função por endereço direto. O endereço está codificado na instrução.
- `jr` (jump register) pula para o endereço em um registrador. É usado para retornos (`jr $ra`) e chamadas indiretas.
- `jalr` (jump and link register) chama uma função cujo endereço está em um registrador. Essencial para vtables e dispatch tables.
- Delay slots: a instrução depois de um `jal` ou `jr` executa **antes** do salto. O compilador do ICO usava `noreorder`, o que significa que os delay slots eram preenchidos manualmente pelo montador (ou pelo compilador). Isso é típico do GCC da Sony para o R5900.

O PS2 Emotion Engine (EE) é um processador MIPS64 com um conjunto estendido de instruções SIMD chamado MMI (Multimedia Instructions). Nem todo disassembler reconhece essas instruções. O Capstone, por exemplo, precisa estar em modo MIPS64 para decodificá-las corretamente. O objdump padrão do Linux falha completamente — ele nem reconhece a arquitetura.

Mas para as funções que eu estava analisando, o assembly era MIPS padrão. Nada de MMI, nada de VU0. Apenas loads, stores, branches e chamadas de função. Quase elegante demais para um jogo de PS2.

---

## 5. Ghidra como escavadeira e microscópio

Ghidra foi minha principal ferramenta de análise. Ele decompila o assembly para um C aproximado, mostra chamadas cruzadas (xrefs), e permite renomear funções e variáveis.

Mas o Ghidra tem limitações sérias para PS2:

- Ele não reconhece instruções MMI (embora com a extensão Emotion Engine Reloaded melhore)
- Ele pode inferir jump table targets errados se os dados não estiverem claros
- Funções sem callers estáticos são fáceis de perder — e o ICO está cheio delas

Uma das primeiras grandes descobertas veio do Ghidra: uma função em `0x001d37c8` que parecia ser um dispatcher. Ela:

1. Recebia um ponteiro de contexto em `a0`
2. Lia `[a0 + 0x15c]` para obter um ponteiro de entidade
3. Lia `[entity + 0x800]` para obter um state block
4. Lia `[state_block + 0x48]` para obter um ID de estado
5. Validava se o ID era menor que 5
6. Usava o ID para indexar uma jump table
7. Pulava para o handler correspondente

Parecia uma máquina de estado de entidade. Cinco estados, numerados de 0 a 4. Uma jump table em `0x00618fb0`. A tentação de nomear os estados como "Yorda", "captura", "menu de morte" foi imensa. Mas o AGENTS.md do projeto já alertava: **nomes especulativos são o maior inimigo da evidência**.

Eu mantive os nomes neutros: `state_0_block`, `state_1_block`, etc. Boa coisa.

---

## 6. A falsa pista do "Continue"

Uma das primeiras coisas que qualquer pessoa faz ao abrir um binário de jogo é procurar strings. Texto aparece facilmente no `.rodata`. E eu encontrei:

```
Continue
Yes
No
```

diretamente no ELF. Minha primeira reação foi: "é aqui que está o texto do menu de morte!"

Pulei para as referências cruzadas. Encontrei funções que carregavam essas strings. Coloquei breakpoints no PCSX2. Testei patches que modificavam as strings. Nada mudou na tela.

O "Continue" do ELF não era o texto do menu. Era uma string de debug: "ContinueAnimation:illegal Animation No." — algo completamente diferente.

O verdadeiro texto do menu de morte `Continue / Yes / No` estava em texturas TM2 — arquivos de imagem que continham o texto renderizado como pixels, não como caracteres ASCII. Eu estava procurando a inscrição na parede errada.

Essa foi a primeira lição metodológica importante: **encontrar uma string não significa entender seu uso**. Uma string pode ser debug, leftover, assertion, mensagem interna ou texto não usado no fluxo visual. Evidência real ainda pode levar a uma conclusão errada se o contexto não for compreendido.

---

## 7. Quando texto vira textura

Depois de semanas seguindo falsas pistas de strings ASCII, comecei a suspeitar que o texto visível do jogo — menus, legendas, instruções — não estava no ELF como texto. Estava em texturas.

O ELF continha referências a arquivos como `TEX2/yesno_p1.tm2`, `TEX/conti_p1.tm2`, `Tex_menu01/`. O formato TM2 é o formato de textura do PS2. Cada arquivo `.tm2` continha uma imagem que podia conter texto renderizado.

Isso muda completamente a abordagem. Em vez de procurar a palavra "Continue" no binário, eu precisava encontrar qual textura era carregada no momento em que o menu de morte aparecia. E isso exigia ou extrair e inspecionar todas as texturas TM2, ou usar breakpoints no emulador para capturar o fluxo de carregamento de texturas.

Eu tentei a primeira abordagem (trocar nomes de arquivos TM2 no ELF por outros nomes, para ver se a tela de morte mudava). Troquei `yesno_p*.tm2` por `cont2_p*.tm2`, e depois por `pause_p*.tm2`. Nada mudou. A tela de morte continuava mostrando `Continue / Yes / No`.

Isso significava que ou as texturas estavam em outro local (talvez dentro do `DATA.DF`), ou o carregamento das texturas não passava por aquelas strings de nome de arquivo. De qualquer forma, a abordagem de troca cega de strings tinha chegado ao limite.

Foi quando decidi mudar completamente de estratégia. Em vez de seguir strings, seguiria **estado**.

---

## 8. Entidades, estados e comportamento

Em uma engine de jogo, uma **entidade** é qualquer objeto que tem comportamento próprio: o jogador, inimigos, objetos interativos, câmeras. Cada entidade tipicamente tem um **update loop** — uma função chamada a cada frame que atualiza seu estado.

Uma **state machine** é uma forma comum de organizar esse update loop. A entidade tem um estado atual (parado, andando, pulando, morrendo) e uma tabela que mapeia cada estado para uma função de atualização. O dispatcher que encontrei em `0x001d37c8` parecia exatamente isso.

A cadeia era:

```
context pointer (a0)
  → [context + 0x15c] → entity pointer
    → [entity + 0x800] → state block pointer
      → [state_block + 0x48] → state ID (0-4)
        → validate < 5
        → ID * 4 → jump table at 0x00618fb0
        → jump to handler
```

Cinco estados. Uma jump table. Parecia clássico.

Mas **não era**. A correção veio meses depois, quando cruzei meus endereços com o projeto ICO-decomp.

---

## 9. Vtables, dispatch tables e chamadas indiretas

Em MIPS, chamadas indiretas são feitas com `jalr rt, rs` — jump and link register. O endereço alvo está em um registrador, não codificado na instrução. Isso significa que a função pode ser determinada em runtime.

Isso é comum em:

- **Vtables (virtual method tables)**: comuns em C++, mas ICO é escrito em C, então improvável
- **Dispatch tables**: arrays de ponteiros de função, indexados por um ID
- **Callbacks**: ponteiros de função registrados em uma estrutura e chamados quando um evento ocorre

O ICO usa extensivamente dispatch tables e callbacks. Uma das cadeias mais importantes que mapeei foi:

```
record +0x40 → 0x0013f7a8 (registration wrapper) → 0x0013f3f0 (storage) → node +0x1c → 0x0013fb70 (dispatcher)
```

Essa cadeia explicava como callbacks eram registrados e chamados. Mas ainda faltava uma peça: quem chamava `0x0013f7a8` com o argumento certo para registrar `0x001d3a30`?

Mapeei **cinco** callsites estáticos de `0x0013f7a8`:

1. `0x001b7ab0` — dentro de `0x001b76f8`, via `entry[+0x24]`
2. `0x001b7acc` — dentro de `0x001b76f8`, via `descriptor_label+0x40` (zero-guard: pulado para ROPE)
3. `0x00240e50` — dentro de `0x00240d40`, via `descriptor_label+0x40` (também zero para ROPE)
4. `0x00240f90` — dentro de `0x00240ea0`, via `t0` do caller (passa `0x001c3720`, não `0x001d3a30`)
5. `0x00201ed4` — dentro de `0x00201e70`, via `a1` do caller (runtime-dependente)

Dos cinco, três foram definitivamente excluídos. Dois permanecem candidatos, mas exigem observação em runtime para confirmação. Um deles, o callsite `0x001b7ab0` via `entry[+0x24]`, exigiria que uma entrada na tabela tivesse `+0x24 == 0x001d3a30` — algo que não vi nos dados estáticos, mas que poderia ser preenchido em runtime por dados de sala.

O gap de registro permanecia aberto.

---

## 10. A hipótese em torno de `0x001d37c8`

Por meses, tratei `0x001d37c8` como um **candidate state dispatcher** — uma máquina de estado genérica para entidades de gameplay. A estrutura era elegante demais para ser outra coisa:

```
entity_ptr
→ [entity + 0x800]
→ candidate_state_block_ptr
→ [candidate_state_block_ptr + 0x48]
→ candidate_state_id (0-4)
→ candidate_state_jump_table (0x00618fb0)
→ handler
```

Os nomes que eu usava eram cautelosamente neutros — `state_0_block`, `state_1_block`, etc. — mas no fundo eu achava que estava lidando com estados de gameplay. Talvez transições da Yorda. Talvez o sistema de captura. Talvez o menu de morte.

**Eu estava errado.**

A correção veio quando cruzei meus endereços com o projeto **ICO-decomp** — um esforço público de decompilação matching do ICO versão PAL. Eles têm 5.792 símbolos nomeados, organizados por arquivo fonte via splat YAML.

O dispatcher `0x001d37c8` e o callback `0x001d3a30` caíram dentro de `sugipon/src/clothAnimation.c` — um arquivo de **simulação de física de pano** (cloth).

Não era gameplay. Não era entidade. Não era Yorda. Era a física do vestido da menina, dos panos pendurados, das bandeiras e cortinas do castelo.

O range `0x001d3000-0x001d4500` continha exclusivamente funções com nomes como:

- `InitCloth4D`
- `GetCloth4D`
- `getCloth4D_preProcess`
- `getCloth4D_postProcess`
- `GetChainNodeGlobalQuaternion`
- `ResetClothAnimation`
- `getCloth4D_PlaneClip`

Cinco estados de simulação de pano (init, wind/forces, collision, constraint solve, post-process) — não cinco estados de gameplay.

Essa foi a correção de domínio mais importante de todo o projeto.

---

## 11. A tabela `0x00618fb0` e a correção do endereço

A jump table do dispatcher passou por uma correção importante.

Inicialmente, eu a localizei em `0x00628fb0`. Mas quando verifiquei os bytes, o endereço real era `0x00618fb0`. Um dígito hex errado — `6` em vez de `5` — que mudava completamente a região de dados referenciada.

A diferença entre:

```asm
sll $3, $0, 2   ; shift $0 (always zero) — não faz sentido
```

e:

```asm
sll $3, $3, 2   ; shift state_id * 4 — faz todo o sentido
```

também foi uma correção importante. A primeira versão, extraída de uma leitura descuidada do Ghidra, sugeria que o índice da jump table era zero — ou seja, sempre o estado 0. A segunda, confirmada por byte-level verification, mostrava que o state_id era de fato usado para indexar a tabela.

Essa correção salvou semanas de análise baseada em um modelo quebrado.

A tabela `0x00618fb0` contém cinco entradas:

```
state 0 → 0x001d3818
state 1 → 0x001d3844
state 2 → 0x001d391c
state 3 → 0x001d39e0
state 4 → 0x001d3a10
```

Cada entrada é um endereço dentro do próprio dispatcher — são basic blocks internos, não funções separadas. Isso significa que os cinco estados compartilham o mesmo prólogo e epílogo, e a transição entre eles é controlada pelo próprio código, não por uma máquina de estados externa. Consistente com um loop de simulação que executa múltiplas fases em sequência.

---

## 12. O perigo das conclusões rápidas

Este projeto me ensinou a diferença entre:

- **Evidência direta**: bytes no binário, instruções verificadas, breakpoints que param no endereço certo
- **Evidência indireta**: uma função é chamada por outra que parece ser de gameplay → então a primeira também deve ser de gameplay (falso!)
- **Inferência plausível**: o dispatcher tem 5 estados e uma jump table → deve ser uma máquina de estado de entidade (também falso!)
- **Hipótese**: o callback `0x001d3a30` é registrado via `entry[+0x24]` com dados de sala (não confirmado, mas possível)
- **Ruído**: strings que aparecem no binário mas não são usadas no fluxo que você está investigando
- **Falso positivo**: "Continue" parece o texto do menu de morte, mas é uma string de debug

Cada uma dessas categorias é um degrau em uma hierarquia de evidência. Subir um degrau sem validação é o erro mais comum em engenharia reversa — e o mais fácil de cometer quando se usa agentes de IA que produzem conclusões rápidas e confiantes.

A regra que adotei: **nunca atualize uma hipótese para conclusão sem evidência direta**. A cadeia de evidência precisa ser auditável: alguém deve poder ler minhas anotações e entender como cheguei a cada conclusão.

---

## 13. DATA.DF, overlays e assets

Paralelamente à análise do ELF, eu sabia que o verdadeiro conteúdo do jogo — modelos, texturas, animações, cenas — estava em `DATA.DF`, um arquivo de mais de 500 MB no disco.

O ELF continha referências a arquivos dentro do `DATA.DF`: `.gcm` (câmera/cutscene), `.p2o` (modelos), `.mob` (animações), `.bga` (background?), `.tm2` (texturas). Mais de 5000 referências no total.

Mas `DATA.DF` se mostrou um osso duro de roer:

- O início do arquivo tem entropia altíssima (7.99 bits/byte) — nada de tabela simples legível no começo
- O meio tem entropia variável — possivelmente dados comprimidos ou empacotados
- O fim é majoritariamente padding (zeros)

Tentei buscas direcionadas usando tokens numéricos das seções de overlay do ELF (`.DVP.overlay...`). Nada. O formato interno do archive não é trivial.

A abordagem mais produtiva foi: **não tentar resolver `DATA.DF` agora**. Primeiro entenda o código que o acessa. Depois use esse conhecimento para navegar pelo archive. O executável tem funções que abrem arquivos dentro do `DATA.DF` — identificar e entender essas funções é a chave.

---

## 14. Arqueologia digital como método

Com o tempo, comecei a ver o projeto não como engenharia reversa, mas como arqueologia digital.

Cada offset é um fragmento de um artefato maior.

Cada função é uma sala soterrada — você sabe que há algo ali, mas precisa escavar com cuidado para não destruir o contexto.

Cada string é uma inscrição — pode ser uma placa informativa ou uma marca de construtor sem importância.

Cada tabela é uma planta parcial de um edifício maior.

Cada falso positivo é uma pedra comum que você confunde com um artefato — e isso faz parte do processo.

A arqueologia digital, como a arqueologia real, exige:

- **Método**: uma abordagem consistente para registrar descobertas
- **Paciência**: resultados não vêm rápido, e conclusões precipitadas atrapalham mais do que ajudam
- **Registro**: cada descoberta precisa ser documentada com o nível de confiança adequado
- **Cautela**: não confunda "encontrei um padrão" com "entendi o sistema"

O projeto adotou um sistema de revisões numeradas (Rev.001 a Rev.039 até agora), cada uma documentando um passo específico da investigação. Cada revisão separa o que é confirmado, o que é provável, o que é possível, o que é desconhecido e o que é descartado.

Isso não é burocracia. É a única maneira de manter a sanidade quando você está lidando com centenas de funções anônimas e milhares de endereços.

---

## 15. Da hipótese de gameplay à física de pano

A correção mais importante de todo o projeto veio quando menos esperava.

Por meses, eu e os agentes de IA que me ajudavam tratamos `0x001d37c8` como uma máquina de estado de entidade. Era uma hipótese confortável: fazia sentido, se encaixava nos padrões, e era narrativamente satisfatória ("estamos mapeando o sistema de estados do ICO!").

Mas a evidência direta nunca apareceu. Nenhuma string de gameplay perto do dispatcher. Nenhum caller de função de gameplay. Nenhum dado de fase ou sala.

Quando finalmente cruzei os endereços com o ICO-decomp, a verdade apareceu: o dispatcher estava em `clothAnimation.c`. Física de pano. Simulação de vértices. Coisas que se mexem com o vento.

A correção foi registrada na Rev.039. A nomenclatura mudou:

| Nome antigo | Nome novo |
|---|---|
| candidate_state_dispatcher | cloth_dispatcher_dispatch |
| state_0_block | cloth_state_0_init_like |
| state_1_block | cloth_state_1_wind_like |
| state_2_block | cloth_state_2_collision_like |
| state_3_block | cloth_state_3_constraint_like |
| state_4_block | cloth_state_4_postprocess_like |
| rope_record_callback | cloth_update_callback |

Essa correção não foi um fracasso. Foi a validação do método. Se tivéssemos nomeado os estados como "Yorda", "captura" ou "menu" logo no início, a correção teria sido muito mais difícil — porque nomes errados criam inércia cognitiva. Mas como mantivemos nomes neutros (`state_0_block`), a correção foi apenas uma atualização de entendimento, não uma vergonha a ser escondida.

---

## 16. A cadeia de callback registration

Uma das descobertas mais importantes foi a cadeia de como callbacks são registrados e chamados.

Ela começa com funções como `0x0013f7a8` — um wrapper minúsculo de 10 instruções que preserva argumentos e delega:

```asm
0013f7a8: move   $v0, $a3          ; preserve original a3
0013f7ac: addiu  $sp, $sp, -0x10   ; prologue
0013f7b0: andi   $a3, $a2, 0xff    ; mask a2 to byte
0013f7b4: move   $t1, $t0          ; preserve caller t0
0013f7b8: move   $a2, $a1          ; a2 = callback pointer
0013f7bc: sd     $ra, ($sp)        ; save return address
0013f7c0: move   $t0, $v0          ; t0 = original a3
0013f7c4: jal    0x13f3f0          ; delegate to storage
0013f7c8: move   $a1, $a0          ; a1 = original a0 (handle)
0013f7cc: ld     $ra, ($sp)        ; restore ra
```

Essa função, por sua vez, chama `0x0013f3f0`, que armazena o callback em `[node + 0x1c]`. E `0x0013fb70` mais tarde carrega esse valor e o chama por `jalr`.

A cadeia completa é:

```
record +0x40 (callback pointer, ex: 0x001d3a30)
  → 0x0013f7a8 (registration wrapper)
    → 0x0013f3f0 (storage function)
      → node +0x1c (callback armazenado)
        → 0x0013fb70 (dispatcher que carrega e chama node+0x1c)
```

Para o caso do descritor "ROPE", o slot `+0x40` contém `0x001d3a30`. Seguindo a cadeia, esse callback deveria ser registrado em `node + 0x1c` e chamado por `0x0013fb70`.

Mas como `0x001d3a30` chega ao `0x0013f7a8`? Quem chama o registration wrapper com o argumento certo?

Essa é a pergunta que ainda não foi respondida.

---

## 17. Ainda não resolvido: o gap ROPE

O "ROPE" do projeto não tem relação com as cordas que Ico escala no jogo. O ICO-decomp tem 79 símbolos com "Rope" no nome — `actCommonRope`, `GetRopeHangablePos`, `ropeGeo`, `HoldRope`, `ReleaseRope` etc. — todos referentes a **gameplay ropes**: cordas que o jogador pode agarrar, balançar e subir.

Nosso "ROPE" é outra coisa. É um label no `.data` do ELF, em `0x002a3974`, que faz parte de uma tabela de descritores. Cada descritor tem 0x64 bytes e contém ponteiros para funções em slots como `+0x38`, `+0x40` e `+0x48`.

O label "ROPE" pode ser uma abreviação interna — talvez "Register Object Physics Entity" ou "Runtime Object Processing Element" — ou pode ser simplesmente o nome de um tipo de objeto que, por coincidência, compartilha o nome com as cordas de gameplay. A coincidência é infeliz, mas não deve ser tratada como evidência.

O gap atual é:

1. **Sabemos** que `0x001d3a30` (o callback de atualização de cloth) está em `ROPE +0x40`
2. **Sabemos** que a cadeia de registro leva de `record +0x40` → `0x0013f7a8` → `0x0013f3f0` → `node +0x1c` → `0x0013fb70`
3. **Não sabemos** qual dos cinco callsites de `0x0013f7a8` realmente passa `0x001d3a30` como argumento em runtime

Três callsites foram definitivamente excluídos (por zero-guard ou argumento errado). Dois permanecem candidatos:

- `entry[+0x24]` em `0x001b76f8` — requer entrada com `+0x46 == 0x14` e `+0x24 == 0x001d3a30`
- `a1` do caller em `0x00201e70` — dependente de runtime

Ambos exigem observação em runtime no PCSX2 para confirmação.

---

## 18. O compilador e as ferramentas

Uma das primeiras perguntas que surgiram foi: **qual compilador foi usado para buildar o ICO?**

A resposta veio de múltiplas fontes:

1. O ELF tem seção `.reginfo`, específica do GNU Assembler (GAS)
2. Os caminhos de fonte são no estilo Unix (`src/item.c`)
3. O código usa instruções de 64 bits (`ld`, `sd`, `daddu`) — típico do R5900 em modo MIPS64
4. O alinhamento e a convenção de chamada são EABI (Embedded ABI), não n64 ABI

O compilador é o **EE GCC 2.9-991111-01** — o fork da Sony do GCC 2.9 para o Emotion Engine. As flags prováveis são:

```
-march=r5900 -mips3 -mgp64 -mabi=eabi -msingle-float -G0 -O2
```

Algumas notas sobre essas flags:

- `-mips3` seleciona o ISA MIPS III como base (o R5900 é MIPS III com extensões)
- `-mabi=eabi` é essencial — define a calling convention de 8 registradores para parâmetros
- `-msingle-float` é obrigatório porque o FPU do R5900 só suporta precisão simples
- `-G0` desabilita o GP-relative optimization (comum em builds de produção PS2)
- `-O2` é o padrão para builds de produção

Essas flags foram validadas contra outros projetos de decompilação PS2 (Klonoa 2, PaRappa 2, Sly Cooper) e contra os patches do GCC para R5900.

---

## 19. A estrutura do código fonte original

Graças ao projeto ICO-decomp, pude vislumbrar a estrutura de diretórios original do jogo:

```
ico2/
├── fumi/       → core engine, IOS, threading, som, atores base
├── omori/      → câmera, AI, colisão de ataque
├── sugipon/    → física, animação, gameplay objects, UI
├── ito/        → chefes, MPEG/cutscene, raio
├── seki/       → rendering, texturas, shadow
├── common/     → compartilhado entre módulos
├── script/     → sistema de script por fase
└── sdk/        → Sony PS2 SDK
```

Cada diretório corresponde a um programador ou dupla:

- **Fumi**: core engine, IOS (sistema operacional do PS2), threading, som, atores principais
- **Omori**: câmera, AI dos inimigos, sistemas de batalha
- **Sugipon**: física (incluindo cloth), animação, objetos de gameplay, UI
- **Ito**: chefes, cutscenes MPEG, efeitos especiais
- **Seki**: rendering, texturas, shadow maps, display lists

O dispatcher de cloth está em `sugipon/src/clothAnimation.c`. A cadeia de callback registration está em `fumi/ios/thread.c` e `fumi/ios/shockdriver.c`. O gap de registro é, essencialmente, uma questão de como o sistema de cloth (sugipon) se conecta ao sistema de IOS threading (fumi).

---

## 20. O que vem depois

O projeto está agora em uma encruzilhada metodológica.

A fase de análise estática (Rev.001-037) está completa. O dispatcher está confirmado e corrigido para cloth physics. A cadeia de callback registration está mapeada. O compilador está identificado. O source tree original está visível.

O que falta:

1. **Resolver o gap ROPE**: qual dos callsites registra `0x001d3a30` em runtime? Isso exige breakpoint no PCSX2.
2. **Submeter scratches ao decomp.me**: as 6 funções mapeadas (dispatcher, ROPE callback, registration chain) estão prontas para crowd-sourced matching.
3. **Explorar o sistema de cloth**: com a correção de domínio, agora faz sentido investigar `clothAnimation.c` mais a fundo — como a simulação funciona, como os 5 estados se relacionam, como o callback de update é acionado.

Mas acima de tudo, o projeto precisa continuar respeitando sua própria regra fundamental:

> **A narrativa pode ser poética; as conclusões técnicas não.**

Cada descoberta precisa ser registrada com seu nível de confiança. Cada hipótese precisa ser testável. Cada conclusão precisa ter uma cadeia de evidência auditável.

Não é o caminho mais rápido. Mas é o único que produz conhecimento confiável.

---

### Epílogo — Não reconstruir apenas código, mas intenção

No final, o que me move não é apenas a curiosidade técnica de saber como ICO funciona por dentro. É o desejo de entender **por que** certas decisões foram tomadas.

Por que o vestido da Yorda é simulado com 5 estados? Por que o callback de atualização de cloth é registrado através de um sistema tão indireto? Por que o label "ROPE" aparece nos dados se não tem relação com as cordas que o jogador escala?

Cada pergunta dessas é um fragmento da intenção original dos desenvolvedores. Reconstruir o código é, em última análise, reconstruir o pensamento de quem o escreveu. É um tipo de arqueologia que não desenterra ossos ou cerâmicas, mas decisões, prioridades e compromissos técnicos.

ICO é um jogo sobre silêncio, solidão e conexão. De certa forma, a engenharia reversa dele é a mesma coisa: um trabalho solitário de escavação silenciosa, na esperança de se conectar com a mente de quem construiu aquela ruína digital.

E mesmo que eu nunca encontre todas as respostas, o processo de procurar já vale a jornada.

---

*Este texto foi escrito em 2026-05-14 como parte do projeto `ico-reconstruction`. As revisões técnicas citadas estão em `research/elf/` e `research/`. O projeto completo está em github.com/odevpedro/ico-reconstruction.*

---

### Nota técnica — Caveats da Rev.040

A narrativa acima foi escrita antes da Rev.040. Para manter a precisão técnica ao usar este texto como referência, os seguintes caveats devem ser observados:

1. **clothAnimation.c no ICO-decomp é ASM-only, não C.** O arquivo existe como mapeamento splat, mas nenhuma função foi decompilada para C. Todos os nomes de símbolos (InitCloth4D, GetCloth4D, etc.) são inferidos de símbolos de linking, não de código fonte validado.
2. **`0x0013f7a8` não é `iosThreadStart`.** O C decompilado de `iosThreadStart` no ICO-decomp não corresponde ao wrapper de 10 instruções em `0x0013f7a8`. Trate como funções distintas.
3. **Os nomes dos state blocks (wind, collision, constraint, post-process) são provisórios.** Não há evidência direta que mapeie cada state ID para uma fase específica da simulação de cloth. São hipóteses baseadas em contexto.
4. **O gap de registro de `0x001d3a30` em `node + 0x1c` continua aberto.** Nenhuma das 5 revisões até Rev.040 resolveu quem chama `0x0013f7a8` com `a1 = 0x001d3a30` em runtime.
5. **Termos antigos como "entity state", "Yorda state", "capture state" foram descartados** a partir da Rev.039. Qualquer ocorrência deles em notas anteriores (Rev.001-037) é registro histórico de hipótese descartada, não conclusão vigente.
6. **Rev.041**: `[state_block + 0x04]` não é boolean simples — indexa uma tabela cloth-domain em `0x004d4188` (8 entries, stride 0x14). O campo é um variant/mode selector.
7. **Rev.042**: writer confirmado do variant field é `0x001d2858` (copia `[initializer_arg+0x30]` para `[payload+0x04]`). `0x001d390c` foi descartado como writer (escreve +0x44, não +0x04).
