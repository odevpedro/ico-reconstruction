# fases-2-4.md — Templates de Execução

> Templates refinados para as fases de detalhamento, roadmap e revisão de decisões.
> Cada template deve ser precedido pelo bloco de inicialização padrão antes de colar no chat.
> Preencha os campos entre colchetes antes de executar.

---

## Bloco de Inicialização Padrão

> Cole este bloco no início de TODA sessão nova, antes de qualquer template.

```
ESTADO ATUAL DO PROJETO:
[colar conteúdo atualizado do backlog.md]

HISTÓRICO DE FLUXOS RELEVANTES:
[colar seção relevante do system-feature-flows.md]

SUA MISSÃO NESTA SESSÃO:
[squad + tarefa específica]

CONTEXTO COMPRIMIDO DA ANÁLISE A–G:
[~300 palavras — preencher após análise inicial]
```

---

## Fase 2 — Detalhamento por Squad (Spec Técnica)

> Objetivo: Transformar a descrição de alto nível de cada squad em uma spec executável.
> Execute um squad por vez. Valide antes de avançar para o próximo.

```
[BLOCO DE INICIALIZAÇÃO PADRÃO]

Com base na análise A–G, detalhe o squad **[NOME DO SQUAD]**.

Entregue:

1. Stack técnica sugerida (linguagens, ferramentas, libs) com justificativa específica para o contexto ICO/PS2
2. Arquitetura de módulos e interfaces com outros squads — inputs e outputs concretos
3. Pipeline de trabalho: do requisito à entrega validada
4. Métricas mensuráveis de sucesso (ex.: matching binário >X% em Y funções, cobertura de Z subsistemas)
5. DoR e DoD técnicos e não genéricos — critérios objetivos e testáveis
6. Automações viáveis (scripts, CI, validação automática)
7. Riscos específicos deste squad + estratégia de mitigação
8. Formato exato de assinatura que este squad usará no backlog.md
9. Como atualizar system-feature-flows.md a cada entrega — isso é DoD obrigatório e não opcional

Esta decisão é validável sem acesso ao binário? [Sim/Não + justificativa]

Limite: 2000 palavras. Use markdown com tabelas e listas.
```

**O que validar na resposta:**
- A stack é realista para o contexto ICO/PS2?
- As interfaces com outros squads estão claramente definidas?
- Os critérios de DoD são mensuráveis?
- O campo de validabilidade sem binário foi respondido?
- O formato de assinatura está correto?

---

## Fase 3 — Roadmap Operacional (Gantt Textual)

> Objetivo: Transformar as fases macro em um plano executável com dependências e marcos de validação.

```
[BLOCO DE INICIALIZAÇÃO PADRÃO]

Com base no roadmap macro da seção D, gere um Gantt textual detalhado.

Para cada fase, inclua:
- Duração estimada (semanas)
- Dependências críticas entre squads (ex.: "Squad Tooling deve entregar X antes de Content iniciar Y")
- Marcos de validação técnica mensuráveis (ex.: "matching binário validado em Z subsistemas")
- Gatilhos de risco (o que faria esta fase atrasar ou falhar)
- Entregáveis concretos (arquivos, binários, docs, testes)
- Como backlog.md e system-feature-flows.md são atualizados em cada marco — assinatura de squad obrigatória

Formato: tabela markdown com colunas:
[Fase | Semanas | Dependências | Marcos de Validação | Entregáveis | Riscos | Atualização .md]

Identifique dependências circulares explicitamente — não as omita.

Para cada fase, responda: "Esta fase é validável sem acesso ao binário? Sim/Não + justificativa."

Limite: 1200 palavras.
```

**O que validar:**
- As dependências fazem sentido técnico?
- Há dependências circulares não resolvidas?
- Os marcos de validação são objetivos e testáveis?
- O plano tem espaço para iteração — não é cascata rígida?

---

## Fase 4 — Revisão de Decisão (IA como Reviewer)

> Objetivo: Validar decisões técnicas contra os limites de acoplamento, regressão e maturidade da base reconstruída.
> Use sempre que tiver uma decisão arquitetural importante antes de executar.

```
[BLOCO DE INICIALIZAÇÃO PADRÃO]

Avalie a seguinte decisão técnica contra os limites de acoplamento, regressão e maturidade da base reconstruída discutidos na análise A–G:

DECISÃO: [descreva sua decisão aqui]
Exemplo: "Vamos encapsular o sistema de física legado em uma camada C++ e expor uma API Lua para scripts novos."

Entregue:

1. Pontuação de viabilidade (0–100) + justificativa técnica
2. Esta decisão é validável sem acesso ao binário? Sim/Não + justificativa
3. Riscos de regressão identificados (liste 3–5)
4. Como registrar no backlog.md com assinatura correta de squad
5. O que atualizar no system-feature-flows.md
6. Alternativa de menor risco, se aplicável

Limite: 600 palavras. Seja direto e técnico.
```

---

## Fase 4b — Black Box (Novo)

> Objetivo: Tratar comportamentos não compreendidos identificados durante a execução.
> Use sempre que um subsistema não puder ser abstraído ou compreendido com as ferramentas disponíveis.

```
[BLOCO DE INICIALIZAÇÃO PADRÃO]

BLACK BOX IDENTIFICADA: [descreva o comportamento não compreendido]
Exemplo: "O subsistema de colisão com plataformas móveis produz resultados não determinísticos que não conseguimos mapear no disassembly."

Entregue:

1. Estratégia de isolamento recomendada: encapsular / observar / substituir — com justificativa
2. Impacto nos squads dependentes deste comportamento
3. Risco de regressão se a black box for ignorada
4. Como registrar no backlog.md como risco ativo com assinatura de squad
5. Critério mínimo e mensurável para reclassificar como "compreendida"
6. Esta black box bloqueia o proof of concept? Sim/Não + justificativa

Limite: 500 palavras.
```

---

## Fase 5 — Geração de Artefatos

> Objetivo: Gerar templates, checklists e documentação operacional para uso recorrente pelos squads.

```
[BLOCO DE INICIALIZAÇÃO PADRÃO]

Com base nas specs dos squads já detalhados, gere os seguintes artefatos:

1. Checklist de DoR para o squad [NOME] — critérios antes de iniciar uma tarefa
2. Checklist de DoD para o squad [NOME] — critérios antes de considerar uma tarefa concluída
3. Template de registro de entrega para o backlog.md — formato padrão de assinatura
4. Template de entrada para o system-feature-flows.md — estrutura de cabeçalho e seções
5. Template de relatório de sessão — o que o agente deve entregar ao final de cada chat

Formato: markdown. Cada artefato em seção separada.
Limite: 1000 palavras.
```

---

_Última atualização: rev.001 — Templates refinados após planejamento inicial._
