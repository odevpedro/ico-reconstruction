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
