# Escavando ICO: a engenharia reversa de um jogo que nunca saiu de mim

## Notas de campo de um estudante de computação obcecado por arqueologia digital

---

## Prólogo — O jogo que não saiu de mim

Eu tinha dez anos quando vi ICO pela primeira vez.

Não foi numa loja. Não foi numa revista. Foi na casa de um primo mais velho,
numa televisão de tubo, com o cabo do controle mal encaixado e a imagem
oscilando de vez em quando. A sala estava escura porque era fim de tarde e
ninguém tinha acendido a luz. E na tela, um menino de chifres caminhava por
um castelo imenso, segurando a mão de uma garota de vestido branco.

Eu não sabia o que estava vendo. Sabia apenas que aquilo me paralisou.

Não era como os outros jogos que eu conhecia. Não tinha placar. Não tinha
fases. Não tinha inimigos explodindo. Tinha silêncio. Tinha vento. Tinha uma
sensação de solidão tão densa que parecia sair da tela e ocupar a sala.
Havia uma garota que não falava, que você precisava proteger, que estendia a
mão para você quando estava longe demais.

Passei a tarde inteira com meu primo. Ele me explicou que o jogo se chamava
ICO, que era de PlayStation 2, que o menino se chamava Ico e a garota Yorda.
Ele me disse que o jogo era curto, que dava para zerar em umas seis horas, e
que não tinha continuação oficial. Mas eu não estava preocupado com o
tamanho. Eu estava preocupado em entender por que aquele jogo mexia comigo
de um jeito que nenhum outro tinha mexido antes.

ICO tem essa qualidade rara. Ele não te explode com estímulos. Ele te
convida para um espaço — um castelo vasto, aberto, quase vazio — e pede
apenas que você atravesse. O jogo inteiro é uma travessia. Você anda,
empurra blocos, puxa alavancas, abre portas, protege a Yorda. Não há
diálogo, não há narração, não há tutoriais. A própria arquitetura do
castelo é o tutorial: se há uma alavanca, é porque você precisa puxá-la. Se
há uma porta, é porque você precisa abri-la. Se há um abismo, é porque você
precisa encontrar um jeito de atravessá-lo.

Anos depois, na faculdade de Ciência da Computação, eu ainda pensava naquela
tarde. Já tinha rejogado ICO umas seis ou sete vezes. Já tinha lido textos
sobre a Team Ico, sobre Fumito Ueda, sobre o design por subtração, sobre a
filosofia de "remover tudo que não é necessário". Já tinha mostrado o jogo
para amigos, já tinha escrito alguns textinhos sentimentais em blogs
pessoais.

Mas nunca tinha conseguido responder à pergunta que realmente me incomodava:

**Como isso foi feito?**

Não no sentido artístico. Não no sentido de design de gameplay. No sentido
técnico. Que instruções a CPU do PlayStation 2 executa quando Ico e Yorda
estão atravessando o salão principal? Como o jogo decide que a Yorda precisa
estender a mão? Onde está o código que controla a transição entre uma sala e
outra? Como um castelo inteiro cabe em 32 MB de RAM — e sobra espaço para o
resto do jogo?

Essas perguntas ficaram comigo por anos, latentes, sem resposta. Eu não
sabia nem por onde começar a respondê-las. Não era como aprender a
programar. Era como tentar ler uma carta escrita numa língua que você não
sabe que existe. O PlayStation 2 é um sistema bizarro, com uma CPU
principal (EE), um processador de I/O (IOP), duas unidades vetoriais (VU0 e
VU1), um chip gráfico (GS), e uma arquitetura de memória fragmentada em
vários barramentos. Não é um PC disfarçado. É um console feito sob medida
para uma época em que o hardware precisava ser profundamente compreendido
para ser bem aproveitado.

Eu não entendia nada disso na época. Só sabia que queria entender.

Até que, numa madrugada de insônia no quinto semestre, eu encontrei o canal
Nomad Colossus.

Estava vendo vídeos aleatórios sobre PlayStation 2 quando o algoritmo
sugeriu um nome que me fez parar: "Nomad Colossus". A referência a Shadow
of the Colossus era óbvia. Entrei.

O vídeo era de 2012. O criador, que se apresenta como Niad (ou simplesmente
Nomad), estava mostrando uma ferramenta que ele mesmo tinha feito para
extrair modelos 3D de ICO. Ele explicava, com uma paciência quase
monástica, como tinha descoberto a estrutura dos arquivos .mob do jogo,
como tinha mapeado os vértices, como tinha reconstruído a malha poligonal
do castelo. Tudo na mão. Tudo documentado. Sem scripts milagrosos, sem
ferramentas de terceiros — apenas ele, um hex editor, e pura força de
vontade analítica.

Eu fiquei hipnotizado. Assisti aquele vídeo inteiro — uns 40 minutos — sem
piscar.

Passei as semanas seguintes consumindo o canal. Alguns vídeos eram de 2011.
Onze anos antes daquele momento. Onze anos que alguém vinha fazendo esse
trabalho, sozinho, documentando cada passo, sem pressa, sem alarde. Nem
todo vídeo era sobre ICO: tinha Shadow of the Colossus, The Last Guardian,
alguns experimentos com outros jogos de PS2. Mas o método era sempre o
mesmo: observar, anotar, testar, confirmar. Arqueologia digital no sentido
mais puro.

Foi ali que eu decidi que queria tentar.

Não porque eu achava que ia conseguir. Não porque eu me sentia preparado.
Eu mal sabia programar direito. Nunca tinha estudado a fundo arquitetura de
PS2. Nunca tinha aberto um disassembler na vida. Mas eu queria tentar.
Queria sentir na pele o que era escavar um binário. Queria ver se eu
conseguia, no meu próprio ritmo, encontrar alguma verdade escondida naquele
executável de 5 megabytes que eu carregava comigo desde a infância.

Este blog é o registro dessa tentativa.

Não é um tutorial. Não é um guia. Não é uma documentação oficial. É um
diário de campo escrito por alguém que está aprendendo a escavar. Alguém
que acertou, errou, voltou atrás, descobriu coisas que ninguém tinha
documentado e outras que já eram conhecidas há anos. Alguém que, no fundo,
só queria entender como uma obra de arte foi construída tijolo por tijolo
— ou, neste caso, instrução por instrução, byte por byte.

Se você chegou até aqui esperando um tutorial de engenharia reversa, sinto
informar que este texto não é isso. Se você veio atrás de uma história —
de tropeços, de pequenas vitórias, de uma relação estranha e profunda entre
um estudante e um jogo de vinte anos atrás — então senta que a jornada
é longa e o caminho é sinuoso.

---

## 1. De onde vem um jogo de PlayStation 2

Toda escavação arqueológica começa com uma pergunta besta: onde está o que
eu quero encontrar? Você não escava no meio de um campo aleatório. Você
escava onde há indícios de que algo está enterrado.

No caso de ICO, a resposta parece óbvia: está no disco. Mas discos de
PlayStation 2 não são como pendrives ou HDs externos. Eles não estão
organizados para serem lidos por curiosos. Estão organizados para serem
lidos por um hardware específico, com um sistema de arquivos específico,
em setores de tamanho específico. Se você simplesmente abrir o arquivo .bin
no Windows Explorer, não vai ver nada além de uma massa de bytes sem sentido.

Minha primeira tarefa foi responder a uma pergunta que parecia simples:
"O que tem dentro desse ISO?"

A cópia que eu tenho localmente está em formato BIN/CUE. Se você nunca viu
um desses, pense assim: é a fotografia mais bruta possível de um CD. O
arquivo `.bin` contém os dados puros, setor por setor, byte por byte, sem
nenhuma interpretação. O `.cue` é um arquivo de texto que descreve onde
cada trilha começa e termina dentro desse monte de bytes.

No caso de ICO, o `.cue` me revelou algo importante: o disco está em
formato `MODE2/2352`. Isso significa CD-ROM no modo 2, com setores físicos
de 2352 bytes cada. Desses 2352 bytes, apenas 2048 são dados úteis — o
resto é cabeçalho, sincronia, detecção de erro. É como se cada página de
um livro viesse com uma borda decorativa que você precisa ignorar para ler
o texto.

Isso significa que, para ler os arquivos corretamente, eu não podia
simplesmente passar o `.bin` para uma ferramenta de extração de ISO. Eu
precisava saber a estrutura exata do setor. Descobri (depois de horas
comparando hex dumps com documentação de CD-ROM) que o offset dos dados
úteis dentro de cada setor é 24 bytes. Um número pequeno, mas que faz
toda a diferença.

Criei meu primeiro script de verdade, que batizei de `iso-index`. Um
programa Python modesto, umas 200 linhas, que abria o `.bin`, calculava
a posição de cada LBA (Logical Block Address), aplicava o offset de 24
bytes, e tentava ler o diretório raiz ISO9660.

O que é LBA? É a numeração dos setores. O setor 0 é o LBA 0, o setor 1
é o LBA 1, e assim por diante. Como cada setor tem 2352 bytes, a posição
física no arquivo .bin do LBA N é: `N * 2352 + 24`. Simples. Mas se você
errar esse cálculo por um único byte, nada funciona.

O que é ISO9660? É o sistema de arquivos padrão para CDs de dados (e
jogos de PlayStation). Ele define uma estrutura de diretórios e arquivos
similar ao que você conhece no Windows ou Linux, mas com limitações:
nomes de arquivo em 8.3 (oito caracteres para o nome, três para a
extensão), aninhamento limitado de diretórios, e uma organização bastante
rígida.

Quando meu script finalmente imprimiu os nomes dos arquivos do disco na
tela, eu senti uma alegria quase boba. Era quase nada — um monte de nomes
num terminal, sem conteúdo, sem contexto. Mas era meu. Eu tinha tirado
aquilo do nada. Do silêncio de 2352 bytes por setor, eu tinha extraído
informação.

Os arquivos encontrados foram:

```
SYSTEM.CNF
SCUS_971.13
modulos IRX
DFDATAS/DATA.DF
LDUMMY.
```

Cinco entradas. Parece pouco, mas cada uma delas é uma pista:

- `SYSTEM.CNF`: diz ao PlayStation 2 qual executável carregar. É um
  arquivo de texto simples, mas crucial.
- `SCUS_971.13`: o executável principal. Um ELF de 5.4 MB de código e
  dados. O cérebro do jogo.
- `modulos IRX`: drivers de sistema — som, controle, cartão de memória.
  São módulos IOP (Input Output Processor) que rodam no processador
  secundário do PS2.
- `DFDATAS/DATA.DF`: o arquivo que contém TUDO o mais. Quinhentos e
  trinta e nove milhões de bytes de texturas, modelos, animações,
  configurações de cena. Uma montanha de bytes ainda não escavada.
- `LDUMMY.`: uma curiosidade. Um arquivo dummy (provavelmente para
  desperdiçar espaço e empurrar os dados para bordas específicas do
  disco, otimizando tempo de busca do laser).

O `SYSTEM.CNF` me disse que o executável principal está no LBA 25. O
`DATA.DF` está no LBA 2898. O executável começa no endereço virtual
`0x00100000` quando carregado na memória do PS2.

Parece informação trivial hoje. Na época, foi um mapa. Pela primeira vez
eu não estava apenas jogando ICO — eu sabia ONDE as coisas estavam. Eu
podia apontar para um endereço e dizer "ali mora o código principal".
Eu podia apontar para outro e dizer "ali moram os dados do jogo".

Essa fase inicial me ensinou uma lição que carreguei pelo projeto inteiro:
**a engenharia reversa não começa com desmontagem de código. Começa com
organização**. Saber onde estão os arquivos. Depois, onde estão os dados
dentro dos arquivos. Depois, o que esses dados significam. Só depois vem
o código.

---

## 2. A parede de bytes — o ELF sem nome

Com o executável identificado como `SCUS_971.13`, eu precisava entender
o que diabos era um ELF.

ELF significa Executable and Linkable Format. Foi originalmente
desenvolvido pela Unix System Laboratories e hoje é o formato padrão de
executáveis na maioria dos sistemas Unix-like (Linux, FreeBSD, e — no
caso — o sistema operacional do PlayStation 2, que roda uma variante do
Unix).

Pense no ELF como um contêiner organizado. Ele não é apenas um bloco de
código jogado na memória. Ele tem:

- **Cabeçalho ELF**: identifica o formato (32 ou 64 bits), arquitetura
  (MIPS, x86, ARM), endianness, entry point
- **Program headers**: dizem ao carregador do sistema operacional quais
  partes do arquivo vão para quais endereços da memória
- **Section headers**: dividem o arquivo em seções lógicas (.text para
  código, .data para dados, .rodata para constantes, etc.)
- **Symbol table (opcional)**: uma lista de nomes de funções e variáveis
  com seus endereços — o equivalente a um catálogo de ruas da cidade

Meu segundo script, `elf-index`, abriu o `SCUS_971.13` e me deu as

informações básicas do cabeçalho:

```
Formato:      ELF32
Endianness:   Little-endian
Arquitetura:  MIPS
Entry Point:  0x00100008
```

O entry point em `0x00100008` me disse que o código começa a ser
executado logo após o início do segmento carregado (que começa em
`0x00100000`). Os primeiros 8 bytes (`0x00100000` a `0x00100007`) são
provavelmente um cabeçalho ou instruções de inicialização do runtime.

O program header revelou um segmento `PT_LOAD` único que mapeia o
arquivo diretamente para a memória:

```
Offset no arquivo: 4096
Endereço virtual:  0x00100000
Tamanho no arquivo: 5.454.790 bytes
Tamanho em memória: 6.417.304 bytes
```

Diferença entre tamanho no arquivo e em memória: ~962 KB. Essa diferença
é o `.bss` — dados que existem em memória mas não ocupam espaço no
arquivo porque são inicializados como zero. Coisas como buffers, arrays
grandes, estruturas temporárias.

As section headers me deram uma visão mais fina do que existe dentro do
ELF:

- `.text`: código principal (a maior parte — milhões de instruções MIPS)
- `.vutext`: código para as Vector Units (processadores matemáticos
  especializados do PS2)
- `.vudata`: dados para as Vector Units
- `.data`: dados inicializados
- `.rodata`: constantes e strings
- `.bss`: dados zerados em memória
- `.DVP.ovlytab`: tabela de overlays — 144 bytes, 12 entradas de 12
  bytes cada
- `.DVP.ovlystrtab`: strings dos overlays — 398 bytes, 12 strings
- `.DVP.overlay.0000` a `.DVP.overlay.0011`: 12 seções de overlay

A existência de overlays mudou minha percepção do projeto. Overlays são
pedaços de código ou dados carregados sob demanda. O PS2 tem apenas 32 MB
de RAM principal (e parte disso é usada pelo sistema operacional). Para
um jogo como ICO, com um castelo inteiro para renderizar, é impossível
manter tudo na memória ao mesmo tempo. A solução: carregar apenas o que
é necessário para a sala atual.

Mas isso significa que **parte do código do jogo não está no .text
principal**. Algumas funções só existem em overlays, carregadas de dentro
do `DATA.DF` ou de áreas específicas do ELF. A análise estática pura
nunca vai encontrar essas funções — elas simplesmente não estão visíveis
até que o jogo as carregue.

E então veio a informação mais impactante: **não há tabela de símbolos**.

A symbol table (.symtab, .dynsym) é opcional em ELFs. Em executáveis de
desenvolvimento (não stripados), ela contém os nomes de TODAS as funções
e variáveis. `player_update`, `render_frame`, `load_texture` — nomes que
os programadores usaram no código-fonte e que foram preservados no
executável.

Em ICO, a tabela de símbolos foi removida. Stripped. Arrancada. Não sei
se foi uma decisão de release (reduzir tamanho do executável) ou de
proteção (dificultar engenharia reversa), mas o resultado é o mesmo:
**3426 funções sem nome**.

O Ghidra as chama de `FUN_00123456`. Não há placas. Não há mapa de ruas.
Há apenas endereços e instruções.

Um executável stripado é uma cidade sem placas. Você sabe que há prédios
(estruturas de dados), ruas (fluxos de código), praças (funções). Mas não
sabe o que acontece em cada um. Precisa entrar, observar, deduzir.

E foi exatamente isso que comecei a fazer.

---

## 3. O arquivo que o jogo protege — DATA.DF

Se o ELF é a parede de bytes, o DATA.DF é a montanha.

539.367.424 bytes. Mais de meio gigabyte. O maior arquivo do disco por
uma margem enorme — o ELF inteiro tem 5.4 MB, cem vezes menor. E DATA.DF
contém o que? Tudo que não está no executável. Texturas, modelos
tridimensionais, animações de esqueleto, dados de câmera das cutscenes,
configurações de fase, áudio, legendas, fontes, scripts de comportamento.
Tudo.

A primeira e mais óbvia pergunta: **como o executável acessa os dados
dentro do DATA.DF?**

Para um archive comum, você esperaria algo como:

```
Offset 0x00000000: N recursos
Offset 0x00000004: Recurso 1: offset=0x00001000, tamanho=0x00002345, nome="..."
Offset 0x00000010: Recurso 2: offset=0x00003345, tamanho=0x00001234, nome="..."
...
```

Uma tabela. Algo legível. Indexável.

O DATA.DF não é assim.

Fiz uma triagem inicial com o `data-df-index`, uma ferramenta que amostra
janelas do arquivo e calcula **entropia** — uma medida de quão
"aleatórios" ou "imprevisíveis" os bytes são. Entropia de 0 significa que
todos os bytes são iguais (ex: tudo zero). Entropia de 8 significa que os
bytes são perfeitamente distribuídos entre todos os 256 valores possíveis
— o que é característico de dados comprimidos, criptografados, ou
densamente empacotados.

O início do DATA.DF deu **7.994 bits por byte**.

Isso é essencialmente o máximo. Um archive com uma tabela simples no
começo teria entropia baixa (valores repetitivos de offsets e tamanhos).
Sete ponto nove significa que o começo do DATA.DF é indistinguível de
dados aleatórios ou fortemente comprimidos.

Para confirmar, amostrei o meio e o fim:

- Meio: entropia menor (mais estruturas reconhecíveis, talvez dados não
  comprimidos)
- Fim: ~76% de zeros (padding — espaço reservado mas não usado)

Ok. Tabela simples no início está descartada.

Segunda abordagem: **busca direcionada**. O ELF tem várias strings que
referenciam caminhos dentro do DATA.DF. E as seções `.DVP.overlay.*` têm
nomes como `.DVP.overlay.0000`, `.DVP.overlay.0001`, etc. Talvez esses
números (`0000`, `0001`) sejam offsets dentro do DATA.DF? Talvez o jogo
use esses números para localizar recursos específicos?

Testei seis tokens diferentes como offsets candidatos. Para cada um,
escaneie uma janela de 256 KB ao redor do offset, calculei entropia,
procurei por padrões de bytes que parecessem uma tabela. Nada. Todas as
janelas tinham entropia alta.

Terceira abordagem: **referência executavel**. Em vez de tentar entender
o DATA.DF olhando apenas para o archive, que tal perguntar: "o
executável menciona o DATA.DF de alguma forma que eu possa rastrear?"

Usei o `mips-immediate-scanner` para procurar referências a endereços
conhecidos (strings como `DFDATAS/DATA.DF` e `DFDATAS`). Encontrei **7
funções** que referenciam esses endereços. Uma delas, em `0x001321c8`,
referencia DOIS endereços DFDATAS diferentes — provavelmente uma função
de abertura de arquivo ou manipulação de caminhos.

Também encontrei a string `DATA.DF` no executável em um endereço
específico. Fiz o `function-ref-correlator` cruzar as funções com seus
callers, e descobri que `0x00132630` tem 7 callers diferentes — um forte
candidato a função central de I/O de arquivo.

Mas nada disso me deu a chave para decifrar o formato do DATA.DF. Eu
sabia QUAIS funções provavelmente o acessavam, mas não COMO elas o
interpretavam.

O que aprendi com DATA.DF foi uma lição dolorosa e necessária: **nem
tudo é trivial**. Existem camadas de proteção, compressão e ofuscação
que não são acidentais. O formato do DATA.DF foi uma escolha de
implementação dos programadores da Team Ico, e escolhas de
implementação levam tempo, ferramentas e contexto para ser desvendadas.

Eu tomei uma decisão consciente: não tentar decifrar o DATA.DF agora.

Coloquei o arquivo numa gaveta mental chamada "volto aqui quando souber
mais e tiver runtime tracing". O jogo consegue ler o DATA.DF — em algum
lugar do executável está a chave. Eu só não tinha ferramentas para
encontrá-la ainda sem um emulador funcionando com breakpoints de memória.
Mudei o foco para o código. O archive esperaria.

---

## 4. Aprendendo a ler MIPS

MIPS é uma arquitetura RISC (Reduced Instruction Set Computer).

Diferente de processadores como o x86, onde uma única instrução pode ser
complexa e fazer múltiplas coisas, MIPS divide tudo em operações pequenas,
regulares e previsíveis. Cada instrução tem exatamente 32 bits. Cada
instrução faz exatamente uma operação. O conjunto de instruções tem cerca
de 80 instruções principais — um número pequeno o suficiente para um
humano memorizar com algum esforço.

Isso é uma faca de dois gumes. É mais fácil de aprender (80 instruções)
mas mais difícil de ler (qualquer tarefa simples vira várias instruções).

Uma operação que em C seria `x = y + z` vira algo como:

```asm
lw   $t0, offset_y($gp)   ; carrega y da memória
lw   $t1, offset_z($gp)   ; carrega z da memória
addu $t2, $t0, $t1        ; t2 = y + z
sw   $t2, offset_x($gp)   ; armazena x na memória
```

Quatro instruções para fazer uma soma. Cada instrução faz uma coisa
pequena, e você precisa ler todas para entender o que está acontecendo.

As instruções que mais aparecem no código de ICO:

- `addiu $sp, $sp, -N` — aloca N bytes na pilha (o prólogo de função
  típico)
- `sw $ra, offset($sp)` — salva o endereço de retorno na pilha
- `jal endereco` — "jump and link": chama uma função, salvando o
  endereço da próxima instrução em $ra
- `jr $ra` — "jump register": retorna da função (volta para o que
  estava em $ra)
- `jalr $reg` — "jump and link register": chamada indireta via ponteiro
  de função (essencial para vtables e dispatch tables)
- `lw $reg, offset($base)` — "load word": carrega 4 bytes da memória
- `sw $reg, offset($base)` — "store word": escreve 4 bytes na memória

Um padrão que me tomou semanas para entender completamente foi o
**split-immediate**.

MIPS tem instruções de 32 bits. Os campos imediatos (valores constantes
embutidos na instrução) têm apenas 16 bits. Mas endereços de memória no
PS2 têm 32 bits. Como você coloca um endereço de 32 bits dentro de uma
instrução que só tem 16 bits para o valor?

Resposta: você usa duas instruções.

```asm
lui   $t5, 0x0055       ; $t5 = 0x00550000 (carrega 16 bits superiores)
addiu $t5, $t5, 0x6a10  ; $t5 = 0x00550000 + 0x6a10 = 0x00556a10
```

O `lui` (Load Upper Immediate) carrega 16 bits nos 16 bits superiores de
um registrador, zerando os 16 bits inferiores. Depois um `addiu` ou `ori`
soma os 16 bits inferiores. Resultado: um endereço completo de 32 bits.

Isso tem uma consequência cruel para a engenharia reversa: um endereço de
32 bits **não aparece como um valor contínuo no binário**. Ele aparece
como DOIS valores de 16 bits, potencialmente separados por outras
instruções. Um scanner ingênuo que procura os bytes `0x00556a10` em
sequência não vai encontrar nada, porque esses bytes não existem como um
bloco contínuo.

Para lidar com isso, criei o `mips-prologue-scan`. A ideia é simples:
funções em MIPS quase sempre começam com `addiu $sp, $sp, -N` para
alocar o stack frame. O valor de N (o tamanho do frame) varia de função
para função. Se eu escanear todo o `.text` procurando esse padrão,
encontro todas as funções — mesmo sem símbolos.

Resultado: **3991 prologues** encontrados. **71 tamanhos de stack
diferentes**, de 16 bytes (funções tiny) até 2784 bytes (funções enormes).

A maioria das funções usa frames pequenos (16-64 bytes). Funções com
frames grandes (>128 bytes) são raras e provavelmente importantes —
funções de inicialização, carregamento de cena, processamento complexo.

Depois criei o `mips-call-graph` para mapear quem chama quem. O script
procura instruções `jal endereco` (chamada direta) e, quando
`endereco` é um dos prologues conhecidos, registra a relação. O
resultado é um grafo direcionado de chamadas.

A função `0x00132630` tem **7 callers** — é a mais chamada de todas.
Provavelmente uma função utilitária central (talvez de I/O de arquivo).
A função `0x001321c8` tem **0 callers diretos** — ou é chamada via
ponteiro, ou é dead code, ou minha detecção falhou.

O call graph me deu o primeiro mapa funcional do executável. Não era um
mapa com nomes de função, mas era um mapa com relações. Eu sabia que
`0x00132630` era importante porque muitas funções dependiam dela. Eu
sabia que `0x001321c8` era misteriosa porque ninguém a chamava
diretamente.

Esse período de estudo de MIPS foi, paradoxalmente, um dos mais felizes
do projeto. Eu passava noites com sessões do terminal abertas, rodando
scanners, analisando output, comparando endereços. Cada padrão que eu
reconhecia era um fragmento a menos no silêncio do executável. Cada
call graph que eu montava era uma pequena vitória.

Aprendi que MIPS parece uma língua morta — e, de certa forma, é. Não se
programa mais para MIPS como nos anos 90, a não ser em sistemas
embarcados muito específicos. Mas ICO fala essa língua, e eu estava
aprendendo a ouvi-la.

---

## 5. Ghidra — a primeira luz

Ghidra é um disassembler e decompilador de código aberto desenvolvido
pela NSA. Não é exagero dizer que ele transformou minha relação com o
projeto.

Antes do Ghidra, eu trabalhava com scripts Python que me davam números,
endereços, contagens. Eu sabia quantas funções existiam, quantas
instruções cada uma tinha, quais chamavam quais. Mas era tudo abstrato —
tabelas e listas de endereços hexadecimais. O Ghidra me deu **visão**.

Importar o ELF no Ghidra 12.0.4 exigiu JDK 21 (uma história de
incompatibilidade de versão que me custou um dia inteiro). A análise
automática, depois de alguns minutos, me deu: **3426 funções**.

Todas com nome `FUN_00123456`. Nenhuma com nome original. Mas agora eu
podia ver o código. Podia ver a decompilação — o Ghidra traduz MIPS para
um pseudo-C que, mesmo imperfeito, é muito mais legível que assembly
bruto.

A primeira coisa que fiz foi navegar pelas funções mais chamadas. A top
1 era `FUN_001b7288`, com 8 callers. O corpo dela ocupava poucas
instruções: carregava `$a0` em um registrador e armazenava em um offset
do GP (Global Pointer). Um setter de variável global, quase certeza.

A top 2 era `FUN_001a6e28`, com 4 callers. Essa era mais complexa: stack
frame grande, acesso a múltiplos offsets de GP, chamadas a outras
funções. Possivelmente uma função de inicialização ou transição de
estado.

O GP (Global Pointer) merece explicação. No PS2, o GP é um registrador
(`$gp`) que aponta para uma área de dados globais do programa. É uma
técnica de compilador para acessar variáveis globais de forma eficiente:
em vez de carregar um endereço absoluto de 32 bits (que precisa de duas
instruções), você usa `lw $reg, offset($gp)` — uma única instrução, com
offset de 16 bits. O compilador organize todas as variáveis globais
próximas umas das outras na memória, e o GP aponta para o meio dessa
região, permitindo offsets positivos e negativos.

Em ICO, a área de dados globais referenciada por GP é enorme. Meus
scanners encontraram **1032 offsets de GP diferentes**, usados por
**9971 instruções** no `.text` inteiro. É a região de dados mais
importante do executável — e a mais densa.

O Ghidra também me deu as **referências cruzadas (xrefs)**. Quando uma
função acessa uma string, o Ghidra mostra de onde ela é acessada. Quando
uma função é chamada de vários lugares, o Ghidra lista todos os callers.
Isso foi revolucionário. Meus scripts me davam números estáticos; o
Ghidra me dava contexto dinâmico — quem usa o quê.

Lembro da primeira vez que cliquei em uma string e vi a lista de xrefs.
Era `camdata/sacrifice.gcm`. O Ghidra mostrou que ela era referenciada
por duas funções: uma em `0x0017bb98` e outra em `0x00199f80`. Eu nem
sabia que essas funções existiam antes. De repente, eu tinha um fio
condutor entre uma string de arquivo e duas funções candidatas. Passei
a tarde inteira seguindo esses fios, entrando em funções que eu nunca
tinha visto, anotando relações.

O Ghidra me deu uma forma de navegação que scripts Python não podem dar.
Com scripts, você pergunta "existe X?" e recebe "sim" ou "não". Com o
Ghidra, você clica em algo e vê o grafo de relações ao redor daquilo.
É uma diferença fundamental entre consultar um banco de dados e explorar
um território.

Mas aprendi rapidamente que o Ghidra não é infalível. A análise
automática comete erros — alguns sutis, outros catastróficos:

- **Delimitação incorreta de funções**: especialmente quando há dados
  misturados com código (como jump tables embedadas no meio de uma
  função). O Ghidra às vezes corta uma função no meio ou funde duas
  funções adjacentes.
- **Jump tables perdidas**: tabelas de endereços (como a de
  `0x00618fb0`) são às vezes tratadas como dados desconexos, e o Ghidra
  não consegue seguir os alvos. Cada entrada vira "FUN_código" em vez de
  ser reconhecida como destino de dispatch.
- **Chamadas indiretas (`jalr`) sem xrefs**: o destino de uma chamada
  indireta não é conhecido estaticamente, então o Ghidra não cria xref.
  Funções chamadas apenas via ponteiro aparecem como "zero callers" —
  mesmo sendo essenciais.
- **Tabelas de dados interpretadas como código**: se um bloco de dados
  ("dead data" no meio do .text) cai na zona de análise, o Ghidra pode
  tentar desassemblá-lo, criando funções fantasmas ou instruções sem
  sentido.
- **Otimizações do compilador**: o compilador EE GCC 2.9-991111-01 usa
  padrões de instrução que o Ghidra às vezes decompila incorretamente,
  especialmente com `sll` (shift) usado para indexação de tabelas.

Estabeleci uma regra de conduta: **quando o Ghidra contradiz os bytes
crus, os bytes crus vencem**. Sempre que uma decompilação parece
estranha, eu verifico o disassembly real. Se o disassembly ainda parece
estranho, eu leio os bytes manualmente com um hex dump. O Ghidra é uma
ferramenta de produtividade, não uma fonte de verdade. Ele acelera o
trabalho, mas não substitui a verificação.

---

## 6. A string que não era — caçando o menu Continue

Em toda investigação de engenharia reversa, as strings são o primeiro
grande achado. São inscrições deixadas pelos programadores — texto
legível dentro da massa de bytes.

O executável de ICO tem strings. Muitas:

- Caminhos de arquivos: `camdata/sacrifice.gcm`, `TEX2/yesno_p1.tm2`,
  `object/sdf/obj_com04/...`
- Nomes de objetos: `pac_continueTag`, `continues`
- Mensagens diversas: `stage select`, `now formatting`, `Unformat`

Quando eu encontrei a string `Continue` no endereço `0x00555db6`, achei
que tinha encontrado a pista definitiva para a tela de morte — o menu que
aparece quando Ico morre, com as opções "Continue", "Yes" e "No".

Passei semanas nessa busca.

Primeiro, fiz uma varredura completa de todas as strings relacionadas:

```
"Continue"         -> 0x00555db6
"pac_continueTag"  -> 0x005551f0
"continues"        -> 0x005539a1
"TEX2/yesno_p1.tm2"  -> (textura do "Yes", provavelmente)
"TEX2/cont2_p1.tm2"  -> (textura do "Continue", provavelmente)
"%s.tm2"           -> 0x00631e00 (template de caminho de textura)
```

Para cada string, usei o `mips-immediate-scanner` para encontrar
referências no código. Para `pac_continueTag`, encontrei referências
em `0x0011a520` e `0x0011a57c` — duas instruções dentro de uma função
candidata. Para `%s.tm2`, encontrei referências em `0x0012d230` e
`0x0012d238`.

Cada uma dessas pistas virou uma campanha de breakpoints no PCSX2.

Colocar breakpoint no PCSX2 é simples: você abre o debugger, navega
até o endereço, clica para adicionar breakpoint de execução. Quando a
CPU executa aquela instrução, a emulação pausa e você pode ver os
registradores, a pilha, o contexto.

Testei `0x0011a520`. Resultado: disparou imediatamente ao apertar "New
Game". Não era específico do menu de morte — era uma função de
inicialização genérica.

Testei o callsite `0x0011a794` (que chama `0x0011a520`). Mesmo resultado:
disparava no New Game.

Testei `0x0012d218` (ligado ao template `%s.tm2`). Não disparou no New
Game. Não disparou quando eu morri. Não disparou quando o menu Continue
apareceu. Nada.

Testei `0x0012fd58` (callsite do `%s.tm2`). Mesmo resultado: silêncio.

Testei dezenas de outros endereços — alguns genéricos demais (disparavam
em tudo), outros específicos demais (nunca disparavam). O menu Continue
continuava aparecendo religiosamente na tela, e eu continuava sem
conseguir rastrear sua origem.

Mudei de estratégia: em vez de strings, testei texturas. A hipótese era
que o texto "Continue" não era uma string renderizada por fonte, mas uma
textura — uma imagem com as letras já desenhadas. É comum em jogos de
PS2, onde fontes verdadeiras consomem memória e processamento.

O ELF tem referências a `TEX2/yesno_p1.tm2` a `_p6.tm2` (6 texturas para
o menu Yes/No) e `TEX2/cont2_p1.tm2` a `_p6.tm2` (6 texturas para o menu
Continue). Fiz um patch (mod5): troquei os nomes `yesno_p*` por
`cont2_p*`. Se o menu de morte usasse essas texturas diretamente, a tela
deveria mudar.

Não mudou.

Fiz um patch mais agressivo (mod6): troquei TUDO — `yesno_p*` por
`pause_p*`, `conti_p*` por `title_p*`, `cont2_p*` por `pause_p*`. A
tela de morte continuou idêntica.

Depois de semanas, dezenas de breakpoints, múltiplos patches sem efeito,
eu tive que aceitar: **eu não sabia como a tela de morte de ICO
funcionava**.

E essa foi uma das lições mais importantes do projeto inteiro.

Engenharia reversa não é um filme onde você encontra a prova no último
segundo. Às vezes você investe semanas numa hipótese e ela está errada.
Ou, pior: você não consegue nem testar se está errada porque não tem a
ferramenta certa.

O menu Continue provavelmente está em texturas carregadas de dentro do
DATA.DF, ou em overlays específicos de UI, ou em algum fluxo de
streaming que depende de estado que eu não consegui reproduzir. Mas eu
não tenho como provar isso sem um tracing de runtime mais avançado.

Então aprendi a conviver com essa incógnita. Aprendi que **não saber
também é um resultado**. Não registrar um falso positivo para parecer
produtivo. Aceitar que algumas perguntas vão ficar sem resposta por
enquanto.

---

## 7. O dispatcher que não era de entidade — cloth animation

A primeira vez que vi `0x001d37c8` no Ghidra, achei que tinha encontrado
a peça central do quebra-cabeça.

A função fazia algo muito específico e reconhecível:

```asm
lw      $v1, 0x48($v0)     ; carrega state_id de [state_block + 0x48]
sltiu   $v0, $v1, 5        ; bounds check: state_id < 5? (0-4, cinco estados)
beqz    $v0, default_path  ; se >= 5, vai para o caminho padrão
sll     $v1, $v1, 2        ; state_id * 4 (cada entrada da tabela tem 4 bytes)
lw      $v0, jtbl_00618fb0($v1)  ; carrega endereço alvo: tabela[estado]
jr      $v0                ; pula para o handler correspondente
```

Isso é um **dispatcher** clássico. Recebe um ID de estado, valida se
está dentro do range, indexa uma jump table, e pula para o handler. A
jump table em `0x00618fb0` tem 5 entradas, cada uma apontando para um
bloco de código diferente.

Cinco estados. Uma jump table com verificação de bounds. Era exatamente
o padrão que um sistema de máquina de estados de entidade deveria ter.

Passei semanas dissecando cada um dos cinco handlers. Olhei os dados que
acessavam, as funções que chamavam, as condições que testavam. Anotei
hipóteses: "estado 0 = idle/parado", "estado 1 = andando/seguindo",
"estado 2 = vento/interação externa", "estado 3 = colisão com objetos",
"estado 4 = pós-processamento". Até criei nomes mais específicos no meu
caderno: "Yorda_wait", "Yorda_follow", "capture_state", "menu_state".

Todos baseados em pura intuição, porque eu não tinha evidência direta.

Aí veio a correção.

Encontrei o repositório do ICO-decomp (mantido por RossyDoubleUnderscore).
É um projeto que decompila funções de ICO usando símbolos reconstruídos
por matching com o código original. Ele não tem o código-fonte completo,
mas tem nomes de funções verificados por comparação com o binário.

Cruzei os endereços. `0x001d37c8` aparecia listada como parte do arquivo
**`clothAnimation.c`**.

Cloth. Animation. Simulação de tecido.

Não era Yorda. Não era captura. Não era menu. Era a física da saia. Da
capa. Das cortinas do castelo. Do tecido que balança quando o vento
passa.

Os cinco estados que eu passei semanas analisando não eram "idle",
"seguir", "capturar". Eram fases de simulação de tecido:

- Estado 0: repouso (vértices em posição neutra)
- Estado 1: aplicação de forças externas (vento, gravidade)
- Estado 2: detecção de colisão com o ambiente e auto-colisão
- Estado 3: solução de constraints (manter o comprimento dos panos)
- Estado 4: integração e pós-processamento

Foi humilhante e libertador ao mesmo tempo.

Humilhante porque eu tinha investido semanas numa hipótese errada,
nomeando funções com base em intuição. Pior: eu estava tão confiante
que quase publiquei minhas conclusões como se fossem fato. Só a
verificação externa (o ICO-decomp) me salvou de documentar uma
interpretação errada como verdade.

Libertador porque a correção veio de uma fonte reproduzível e
verificável. Eu pude confirmar: sim, o ICO-decomp diz clothAnimation.c,
sim, os endereços batem, sim, o contexto de chamada faz sentido
(0x001d3a30 chama o dispatcher, e o range de cloth inclui ambos).

A correção também resolveu outro mistério: por que a função 0x001d3a30
(o único caller estático do dispatcher) não se comportava como um
callback de entidade? Porque ela é uma função de física de tecido,
chamada durante o passo de simulação, não durante o update de entidade.

Corrigi todos os meus documentos. Apaguei as anotações especulativas.
Renomeie os estados de "Yorda_state_0" para "cloth_state_0".

E aprendi a lição definitiva: **nunca confie em nomes que você mesmo
inventou**. Um nome é uma hipótese disfarçada de conclusão. Use
identificadores neutros (`dispatcher_0x1d37c8`, `state_0_block`,
`state_1_block`) até que uma fonte externa confirmável forneça um nome
real.

---

## 8. A descoberta do século (/local) — o live dispatch system

Se o cloth dispatcher foi uma pista falsa que me ensinou humildade, o
**live dispatch system** foi a descoberta que me mostrou o motor real
de ICO.

A função `0x00166E10` não é chamada diretamente por muitas outras
funções — tem poucos callers estáticos. Mas o que ela faz é o coração
do jogo. É o sistema que gerencia a atualização de todas as entidades
principais a cada frame.

Vamos por partes.

**O main loop: 12 passos por frame**

O loop principal de ICO está em `0x00101C80`. A cada frame (~16.67ms
em 60fps), ele executa 12 passos em sequência:

1. Processamento de entrada (controles)
2. Atualização de câmera
3. Dispatcher de áudio
4. Sincronização de overlays (carregamento sob demanda)
5. Física global
6. Preparação de dados de rendering
7. **Dispatcher de callbacks Nível 1** (`0x0013F9D0`, máscara GP+0x98DC)
8. Processamento de efeitos
9. **Dispatcher de callbacks Nível 2** (`0x0013FC00`)
10. Finalização de física
11. Submissão de rendering
12. Sincronização VBlank

Os passos 7 e 9 são os que disparam os callbacks das entidades. O passo 7
usa uma tabela de 8 entradas em `0x00281A70`, com uma máscara de seleção
em GP+0x98DC. O passo 9 usa uma tabela em `0x00281AB0`.

**A tabela de 17 slots**

Dentro desse ecossistema, o **live dispatch** em `0x00166E10` gerencia
17 slots, cada um com 16 bytes (stride 0x10). A tabela está em
`0x00282690`. Cada slot contém:

- Ponteiros para callbacks de 3 tiers diferentes
- Flags de ativação
- Contexto (ponteiro para a entidade associada)

Os 3 tiers de callback:

- **Tier 1 (Group 1)**: callbacks leves de posição/rotação. São funções
  que atualizam transformações de objetos. 4 slots usam este tier (1, 2,
  5 e mais um).
- **Tier 2 (Hybrid G1+G2)**: callbacks mistos que fazem transformação E
  algo adicional. 6 slots são híbridos (3, 4, 6, 7, 10, 11).
- **Tier 3 (Group 2)**: pipeline completo — transformação, físicas
  matemáticas, utilitários diversos. 4 slots são full pipeline (0, 12,
  14, 15).

Existem dois caminhos de execução:

- **Hot path**: o caminho principal, executado a cada frame, processando
  todos os slots ativos sequencialmente.
- **Cold paths**: caminhos alternativos (0x00167230 e 0x00167258) que só
  são executados em condições especiais — provavelmente quando um slot
  precisa ser religado ou um callback trocado.

O sistema também tem duas **implementações alternativas**: `0x00169F80`
e `0x0016A058`. Elas são versões do live dispatch para pipeline VU0 DMA
— mas, como a runtime revelou depois, **nunca são usadas em gameplay**.

**Dois sistemas de entidades independentes**

A descoberta mais surpreendente veio quando comecei a cruzar os dados do
live dispatch com os do `callback_register` (a função `0x0013F7A8`).

São dois sistemas **completamente independentes**:

1. **Callback register system**: gerencia objetos de cena. Tem um "hall
   de entrada" em `0x0022B1B8` com 48 registros de 0xF0 bytes cada.
   Cada registro contém uma `init_fn` e um `cb_id`. Quando o jogo
   carrega uma cena, percorre esse hall e chama cada init_fn. Total:
   **52 objetos de cena, 28 init_fn diferentes**. Inclui HUD, itens,
   barris, cordas, UI, efeitos ambientais.

2. **Live dispatch system**: gerencia as entidades principais do jogo.
   **8 entidades principais, ~20 contextos/frame** (algumas entidades
   têm múltiplos contextos, como sub-objetos).

Os dois sistemas **compartilham apenas 3 objetos**. A maioria dos
objetos de cenário está isolada no callback_register, enquanto os atores
principais estão no live dispatch.

Isso mudou completamente meu modelo mental de como ICO funciona. Não é
uma engine monolítica onde "tudo é entidade". São dois sistemas paralelos
com propósitos diferentes, convivendo e raramente se cruzando.

**A tabela de descritores: 68 tipos de entidade**

No centro de um desses sistemas está a **descriptor table** em
`0x002A31B8`. São 68 entradas, cada uma com 0x64 bytes (100 bytes). Cada
entrada define um tipo de entidade com campos como:

- Nome interno (BOY, GIRL, ENEMY1, BARREL, ROPE, QUEEN, BIRD, etc.)
- Model ID (referência ao modelo 3D)
- Massa, raio, threshold (para física de colisão)
- Type ID (identificador numérico)
- `init_fn` (função de inicialização — só 12 dos 68 tipos têm)
- `cb_routine1` a `cb_routine4` (até 4 callbacks específicos do tipo)
- Ponteiro de construtor (em +0x60)

A designação `+0x60` como "vtable pointer" foi uma correção importante
que fiz durante a análise. Inicialmente eu achei que esse campo era uma
vtable (tabela de métodos virtuais). Mas depois verifiquei que o valor
apontado começa com prólogo de função MIPS (`addiu $sp, $sp, -N`), não
com uma tabela de ponteiros. É um **construtor**, não uma vtable.

A tabela de descritores é referenciada por uma **entry table** em
`0x002A4C48`: 512 entradas de 0x4C bytes cada. Cada entrada define uma
instância específica: posição de spawn, escala, class ID, subtipo de
modelo. É a lista de "quem aparece onde no mundo".

As vtables reais de ICO estão em `0x002828C0` e `0x00553760` — duas
regiões diferentes, provavelmente para grupos diferentes de objetos.

**A máscara de seleção**

O sistema usa uma máscara para controlar quais callbacks estão ativos.
A função `mask_set` em `0x0013ED40` escreve bits em um registrador de
máscara em GP+0x6724 (ou `0x006321CC` se você preferir o endereço
absoluto).

O ICO-decomp identificou essa função como
**ShockRequestBox_RequestCancel**. E o padrão de uso? Apenas **bit 0**
é usado. Durante a inicialização de uma cena, o bit 0 é limpo (callbacks
desativados), as entidades são carregadas, e depois o bit 0 é setado
(callbacks ativados). É um mecanismo simples de "pausa global de
callbacks durante carregamento".

Mapear tudo isso levou cerca de um mês — noites e fins de semana
desenhando diagramas, ligando endereços com setas, anotando offsets.
Quando finalmente montei o quadro completo, fiquei um bom tempo apenas
olhando para minhas anotações.

Não era emocionante no sentido hollywoodiano. Mas era real. Era o motor
de ICO funcionando na minha frente, em endereços e instruções que eu
mesmo tinha mapeado, sem nomes originais, uma peça de cada vez.

---

## 9. A noite que o PCSX2 confirmou tudo

Análise estática é bonita, mas não passa de especulação educada até que
algo confirme ou contradiga.

Depois de meses construindo o modelo, veio a noite de validação.

Coloquei ICO no PCSX2 com uma bateria de breakpoints e logpoints nos
endereços que eu considerava críticos: o main loop, o live dispatch,
os cold paths, os alt_impls, os callbacks registrados. Configurei o
PCSX2 para registrar cada breakpoint hit com contexto (registradores,
slot index, callback target).

15 minutos de gameplay. Eu joguei normalmente: andei pelo castelo,
puxei alavancas, empurrei blocos, enfrentei sombras, deixei a Yorda
morrer algumas vezes, fiz o caminho até o próximo save point.

O resultado: **9.151.217 eventos registrados**. 4.5 GB de log.

Processar isso levou a noite inteira.

Os números que saíram:

| Slot | Eventos  | Percentual | Tier          |
|------|----------|------------|---------------|
| 12   | 851.346  | 38.5%      | Full pipeline |
| 1    | 591.922  | 26.8%      | Leaf pos/rot  |
| 3    | 316.996  | 14.3%      | Hybrid G1+G2  |
| 2    | 202.906  | 9.2%       | Leaf pos/rot  |
| 6    | 134.228  | 6.1%       | Hybrid G1+G2  |
| 14   | 62.898   | 2.8%       | Full pipeline |
| 15   | 22.418   | 1.0%       | Full pipeline |
| 4,7,10,11,5 | 58.696 | 2.7% | Vários       |

O slot 12 é o mais ativo com folga. Quase 40% de todas as execuções.
850 mil vezes em 15 minutos — ~947 vezes por segundo. É o callback que
faz o trabalho essencial, provavelmente associado ao personagem
principal (Ico).

O slot 1 e o slot 2 (leaf pos/rot) somam 36%. São as funções mais
simples — atualizar posição e rotação de objetos. Devem estar ligados
a objetos secundários que só precisam de transformação básica.

Os slots full pipeline (12, 14, 15) somam 42.3%. São os mais pesados —
fazem transformação, físicas, utilitários.

**As grandes revelações:**

**Slot 0: morto.** Zero execuções. O callback `0x00168DA8` simplesmente
nunca é selecionado. Diferente do slot 1 (que tem filtragem e é ativo),
o slot 0 não tem lógica de seleção — ou é resquício de código, ou é um
slot condicional que nunca foi ativado na minha sessão.

**Alt_impl A/B: zero hits.** `0x00169F80` e `0x0016A058` nunca foram
executados. Eles existem (referências estáticas estão lá), mas o runtime
nunca passa por eles. São código para pipeline VU0 DMA — provavelmente
um vestígio de desenvolvimento ou um caminho para situações específicas
(não ocorridas na sessão).

**VU0 kick 0x117C40: não é um kick.** É um utilitário de modo macro
COP2. A função em `0x00117768` não contém instruções VU0. É uma fila
encadeada de processamento deferido. Outro nome que eu tinha dado
errado.

**GP = 0x006388F0 confirmado.** Essa validação sozinha valeu noites de
trabalho. O GP (Global Pointer) é a âncora de todas as variáveis
globais do jogo. Quando você faz análise estática e encontra uma
instrução como `lw $v0, -0x49B4($gp)`, você não sabe o valor de `$gp`
— você assume, baseado em contexto, que é um valor específico. Se seu
chute estiver errado, TODOS os offsets de variáveis globais que você
mapeou estão errados. Confirmar o GP significa confirmar o mapa inteiro
de variáveis. Exceto por uma exceção: o scene loader (kanban.c) usa
GP = 0x0027A7A8. É uma unidade de compilação separada, compilada com
suas próprias configurações de otimização, e portanto tem seu próprio
contexto global. Um detalhe que eu suspeitava mas não tinha provas.

**Match rate: 58%.** Esse número foi o mais honesto de toda a noite.
58% dos eventos correspondiam exatamente ao que eu previa estaticamente.
Para cada chamada de callback, eu sabia qual slot, qual tier, qual
contexto. Os 42% restantes incluíam comportamentos que eu não tinha
antecipado — slots que eu pensava serem ativos mas nunca executavam,
callbacks que eu não tinha mapeado, e padrões condicionais que só
aparecem quando o jogo está em um estado específico (cutscene, combate,
transição). 58% é um número que não permite arrogância. É um lembrete
de que o modelo está no caminho certo, mas incompleto.

A noite teve momentos de pura frustração também. Eu esperava que certos
breakpoints nunca disparassem — e eles nunca dispararam mesmo, o que
era bom. Mas também esperava que outros disparassem com frequência — e
alguns deles ficaram mudos. Cad aver silêncio desses era uma hipótese
que caía por terra. Eu olhava para o log, via zero hits para um
endereço que eu tinha certeza que seria chamado, e precisava revisar
meu modelo mental.

Passei a noite processando o log de 4.5 GB, extraindo distribuições,
montando tabelas em Python, comparando frequências com minhas anotações
estáticas. Quando o sol nasceu e o café tinha acabado, eu tinha algo
que não tinha tido em nenhum momento do projeto até então: **validação
empírica**.

Meu modelo estava parcialmente certo. Mas também parcialmente errado. E
errar com dados de runtime é o melhor tipo de erro, porque o erro vem
com a correção embutida. Cada discrepância entre previsão e realidade
aponta diretamente para o que precisa ser ajustado. Não é especulação.
É dado.

---

## 10. O resolvedor que não era ROPE — BARREL

Das muitas histórias de correção deste projeto, poucas são tão
instrutivas quanto a do "ROPE callback registration gap".

A função `0x0013f7a8` é uma função de registro de callback. Ela associa
um callback (um ponteiro de função) a um slot do sistema de entidades.
Eu encontrei 5 lugares no código que a chamam — os 5 callsites.

Um desses callsites, em tese, deveria registrar o callback `0x001d3a30`,
que por sua vez chama o dispatcher `0x001d37c8` (o de cloth animation).

Mas nenhum dos 5 callsites referenciava `0x001d3a30` diretamente.

Três callsites foram excluídos por contexto: estavam em funções de
inicialização de outros sistemas completamente diferentes (HUD, camera,
UI). Dois callsites restavam como candidatos, mas eu não conseguia
provar que eles registravam especificamente `0x001d3a30`.

Era o "gap": uma função que era chamada em runtime (confirmado por
breakpoint no PCSX2), que entrava no dispatcher, mas cujo registro eu
não conseguia rastrear estaticamente. Por meses, esse gap foi o meu
maior obstáculo.

A resolução veio de onde eu menos esperava: a **descriptor table**.

Durante semanas, eu tratei a descriptor table como um catálogo de
tipos de entidade — útil para saber o que existe, mas não essencial
para entender o fluxo de chamadas. Eu listava os campos mecanicamente:
`init_fn` em +0x30, `cb_routine1` em +0x48, `cb_routine2` em +0x50,
`cb_routine3` em +0x58, `cb_routine4` em +0x60 (que na verdade era o
construtor, mas isso é outra história). Eu sabia que esses campos
existiam. Mas não tinha conectado os pontos.

Até que eu resolvi fazer o exercício completo: listar
sistematicamente o `cb_routine2` de todos os 68 tipos de entidade. Criei
uma tabela com type_id, nome provisório, e endereço de cada cb_routine.
Quando cheguei na entrada que eu identificava como BARREL (type_id
0x23), o campo `cb_routine2` apontava para **`0x001d3a30`**.

Na hora, eu parei. Puxei a cadeira para perto da mesa. Verifiquei três
vezes. Peguei o hex dump da entrada da tabela, calculei o offset,
confirmei que o valor estava correto. `0x001d3a30` estava ali, no campo
+0x50 da entrada BARREL da descriptor table.

A função `0x001d3a30` não é registrada via `0x0013f7a8`.

A função `0x001d3a30` não é registrada via `0x0013f7a8`. Ela é
registrada **via tabela de descritores**. Quando um barril é instanciado
no mundo, o construtor do tipo BARREL lê o descritor, pega o callback em
+0x50, e associa ao slot apropriado do live dispatch.

Não era ROPE. Era BARREL. Physics constraint solver do barril.

A correção veio de um nome que o ICO-decomp confirmou: o range de
`clothAnimation.c` inclui tanto `0x001d37c8` (cloth dispatcher) quanto
`0x001d3a30`. Mas essa não é uma função de tecido — é uma função de
**física de constraint**, que resolve a interação entre o barril (corpo
rígido) e o ambiente (colisão com o chão, paredes, o próprio Ico).

Por meses eu procurei `0x001d3a30` em callsites de `0x0013f7a8`, quando
o registro dela acontecia por um caminho completamente diferente. Se eu
tivesse confiado apenas na análise dos 5 callsites, nunca teria
resolvido. Foi preciso uma visão mais ampla — cruzar a tabela de
descritores, o ICO-decomp, e o contexto de chamada — para entender.

Essa história consolidou duas lições:

1. **Tabelas de dados são tão importantes quanto código.** O código diz
   o que acontece. As tabelas dizem o que existe. Ignorar uma delas
   deixa cego.

2. **Fontes externas de verdade salvam meses de especulação.** O
   ICO-decomp não é perfeito, mas ter um nome verificado para uma função
   muda completamente a qualidade da análise. Um símbolo real vale mais
   que dez intuições.

---

## 11. O mapa que construímos — tudo que foi descoberto

Depois de meses de trabalho, eis o estado do conhecimento.

**Main loop (0x00101C80)**: 12 passos por frame, incluindo dois passos
de dispatch de callbacks (passo 7 e passo 9).

**Dois sistemas de entidades independentes:**

1. **Callback register** (`0x0013F7A8`, hall de entrada em `0x0022B1B8`):
   48 registros, 52 objetos de cena, 28 init_fn. Inclui HUD (10 funções
   em `0x236xxx`), UI/menu (9%), cloth/physics aux (5%), efeitos ambientais
   (3%), objetos de cenário diversos (60%).

2. **Live dispatch** (`0x00166E10`, tabela em `0x00282690`): 17 slots ×
   16 bytes, callbacks em 3 tiers. 8 entidades principais.

**Tabela de descritores** (`0x002A31B8`): 68 tipos de entidade, stride
0x64 (100 bytes). Nome, model_id, massa, raio, threshold, type_id,
init_fn (só 12/68 têm), 3-4 cb_routines, construtor (+0x60).

**Tabela de entrada** (`0x002A4C48`): 512 entradas × 0x4C bytes.
Posições de spawn, escalas, class IDs, subtipos. Quem, onde, como.

**Sistema de máscaras** (`mask_set` 0x0013ED40): escreve em GP+0x6724
(`0x006321CC`). Só bit 0 é usado. É o ShockRequestBox_RequestCancel.
Usado para pausar callbacks durante carregamento de cena.

**Tabela de 404 bytes** (`0x005F2F98`): originalmente confundida com
tabela de entidades. É uma **configuração de sala/room**, indexada por
world_state. Cada sala do castelo tem uma entrada aqui.

**Tabela de halfwords** (`0x006AB080`): uma **hash espacial 32×32**.
Reconstruída a cada ciclo de dispatch. Usada para consultas rápidas de
"quem está perto de quem". Compacta, eficiente, brilhante.

**Tabela de debug** (`0x00613E00`): 47 entradas de debug visualization,
cada uma com 28 bytes. Isso inclui informações de debug que os
programadores usavam durante o desenvolvimento — visualizações de
colisão, estados de animação, dados de física. Uma das entradas tem um
callback em `0x00168650`, identificado como CollisionOldProc. É a única
entrada com callback — as outras 46 são provavelmente apenas dados
descritivos. Essa tabela é uma janela para o processo de desenvolvimento
da Team Ico: eles construíram ferramentas de debug robustas, o que
sugere um ambiente de desenvolvimento estruturado, possivelmente com
uma engine que suportava hot-reload de dados de debug.

**Tabela de efeitos ambientais** (`0x0029A640`): 7 tipos × 0x30 bytes.
Cada entrada descreve uma região de gatilho espacial — uma área do mapa
que dispara um efeito quando uma entidade entra nela. Os tipos prováveis:
vento, partículas, mudança de iluminação, triggers de áudio, ativação de
inimigos, mudança de câmera. O fato de haver apenas 7 tipos sugere que
ICO usa um número limitado de efeitos ambientais, o que é consistente
com o design minimalista do jogo.

**Três grupos de vtable**: (1) personagens principais em `0x00202A60`,
(2) props de física em `0x0023D660`, (3) específicos por entidade. As
vtables são tabelas de ponteiros de função que implementam polimorfismo
— cada tipo de entidade pode ter sua própria implementação de métodos
como "iniciar", "atualizar", "destruir". A existência de TRÊS grupos de
vtable separados sugere que o código de ICO tem pelo menos três
hierarquias de classes diferentes, compiladas separadamente e depois
linkadas no mesmo executável.

**Contador VBlank** (`0x00274EC0`): incrementa a cada frame. Mas
**nenhuma instrução em todo o .text o incrementa**. O incremento vem
de algum lugar — provavelmente VBlank ISR em modo kernel, ou IOP. É um
dos poucos mistérios irresolvidos da análise estática.

**GP map**: `0x006388F0` (confirmado). 1032 offsets únicos, 9971
referências. O mais referenciado é GP-0x49B4 (`0x00633F3C`), com 434
referências — propósito ainda desconhecido. A segunda região mais densa
é GP+0x6F60 (`0x00631990`), provavelmente world_state_main.

**HUD init functions** em `0x236xxx`: 10 funções, 2 padrões. Criam
estruturas de HUD em `0x4Dxxxx`. Resource IDs 0x33-0x3C.

**VU0 "kick"**: a função em `0x00117768` não contém instruções VU0.
É uma fila encadeada de processamento deferido. A função `0x00117C40`
é um utilitário COP2 macro-mode. Nada disso é um "VU0 kick" real.

**Barrel physics** (`0x001D3A30`): 128 instruções, resolvedor de
constraint de 5 estados (0x001D37C8). Estados: repouso, força externa,
colisão, constraint solve, pós-processamento.

---

## 12. O que não investigamos (por decisão) — o escopo deliberado

Em todo projeto de arqueologia, você precisa escolher o que escavar. Não
dá para cavar o sítio inteiro de uma vez.

Existem áreas de ICO que eu **deliberadamente** não investiguei e
continuo sem investigar:

- **DATA.DF**: archive de 539 MB com entropia alta. Investigá-lo
  significaria meses de análise de formato, compressão, extração. Sem
  acesso a runtime tracing avançado, não há garantia de sucesso.
- **Arquivos .gcm**: câmera/cutscene/demo. São arquivos de dados de
  câmera para cutscenes. Interessantes, mas desconectados do sistema de
  entidades.
- **Texturas TM2**: o formato de textura do PS2. A frente da UI de
  morte mostrou que troca cega de strings de textura não funciona.
- **Strings de gameplay**: Yorda, capture, shadow, continue. Strings são
  evidência fraca sem xrefs verificadas.
- **Overlays DVP**: extração de overlay é um projeto em si. Requer
  entender o formato DVP, o linker, o carregador.
- **ISO scanning amplo**: buscar dados no ISO sem um alvo específico
  é ineficiente.

Por que essas áreas foram excluídas?

Porque o escopo do projeto era **entender o sistema de entidades e
dispatch de ICO**. Não era "entender tudo sobre ICO". Um projeto com
escopo infinito nunca termina. Um projeto com escopo finito pode
produzir resultados reais e auditáveis.

DATA.DF é umarchive de 500+ MB. Investigá-lo significaria semanas ou
meses de análise de formato de archive — compressão, checksums, tabelas
de alocação — sem nenhuma garantia de que isso ajudaria a entender o
sistema de entidades. É uma frente paralela, não um pré-requisito.

As strings de gameplay (Yorda, capture, shadow) são evidência fraca. Uma
string pode estar em qualquer lugar do binário e significar qualquer
coisa. "Yorda" pode ser um nome de textura, de arquivo de áudio, de
configuração de IA, de modelo 3D, de legenda. Sem xrefs confiáveis para
código, uma string é apenas uma inscrição sem contexto.

A tela de morte — que me consumiu semanas e me ensinou mais do que
qualquer descoberta "positiva" — provavelmente está em texturas TM2
dentro do DATA.DF, ou em overlays de UI carregados sob demanda, ou em
algum mecanismo de streaming que depende de estado específico de jogo.
O que eu aprendi com essa busca não foi insignificante: aprendi que
engenharia reversa não é sobre ter razão, é sobre formular hipóteses
testáveis. Aprendi que uma hipótese falsa, quando bem documentada, é
mais valiosa que uma conclusão verdadeira sem evidência.

Essas áreas não são menos importantes que as que investiguei. São
apenas **outro escopo**. Fazer tudo ao mesmo tempo é garantia de não
terminar nada. Se o projeto continuar em uma segunda fase, DATA.DF,
overlays, .gcm e strings de gameplay serão a pauta. Mas não agora.
A prioridade era o sistema de dispatch. E esse sistema foi entregue.

A prioridade era o sistema de dispatch. E esse sistema está mapeado. Não
perfeitamente, não completamente — mas funcionalmente. Dá para olhar
para ICO rodando e saber, em linhas gerais, o que está acontecendo em
cada frame. Dá para nomear cada passo do main loop. Dá para listar os
tipos de entidade, seus callbacks, seus slots.

Isso já é mais do que eu tinha quando comecei.

---

## Epílogo — A escavação continua

A análise estática está completa.

Não no sentido de que eu descobri tudo — longe disso. Mas no sentido de
que tudo que podia ser descoberto **sem execução do jogo em ambiente
controlado** foi descoberto.

O que resta agora não é mais análise. É validação:

1. O bit 0 da máscara (ShockRequestBox_RequestCancel) toggla durante
   transições de cutscene? Ou só no loading?
2. O `world_state_main` (GP+0x6F60, `0x00631990`) muda durante transições
   de sala? Dá para mapear world_state → nome de sala?
3. A halfword table 0x6AB080 — os valores escritos são realmente índices
   de tipo de entidade, formando uma hash espacial?
4. GP-0x49B4 (0x00633F3C) — com 434 referências, é a variável mais
   acessada do jogo inteiro. O que ela é?
5. Slot 0 callback 0x168DA8 — em que circunstância (se alguma) ele é
   selecionado?
6. O contador 0x274EC0 — que EPC o incrementa? VBlank ISR? IOP?

Todas essas perguntas exigem PCSX2. Exigem breakpoints bem colocados,
logpoints, watches de memória. Não exigem mais meses de análise estática.

Isso me coloca numa posição estranha. O ritmo do projeto mudou. Durante
meses, cada semana trazia uma nova descoberta: uma nova tabela, uma nova
função, uma nova relação entre sistemas. Agora o ritmo é outro. A fase
de exploração terminou. A fase de escavação fina começou.

Não sei quanto tempo vai levar. Pode ser um mês, pode ser um ano. Depende
de quando eu conseguir sentar com o PCSX2 e fazer as perguntas certas nos
lugares certos.

E é estranho, também, o que esse projeto fez com minha relação com ICO.

Antes, ICO era um jogo que eu amava mas não entendia. Era uma caixa preta
que produzia emoção. Agora é um sistema que eu posso descrever: tem 12
passos por frame, 17 slots de callback, 68 tipos de entidade, dois
sistemas paralelos de dispatch.

Mas isso não diminuiu o jogo para mim. Pelo contrário.

Quando eu vejo Ico andando pelo castelo agora, eu não vejo menos —
vejo mais. Vejo o main loop girando 60 vezes por segundo. Vejo o slot 12
sendo chamado 947 vezes a cada segundo. Vejo a tabela de descritores
sendo consultada, o callback sendo resolvido, a física sendo calculada,
a transformação sendo aplicada.

A arte não está no código. Mas o código é o suporte físico da arte.
Alguém sentou numa mesa há mais de vinte anos, escreveu aquelas
instruções, fez escolhas entre alternativas técnicas, debateu tradeoffs
de desempenho e memória, e criou, byte por byte, instrução por instrução,
o jogo que me acompanha desde os dez anos.

Agora eu consigo ler essas escolhas. Pelo menos em parte. E isso não
tira a magia — acrescenta uma camada de compreensão que eu não tinha
antes.

O castelo de ICO não existe. É um monte de polígonos, texturas e
instruções MIPS executando em um hardware de vinte anos atrás.

Mas quando eu olho para o código, eu vejo o castelo.

Não pela magia. Pela engenharia.

A escavação continua. Há muito mais abaixo da superfície. DATA.DF me
espera. Os overlays DVP me esperam. A tela de morte ainda não foi
encontrada. E eu ainda tenho as mãos sujas de terra.
