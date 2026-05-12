# registro_funcionalidades.md — Instruções Operacionais do system-feature-flows

Para cada nova feature, ajuste, correção ou refatoração relevante implementada neste sistema, além do código-fonte, gere também uma **documentação de fluxo atualizada em Markdown**.

Essa documentação deve ser mantida em um arquivo central:

docs/system-feature-flows.md

Esse arquivo funciona como um **registro histórico das funcionalidades e seus fluxos internos**.

## Comportamento esperado

Sempre que uma nova funcionalidade for implementada:

1. Gere a explicação de fluxo.
2. Atualize o arquivo Markdown existente.
3. Adicione a nova funcionalidade **como uma nova seção no final do documento**.
4. Nunca apague funcionalidades anteriores.

## Objetivo

Permitir que qualquer desenvolvedor ou agente entenda:
- como cada feature percorre o sistema
- quais camadas participam
- quais decisões técnicas foram tomadas
- quais regras de negócio existem

A documentação deve refletir a arquitetura real do projeto.

## Estrutura obrigatória para cada feature

```markdown
# Feature: [nome da funcionalidade]

> Squad responsável: [SQUAD-X]
> Revisão: rev.XXX
> Sessão: [data ou identificador]
> Status: Estável / Em revisão / Black box

## Resumo
Explique brevemente o objetivo da funcionalidade e qual problema ela resolve.

## Fluxo principal

### 1. Ponto de entrada
### 2. Validação de entrada
### 3. Orquestração da aplicação
### 4. Regras de negócio
### 5. Persistência / Integrações
### 6. Resposta final

## Fluxos alternativos e erros

## Decisões técnicas importantes

## Trechos de código relevantes (opcional)
```

## Regras importantes

- O foco principal deve ser a **explicação textual didática**.
- O documento deve crescer **incrementalmente**.
- Não sobrescreva funcionalidades anteriores.
- Sempre adicione novas features ao final do arquivo.
- **Atualizar este arquivo é DoD obrigatório de toda entrega de squad.**

## Resultado esperado

Ao longo do tempo, o arquivo `system-feature-flows.md` deve se tornar um **mapa completo do comportamento do sistema**, documentando a evolução das funcionalidades e seus fluxos internos.
