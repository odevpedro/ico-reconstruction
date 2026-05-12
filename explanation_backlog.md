# explanation_backlog.md — Instruções Operacionais do Backlog

Este projeto deve manter um arquivo Markdown chamado:

backlog.md

Esse arquivo funciona como um **registro visual e simples do progresso do projeto**, listando funcionalidades implementadas, em andamento e planejadas.

## Estrutura do backlog

O arquivo `backlog.md` deve conter três seções principais:

### Em andamento
Funcionalidades atualmente sendo desenvolvidas.

### Pendentes
Funcionalidades planejadas para desenvolvimento futuro.

### Concluídas
Funcionalidades já implementadas.

## Formato dos itens

Cada funcionalidade deve ser representada usando checkboxes Markdown com assinatura de squad obrigatória.

Exemplo:

- [ ] Implementar encapsulamento do subsistema de física
  > [SQUAD-RUNTIME | AGUARDANDO | rev.002]
  > Dependência: disassembly completo — rev.001
  > Pendência para próximo agente: API Lua não definida ainda

- [x] Setup de ambiente de disassembly
  > [SQUAD-RUNTIME | ENTREGA | rev.001]
  > Resultado: ambiente validado, 3 ferramentas documentadas
  > Evidência: system-feature-flows.md seção "Setup Ambiente"

Onde:
- `[ ]` indica funcionalidade **pendente ou em andamento**
- `[x]` indica funcionalidade **concluída**

## Esquema de Assinaturas

Formato obrigatório: `[SQUAD-ID | STATUS | rev.XXX]`

**Status possíveis:**
- `ENTREGA` — tarefa concluída e validada
- `EM ANDAMENTO` — tarefa em execução na sessão atual
- `AGUARDANDO` — tarefa pendente, dependência não satisfeita
- `BLOQUEADO` — tarefa impedida por black box ou decisão pendente
- `RISCO ATIVO` — comportamento não compreendido registrado

**IDs de squad sugeridos:**
- `SQUAD-ARCH` — arquitetura e análise
- `SQUAD-RUNTIME` — reverse engineering e runtime
- `SQUAD-TOOLING` — ferramentas e pipeline
- `SQUAD-GAMEPLAY` — gameplay e core systems
- `SQUAD-QA` — qualidade e estabilidade

## Atualização do backlog

- Ao iniciar uma funcionalidade → movê-la para **Em andamento**, atualizar status para `EM ANDAMENTO`
- Ao concluir uma funcionalidade → marcar `[x]`, status `ENTREGA`, mover para **Concluídas**
- Ao identificar black box → adicionar em **Registro de Black Boxes** com status `RISCO ATIVO`
- Ao planejar novas funcionalidades → adicionar em **Pendentes** com status `AGUARDANDO`

**O backlog deve sempre refletir o estado atual do projeto.**
**Nenhuma tarefa é concluída sem atualização do backlog — isso é DoD obrigatório.**

## Seção de Black Boxes

O backlog deve conter uma seção específica para black boxes:

```markdown
## Registro de Black Boxes
> Comportamentos não compreendidos identificados durante a execução.
> Formato: [SQUAD-ID | RISCO ATIVO | rev.XXX]
```

## Objetivo

Permitir que qualquer agente em sessão nova entenda imediatamente:
- o que já foi implementado e por qual squad
- o que está sendo desenvolvido
- o que está bloqueado e por quê
- o que o próximo agente precisa saber antes de começar
