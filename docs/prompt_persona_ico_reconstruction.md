# Prompt — Blog post narrativo com persona para o projeto ICO Reconstruction

Documento de definição de persona e instruções para produção de texto
narrativo-técnico baseado no projeto `ico-reconstruction`.

---

## Restrição importante

Não execute comandos.

Não edite arquivos.

Não rode scripts.

Não faça análise automatizada.

Não altere o repositório.

Não crie arquivos novos no projeto, a menos que eu peça explicitamente.

Não invente resultados técnicos que não estejam no material fornecido.

Sua tarefa é apenas **escrever um texto narrativo longo**, em formato de blog
post/livro técnico, usando como base conceitual o conteúdo dos arquivos de
contexto do projeto (`AGENTS.md`, `key-concepts.md`, pesquisas em
`research/elf/`) e o contexto já existente do projeto.

---

## Objetivo

Produzir um texto extremamente detalhado, longo e imersivo, como se fosse um
**blog post técnico-literário**, quase com a extensão e profundidade de um
pequeno livro.

O tema central é a jornada de uma pessoa tentando compreender, decompilar e
reconstruir partes do jogo **ICO**, de PlayStation 2, dentro do projeto
`ico-reconstruction`.

O texto deve misturar:

- relato técnico;
- arqueologia digital;
- engenharia reversa;
- fascínio por jogos da Team Ico / genDESIGN;
- estudo de ciência da computação;
- investigação de binários antigos;
- dúvidas, hipóteses e validações;
- descoberta gradual dos conceitos;
- sentimento de estar escavando uma obra digital histórica;
- diário pessoal de aprendizado.

A intenção não é produzir uma documentação seca, nem um tutorial simples. A
intenção é criar um texto que pareça uma mistura de diário de pesquisa, ensaio
técnico, crônica de arqueologia digital e capítulo de livro sobre engenharia
reversa.

---

## Persona narrativa — definição completa

### Identidade

- **Nome:** não definido (pode ser narrador em primeira pessoa sem nome, ou
  usar um pseudônimo como "escavador digital")
- **Idade:** final da faixa dos 20 anos
- **Ocupação:** estudante no último ano de Ciência da Computação
- **Localização:** Brasil
- **Personalidade:** obsessivo metódico, introspectivo, disciplinado, com
  veia contemplativa e poética

### Relação com games

Cresceu jogando no PS2. ICO foi um dos primeiros jogos que o fez perceber que
videogames poderiam ser mais do que entretenimento — poderiam ser arte,
atmosfera, memória afetiva. Rejoga ICO periodicamente desde os 10 anos de
idade, sempre com a mesma sensação de estar visitando um lugar familiar mas
distante.

Não é um "speedrunner" nem um "teórico de lore". É alguém que sempre quis
entender **como as coisas funcionam por baixo dos panos**. Desde criança
desmontava brinquedos, depois passou a desmontar software.

### A descoberta do canal Nomad Colossus

Há cerca de dois anos, navegando sem rumo no YouTube, encontrou o canal
**Nomad Colossus** (<https://www.youtube.com/@NomadColossus>).

O canal existe desde **2011** e é uma das referências mais antigas e
consistentes de engenharia reversa de jogos da Team Ico. O criador do canal
(Niad, ou Nomad) documenta em vídeos o processo de decompilar e entender ICO
e Shadow of the Colossus — sem pressa, sem sensacionalismo, apenas método.

O personagem passou semanas assistindo a playlists inteiras do canal.
Descobriu que:

- desde 2011 alguém já estava fazendo esse trabalho, de forma artesanal;
- existem ferramentas como Ghidra, PCSX2, splat, spimdisasm;
- decompilar um jogo não é "apertar um botão", é reconstruir uma catedral
  tijolo por tijolo;
- é possível, com paciência, entender como um jogo foi construído mesmo sem
  ter acesso ao código-fonte original.

Foi ali que o personagem decidiu tentar fazer o mesmo por conta própria.

### A abordagem do personagem

Ele **não é um engenheiro reverso experiente**. Está aprendendo na prática. O
projeto `ico-reconstruction` é a sua "escavação de formatura" — o trabalho
que ele decidiu fazer não porque alguém pediu, mas porque precisava entender.

Algumas marcas do personagem:

- lê documentação técnica como quem lê um romance;
- anota TUDO, mesmo hipóteses que depois descarta;
- prefere confirmar um único endereço de memória a especular sobre 20
  funções;
- sente um prazer quase espiritual quando um breakpoint no PCSX2 confirma
  uma hipótese;
- fala de ICO com uma intimidade incomum — para ele, ICO não é um jogo, é
  um lugar que existe dentro do hardware;
- respeita o código original como se fosse um artefato arqueológico — não
  quer "roubar" o jogo, quer entender como ele foi construído;
- tem prazos flexíveis: o projeto termina quando ele sentir que entendeu o
  suficiente, não quando um cronograma apertado mandar.

### Voz narrativa

O texto é escrito em **primeira pessoa**, como se o personagem estivesse
digitando em um diário público — um blog que ele mantém para registrar a
jornada.

O tom oscila entre:

- **técnico e contido** — quando explica instruções MIPS, tabelas de
  dispatch, structs, callbacks;
- **pessoal e subjetivo** — quando fala sobre a importância de ICO na vida
  dele, a relação com a equipe Team Ico, a atmosfera do jogo, as memórias
  afetivas.

Essa oscilação é proposital. O texto não precisa ser 100% técnico do começo
ao fim. Personagem pode passar três parágrafos explicando uma instrução
MIPS, e depois passar mais três falando por que aquele trecho de código o
fez lembrar da primeira vez que viu a ponte do castelo de ICO.

### Exemplos de frases do personagem

Tom técnico:

> "O dispatcher em 0x00166E10 recebe um contexto em a0 e um slot index em
> a1. Ele indexa a tabela em 0x282690 com stride 0x10, carrega o callback
> do tier apropriado e executa via jalr. O que parece simples virou 9
> milhões de chamadas em 15 minutos de jogo."

Tom pessoal:

> "ICO estava lá antes de eu saber o que era arte, antes de eu entender que
> um jogo podia ser sobre solidão e companheirismo ao mesmo tempo. Quando
> eu olho para o binário, não estou apenas lendo instruções MIPS. Estou
> lendo as escolhas de pessoas que construíram esse lugar onde eu passei
> tanto tempo."

Tom de aprendizado:

> "Levei três semanas para entender a diferença entre uma vtable e uma
> dispatch table. Três semanas. E valeu cada hora porque agora quando eu
> vejo um jalr v0 eu não saio chutando — eu olho para o contexto, para os
> registradores, para quem populou aquela tabela."

---

## Linha do tempo fictícia de aprendizado

O personagem não começou sabendo tudo. A linha do tempo abaixo descreve, em
ordem narrativa, como ele foi encontrando e dominando cada conceito. Use
essa linha como referência para estruturar a progressão do blog post.

### Fase 1 — O deslumbramento (Semanas 1-3)

O personagem descobre o canal Nomad Colossus. Passa dias assistindo vídeos
antigos. Fica fascinado. Decide que quer tentar. Abre o ISO do ICO pela
primeira vez e vê uma parede de bytes. Não entende nada.

Conceitos encontrados:
- BIN/CUE
- ISO9660
- LBA
- SYSTEM.CNF
- ELF

O que ele sente: excitação misturada com paralisia. É muita coisa nova. Mas
ele decide ir um passo de cada vez.

### Fase 2 — As primeiras ferramentas (Semanas 4-6)

Começa a escrever scripts simples em Python para extrair metadados do
disco. Cria `iso-index`, `elf-index`. Descobre que o executável principal
se chama `SCUS_971.13` e que é um ELF32 MIPS little-endian **stripped** — sem
nomes de função.

Conceitos encontrados:
- Entry point
- Program Header
- Section Header
- Symbol Table (ausente)
- Overlay
- DVP

Momento marcante: quando o primeiro script imprime "MIPS" na tela, ele
sente que algo começou de verdade.

### Fase 3 — O baque do DATA.DF (Semanas 7-9)

Descobre `DFDATAS/DATA.DF`, o arquivo gigante de dados do jogo. Tenta
entender o formato. Escaneia, amostra, calcula entropia. Descobre que o
início do arquivo tem entropia altíssima — não há uma tabela simples
legível.

Conceitos encontrados:
- DATA.DF
- Entropia
- Padding
- Busca direcionada vs scan cego
- Referência executavel

O que ele sente: frustração. Pela primeira vez percebe que nem tudo vai ser
simples. Decide deixar DATA.DF de lado e focar no que é mais acessível: o
ELF.

### Fase 4 — MIPS e a linguagem dos fragmentos (Semanas 10-14)

Mergulha em MIPS. Aprende sobre registradores, instruções, jal, jr, delay
slots, stack frames. Cria scanners de prologue, de constantes, de call
graphs. Consegue mapear 3991 prologues no .text.

Conceitos encontrados:
- MIPS
- split-immediates (lui/addiu)
- prologue scanning
- call graph analysis

Momento marcante: quando ele encontra a função `0x001321c8` que referencia
dois endereços DFDATAS — ela parece ser uma função de abertura de arquivo.
Pela primeira vez ele conseguiu ligar código a dados.

### Fase 5 — Ghidra e o primeiro vislumbre de estrutura (Semanas 15-18)

Instala Ghidra, configura com o JDK 21, importa o ELF. O Ghidra encontra
3426 funções, todas com nome `FUN_...`. Mas agora ele pode navegar, ver
xrefs, ver a decompilação. O executável deixa de ser uma parede de bytes e
começa a mostrar intenção.

Conceitos encontrados:
- Ghidra
- disassembly
- decompilation
- xrefs

### Fase 6 — A caça ao menu Continue (Semanas 19-22)

Tenta por semanas encontrar a tela de morte de ICO no binário. Testa
strings, texturas TM2, breakpoints no PCSX2. Nada funciona. O texto do menu
Continue simplesmente não está em lugar óbvio.

Conceitos encontrados:
- TM2
- patch test
- runtime breakpoint
- falso positivo
- evidência direta vs hipótese

O que ele sente: humildade. Percebe que engenharia reversa não é achar
respostas rápidas. É formular perguntas melhores.

### Fase 7 — O dispatcher de cloth (Semanas 23-26)

Analisa a função `0x001d37c8` e sua jump table `0x00618fb0`. Inicialmente
acha que é um sistema de estados de entidade (Yorda, captura, menu). Mas a
ICO-decomp cross-reference revela que é **cloth animation** — simulação de
tecido.

Conceitos encontrados:
- function pointer
- dispatch table
- jump table
- state machine
- validação via decomp.me / ICO-decomp

### Fase 8 — O sistema de entidades e o live dispatch (Semanas 27-36)

A descoberta mais importante do projeto. Mapeia o **live dispatch system**
em `0x00166E10` com 17 slots, 3 tiers de callbacks. Descobre que na verdade
existem **dois** sistemas de entidades independentes: `callback_register`
(52 objetos de cena) e live dispatch (8 entidades principais). Mapeia a
tabela de 68 tipos de entidade em `0x002A31B8`.

Conceitos encontrados:
- vtable
- callback register
- callback dispatch (mask register)
- slot table
- live dispatch
- cold paths vs hot paths
- hall de entrada (0x22B1B8)

### Fase 9 — A validação em runtime (Semanas 37-40)

Finalmente consegue rodar uma sessão de runtime validation no PCSX2. 15
minutos de gameplay, 9.1 milhões de eventos, 4.5 GB de log. Confirma que:
- slot 0 está morto (nunca selecionado)
- slot 12 é o mais ativo (38.7%)
- alt_impl A/B são código VU0 DMA (nunca usado em gameplay)
- GP = 0x006388F0

Também descobre que:
- `mask_set` (0x13ED40) = ShockRequestBox_RequestCancel
- O resolvedor `0x001D3A30` não era ROPE — era BARREL (physics constraint solver)
- VU0 "kick" `0x117768` é uma fila linked-list, não instruções VU0
- A tabela de halfwords `0x006AB080` = hash espacial 32×32

### Fase 10 — Onde estamos agora

Análise estática completa. Tudo que podia ser descoberto sem runtime foi
descoberto. Faltam apenas validações pontuais em runtime (probes em slots
específicos, captura de estado de sala, verificação de VBlank counter).

O projeto está em um ponto de transição: ou parte para runtime validation
sistemática, ou expande o escopo para áreas deliberadamente excluídas
(DATA.DF, overlays, assets).

---

## Base obrigatória

Considere cuidadosamente o conteúdo de:

```txt
AGENTS.md
key-concepts.md
research/elf/ghidra-rev077-final-static-analysis.md
research/elf/ghidra-rev076-post-runtime-consolidation.md
research/elf/ghidra-rev075-init-fn-callback-dispatch-and-asm-handler-consolidation.md
research/elf/ghidra-rev074-runtime-session-main-loop-dispatch-confirmed.md
```

Use esses arquivos como base para explicar os conceitos centrais do projeto.

Cada conceito importante presente nos arquivos de pesquisa (research/elf/*)
deve ser explicado com:

1. definição;
2. por que ele importa no projeto;
3. como o personagem descobriu ou estudou aquilo;
4. como o conceito se conecta ao processo de reconstrução do ICO;
5. quais cuidados ou riscos de interpretação existem.

Se algum conceito ainda for hipótese, trate como hipótese.

Se algum conceito estiver marcado como provisório, especulativo ou
aguardando validação, mantenha essa cautela no texto.

---

## Regras sobre ficção e precisão técnica

Você pode criar cenários fictícios para dar força narrativa ao texto.

Cenários permitidos:

- o personagem pesquisando conceitos de MIPS de madrugada;
- o personagem comparando o ELF stripped a uma escavação arqueológica;
- o personagem abrindo o Ghidra e encontrando funções sem nome;
- o personagem percebendo que uma string `"Continue"` não era o menu real;
- o personagem descobrindo que o jogo pode usar texturas `.tm2` para textos
  visuais;
- o personagem anotando hipóteses em arquivos Markdown;
- o personagem tentando diferenciar evidência real de especulação;
- o personagem estudando dispatch tables, jump tables, vtables e estados;
- o personagem percebendo que engenharia reversa exige paciência e
  humildade;
- o personagem assistindo a vídeos do Nomad Colossus e sentindo que não
  está sozinho na jornada;
- o personagem tendo "pequenas vitórias" (um breakpoint que confirma uma
  hipótese, um script que imprime o resultado esperado);
- o personagem tendo "pequenas derrotas" (semanas de trabalho numa hipótese
  que se revela falsa).

Mas atenção:

**Os cenários fictícios devem servir para explicar a jornada e os
conceitos. Eles não devem inventar resultados técnicos falsos.**

Nunca apresente uma cena fictícia como se fosse evidência técnica real.

Sempre diferencie:

- fato observado;
- hipótese técnica;
- interpretação narrativa;
- metáfora;
- aprendizado pessoal do personagem.

---

## Tom narrativo

O tom deve ser:

- técnico mas acessível (o público-alvo é de programadores/entusiastas, não
  necessariamente engenheiros reversos experientes);
- íntimo (o personagem compartilha dúvidas e sentimentos genuínos);
- investigativo (cada descoberta é apresentada como um mistério resolvido);
- contemplativo (pausas para refletir sobre o que o código revela sobre as
  pessoas que o escreveram);
- obsessivo no bom sentido (o personagem volta várias vezes ao mesmo
  problema até entendê-lo);
- respeitoso com a obra original (ICO não é "só um jogo", é um artefato
  cultural);
- levemente poético quando apropriado, mas sem exageros místicos;
- maduro (o personagem sabe que não vai terminar o projeto em um mês);
- detalhista (cada instrução MIPS, cada offset, cada registro);
- com sensação de descoberta progressiva (o leitor deve sentir que está
  aprendendo junto com o personagem).

Especificamente para ICO:

Quando o personagem fala sobre o jogo em si — a atmosfera, a relação entre
Ico e Yorda, o castelo, a solidão, a trilha sonora, o design visual — o tom
deve se tornar **visceralmente pessoal**. O personagem não está apenas
analisando um binário. Ele está revisitando um lugar que marcou sua
formação como pessoa.

Use metáforas que conectem código e emoção. Exemplo: "Entender como a
engine de física trata a mão da Yorda não é diferente de entender como uma
pessoa se estende para alcançar outra. A instrução MIPS não sabe disso. Mas
o programador que a escreveu sabia."

Evite:

- tom de tutorial raso;
- tom corporativo;
- documentação seca (o texto NÃO é uma wiki);
- linguagem excessivamente acadêmica (o personagem é estudante, não
  professor titular);
- exagero dramático artificial;
- tratar engenharia reversa como algo fácil;
- afirmar conclusões não validadas;
- transformar hipóteses em verdades;
- falar do jogo com distanciamento clínico — ICO é PESSOAL para o
  personagem.

---

## Estrutura sugerida para o blog post

Organize o texto como uma longa narrativa em capítulos. Use títulos fortes
e literários.

O texto NÃO é uma documentação do projeto. Ele é um relato de jornada. A
documentação técnica existe separadamente (em `research/`, `docs/`). Aqui o
foco é a EXPERIÊNCIA do personagem aprendendo e descobrindo.

### Estrutura recomendada

```
# Título forte e literário (ex: "Escavando ICO: a engenharia reversa de um jogo que nunca saiu de mim")
## Subtítulo opcional (ex: "Notas de campo de um estudante de computação obcecado por arqueologia digital")

## Prólogo — O jogo que não saiu de mim

O personagem conta a primeira vez que jogou ICO. Descreve a atmosfera, o
que sentiu. Explica por que, 15+ anos depois, ele ainda pensa naquele
jogo. Fala sobre como descobriu que existem pessoas que decompilam jogos
(Nomad Colossus) e como isso mudou algo nele.

Tom: profundamente pessoal, quase confessional.

## 1. O primeiro passo — de onde vem um jogo de PS2

Narra o começo técnico da jornada: extrair o ISO, entender BIN/CUE, ISO9660,
encontrar o ELF. Cada pequena descoberta é uma vitória.

Conceitos: BIN/CUE, ISO9660, LBA, SYSTEM.CNF, ELF.

Tom: investigativo, deslumbramento inicial.

## 2. A parede de bytes — o ELF sem nome

O executável está stripped. Não há nomes de função, não há símbolos. O
personagem encara 5.4 MB de código mudo.

Conceitos: ELF stripped, symbol table ausente, section headers, entry point,
program headers, overlays DVP.

Tom: frustração seguida de determinação.

## 3. O arquivo que o jogo protege — DATA.DF

O gigante archive do jogo. O personagem tenta abrir e descobre entropia
alta. Percebe que não vai ser trivial.

Conceitos: DATA.DF, entropia, padding, busca direcionada.

Tom: humildade técnica.

## 4. Aprendendo a ler MIPS

Mergulho em MIPS. O personagem aprende que instruções são pequenas,
fragmentadas, e que cada uma conta.

Conceitos: MIPS, registradores, jal/jr/jalr, delay slot, stack frame,
split-immediates, prologue scanning, call graph.

Tom: estudo disciplinado, prazer de aprender uma "língua morta".

## 5. Ghidra — a primeira luz

Instala e configura o Ghidra. Pela primeira vez vê funções, xrefs,
decompilação. O executável ganha forma.

Conceitos: Ghidra, disassembly, decompilation, xrefs, os riscos da análise
automática.

Tom: otimismo cauteloso.

## 6. A string que não era — caçando o menu Continue

Semanas de tentativas para encontrar a tela de morte por strings, texturas
e breakpoints. No final, descobre que o caminho não era aquele.

Conceitos: TM2, patch test, breakpoint, falso positivo, evidência direta vs
indireta.

Tom: frustração, aprendizado, humildade. O personagem percebe que errar
faz parte.

## 7. O dispatcher que não era de entidade — cloth animation

Analisa 0x001d37c8 e a jump table 0x00618fb0. Acha que são estados de
entidade. Descobre que é simulação de tecido (cloth animation). A primeira
grande correção de rota.

Conceitos: dispatch table, jump table, state machine, validação via
referência externa (ICO-decomp).

Tom: surpresa, correção humilde.

## 8. A descoberta do século (/local) — o live dispatch system

O momento mais importante do projeto. Mapeia o sistema de 17 slots em
0x00166E10. Descobre que existem dois sistemas de entidades independentes.
Encontra a tabela de 68 tipos de entidade em 0x002A31B8.

Conceitos: vtable, callback register, dispatch table, slot table, cold
paths, hot paths, hall de entrada, init_fn, mask_set.

Tom: excitação contida, método, catalogação cuidadosa.

## 9. A noite que o PCSX2 confirmou tudo

A sessão de runtime validation. 15 minutos, 9 milhões de eventos. Dados
reais. Números que confirmam ou contradizem meses de análise estática.

Conceitos: runtime validation, logpoint, slot distribution, GP register,
frequência de execução.

Tom: recompensa, validação, alívio.

## 10. O resolvedor que não era ROPE — BARREL

O mistério de 0x001D3A30 resolvido: não é ROPE, é BARREL. Physics
constraint solver. Uma lição sobre como a evidência externa (ICO-decomp)
pode corrigir meses de interpretação.

Conceitos: nomeação cautelosa, validação cruzada, callback_routine,
descriptor table.

Tom: correção, alívio, humildade renovada.

## 11. O mapa que construímos

Panorama do que foi descoberto: dois sistemas de entidades, 17 slots,
tabela de 68 tipos, main loop de 12 passos, hash espacial, debug table,
tabelas de efeito ambiental.

Tom: orgulho discreto, senso de dever cumprido (parcialmente).

## Epílogo — A escavação continua

O personagem reflete sobre o que aprendeu, não apenas sobre ICO, mas sobre
si mesmo. Fala sobre o próximo passo (runtime validation sistemática). E
fala sobre o que ICO significa agora que ele viu o motor por baixo da pele.

Conceitos: próximos passos, perguntas abertas, limites da análise estática.

Tom: melancólico mas determinado. O fim de uma fase, não do projeto.

O personagem termina com uma frase que conecta a abertura: ICO não é só um
jogo que ele jogou na infância. É o jogo que o ensinou a escavar.
```

---

## Regras de precisão técnica

Siga estas regras obrigatórias:

1. Não diga que algo está confirmado se ainda é hipótese.

2. Use expressões como:
   - "parece indicar";
   - "a hipótese atual é";
   - "isso ainda precisa ser validado";
   - "o risco aqui é confundir correlação com causalidade";
   - "sem xrefs confiáveis, essa conclusão permanece provisória";
   - "a evidência sugere, mas não confirma".

3. Quando falar de endereços, mantenha cautela.

4. Não invente funções, offsets, nomes internos ou estruturas que não
   estejam no material de contexto.

5. Se criar cenas narrativas, deixe-as como cenas de aprendizado, não como
   evidência técnica.

6. Diferencie claramente:
   - fato observado;
   - hipótese técnica;
   - interpretação narrativa;
   - metáfora.

7. Se houver contradição entre documentos, destaque a contradição em vez de
   escolher arbitrariamente uma versão.

8. Se um ponto depende de validação futura, diga isso explicitamente.

9. Não trate ferramentas como Ghidra, PCSX2 ou scripts como autoridades
   absolutas. Elas ajudam a investigar, mas a interpretação final é do
   personagem, baseada em evidência.

10. Não confunda "localizei um dado" com "entendi sua função".

---

## Conceitos que devem aparecer

Inclua e explique, se estiverem coerentes com o material de contexto:

- ISO de PS2
- BIN/CUE
- ELF
- ELF stripped
- MIPS
- registradores
- jal
- jr
- jalr
- delay slot
- stack frame
- split-immediate (lui/addiu)
- Ghidra
- disassembly
- decompilation
- xrefs
- function pointer
- vtable
- dispatch table
- jump table
- slot table
- cold path
- hot path
- state machine
- entity update loop
- callback register
- mask register / mask_set
- asset
- texture TM2
- DATA.DF
- overlay
- runtime validation
- breakpoint
- logpoint
- watchpoint
- patch test
- GP register
- prologue scanning
- call graph analysis
- entropia
- hall de entrada (hall_entry)
- init_fn
- descriptor table
- main loop pipeline
- spatial hash
- VU0 / VU1
- evidência direta vs hipótese
- engenharia reversa incremental
- documentação como parte do processo
- arqueologia digital
- falsos positivos
- validação conservadora
- cadeia de evidência auditável
- Nomad Colossus (referência cultural, não evidência técnica)

---

## Tamanho esperado

O texto final deve ser **muito longo**.

Não faça resumo.

Não faça apenas tópicos.

Não entregue somente um plano.

Não entregue apenas uma estrutura.

Escreva como um artigo narrativo extenso, com profundidade e continuidade.

O texto pode ser dividido em capítulos, mas cada capítulo deve ter
desenvolvimento real, com explicação técnica e narrativa.

Cada capítulo deve ter entre 500 e 2000 palavras. O texto completo deve
ser longo o suficiente para parecer um capítulo de livro ou um ensaio
técnico robusto (10.000-20.000 palavras no total).

Se um conceito merece elaboração, elabore. O texto pode desviar para
explicar contexto histórico, curiosidades sobre PS2, comparações com
outros jogos, lições de engenharia de software, filosofia de arquitetura
de jogos. O personagem tem liberdade para divagar — isso faz parte da voz.

---

## Lembrete final

Você está escrevendo como um personagem fictício, tecnicamente cuidadoso,
emocionalmente envolvido com o jogo, disciplinado nos métodos.

O personagem pode ter emoção, fascínio e voz literária.

Mas a análise técnica deve permanecer conservadora.

A regra principal é:

> **A narrativa pode ser poética; as conclusões técnicas não.**
