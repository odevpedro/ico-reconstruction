# Prompt — Blog post narrativo com persona para o projeto ICO Reconstruction

Ainda referente ao projeto `ico-reconstruction` / `ICO decompiler`.

---

## Restrição importante

Não execute comandos.

Não edite arquivos.

Não rode scripts.

Não faça análise automatizada.

Não altere o repositório.

Não crie arquivos novos no projeto, a menos que eu peça explicitamente.

Não invente resultados técnicos que não estejam no material fornecido.

Sua tarefa é apenas **escrever um texto narrativo longo**, em formato de blog post/livro técnico, usando como base conceitual o conteúdo do arquivo `key-concepts.md` e o contexto já existente do projeto.

---

# Objetivo

Quero que você escreva um texto extremamente detalhado, longo e imersivo, como se fosse um **blog post técnico-literário**, quase com a extensão e profundidade de um pequeno livro.

O tema central é a jornada de uma pessoa tentando compreender, decompilar e reconstruir partes do jogo **ICO**, de PlayStation 2, dentro do projeto `ico-reconstruction`.

O texto deve misturar:

- relato técnico;
- arqueologia digital;
- engenharia reversa;
- fascínio por jogos da Team Ico / genDESIGN;
- estudo de ciência da computação;
- investigação de binários antigos;
- dúvidas, hipóteses e validações;
- descoberta gradual dos conceitos;
- sentimento de estar escavando uma obra digital histórica.

A intenção não é produzir uma documentação seca, nem um tutorial simples. A intenção é criar um texto que pareça uma mistura de diário de pesquisa, ensaio técnico, crônica de arqueologia digital e capítulo de livro sobre engenharia reversa.

---

# Persona narrativa

Crie uma persona fictícia com as seguintes características:

- é profundamente apaixonada por jogos da **Team Ico / genDESIGN**;
- considera `ICO`, `Shadow of the Colossus` e `The Last Guardian` obras quase arqueológicas, poéticas e técnicas;
- é entusiasta de **arqueologia digital**;
- é estudante de **Ciência da Computação**;
- está aprendendo engenharia reversa na prática;
- usa ferramentas como Ghidra, PCSX2, análise de ELF, MIPS, ISO de PS2, scripts e documentação;
- tem uma postura curiosa, obsessiva, respeitosa e quase contemplativa diante do jogo;
- não se apresenta como “hacker mágico”, mas como alguém que está aprendendo com método, cautela e fascínio;
- entende que engenharia reversa não é apertar um botão para recuperar código-fonte, mas reconstruir evidências aos poucos.

O texto deve ser escrito **na perspectiva dessa persona**, em primeira pessoa, como se ela estivesse relatando a própria jornada.

Exemplo de tom desejado:

> “Quando abri o ELF pela primeira vez, tive a sensação estranha de estar diante de uma ruína. Não havia nomes de funções, não havia comentários, não havia mapas. Apenas endereços, registradores e pedaços de intenção fossilizados em MIPS.”

A persona pode ter voz própria, sensibilidade e pequenas cenas narrativas, mas não deve transformar ficção em fato técnico.

---

# Base obrigatória

Considere cuidadosamente o conteúdo de:

```txt
key-concepts.md
```

Use esse arquivo como base para explicar os conceitos centrais do projeto.

Além disso, antes de escrever qualquer texto narrativo novo, consulte as revisões validadas mais recentes do projeto, especialmente:

```txt
research/elf/ghidra-rev023-dispatcher-table-resolution.md
research/elf/ghidra-rev024-internal-state-block-semantics.md
research/elf/ghidra-rev025-runtime-confirmed-caller-context.md
research/elf/ghidra-rev026-rope-record-table-context.md
research/elf/ghidra-rev027-rope-state-block-initializer.md
research/elf/ghidra-rev028-state-block-provider-contract.md
research/elf/ghidra-rev029-state-block-provider-deeper-static.md
research/elf/ghidra-rev030-provider-caller-survey.md
research/elf/ghidra-rev031-record-callback-dispatchers.md
research/elf/ghidra-rev032-static-callback-follow-through.md
research/elf/ghidra-rev033-node-callback-dispatch-chain.md
research/elf/ghidra-rev034-callback-signature-and-record-selection.md
```

Use essas revisões para manter a narrativa alinhada com o estado atual da investigação:

- a jump table real do dispatcher confirmado é `0x00618fb0`, não `0x00628fb0`;
- o dispatcher confirmado fica em `0x001d37c8`;
- os cinco alvos internos são basic blocks dentro do dispatcher, não entry points independentes;
- `0x001d3a30` foi confirmado em runtime como caller que alcança `0x001d37c8`;
- a sessão PCSX2 de 2026-05-13 observou o caminho `0x001d3a30 -> 0x001d37c8 -> 0x001d37f0 -> 0x001d380c`;
- em `0x001d380c`, foi observado `$v1 = 0x00618fc0`, correspondente à entrada de tabela do `candidate_state_id = 4`;
- o alvo carregado em `$a0` após `lw a0,0($v1)` ainda não foi capturado com confiança, então não apresente isso como fato runtime direto;
- `0x001d3a30` aparece como referência direta em `.data` em `0x002a3974`, sugerindo chamada indireta/tabela;
- a palavra ASCII `ROPE` aparece próxima a essa região de dados, mas deve ser tratada como rótulo/metadado fraco, não como prova semântica de gameplay.
- Rev.026 vinculou `0x001d3a30` a um record `.data` rotulado `ROPE`, com provável início em `0x002a3934`;
- esse record tem stride aparente de `0x64` em relação a records vizinhos como `BARREL`, `CHAIN`, `FLEVER` e `FLEVER_TRISTATE`;
- no record `ROPE`, os slots `+0x38`, `+0x40` e `+0x48` apontam para `0x001d3b28`, `0x001d3a30` e `0x001d27a8`;
- `0x001d27a8` escreve o ponteiro usado em `[entity + 0x800]`, enquanto `0x001d3a30` pode chamar o dispatcher `0x001d37c8`;
- mesmo com o label `ROPE`, não nomeie os estados internos do dispatcher como estados de gameplay sem evidência direta adicional.
- Rev.027 confirmou que `0x001d27a8` copia um template de `0x90` bytes de `0x004c46b0`, instala o ponteiro retornado em `[entity + 0x800]`, e esse template inicializa `[state_block + 0x48]` com `0`;
- o offset `[state_block + 0x48]` é o `candidate_state_id` usado pelo dispatcher de Rev.024, então o state block do record `ROPE` começa estruturalmente em state id `0`;
- o template também contém `0x0000012c` em `[state_block + 0x44]`, campo relacionado ao contador/decremento observado em `state_1_block`;
- não generalize automaticamente o papel do slot `+0x48` para todos os records vizinhos; em Rev.027, essa conclusão é confirmada apenas para `ROPE`.
- Rev.028 corrigiu a linguagem sobre `0x0013a0f8`: ela não deve ser chamada definitivamente de alocador;
- o comportamento confirmado de `0x0013a0f8` é chamar `0x00138e30`, retornar o ponteiro se não for zero, e entrar em caminho diagnóstico/assert-like se o retorno for zero;
- portanto, use termos cautelosos como “provedor/resolvedor de ponteiro de state block” até `0x00138e30` ser analisada.
- Rev.029 aprofundou `0x00138e30`: para o caso `ROPE`, `0x001d27a8` pede `0x90` bytes, passa o metadado curto `src/item.c` e `0x1b2`, e o provider retorna um payload pointer após overhead/header interno de `0x40`;
- trate `0x00138e30` como allocator/pool-like, mas ainda não como nome definitivo de subsistema;
- o ponteiro gravado em `[entity + 0x800]` é o payload retornado, não o header interno do provider.
- Rev.030 mostrou que `0x0013a0f8` tem 247 callsites estáticos e é um provider allocator-like amplo, não específico de `ROPE`;
- o caso `ROPE +0x48` continua bem delimitado: `0x001d27e8` passa `a1 = 0x90`, `a2 = "src/item.c"` e `a3 = 0x1b2`;
- `src/item.c` não apareceu em outros callsites inferidos na varredura local de Rev.030;
- outros records vizinhos chamam o provider por callbacks `+0x48`, mas com metadados/tamanhos diferentes, como `src/switch.c` e `src/rotObject.c`.
- Rev.031 confirmou estaticamente dispatchers de callbacks de records: `0x0013fc00` chama callbacks em `+0x48`, e `0x0013d140` chama callbacks em `+0x38`;
- em `0x0013fc00`, o callback `+0x48` é chamado com o próprio record/objeto em `a0`, o que fornece um mecanismo estático plausível para invocar `ROPE +0x48 = 0x001d27a8`;
- o slot `+0x40` do `ROPE`, onde fica `0x001d3a30`, continua sem dispatcher estático isolado; trate isso como lacuna aberta, não como ausência de chamada;
- não nomeie definitivamente os slots `+0x38`, `+0x40` e `+0x48` como fases de lifecycle até haver evidência mais direta.
- Rev.032 aprofundou `0x0013fc00`: ele percorre uma estrutura/lista com head em `gp - 0x671c`, buckets em torno de `0x00281ab0`, encadeamento por `+0x34`, filtro por `+0x16c` e máscara `+0x50`, e chama `+0x48` dos candidatos aceitos;
- Rev.032 rastreou `0x0013f7a8` e mostrou que ele não chama diretamente o valor recebido em `a1`; ele repassa esse valor para `0x0013f3f0`, onde pode ser armazenado em `node + 0x1c`;
- a busca por `0x001d3a30` continua encontrando apenas a referência direta em `ROPE +0x40` (`0x002a3974`) e nenhum `jal` direto;
- a comparação de `+0x40` em `BARREL`, `ROPE`, `CHAIN`, `FLEVER` e `FLEVER_TRISTATE` sugere um slot comportamental/update-like, porque esses callbacks acessam `[a0 + 0x15c]` e frequentemente `[entity + 0x800]`, mas esse nome ainda não está provado;
- o próximo avanço sem gameplay deve rastrear usos de `node + 0x1c`, especialmente `jalr` derivados desse offset, e mapear quem escreve `gp - 0x671c` e `0x00281ab0`.
- Rev.033 confirmou um dispatcher estático para valores armazenados em `node + 0x1c`: `0x0013fb64` carrega `node + 0x1c` e `0x0013fb70` chama esse valor por `jalr` com `a0 = s2`;
- Rev.033 conectou a cadeia `record +0x40 -> 0x0013f7a8 -> 0x0013f3f0 -> node +0x1c -> 0x0013fb70`, tornando esse o mecanismo estático provável para callbacks `+0x40`;
- para `ROPE`, como `ROPE +0x40 = 0x001d3a30`, o melhor modelo estático atual é que `0x001d3a30` seja registrado em `node +0x1c` e depois chamado por `0x0013fb70`;
- essa cadeia ainda não prova que o runtime observado usou especificamente `s7 == ROPE`; trate como mecanismo estático provável, não como confirmação runtime específica;
- Rev.033 diferenciou dois buckets/fases: `0x00281a70` alimenta o loop que chama `node +0x1c`, enquanto `0x00281ab0` alimenta o loop `0x0013fc00` que chama callbacks `+0x48`;
- `gp - 0x671c` foi mapeado como head de lista global ordenada por `+0x44`, usada por `0x0013fc00` para o dispatcher `+0x48`.
- Rev.034 confirmou que `0x001d3a30` é compatível com a chamada por `0x0013fb70`, porque espera `a0` como objeto/contexto e acessa `[a0 +0x15c] -> +0x800`;
- os callbacks `+0x40` vizinhos (`BARREL`, `ROPE`, `CHAIN`, `FLEVER`, `FLEVER_TRISTATE`) seguem a mesma convenção de argumento `a0 = object/context`;
- `0x001b76f8` tem três callers diretos em `0x001b7d90`, `0x001b7df8` e `0x001b7e6c`;
- Rev.034 mostrou que `s7` é selecionado por `0x002a31b8 + ([s4+0x46] * 0x64)`;
- o record `ROPE` em `0x002a3934` corresponde ao índice `0x13` a partir da base `0x002a31b8`, então `[s4+0x46] == 0x13` selecionaria `ROPE`;
- ainda não trate o runtime observado como prova de `[s4+0x46] == 0x13`; isso permanece como o próximo valor útil a capturar se o debugger for retomado.

Quando uma revisão nova contradisser uma antiga, prefira a revisão validada mais recente e explique a correção como parte da jornada de pesquisa.

Cada conceito importante presente no arquivo deve ser explicado com:

1. definição;
2. por que ele importa no projeto;
3. como a persona descobriu ou estudou aquilo;
4. como o conceito se conecta ao processo de reconstrução do ICO;
5. quais cuidados ou riscos de interpretação existem.

Se algum conceito do arquivo `key-concepts.md` ainda for hipótese, trate como hipótese.

Se algum conceito estiver marcado como provisório, especulativo ou aguardando validação, mantenha essa cautela no texto.

---

# Regras sobre ficção e precisão técnica

Você pode criar cenários fictícios para dar força narrativa ao texto.

Exemplos de cenários permitidos:

- a persona pesquisando conceitos de MIPS de madrugada;
- a persona comparando o ELF stripped a uma escavação arqueológica;
- a persona abrindo o Ghidra e encontrando funções sem nome;
- a persona percebendo que uma string `"Continue"` não era o menu real;
- a persona descobrindo que o jogo pode usar texturas `.tm2` para textos visuais;
- a persona anotando hipóteses em arquivos Markdown;
- a persona tentando diferenciar evidência real de especulação;
- a persona estudando dispatch tables, jump tables, vtables e estados;
- a persona percebendo que engenharia reversa exige paciência e humildade.

Mas atenção:

**Os cenários fictícios devem servir para explicar a jornada e os conceitos. Eles não devem inventar resultados técnicos falsos.**

Nunca apresente uma cena fictícia como se fosse evidência técnica real.

Sempre diferencie:

- fato observado;
- hipótese técnica;
- interpretação narrativa;
- metáfora;
- aprendizado pessoal da persona.

---

# Tom narrativo

O tom deve ser:

- técnico;
- íntimo;
- investigativo;
- contemplativo;
- obsessivo no bom sentido;
- respeitoso com a obra original;
- levemente poético, mas não exageradamente místico;
- maduro;
- detalhista;
- com sensação de descoberta progressiva.

Evite:

- tom de tutorial raso;
- tom corporativo;
- documentação seca;
- linguagem excessivamente acadêmica;
- exagero dramático artificial;
- tratar engenharia reversa como algo fácil;
- afirmar conclusões não validadas;
- transformar hipóteses em verdades.

---

# Estrutura sugerida

Organize o texto em capítulos ou grandes seções.

Use uma estrutura parecida com esta:

```md
# Título forte e literário

## Subtítulo opcional

## Prólogo — A ruína digital chamada ICO

Apresente a persona, a relação dela com os jogos da Team Ico/genDESIGN e a motivação para investigar ICO.

Mostre que o projeto nasce de uma mistura de fascínio artístico, curiosidade técnica e desejo de compreender como uma obra digital foi construída.

---

## 1. O primeiro contato com o binário

Explique:

- ISO de PS2;
- executável principal;
- arquivos do jogo;
- ELF;
- arquitetura do PlayStation 2;
- por que abrir um jogo antigo pode parecer uma escavação arqueológica.

A persona deve narrar a sensação de olhar para arquivos que, à primeira vista, parecem apenas nomes, offsets e blocos de dados, mas que escondem a estrutura viva de um jogo.

---

## 2. O que significa decompilar um jogo

Explique a diferença entre:

- emular;
- modificar;
- descompilar;
- reconstruir;
- fazer source port;
- fazer matching;
- documentar comportamento.

Deixe claro que o projeto não é “apertar um botão e recuperar o código-fonte”.

Mostre que decompilar/reconstruir é um processo gradual de aproximação: identificar dados, mapear funções, entender comportamento, validar hipóteses e só depois reconstruir algo com segurança.

---

## 3. O ELF stripped e o silêncio dos nomes perdidos

Explique:

- o que é um ELF;
- o que significa o ELF estar stripped;
- por que a ausência de símbolos dificulta tudo;
- por que funções aparecem sem nomes;
- como isso transforma o trabalho em investigação.

Use a metáfora da ruína sem placas, sem inscrições claras, onde cada sala precisa ser identificada pela função que exercia.

---

## 4. O território MIPS

Explique:

- MIPS;
- registradores;
- instruções;
- `jal`;
- `jr`;
- `jalr`;
- stack;
- delay slots;
- chamadas diretas e indiretas;
- por que isso importa no PS2.

A persona deve narrar o aprendizado como se estivesse aprendendo uma língua antiga: cada instrução é pequena, mas a combinação delas revela comportamento.

---

## 5. Ghidra como escavadeira e microscópio

Explique o papel do Ghidra:

- disassembly;
- decompilation;
- xrefs;
- funções sem nome;
- criação de data types;
- renomeação cuidadosa;
- riscos da análise automática;
- diferença entre o que o Ghidra sugere e o que está confirmado.

Mostre que o Ghidra não “resolve” o jogo sozinho. Ele oferece ferramentas para formular perguntas melhores.

---

## 6. A falsa pista do “Continue”

Narre a descoberta de que encontrar uma string `"Continue"` não significa entender o menu.

Explique que uma string ASCII pode ser:

- texto de debug;
- assertion;
- leftover;
- mensagem interna;
- dado não usado no fluxo visual;
- parte de outro sistema.

Explique por que isso é importante metodologicamente: uma evidência real ainda pode levar a uma conclusão errada.

---

## 7. Quando texto vira textura

Explique a hipótese de que o menu `Continue / Yes / No` pode estar em texturas `.tm2`.

Aborde:

- o que são texturas;
- por que jogos antigos frequentemente renderizavam texto como imagem;
- o que é `.tm2`;
- por que procurar ASCII pode falhar;
- como texturas podem conter palavras visualmente, mas não como string pesquisável no binário.

Mostre a mudança de mentalidade: em vez de procurar a palavra, procurar o sistema que desenha a imagem.

---

## 8. Entidades, estados e comportamento

Explique:

- o que é uma entidade em uma engine de jogo;
- o que pode ser um update loop;
- o que é uma state machine;
- o que é um campo de estado;
- por que jogos usam estados para controlar comportamento;
- por que encontrar um dispatcher de estado seria uma descoberta importante.

Use a ideia de que um jogo não é apenas arte na tela, mas um conjunto de objetos em transição.

---

## 9. Vtables, dispatch tables e chamadas indiretas

Explique:

- ponteiros de função;
- chamadas indiretas;
- `jalr`;
- vtables;
- dispatch tables;
- jump tables;
- diferença entre vtable, jump table e tabela de função;
- por que funções sem callers estáticos podem ser importantes;
- por que nem toda função sem caller estático é automaticamente relevante.

A persona deve mostrar cautela: quando algo parece uma vtable, ainda precisa ser validado.

---

## 10. A hipótese em torno de `0x001d37c8`

Explique com cautela a hipótese de que essa função pode ser um dispatcher.

Use termos provisórios:

- `candidate_state_dispatcher`;
- `candidate_state_block_ptr`;
- `candidate_state_id`;
- `candidate_state_jump_table`.

Explique a possível cadeia:

```txt
entity_ptr
→ [entity + 0x800]
→ candidate_state_block_ptr
→ [candidate_state_block_ptr + 0x48]
→ candidate_state_id
→ candidate_state_jump_table
→ handler
```

Deixe claro que isso precisa de validação.

Não trate como confirmado se ainda houver inconsistências.

---

## 11. A tabela `0x00628fb0`

Explique:

- o que uma jump table poderia ser;
- por que uma tabela com cinco entradas chama atenção;
- por que isso poderia sugerir estados `0-4`;
- como validar se cada entrada é ponteiro para código;
- por que valores pequenos, desalinhados ou fora da região `.text` enfraquecem a hipótese;
- por que a instrução de indexação precisa ser conferida.

Inclua a cautela sobre a diferença entre:

```asm
sll $3,$3,2
```

e:

```asm
sll $3,$0,2
```

Explique por que isso muda completamente a interpretação.

---

## 12. O perigo das conclusões rápidas

Explique a diferença entre:

- evidência direta;
- evidência indireta;
- inferência plausível;
- hipótese;
- ruído;
- falso positivo.

Mostre como agentes de IA podem acelerar muito o processo, mas também podem gerar acúmulo de interpretações sem validação.

A persona deve refletir sobre a necessidade de transformar “descobertas geradas por agentes” em “cadeia de evidência auditável”.

---

## 13. DATA.DF, overlays e assets

Explique:

- o que pode ser um container de assets;
- por que `DATA.DF` é relevante;
- por que um arquivo enorme pode esconder texturas, modelos, scripts, cenas ou dados compactados;
- o que são overlays;
- por que tentar resolver um container inteiro cedo demais pode virar um pântano;
- por que é melhor voltar para `DATA.DF` com um alvo específico.

Deixe claro:

```txt
DATA.DF existe e é relevante,
mas o formato interno ainda não está compreendido.
```

Não diga que ele é especulativo se o projeto já confirmou sua existência.

---

## 14. Arqueologia digital como método

Desenvolva a metáfora da escavação:

- cada offset é um fragmento;
- cada função é uma sala soterrada;
- cada string é uma inscrição;
- cada tabela é uma planta parcial;
- cada falso positivo é uma pedra comum confundida com artefato;
- cada validação é como encontrar o encaixe correto entre fragmentos.

Mostre que arqueologia digital não é apenas romantizar binários antigos. É método, paciência, registro e cautela.

---

## 15. O próximo passo real

Conclua com a decisão metodológica:

Antes de novas explorações, validar o fluxo:

```txt
0x001d3a30
→ 0x001d37c8
→ candidate_state_id
→ 0x00628fb0
→ handler
```

Explique por que esse é o próximo passo mais importante.

Mostre que o projeto precisa sair de:

```txt
descobertas geradas por agentes
```

para:

```txt
cadeia de evidência auditável
```

---

## Epílogo — Não reconstruir apenas código, mas intenção

Feche com uma reflexão sobre reconstruir comportamento, entender decisões de engine e respeitar a obra original.

A conclusão deve transmitir que decompilar/reconstruir ICO não é apenas recuperar código, mas tentar entender a intenção técnica por trás de uma obra artística.
```

---

# Regras de precisão técnica

Siga estas regras obrigatórias:

1. Não diga que algo está confirmado se ainda é hipótese.

2. Use expressões como:

   - “parece indicar”;
   - “a hipótese atual é”;
   - “isso ainda precisa ser validado”;
   - “o risco aqui é confundir correlação com causalidade”;
   - “sem xrefs confiáveis, essa conclusão permanece provisória”;
   - “a evidência sugere, mas não confirma”.

3. Quando falar de endereços, mantenha cautela.

4. Não invente funções, offsets, nomes internos ou estruturas que não estejam no contexto.

5. Se criar cenas narrativas, deixe-as como cenas de aprendizado, não como evidência técnica.

6. Diferencie claramente:

   - fato observado;
   - hipótese técnica;
   - interpretação narrativa;
   - metáfora.

7. Se houver contradição entre documentos, destaque a contradição em vez de escolher arbitrariamente uma versão.

8. Se um ponto depende de validação futura, diga isso explicitamente.

9. Não trate ferramentas como Ghidra, PCSX2 ou scripts como autoridades absolutas. Elas ajudam a investigar, mas a interpretação precisa ser validada.

10. Não confunda “localizei um dado” com “entendi sua função”.

---

# Conceitos que devem aparecer

Inclua e explique, se estiverem coerentes com `key-concepts.md`:

- ISO de PS2;
- ELF;
- ELF stripped;
- MIPS;
- registradores;
- `jal`;
- `jr`;
- `jalr`;
- delay slot;
- stack;
- Ghidra;
- disassembly;
- decompilation;
- xrefs;
- function pointer;
- vtable;
- dispatch table;
- jump table;
- state machine;
- entity update loop;
- asset;
- texture;
- `.tm2`;
- `.gcm`;
- `DATA.DF`;
- overlay;
- runtime validation;
- breakpoint;
- watchpoint;
- patch test;
- evidência direta vs hipótese;
- engenharia reversa incremental;
- documentação como parte do processo;
- arqueologia digital;
- falsos positivos;
- validação conservadora;
- cadeia de evidência auditável.

---

# Tamanho esperado

Quero um texto muito longo.

Não faça resumo.

Não faça apenas tópicos.

Não entregue somente um plano.

Não entregue apenas uma estrutura.

Escreva como um artigo narrativo extenso, com profundidade e continuidade.

O texto pode ser dividido em capítulos, mas cada capítulo deve ter desenvolvimento real, com explicação técnica e narrativa.

A resposta deve ser longa o suficiente para parecer um capítulo de livro ou um ensaio técnico robusto.

---

# Resultado esperado

Entregue o texto final em Markdown.

O texto deve ter:

- título forte;
- subtítulo opcional;
- capítulos bem nomeados;
- narrativa em primeira pessoa;
- explicações técnicas detalhadas;
- metáforas de arqueologia digital;
- cautela metodológica;
- fechamento reflexivo.

Não entregue plano.

Não entregue checklist.

Não entregue comentários sobre como escrever.

Não explique o que você vai fazer.

Entregue diretamente o blog post completo.

---

# Lembrete final

Você está escrevendo como uma persona fictícia, mas tecnicamente cuidadosa.

A persona pode ter emoção, fascínio e voz literária.

Mas a análise técnica deve permanecer conservadora.

A regra principal é:

> **A narrativa pode ser poética; as conclusões técnicas não.**
