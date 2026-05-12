# Prompt E–G — Matriz de Decisão, Squads e Recomendação Final

> **Instruções de uso:**
> Execute apenas após validar a análise A–D e gerar o contexto comprimido.
> Cole o bloco de inicialização padrão antes deste prompt.
> Este prompt depende diretamente das conclusões de A–D.

---

## Bloco de Inicialização (obrigatório antes de executar)

```
CONTEXTO COMPRIMIDO DA ANÁLISE A–D:
[colar ~300 palavras geradas após validação de A–D]

ESTADO ATUAL DO BACKLOG:
[colar conteúdo atualizado do backlog.md]
```

---

## Persona

Você atuará como um **arquiteto de software sênior, especialista em reverse engineering de consoles legados, designer de engines 3D de PS2 e líder de pipelines de reconstrução de codebases comerciais**.

Você já realizou o diagnóstico técnico completo (seções A–D). Agora produzirá a matriz de decisão arquitetural, a arquitetura de squads e a recomendação final.

---

## Premissas

1. As conclusões da análise A–D sobre ICO são o grounding desta análise. Não repita diagnósticos já feitos — referencie-os.
2. Toda recomendação de squad deve ser coerente com o mapa de viabilidade da Seção C, especialmente para ISO/ELF, engine/runtime, assets, salas, câmera, actor system, colisão, animação, Yorda/IA de companheira e eventos.
3. Classifique incertezas explicitamente. Não omita o que não é possível saber sem acesso ao binário.

---

## Seções Obrigatórias

### E. Matriz de Decisão Arquitetural

Defina quando aplicar cada estratégia:

| Estratégia | Quando aplicar | Gatilho observável | Risco associado | Ação recomendada |
|---|---|---|---|---|
| Extender | | | | |
| Encapsular | | | | |
| Refatorar | | | | |
| Reescrever | | | | |
| Substituir | | | | |
| Preservar | | | | |

**Inclua exemplos concretos de gatilhos operacionais para:**
- Regressão recorrente em subsistema legado
- Ausência de pontos de extensão
- Dependência de comportamento não abstraído
- Custo de validação desproporcional
- Acoplamento impeditivo entre conteúdo e runtime

---

### F. Arquitetura de Squads

Crie framework de **6–8 squads essenciais**. Para cada squad, entregue tabela markdown com:

| Campo | Valor |
|---|---|
| Missão | |
| Escopo | |
| Inputs | |
| Outputs | |
| Dependências críticas | |
| Métricas de sucesso | |
| Limitações conhecidas | |
| Automações viáveis | |
| DoR | |
| DoD | |
| Formato de assinatura no backlog.md | |
| Como atualiza system-feature-flows.md | |

**Requisitos adicionais:**
- O DoD de cada squad deve incluir obrigatoriamente atualização dos arquivos .md
- As dependências entre squads devem ser consistentes com o roadmap da Seção D
- Identifique explicitamente quais squads podem operar em paralelo e quais são sequenciais

---

### G. Recomendação Final

Responda objetivamente:

1. **Isso ainda é modding sofisticado ou já configura pipeline real de desenvolvimento?** Justifique tecnicamente.
2. **Em que ponto técnico passa a configurar "nova versão genuína"?** Expresse em termos de: abstração de sistemas, independência do runtime legado e capacidade de autoria de conteúdo novo.
3. **O que precisa existir para que essa abordagem seja sustentável?** Liste os pré-requisitos técnicos não negociáveis.
4. **Classificação de viabilidade geral:** Alta / Média / Baixa
5. **Nível de confiança da análise:** Alto / Médio / Baixo + justificativa
6. **Black boxes principais:** liste com formato `[SQUAD-ID | RISCO ATIVO | rev.XXX]`
7. **Riscos de regressão críticos:** liste os 3–5 mais graves

**Critério de Abort:**
> Se viabilidade = Baixa: quais squads ainda fazem sentido? O que muda no fluxo? Qual seria a estratégia alternativa mínima viável?

---

## Definição Operacional Final do Projeto

Finalize com 3–4 frases precisas definindo o projeto tecnicamente, distinguindo-o de:
- emulação
- port simples
- modding
- total conversion

Enfatize que o alvo é ICO de PlayStation 2, e que OpenGOAL é apenas uma referência metodológica quando aplicável, não uma base técnica diretamente reutilizável.

---

## Regras de Execução

- Use linguagem técnica precisa.
- Não repita diagnósticos já cobertos em A–D — referencie-os.
- Classifique incertezas explicitamente.
- Use markdown com tabelas e listas.
- **Não há limite rígido de palavras. Priorize completude técnica.**

---

> **Após receber a resposta:**
> 1. Valide cada squad — as interfaces fazem sentido com o mapa de viabilidade?
> 2. Escolha o primeiro squad para detalhar via Fase 2
> 3. Atualize o backlog.md com assinatura `[SQUAD-ARCH | ENTREGA | rev.008]`
> 4. Registre no system-feature-flows.md
> 5. Decida: ir para Proof of Concept ou iniciar Fase 2
