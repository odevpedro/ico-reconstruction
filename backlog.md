# Backlog — ICO Reconstruction

> **Protocolo de assinaturas:**
> Cada item deve conter uma assinatura no formato:
> `[SQUAD-ID | STATUS | rev.XXX]`
> Status possíveis: `ENTREGA` | `EM ANDAMENTO` | `AGUARDANDO` | `BLOQUEADO` | `RISCO ATIVO`

---

## Em andamento

_Nenhum item em andamento._

---

## Pendentes

- [ ] Análise arquitetural A–D (diagnóstico de ICO, viabilidade, critérios, roadmap macro)
  > [SQUAD-ARCH | AGUARDANDO | rev.000]
  > Dependência: nenhuma — ponto de entrada do projeto
  > Pendência para próximo agente: contexto comprimido (~300 palavras) a ser gerado após validação humana

- [ ] Análise arquitetural E–G (matriz de decisão, squads, recomendação final para ICO)
  > [SQUAD-ARCH | AGUARDANDO | rev.000]
  > Dependência: aprovação humana da análise A–D
  > Pendência para próximo agente: definição final de squads e DoD por domínio

- [ ] Proof of concept — primeira alteração visível funcionando em ICO
  > [SQUAD-RUNTIME | AGUARDANDO | rev.000]
  > Dependência: análise A–D concluída
  > Pendência para próximo agente: evidência empírica de intervenção mínima no binário/asset de ICO

- [ ] Setup de ambiente de extração e disassembly para ICO
  > [SQUAD-RUNTIME | AGUARDANDO | rev.000]
  > Dependência: nenhuma — pode iniciar em paralelo com análise A–D
  > Pendência para próximo agente: ferramentas validadas contra a ISO alvo de ICO e ambiente documentado

- [ ] Detalhamento por squad — Fase 2 (um squad por vez)
  > [SQUAD-ARCH | AGUARDANDO | rev.000]
  > Dependência: análise E–G concluída e validada
  > Pendência para próximo agente: specs executáveis por domínio

- [ ] Roadmap operacional — Fase 3 (Gantt textual)
  > [SQUAD-ARCH | AGUARDANDO | rev.000]
  > Dependência: pelo menos 1 squad detalhado via Fase 2
  > Pendência para próximo agente: dependências entre squads mapeadas

---

## Concluídas

- [x] Planejamento estratégico inicial e refinamento do sistema de prompts
  > [SQUAD-ARCH | ENTREGA | rev.001]
  > Resultado: fluxo A–G definido, esquema de assinaturas aprovado, protocolo de sessão estabelecido
  > Evidência: arquivos backlog.md e system-feature-flows.md criados e carregados no projeto

- [x] Retarget oficial do projeto para ICO Reconstruction
  > [SQUAD-ARCH | ENTREGA | rev.002]
  > Resultado: prompts, backlog e decisões iniciais reescritos para tratar ICO como alvo técnico principal
  > Evidência: prompt-A-D.md, prompt-E-G.md, decisoes-iniciais.md, fases-2-4.md, backlog.md e system-feature-flows.md atualizados

- [x] README público inicial para colaboração comunitária
  > [SQUAD-ARCH | ENTREGA | rev.003]
  > Resultado: README.md criado com objetivo, escopo, limites legais, roadmap inicial e instruções de contribuição
  > Evidência: README.md adicionado ao repositório

- [x] README mesclado com template público do repositório
  > [SQUAD-ARCH | ENTREGA | rev.004]
  > Resultado: README.md preservado e ampliado com badges, stack, estrutura de pastas, setup local, testes, documentação, status, contribuição e licença
  > Evidência: README.md atualizado com base em /home/peter/Documentos/repos/claude-config/README-template.md, ignorando seções de banco/API sem relação com o projeto

- [x] Wallpaper de ICO adicionado ao README
  > [SQUAD-ARCH | ENTREGA | rev.005]
  > Resultado: wallpaper local copiado para assets/ico-wallpaper.webp e referenciado no topo do README.md
  > Evidência: assets/ico-wallpaper.webp criado e README.md atualizado

- [x] Estrutura mínima de pastas para GitHub
  > [SQUAD-ARCH | ENTREGA | rev.006]
  > Resultado: estrutura mínima criada para documentação, pesquisa, tooling, testes e templates de issues
  > Evidência: docs/, research/, tools/, tests/fixtures/ e .github/ISSUE_TEMPLATE/ criados com .gitkeep; README.md atualizado

---

## Registro de Black Boxes

> Comportamentos não compreendidos identificados durante a execução.
> Formato: `[SQUAD-ID | RISCO ATIVO | rev.XXX]`

_Nenhuma black box identificada ainda._

---

## Protocolo de Sessão

Toda nova sessão deve começar com o seguinte bloco passado ao agente:

```
ESTADO ATUAL DO PROJETO:
[colar conteúdo atualizado do backlog.md]

HISTÓRICO DE FLUXOS RELEVANTES:
[colar seção específica do system-feature-flows.md]

SUA MISSÃO NESTA SESSÃO:
[squad + tarefa específica]

CONTEXTO COMPRIMIDO DA ANÁLISE A–G:
[~300 palavras — preencher após análise inicial]
```

---

_Última atualização: rev.006 — Estrutura mínima de pastas criada para GitHub._
