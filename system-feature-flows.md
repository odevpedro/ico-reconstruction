# system-feature-flows.md — ICO Reconstruction

> Registro histórico das funcionalidades e seus fluxos internos.
> Nunca apague entradas anteriores. Sempre adicione novas ao final.
> Formato de cabeçalho obrigatório por entrada:
> Squad responsável | Revisão | Status

---

# Feature: Retarget do Projeto para ICO Reconstruction

> Squad responsável: SQUAD-ARCH
> Revisão: rev.002
> Sessão: 2026-05-12
> Status: Estável

## Resumo
O projeto foi reorientado de uma reconstrução genérica inspirada por OpenGOAL para um esforço específico sobre **ICO** de PlayStation 2. OpenGOAL permanece como referência metodológica de reconstrução, validação incremental, tooling e runtime moderno, mas deixa de ser tratado como arquitetura transferível.

## Fluxo principal

### 1. Ponto de entrada
O PO definiu ICO como alvo técnico principal por ser um jogo de início de geração do PS2, com escopo mais contido que muitos AAA tardios.

### 2. Validação de entrada
A documentação foi revisada para remover o alvo anterior e introduzir premissas específicas de ICO: ISO/ELF, assets, salas, câmera, actor system, colisão, animação, Yorda/IA de companheira, eventos e rendering.

### 3. Orquestração da aplicação
O backlog mantém a análise A-D como ponto de entrada, seguida por E-G, proof of concept e detalhamento por squads.

### 4. Regras de negócio
Toda análise futura deve distinguir evidência técnica observável de inferência. Decisões não validadas contra a ISO ou binário de ICO devem ser marcadas como "sem referência validada".

### 5. Persistência / Integrações
Foram atualizados `backlog.md`, `decisoes-iniciais.md`, `prompt-A-D.md`, `prompt-E-G.md`, `fases-2-4.md` e este registro.

### 6. Resposta final
O estado oficial do projeto passa a ser **ICO Reconstruction**, com OpenGOAL mantido apenas como comparação metodológica.

## Fluxos alternativos e erros
Se a triagem da ISO de ICO revelar acoplamento impeditivo, ausência de pontos de intervenção ou formatos opacos demais, o fluxo recomenda executar uma PoC mínima antes de detalhar todos os squads.

## Decisões técnicas importantes
- Tratar ICO como alvo principal e único da análise inicial.
- Priorizar evidência empírica do binário, assets e layout da ISO.
- Separar port/reconstrução de modding e de emulação.
- Usar OpenGOAL como referência de processo, não como base técnica reutilizável.

# Feature: README Público Inicial

> Squad responsável: SQUAD-ARCH
> Revisão: rev.003
> Sessão: 2026-05-12
> Status: Estável

## Resumo
Foi criado um README inicial para disponibilizar o projeto no GitHub e atrair colaboração comunitária sem confundir o escopo técnico com distribuição de jogo, assets ou binários.

## Fluxo principal

### 1. Ponto de entrada
O PO solicitou uma apresentação pública do projeto para encontrar ajuda da comunidade.

### 2. Validação de entrada
O README foi estruturado para explicar o objetivo de reconstrução, o estado inicial do projeto, os limites legais e as áreas onde colaboradores podem ajudar.

### 3. Orquestração da aplicação
O arquivo `README.md` passa a ser a porta de entrada pública. Os arquivos operacionais continuam responsáveis pelo backlog, decisões, prompts e registros de fluxo.

### 4. Regras de negócio
O README reforça que o repositório não deve conter ISO, binários, assets, código proprietário ou dados extraídos do jogo.

### 5. Persistência / Integrações
Foram atualizados `README.md`, `backlog.md`, `prompt-A-D.md`, `prompt-E-G.md` e `system-feature-flows.md`.

### 6. Resposta final
O projeto agora possui uma descrição pública inicial adequada para GitHub, com foco em colaboração técnica e pesquisa documentada.

## Fluxos alternativos e erros
Se futuras ferramentas forem adicionadas, o README deve ser revisado para explicar requisitos, uso e limites de entrada sem incluir dados proprietários.

## Decisões técnicas importantes
- O README deve declarar explicitamente que OpenGOAL é inspiração metodológica, não base técnica.
- A colaboração deve priorizar pesquisa, documentação, tooling e triagem técnica.
- Qualquer trabalho futuro deve exigir cópia local fornecida pelo usuário quando depender de dados do jogo.

# Feature: Mescla do README com Template Público

> Squad responsável: SQUAD-ARCH
> Revisão: rev.004
> Sessão: 2026-05-12
> Status: Estável

## Resumo
O README público foi mesclado com a estrutura do template em `/home/peter/Documentos/repos/claude-config/README-template.md`, preservando o conteúdo já escrito e adaptando o formato para um projeto de pesquisa/documentação.

## Fluxo principal

### 1. Ponto de entrada
O PO solicitou que o conteúdo do diretório `claude-config` fosse considerado e mesclado no README atual, ignorando a parte de banco de dados sem relação com ICO Reconstruction.

### 2. Validação de entrada
Foram consultados `README-template.md`, `CLAUDE.md`, `backlog-template.md` e `system-feature-flows-template.md`. O template de modelo de dados foi ignorado por não se aplicar ao projeto.

### 3. Orquestração da aplicação
O README recebeu seções de template adaptadas: badges, stack e arquitetura, estrutura de pastas, setup local, testes, documentação, status, contribuição e licença.

### 4. Regras de negócio
Nenhuma seção existente foi removida. Conteúdo de banco de dados, API backend, ORM, autenticação e endpoints foi excluído por não representar o projeto.

### 5. Persistência / Integrações
Foram atualizados `README.md`, `backlog.md`, `prompt-A-D.md`, `prompt-E-G.md` e `system-feature-flows.md`.

### 6. Resposta final
O README agora segue uma estrutura pública mais completa sem inventar uma aplicação, banco de dados ou runtime inexistente.

## Fluxos alternativos e erros
Se o projeto ganhar ferramentas executáveis, a seção de setup e testes deve ser revisada com comandos reais.

## Decisões técnicas importantes
- Preservar integralmente o conteúdo anterior do README.
- Adaptar o template para documentação e pesquisa, não para API/backend.
- Declarar explicitamente que não há banco de dados, ORM, autenticação ou serviço web no estado atual.

# Feature: Wallpaper de ICO no README

> Squad responsável: SQUAD-ARCH
> Revisão: rev.005
> Sessão: 2026-05-12
> Status: Estável

## Resumo
Foi adicionado um wallpaper de ICO ao topo do README para melhorar a apresentação pública do repositório no GitHub.

## Fluxo principal

### 1. Ponto de entrada
O PO informou que o wallpaper estava em `/home/peter/Músicas/` como arquivo WebP.

### 2. Validação de entrada
O arquivo `/home/peter/Músicas/ico.webp` foi identificado como imagem WebP de 1080x607.

### 3. Orquestração da aplicação
A imagem foi copiada para `assets/ico-wallpaper.webp` e referenciada no topo de `README.md` com caminho relativo.

### 4. Regras de negócio
O README continua declarando que o repositório não deve conter ISO, binários, assets extraídos do jogo ou dados proprietários. A imagem é tratada como asset público de apresentação fornecido pelo PO.

### 5. Persistência / Integrações
Foram atualizados `README.md`, `backlog.md`, `prompt-A-D.md`, `prompt-E-G.md` e `system-feature-flows.md`; o arquivo `assets/ico-wallpaper.webp` foi criado.

### 6. Resposta final
O README agora exibe o wallpaper de ICO no topo quando renderizado no GitHub.

## Fluxos alternativos e erros
Se o arquivo de imagem precisar ser substituído por arte original ou material com licença explícita, basta trocar `assets/ico-wallpaper.webp` mantendo o mesmo caminho no README.

## Decisões técnicas importantes
- Guardar a imagem dentro do repositório para evitar dependência de caminho local.
- Usar caminho relativo `./assets/ico-wallpaper.webp` para compatibilidade com GitHub.

# Feature: Estrutura Mínima de Pastas para GitHub

> Squad responsável: SQUAD-ARCH
> Revisão: rev.006
> Sessão: 2026-05-12
> Status: Estável

## Resumo
Foi criada uma estrutura mínima de diretórios para publicar o projeto no GitHub sem sugerir que já existe uma implementação funcional.

## Fluxo principal

### 1. Ponto de entrada
O PO solicitou uma estrutura de pastas bem mínima para subir o projeto no GitHub.

### 2. Validação de entrada
A estrutura atual continha apenas arquivos Markdown de planejamento e a pasta `assets/` com o wallpaper.

### 3. Orquestração da aplicação
Foram criados diretórios vazios versionáveis com `.gitkeep`: `docs/`, `research/`, `tools/`, `tests/`, `tests/fixtures/` e `.github/ISSUE_TEMPLATE/`.

### 4. Regras de negócio
A estrutura não inclui código, binários, assets extraídos de jogo, ISO, fixtures proprietárias ou ferramentas especulativas.

### 5. Persistência / Integrações
Foram atualizados `README.md`, `backlog.md`, `prompt-A-D.md`, `prompt-E-G.md` e `system-feature-flows.md`.

### 6. Resposta final
O repositório agora tem uma base mínima para organização pública: documentação, pesquisa, tooling futuro, testes futuros e templates de issue.

## Fluxos alternativos e erros
Se forem adicionadas ferramentas reais, a pasta `tools/` deve receber README específico com comandos e limites legais.

## Decisões técnicas importantes
- Usar `.gitkeep` para versionar diretórios ainda vazios.
- Manter a estrutura pequena para não prometer módulos inexistentes.
- Reservar `tests/fixtures/` apenas para fixtures sintéticas ou não protegidas por copyright.

# Feature: Limpeza Documental e Guias Base de Pesquisa

> Squad responsável: SQUAD-ARCH
> Revisão: rev.006.1
> Sessão: 2026-05-12
> Status: Estável

## Resumo
A documentação pública foi alinhada com o estado real do repositório. A referência futura e inadequada a `docs/data-model.md` foi removida, e foram adicionados guias base para metodologia de pesquisa, limites legais, direção de tooling, notas de pesquisa e ferramentas futuras.

## Fluxo principal

### 1. Ponto de entrada
O PO solicitou a limpeza após revisar os próximos passos recomendados para o projeto.

### 2. Validação de entrada
Foi confirmado que o repositório continua documentation-first, sem aplicação executável, runtime, banco de dados, API, source port ou tooling implementado.

### 3. Orquestração da aplicação
O README agora aponta para documentos existentes e para os novos guias base. O backlog registra a limpeza como revisão intermediária sem consumir a `rev.007`, que permanece reservada para a análise arquitetural A-D.

### 4. Regras de negócio
A documentação continua proibindo commit de ISO, binários, assets extraídos, código proprietário ou dados protegidos do jogo. Tooling futuro deve operar sobre cópia local fornecida pelo usuário.

### 5. Persistência / Integrações
Foram atualizados `README.md`, `docs/backlog.md`, `docs/architecture-log.md` e este registro. Foram adicionados `docs/research-methodology.md`, `docs/legal-boundaries.md`, `docs/tooling-plan.md`, `research/README.md` e `tools/README.md`.

### 6. Resposta final
O repositório agora tem uma estrutura pública mais limpa e documentação base suficiente para sustentar a próxima etapa de arquitetura.

## Fluxos alternativos e erros
Se o projeto introduzir ferramentas executáveis reais, o README e `docs/tooling-plan.md` devem ser revisados com comandos de setup, caminhos de saída gerados e instruções de teste.

## Decisões técnicas importantes
- Manter a `rev.007` atribuída à análise arquitetural A-D.
- Substituir a referência inutilizada a modelo de dados por guias de pesquisa, limites legais e tooling.
- Manter o repositório como documentation-first até existir tooling empírico.

# Feature: Análise Arquitetural A-D

> Squad responsável: SQUAD-ARCH
> Revisão: rev.007
> Sessão: 2026-05-12
> Status: Estável

## Resumo
Foi documentado o primeiro mapa de viabilidade por subsistema para ICO Reconstruction. A análise separa o estado confirmado do repositório de hipóteses e pontos desconhecidos que exigem validação empírica local com cópia própria do jogo.

## Fluxo principal

### 1. Ponto de entrada
O projeto avançou da limpeza documental para a tarefa pendente de arquitetura `rev.007`.

### 2. Validação de entrada
Não há ISO, ELF, executável, asset ou dado extraído do jogo no repositório. Por isso, a análise usa apenas evidência documental e marca afirmações técnicas específicas de ICO como hipóteses ou desconhecidas.

### 3. Orquestração da aplicação
Foi criado `docs/architectural-analysis-a-d.md` com tabela de viabilidade por subsistema, itens validáveis sem binário, itens que exigem teste empírico e caminho técnico recomendado.

### 4. Regras de negócio
O caminho recomendado começa por verificação local metadata-only e indexação da ISO. Ele evita commit de dados proprietários e evita reconstrução de gameplay antes de existirem metadados empíricos.

### 5. Persistência / Integrações
Foram atualizados `README.md`, `docs/backlog.md`, `docs/architecture-log.md` e este registro. Foi adicionado `docs/architectural-analysis-a-d.md`.

### 6. Resposta final
A `rev.007` está concluída. O próximo passo prático é setup de ambiente ou implementação de verificador local metadata-only antes de aprofundar afirmações por subsistema.

## Fluxos alternativos e erros
Se a inspeção empírica de ISO/ELF contradisser premissas da análise A-D, o relatório deve ser revisado com observações confirmadas e alteração do nível de evidência.

## Decisões técnicas importantes
- Tratar viabilidade de reconstrução/source port como não comprovada até inspeção local de metadados da ISO e executável.
- Priorizar tooling metadata-only antes de extração de assets ou PoCs de mutação.
- Manter a `rev.008` limitada pelas evidências da `rev.007` e de tooling local futuro.

# Feature: Fundação de Tooling Metadata-Only Local

> Squad responsável: SQUAD-TOOLING
> Revisão: rev.007.1
> Sessão: 2026-05-12
> Status: Estável

## Resumo
O repositório recebeu a primeira fundação prática de tooling: regras de segurança no `.gitignore`, templates de issue no GitHub e um verificador metadata-only para entradas locais fornecidas pelo usuário.

## Fluxo principal

### 1. Ponto de entrada
O PO solicitou executar de uma vez os próximos passos práticos: ignorar outputs locais, criar templates de issue e implementar o primeiro verificador metadata-only.

### 2. Validação de entrada
Nenhum dado real de ICO foi usado. Foi adicionada uma fixture sintética em `tests/fixtures/` para validar o verificador sem conteúdo proprietário.

### 3. Orquestração da aplicação
O verificador aceita arquivo ou diretório local, registra metadados e hashes SHA-256 opcionais, e grava um relatório JSON em `.local/reports/`, pasta ignorada pelo git.

### 4. Regras de negócio
A ferramenta não copia, extrai, modifica, decodifica ou redistribui dados do jogo. Relatórios são locais por padrão e qualquer metadado de uma cópia real deve ser revisado antes de commit.

### 5. Persistência / Integrações
Foram adicionados `.gitignore`, templates em `.github/ISSUE_TEMPLATE/`, `tools/verify-local-copy/` e `tests/fixtures/metadata-sample.txt`. Foram atualizados README, documentação de tooling, backlog, `docs/architecture-log.md` e este registro.

### 6. Resposta final
O projeto agora possui a primeira ferramenta local segura de verificação e templates de colaboração. O setup completo de runtime/disassembly permanece pendente.

## Fluxos alternativos e erros
Se um diretório local tiver arquivos demais, o verificador recusa a varredura acima de `--max-files` até o usuário aumentar explicitamente esse limite. O usuário pode usar `--no-hash` para evitar hashing custoso.

## Decisões técnicas importantes
- Manter relatórios gerados em `.local/reports/` e ignorados pelo git.
- Usar apenas biblioteca padrão do Python na primeira ferramenta.
- Tratar o verificador como fundação metadata-only, não como parser ou extrator de ISO.

# Feature: Índice Local BIN/CUE e Observação Inicial

> Squad responsável: SQUAD-TOOLING
> Revisão: rev.007.2
> Sessão: 2026-05-12
> Status: Estável

## Resumo
Foi adicionado um indexador ISO9660/BIN/CUE metadata-only e usado contra a imagem local BIN/CUE de ICO USA fornecida pelo usuário. O projeto agora tem sua primeira observação confirmada de layout de disco sem copiar ou extrair arquivos proprietários para o repositório.

## Fluxo principal

### 1. Ponto de entrada
O PO forneceu o caminho local `/home/peter/Downloads/Ico (USA)/` e esclareceu que o jogo está em CD-ROM BIN/CUE, não em DVD ISO.

### 2. Validação de entrada
O diretório local contém `Ico (USA).bin` e `Ico (USA).cue`. O CUE declara `TRACK 01 MODE2/2352`. Os arquivos permaneceram fora do repositório.

### 3. Orquestração da aplicação
`tools/verify-local-copy/` gerou relatório local de metadados com hashes. `tools/iso-index/` detectou layout bruto de setor 2352 bytes com offset de dados 24, gerou índice ISO9660 metadata-only local e analisou metadados básicos do cabeçalho ELF do candidato a executável.

### 4. Regras de negócio
Nenhum conteúdo do jogo foi copiado, extraído, decodificado, modificado ou commitado. Apenas metadados seguros, nomes de arquivo, tamanhos, LBAs, hashes e notas interpretativas foram registrados.

### 5. Persistência / Integrações
Foram adicionados `tools/iso-index/` e `research/iso-layout/ico-usa-bin-cue-initial-index.md`. Foram atualizados README, documentação de tooling, backlog, `docs/architecture-log.md` e este registro.

### 6. Resposta final
O projeto agora possui metadados locais confirmados do disco: candidato a executável principal `SCUS_971.13`, metadados de cabeçalho ELF32, candidato a arquivo principal de dados/archive `DFDATAS/DATA.DF` e detalhes do layout BIN/CUE.

## Fluxos alternativos e erros
Se outro dump regional usar layout de setor diferente, `iso-index` deve detectar o Primary Volume Descriptor com um dos layouts suportados ou ser estendido com outro candidato de layout.

## Decisões técnicas importantes
- Tratar ICO USA como workflow CD-ROM BIN/CUE para a análise local atual.
- Manter JSONs gerados em `.local/reports/` e commitar apenas resumos de metadados revisados.
- Usar metadados do disco para guiar o próximo passo: inspeção metadata-only dos program/section headers do ELF e triagem estrutural de `DATA.DF`.

# Feature: Índice Local de Metadados ELF e Observação Inicial

> Squad responsável: SQUAD-TOOLING
> Revisão: rev.007.3
> Sessão: 2026-05-12
> Status: Estável

## Resumo
Foi adicionado um indexador ELF32 metadata-only e usado contra o executável embutido `SCUS_971.13` da imagem local BIN/CUE de ICO USA. O projeto agora possui metadados confirmados de layout do executável sem extrair ou commitar o executável.

## Fluxo principal

### 1. Ponto de entrada
O índice de disco anterior identificou `SCUS_971.13` no LBA 25 com tamanho de 5.481.608 bytes.

### 2. Validação de entrada
O executável foi lido da imagem BIN local usando setor 2352 e offset de dados 24. Nenhuma cópia do executável foi gravada no repositório.

### 3. Orquestração da aplicação
`tools/elf-index/` analisou cabeçalho ELF, program headers, section headers, nomes de seções e presença de tabelas de símbolos em memória, depois gravou relatório JSON local em `.local/reports/`.

### 4. Regras de negócio
O relatório é metadata-only. Ele não inclui bytes do executável, disassembly, corpos de função, código proprietário copiado ou conteúdo extraído do jogo.

### 5. Persistência / Integrações
Foram adicionados `tools/elf-index/` e `research/elf/ico-usa-scus-97113-elf-metadata.md`. Foram atualizados README, documentação de tooling, backlog, `docs/architecture-log.md` e este registro.

### 6. Resposta final
Foram confirmados metadados ELF32 little-endian: entry point `0x00100008`, um segmento `PT_LOAD`, 27 section headers, ausência de tabela de símbolos, seções relacionadas a vector unit e seções `.DVP.overlay...` que exigem investigação posterior.

## Fluxos alternativos e erros
Se outro executável regional tiver layout ELF diferente, rodar `elf-index` com LBA e tamanho reportados por `iso-index` para aquele dump e registrar uma observação regional separada.

## Decisões técnicas importantes
- Manter relatórios ELF como metadata-only e locais por padrão.
- Não listar nomes individuais de símbolos; registrar apenas presença e contagens de tabelas de símbolos.
- Usar metadados das seções `.DVP.overlay...` para guiar a próxima investigação sobre overlays e `DFDATAS/DATA.DF`.

# Feature: Triagem Local de Metadados de DATA.DF

> Squad responsável: SQUAD-TOOLING
> Revisão: rev.007.4
> Sessão: 2026-05-12
> Status: Estável

## Resumo
Foi adicionada uma ferramenta metadata-only de triagem de `DATA.DF` e usada contra o candidato a archive embutido `DFDATAS/DATA.DF` da imagem local BIN/CUE de ICO USA.

## Fluxo principal

### 1. Ponto de entrada
O índice de disco identificou `DFDATAS/DATA.DF` no LBA 2.898 com tamanho de 539.367.424 bytes.

### 2. Validação de entrada
O candidato a archive foi lido da imagem BIN local usando setor 2352 e offset de dados 24. Nenhuma cópia do archive ou de conteúdos internos foi gravada no repositório.

### 3. Orquestração da aplicação
`tools/data-df-index/` amostrou janelas de início, meio e fim, calculou metadados de perfil de bytes e procurou no início candidatos simples a tabelas monotônicas de offsets 32-bit e tabelas de registros fixos.

### 4. Regras de negócio
O relatório é metadata-only. Ele não inclui bytes brutos do archive, entradas extraídas, payloads de assets, nomes de arquivos internos ou conteúdo decodificado do jogo.

### 5. Persistência / Integrações
Foram adicionados `tools/data-df-index/` e `research/data-df/ico-usa-data-df-initial-triage.md`. Foram atualizados README, documentação de tooling, backlog, `docs/architecture-log.md` e este registro.

### 6. Resposta final
A triagem inicial encontrou alta entropia no início, entropia menor no meio, fim majoritariamente zero/padding e nenhum candidato simples a tabela de offsets no início sob a heurística atual.

## Fluxos alternativos e erros
Se análise futura do executável ou de overlays identificar offset de tabela fora do primeiro megabyte, `data-df-index` deve ser estendido para escanear offsets direcionados em vez de depender da heurística genérica do início.

## Decisões técnicas importantes
- Manter análise de archive como metadata-only até o formato ser entendido.
- Evitar extrair ou nomear entradas internas até definir uma representação limpa e segura.
- Usar metadados ELF `.DVP.*` e referências do executável para guiar a próxima passada direcionada em `DATA.DF`.

# Feature: Correlação Local de Metadados de Overlay DVP

> Squad responsável: SQUAD-TOOLING
> Revisão: rev.007.5
> Sessão: 2026-05-12
> Status: Estável

## Resumo
Foi adicionado um indexador metadata-only de overlays `.DVP.*` e usado contra o executável embutido `SCUS_971.13`. A ferramenta correlaciona metadados das seções DVP com o range carregado do ELF e o contexto de tamanho conhecido de `DATA.DF`.

## Fluxo principal

### 1. Ponto de entrada
A análise ELF anterior identificou `.DVP.ovlytab`, `.DVP.ovlystrtab` e 12 seções `.DVP.overlay...`.

### 2. Validação de entrada
O executável foi lido da imagem BIN local usando setor 2352 e offset de dados 24. `DATA.DF` foi usado apenas como contexto de tamanho. Nenhum byte do executável, payload de overlay ou conteúdo de archive foi commitado.

### 3. Orquestração da aplicação
`tools/dvp-index/` analisou metadados das seções `.DVP.*`, contou entradas da string table de overlays, interpretou `.DVP.ovlytab` como entradas de 12 bytes e classificou valores numéricos contra o range carregado do ELF e o tamanho conhecido de `DATA.DF`.

### 4. Regras de negócio
O relatório permanece metadata-only. Ele não inclui bytes brutos do executável, disassembly, conteúdo de overlays, entradas extraídas de archive ou assets decodificados.

### 5. Persistência / Integrações
Foram adicionados `tools/dvp-index/` e `research/dvp/ico-usa-dvp-overlay-metadata.md`. Foram atualizados README, documentação de tooling, backlog, `docs/architecture-log.md` e este registro.

### 6. Resposta final
Foram confirmadas 14 seções `.DVP.*`, 12 seções de overlay, 12 entradas na tabela de overlays e 12 entradas na string table de overlays. A tabela de overlays parece incluir referências de memória ELF, então a interpretação direta como offsets de `DATA.DF` permanece não confirmada.

## Fluxos alternativos e erros
Se scans direcionados futuros de `DATA.DF` validarem algum token numérico de `.DVP.overlay...` como offset de archive, a nota DVP deve ser revisada de hipótese para mapeamento confirmado daquele token.

## Decisões técnicas importantes
- Não tratar valores numéricos como offsets de `DATA.DF` apenas porque cabem dentro do tamanho do archive.
- Comparar valores DVP tanto contra ranges carregados do ELF quanto contra contexto de tamanho de `DATA.DF`.
- Usar tokens numéricos DVP como sementes de busca para scans direcionados de `DATA.DF`, não como entradas confirmadas de tabela.

# Feature: Scans Direcionados de DATA.DF ao Redor de Tokens DVP

> Squad responsável: SQUAD-TOOLING
> Revisão: rev.007.6
> Sessão: 2026-05-12
> Status: Estável

## Resumo
`tools/data-df-index/` foi estendido com scans direcionados repetíveis por offset e usado para inspecionar janelas ao redor de tokens numéricos dos nomes de seções `.DVP.overlay...`.

## Fluxo principal

### 1. Ponto de entrada
A análise de overlays DVP identificou tokens numéricos que poderiam ser usados como sementes de busca em `DATA.DF`.

### 2. Validação de entrada
O scan usou metadados locais de `DFDATAS/DATA.DF` da imagem BIN/CUE de ICO USA. Nenhum byte do archive, entrada extraída ou dado decodificado do jogo foi commitado.

### 3. Orquestração da aplicação
`data-df-index` agora aceita argumentos repetíveis `--target-offset` e escaneia janelas limitadas ao redor de cada candidato. O scan dos tokens DVP usou janelas de 262.144 bytes ao redor de seis offsets candidatos.

### 4. Regras de negócio
O relatório permanece metadata-only e registra apenas entropia, estatísticas de classes de bytes, hashes e contagens de candidatos a tabelas.

### 5. Persistência / Integrações
Foram atualizados `tools/data-df-index/`, README, documentação de tooling, backlog, `docs/architecture-log.md` e este registro. Foi adicionado `research/data-df/ico-usa-data-df-dvp-targeted-scan.md`.

### 6. Resposta final
Nenhuma tabela local simples de offsets ou candidatos a registros fixos foi encontrada ao redor dos tokens numéricos DVP testados. Os tokens DVP continuam úteis como sementes de busca, mas não estão confirmados como offsets de `DATA.DF`.

## Fluxos alternativos e erros
Scans futuros podem ajustar `--target-window-bytes`, adicionar mais offsets ou escanear offsets encontrados por análise de referências do executável.

## Decisões técnicas importantes
- Preservar a triagem genérica de início/meio/fim enquanto adiciona scans direcionados.
- Tratar janelas direcionadas de alta entropia como evidência contra estruturas locais simples de tabela, não como prova de que os offsets não têm significado.
- Mover a próxima investigação para análise de referências do executável em vez de scans cegos mais amplos do archive.

# Feature: Scan de Referências do Executável para DATA.DF e Tokens DVP

> Squad responsável: SQUAD-TOOLING
> Revisão: rev.007.7
> Sessão: 2026-05-12
> Status: Estável

## Resumo
Foi adicionado um scanner metadata-only de referências do executável e usado contra o `SCUS_971.13` embutido para procurar strings e constantes 32-bit exatas relacionadas a `DATA.DF`, metadados DVP e tokens candidatos anteriores.

## Fluxo principal

### 1. Ponto de entrada
Os scans direcionados de `DATA.DF` ao redor de tokens DVP não revelaram tabelas locais simples, então a investigação avançou para referências no executável.

### 2. Validação de entrada
O executável foi lido da imagem BIN local usando setor 2352 e offset de dados 24. Nenhum byte do executável, disassembly ou dump arbitrário de strings foi commitado.

### 3. Orquestração da aplicação
`tools/exe-ref-index/` procura apenas strings e constantes fornecidas pelo usuário, depois registra contagens de matches, offsets de arquivo, endereços virtuais e seções.

### 4. Regras de negócio
O relatório permanece metadata-only e limitado a consultas exatas. Ele não extrai conteúdo do executável, não recupera funções e não faz dump arbitrário de strings.

### 5. Persistência / Integrações
Foram adicionados `tools/exe-ref-index/` e `research/exe-refs/ico-usa-scus-97113-data-df-dvp-references.md`. Foram atualizados README, documentação de tooling, backlog, `docs/architecture-log.md` e este registro.

### 6. Resposta final
Foram confirmadas referências diretas às strings `DATA.DF` e `DFDATAS` em seções de dados do executável. Os tokens numéricos DVP testados não foram encontrados como constantes 32-bit diretas.

## Fluxos alternativos e erros
Constantes podem ser formadas por sequências MIPS de immediates separados, então ausência de constantes 32-bit exatas não prova que o executável não usa esses valores.

## Decisões técnicas importantes
- Procurar apenas strings e constantes explicitamente fornecidas.
- Registrar offsets e seções exatas, não bytes ao redor ou disassembly.
- Mover a próxima investigação para scanning de padrões de immediates/referências MIPS.

# Feature: Setup de Ambiente para Disassembly e Emulação

> Squad responsável: SQUAD-TOOLING
> Revisão: rev.011
> Sessão: 2026-05-12
> Status: Estável

## Resumo
Foi configurado o ambiente para desassembly do executável ICO usando Ghidra em modo headless. O processo envolveu instalação do JDK 21 (necessário para Ghidra 12.x), configuração do Ghidra, extração do ELF e execução bem-sucedida da análise.

## Fluxo principal

### 1. Ponto de entrada
O projeto avanzou para a etapa de disassembly após a análise de call graph identificar funções-chave no executável.

### 2. Validação de entrada
O JDK 17 originally tentado não era suficiente - Ghidra 12.x exige JDK 21. O JDK 21 estava disponível no sistema (java-21-openjdk) mas necessitou instalar o pacote-devel para ter javac disponível.

### 3. Orquestração da aplicação
- Instalação de java-21-openjdk-devel
- Configuração de launch.properties do Ghidra
- Extração do ELF para .local/extracted/
- Execução bem-sucedida do Ghidra headless com processador MIPS:LE:32:default

### 4. Regras de negócio
O projeto Ghidra permanece local (não commitado). Apenas relatórios de análise e scripts são commitados.

### 5. Persistência / Integrações
- Adicionados scripts .local/ghidra/AnalyzeKnownFunctions.java e AnalyzeUnknownAddresses.java
- Adicionado research/elf/ghidra-analysis-rev011.md com resultados da análise
- Atualizados README.md e este registro

### 6. Resposta final
3426 funções identificadas no executável. Funções mais chamadas: FUN_001b7288 (8 callers), FUN_001a6e28 (4 callers), FUN_001b0a80 (3 callers). Endereços conhecidos 0x00132630, 0x00185ca8, 0x0019fb34 não foram reconhecidos como funções (provavelmente dados).

## Fluxos alternativos e erros
Se o Ghidra continuar com problemas de TTY, usar a interface GUI diretamente. Alternativas como radare2 podem ser consideradas.

## Decisões técnicas importantes
- Ghidra 12.x exige JDK 21, não JDK 17
- Usar linguagem MIPS:LE:32:default, não MIPS:LE:32:GCC
- Manter projeto local, não commitá-lo ao repositório

# Feature: Análise de Contexto dos Callers do State Resolver

> Squad responsável: SQUAD-RUNTIME
> Revisão: rev.019
> Sessão: 2026-05-13
> Status: Estável

## Resumo
Após a análise de dispatch de transição de estado do rev.018, esta rodada abriu o contexto dos callers de `0x0013eb50` para entender como os IDs de estado se mapeiam para objetos de entidade e estado de gameplay. A investigação confirmou que 0x0013eb50 tem 150+ callers e opera em estruturas de dados de entidade com lookups de estado indexados.

## Fluxo principal

### 1. Ponto de entrada
A análise do rev.018 identificou `0x0013eb50` como um resolvedor de estado com callers pasando IDs 0x11, 0x34 e 0x35. O próximo passo foi abrir o contexto das duas funções pai que chamam este resolvedor e entender o fluxo de dados.

### 2. Método de análise
Ghidra não estava disponível localmente, então a análise usou disassembly MIPS manual contra o ELF extraído. O padrão de chamada foi rastreado através de prologues de função, sequências de instruções e padrões de uso de registradores.

### 3. Achados

**Função 0x00199f80 (pai de 0x0019a138, 0x0019a144):**
- Stack frame: 0x130 bytes
- Carrega ponteiros de entidade via `0x015c($registrador)` - dereferência de estrutura de entidade
- Chama `0x00203aa0` (desconhecido - provavelmente loader de cena ou arquivo)
- Chama `0x0013eb50` com IDs de estado 0x34 e 0x35 em sequência
- O valor de retorno de 0x0013eb50 é usado como índice para olhar campos de entidade em `+0x800`
- Função retorna via `jr $2` - usando resultado do resolvedor de estado como alvo de continuação
- Também chama `0x0013ebe0` (função irmã) com membro de entidade no offset `+0x610`

**Função 0x0017bb98 (pai de 0x0017bd38):**
- Stack frame: 0xf0 bytes
- Uso intenso de VU/coprocessador - operações de ponto flutuante com instruções c1
- Carrega de `0x0020($17)`, `0x0024($17)`, `0x0028($17)`, `0x002c($17)` - arrays de floats VU
- Múltiplas cargas de constantes GP-relativas (0x4248, 0xc336, 0xc38c, etc.)
- Chama `0x00104508` - função de renderização ou transformação
- Chama `0x0013eb50` com ID de estado 0x11
- ID de estado 0x11 aparece em contexto VU/ponto flutuante, sugerindo linkedagem a animação ou câmera

**IDs de Estado Observados:**
- 0x11: 17 decimal - contexto VU/ponto flutuante
- 0x2e: 46 decimal - em contexto de arquivo/carregamento
- 0x34: 52 decimal - sequencial com 0x35
- 0x35: 53 decimal - sequencial com 0x34

### 4. Persistência / Integrações
Foi adicionado `research/elf/ghidra-rev019-state-resolver-caller-context.md`. Foram atualizados README.md, backlog.md, docs/architecture-log.md e este registro.

### 5. Resposta final
A `rev.019` está concluída. Os IDs de estado foram mapeados ao contexto de entidade/VU, e os pontos de entrada das funções pai (0x00199f80 e 0x0017bb98) foram identificados como alvos de breakpoint mais produtivos que o resolvedor central.

## Fluxos alternativos e erros
Se os breakpoints nos pontos de entrada das funções pai não dispararem durante o fluxo de morte, a estratégia deve mudar para breakpoints de memória nos flags globais de estado (DAT_006321c0, DAT_00633ca0).

## Decisões técnicas importantes
- 0x0013eb50 não é específico do menu de morte, mas é um dispatcher central de estado do jogo
- Pontos de entrada de funções pai (0x00199f80, 0x0017bb98) são melhores alvos de breakpoint que 0x0013eb50 diretamente
- IDs de estado provavelmente mapeiam para animação, cena ou estado de entidade, não estado de UI
- A estrutura de entidade com offsets +0x15c, +0x800, +0x610 é uma âncora estrutural chave para análise futura
