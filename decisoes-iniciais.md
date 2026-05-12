# decisoes-iniciais.md — ICO Reconstruction

> Registro das decisões tomadas durante o planejamento inicial do projeto.
> Este arquivo serve como contexto de origem para qualquer agente ou IA que retome o projeto.
> Não modifique entradas anteriores. Adicione novas decisões ao final.

---

## Contexto do Projeto

O projeto ICO Reconstruction tem como objetivo transformar o jogo **ICO** de PlayStation 2 em uma **base recompilável e documentada de desenvolvimento** — não para emular, não para fazer mods isolados, mas para criar uma fundação técnica sobre a qual novas versões, restaurações, experimentos sistêmicos e ferramentas de autoria possam ser desenvolvidos.

O paradigma metodológico de referência é o **OpenGOAL** — não porque sua arquitetura seja diretamente transferível, mas porque demonstra uma disciplina de reconstrução, extração de assets, runtime moderno, validação incremental e tooling de desenvolvimento. ICO provavelmente exigirá uma abordagem mais tradicional de engenharia reversa sobre código MIPS/C/C++ e formatos proprietários da Team Ico/Sony Japan Studio.

---

## Papel dos Participantes

- **Pedro** atua como **Product Owner e orquestrador** do projeto
- **Agentes de IA** atuam como time técnico especializado por squad
- Pedro é o único ponto de continuidade humana entre sessões
- Pedro valida entregas antes de passar o bastão entre squads

---

## Decisões de Arquitetura do Sistema de Prompts

### 1. Separação do Prompt Principal em A–D e E–G
**Decisão:** O prompt principal foi dividido em duas chamadas separadas.
**Razão:** E–G depende do diagnóstico de A–D. Executar tudo junto compromete qualidade das seções finais e propaga erros de alto nível para specs de squad.
**Impacto:** Obrigatório executar A–D, validar, gerar contexto comprimido, depois executar E–G.

### 2. Limite de Palavras Removido do Prompt A–D
**Decisão:** Sem limite rígido de palavras no Prompt A–D.
**Razão:** 3500 palavras era insuficiente para cobrir A–D com densidade técnica real, especialmente a Seção C (13 domínios) e a Seção F (8 squads em tabela).
**Impacto:** Priorizar completude técnica sobre brevidade.

### 3. Grounding Obrigatório na Seção C
**Decisão:** Toda avaliação de maturidade na Seção C deve citar evidência técnica observável em ICO, comparação metodológica com OpenGOAL, ou análogo público de projetos de decompilação PS2.
**Razão:** Sem grounding, classificações Baixa/Média/Alta ficam subjetivas e inutilizáveis operacionalmente. Como ICO não possui a mesma vantagem estrutural de uma linguagem de jogo já reconstruída, inferências devem ser separadas de evidência real.
**Impacto:** Avaliações sem referência devem ser sinalizadas como "sem referência validada".

### 4. Atualização dos .md como DoD Obrigatório
**Decisão:** Atualização do backlog.md e system-feature-flows.md é critério de conclusão de cada entrega, não atividade paralela.
**Razão:** Documentar "enquanto avança" na prática vira nunca. Precisa ser gate de entrega.
**Impacto:** Nenhuma tarefa é considerada concluída sem assinatura no backlog e registro no system-feature-flows.

### 5. Critério de Abort Explícito
**Decisão:** O Prompt A–D inclui seção obrigatória de critério de abort.
**Razão:** Se viabilidade geral vier como Baixa, as fases subsequentes precisam ser reavaliadas — o fluxo original não tratava esse cenário.
**Impacto:** Ao final de A–D, o agente deve responder explicitamente o que muda se viabilidade = Baixa.

### 6. Campo de Validabilidade sem Binário
**Decisão:** Templates das Fases 2 e 4 incluem campo obrigatório: "Esta decisão é validável sem acesso ao binário? Sim/Não + justificativa."
**Razão:** Força a IA a sinalizar quando avaliação é inferência, não grounding real.
**Impacto:** Separa análises confiáveis de hipóteses que precisam de evidência empírica.

### 7. Template de Black Box (Fase 4b)
**Decisão:** Criado template específico para black boxes identificadas durante execução.
**Razão:** O fluxo original não tinha mecanismo para lidar com comportamentos não compreendidos encontrados em runtime.
**Impacto:** Toda black box identificada deve ser registrada no backlog como risco ativo com assinatura de squad.

---

## Decisões de Infraestrutura

### 8. Esquema de Assinaturas no backlog.md
**Decisão:** Formato `[SQUAD-ID | STATUS | rev.XXX]` em cada item do backlog.
**Razão:** Permite que qualquer agente em sessão nova saiba o que foi feito, por qual squad, em que estado e o que está pendente — sem depender de memória da IA.
**Status possíveis:** `ENTREGA` | `EM ANDAMENTO` | `AGUARDANDO` | `BLOQUEADO` | `RISCO ATIVO`

### 9. Protocolo de Sessão Padrão
**Decisão:** Toda sessão nova começa com bloco de inicialização padrão.
**Razão:** Resolve perda de contexto entre sessões sem depender de memória nativa da IA.
**Formato:**
```
ESTADO ATUAL DO PROJETO: [backlog.md atualizado]
HISTÓRICO DE FLUXOS RELEVANTES: [seção relevante do system-feature-flows.md]
SUA MISSÃO NESTA SESSÃO: [squad + tarefa específica]
CONTEXTO COMPRIMIDO DA ANÁLISE A–G: [~300 palavras]
```

### 10. Estrutura de Chats dentro do Projeto
**Decisão:** Um Projeto no Claude.ai, múltiplos chats — cada chat é um agente/squad.
**Razão:** Arquivos do projeto são compartilhados automaticamente entre todos os chats. Contexto compartilhado resolvido nativamente.
**Impacto:** Pedro atualiza os arquivos .md após cada entrega. A IA não escreve de volta nos arquivos — o commit é sempre humano.

### 11. Portabilidade para Outras IAs
**Decisão:** Todo o sistema deve ser portável para qualquer IA via pasta de arquivos .md.
**Razão:** Independência de plataforma. O fluxo não pode depender de funcionalidades exclusivas do Claude.ai.
**Como replicar:** Colar arquivos de instrução no system prompt da IA destino + arquivos de estado como contexto.

---

## Decisão sobre Execução

### 12. Proof of Concept Antes de E–G
**Decisão recomendada:** Após validar A–D, considerar ir direto para proof of concept antes de executar E–G.
**Razão:** A primeira alteração funcionando em ICO revela o que é real — não apenas o que é viável em teoria. Specs de squad fazem mais sentido com evidência empírica do binário, do layout da ISO e dos formatos de asset.
**Status:** Decisão final do PO após receber análise A–D.

### 13. Retarget Oficial do Projeto para ICO
**Decisão:** O projeto passa oficialmente a se chamar ICO Reconstruction.
**Razão:** O alvo técnico escolhido pelo PO é ICO, jogo de início de geração do PS2 com escopo mais contido que muitos AAA tardios, mas sem a vantagem arquitetural específica de uma linguagem de jogo já reconstruída.
**Impacto:** Todos os prompts, backlog e critérios de viabilidade devem tratar OpenGOAL apenas como referência metodológica. A análise deve priorizar ISO, ELF, assets, câmera, actor system, colisão, animação, Yorda/IA de companheira, eventos, salas e rendering de ICO.

---

_Última atualização: Retarget para ICO Reconstruction — rev.002_
