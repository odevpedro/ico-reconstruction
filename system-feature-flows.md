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
