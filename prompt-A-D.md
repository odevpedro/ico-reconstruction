# Prompt A–D — Análise Arquitetural: ICO Reconstruction

> **Instruções de uso:**
> Cole este prompt inteiro em um chat novo dentro do Projeto "ICO Reconstruction".
> Após receber a resposta, valide seção por seção antes de executar o Prompt E–G.
> Gere o contexto comprimido (~300 palavras) a partir desta resposta antes de avançar.

---

## Persona

Você atuará como um **arquiteto de software sênior, especialista em reverse engineering de consoles legados, designer de engines 3D de PS2 e líder de pipelines de reconstrução de codebases comerciais**.

---

## Contexto e Premissas Obrigatórias

1. A ISO alvo contém o jogo **ICO** de PlayStation 2, com binários compilados, assets empacotados, dados de salas, animações, câmera, áudio e estruturas de engine proprietária. Não há código-fonte original legível.
2. O objetivo é alcançar reconstrução via disassembly/decompilação, recompilação com matching binário aproximado quando aplicável, abstração progressiva de sistemas e execução em runtime moderno.
3. O paradigma **não é emulação**. O foco é: base recompilável, runtime próprio, tooling de autoria, pipeline de conteúdo e evolução contínua.
4. O foco da análise é **estritamente técnico e arquitetural**. Ignore licenciamento, distribuição e aspectos legais.
5. Assume-se maturidade desigual da base reconstruída: alguns sistemas poderão estar compreendidos a ~95%, outros permanecerão como **black boxes operacionais**.
6. O paradigma metodológico de referência é **OpenGOAL**, mas ICO não deve ser tratado como tecnicamente equivalente a esse projeto. OpenGOAL serve como referência de disciplina de reconstrução, extração, runtime moderno, documentação e validação incremental; a arquitetura de ICO deve ser inferida e validada empiricamente.
7. Toda avaliação de maturidade na Seção C **deve citar evidência técnica observável em ICO, comparação metodológica com OpenGOAL, ou análogo público de decompilação PS2**. Classificações sem grounding técnico não são aceitáveis.

---

## Objetivo Central

Responder com rigor técnico:

**Até que ponto uma reconstrução recompilável de ICO permite que uma equipe deixe de apenas "modificar" o jogo e passe a "desenvolver novas versões, restaurações ou expansões genuínas" sobre essa fundação?**

---

## Ordem de Prioridade da Resposta

1. Diagnosticar se a fundação técnica é suficiente para sustentar desenvolvimento genuíno.
2. Classificar a viabilidade por domínio técnico.
3. Definir critérios objetivos para distinguir modding de nova versão genuína.
4. Propor roadmap macro operacional.

---

## Seções Obrigatórias

### A. Diagnóstico Técnico Geral

- Explique o que está sendo proposto em termos de arquitetura, runtime, abstração e pipeline.
- Diferencie estruturalmente:
  - modding avançado de ICO
  - base recompilável
  - plataforma de desenvolvimento derivada
- Seja específico sobre o que cada nível permite e o que impede tecnicamente.

---

### B. Níveis de Intervenção

Defina critérios técnicos objetivos para cada nível:

| Nível | Definição técnica | O que permite | O que impede |
|---|---|---|---|
| Mod superficial | | | |
| Mod estrutural | | | |
| Total conversion | | | |
| Fork de gameplay | | | |
| Reconstrução de campanha | | | |
| Nova versão genuína | | | |

**Indique o limiar técnico exato** onde o projeto deixa de ser mod e passa a ser nova versão. Esse limiar deve ser expresso em termos de: abstração de sistemas, independência do runtime legado e capacidade de autoria de conteúdo novo.

---

### C. Mapa de Viabilidade por Camadas

Para cada domínio abaixo, entregue uma avaliação estruturada:

**Domínios:** engine/runtime | gameplay/core loop | actor system | Yorda/IA de companheira | inimigos/sombras | câmera | colisão/física | animação/IK | scripting/event flow | salas/streaming | UI/HUD | áudio | cutscenes | progressão/save | rendering/efeitos | tooling | content pipeline | testes/QA

Para cada domínio, informe em tabela:

| Domínio | Maturidade de compreensão | Maturidade de intervenção | Classificação | Evidência/Análogo | Justificativa |
|---|---|---|---|---|---|

Classificações possíveis:
- reaproveitável direto
- reaproveitável com encapsulamento
- exige reescrita parcial
- exige reescrita total

Justificativa deve cobrir: acoplamento, dependência de hardware legado, ausência de tooling, risco de regressão e previsibilidade operacional.

**Se não houver evidência técnica observável ou análogo direto útil, sinalize explicitamente como "sem referência validada".**

---

### D. Roadmap Macro em Fases

Estruture 5–6 fases. Para cada uma, entregue:

| Fase | Objetivo | Pré-requisitos | Entregáveis | Riscos críticos | DoD |
|---|---|---|---|---|---|

**Requisitos adicionais:**
- Identifique dependências circulares entre fases explicitamente.
- Para cada fase, indique: "Esta fase é validável sem acesso ao binário? Sim/Não + justificativa."
- O DoD de cada fase deve incluir obrigatoriamente: atualização do `backlog.md` com assinatura de squad e atualização do `system-feature-flows.md`.

---

## Critério de Abort

Ao final da Seção D, responda explicitamente:

> **Se a viabilidade geral for classificada como Baixa:** quais fases ainda fazem sentido executar? O que muda no fluxo? Qual seria a estratégia alternativa mínima viável?

---

## Regras de Execução

- Use linguagem técnica precisa. Nada de linguagem vaga ou consultiva sem mecanismo técnico claro.
- Classifique incertezas explicitamente — não omita o que não é possível saber sem acesso ao binário.
- Use markdown com tabelas e listas.
- Priorize densidade técnica, clareza estrutural e utilidade operacional.
- **Não há limite rígido de palavras. Priorize completude técnica sobre brevidade.**

---

## Definição Operacional do Projeto

Ao final da Seção D, inclua 3–4 frases precisas definindo o projeto tecnicamente, distinguindo-o de:
- emulação
- port simples
- modding
- total conversion

Enfatize que se trata de uma tentativa de transformar ICO em uma **base recompilável de desenvolvimento**, com **tooling próprio**, **pipeline de conteúdo**, **abstrações progressivas** e **capacidade de restauração, expansão e experimentação sistêmica contínua**.

---

> **Após receber a resposta:**
> 1. Valide cada seção — ajuste ou refute o que não fizer sentido técnico
> 2. Gere o contexto comprimido (~300 palavras) a partir desta análise
> 3. Atualize o `backlog.md` com assinatura `[SQUAD-ARCH | ENTREGA | rev.007]`
> 4. Registre o fluxo no `system-feature-flows.md`
> 5. Decida: ir para Proof of Concept ou executar Prompt E–G
