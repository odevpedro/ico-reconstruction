# Rev.148 — Gap analysis mínima jogabilidade: input, colisão e lógica do BOY (native-port)

- **Date:** 2026-09-07
- **Objective:** Ponderar (sem implementar) o plano do relatório externo `instrucoes-agente-input-colisao-boy.md` (Fases A/B/C: input real de jogador, colisão mínima, lógica real do BOY) contra o estado verificado da árvore, e listar o que falta de fato para uma "experiência minimamente jogável".
- **Scope:** Análise apenas. Nenhuma implementação nesta revisão. Branch `native-port`.
- **Sources:** `instrucoes-agente-input-colisao-boy.md` (baseline Rev.146 `ef1ce3f`); Rev.147 (commit `4c1df79`) já aplicado; `research/elf/ghidra-rev097-isysgobj-clip-girlbrain-consolidation.md`; `src/entity/boy.c` (boy_hA/hB/hC); `src/entity/asm/boy_hA.s` (byte-exact); busca global por `_Clip`/`0x166E10` na árvore.

## Veredito geral

O plano do relatório faz sentido e a sequência de fases (diagnóstico → input → colisão → lógica do BOY, com critério de saída por fase) está correta e alinhada à disciplina do projeto. Duas correções factuais são necessárias contra o estado atual da árvore:

| Afirmação do relatório | Estado real verificado |
|------------------------|------------------------|
| Item 1: duplicação 30.322 vs 15.161 "ainda sem diagnóstico" | **Já diagnosticado e resolvido no Rev.147** (4c1df79). Causa: contrato quad `[A,B,C,C]` + EBO estático → 2 triângulos GPU por triângulo fonte, metade degenerado. Resolução: `glMultiDrawArrays(GL_TRIANGLE_STRIP)`; medido 15.161 tris → 15.163 GPU (0 degenerado, +2 = quad do marker). O pré-requisito do relatório ("diagnosticar antes de plugar colisão") está cumprido. |
| Fase B: "_Clip (0x166E10) já está decompilado" | **Imprecisão.** `_Clip` está **catalogado** (rev097: função de colisão/clipping dentro de `DispCollisionPC`, 2 callers estáticos, tabela 0x282690 = `.data` de 17 slots), mas **não há `.s` nem C semântico** na árvore. O plano já manda não portar `_Clip` byte-exato (ponte semântica); só que o `ClipBridge` terá de ser desenhado do comportamento documentado, sem `.s` de referência de intrução. |

## Estado atual (já existente, não refazer)

- Sala real (`.p2o` + `.tm2`) renderizando texturizada; strip semantics validada (Rev.143/147).
- **Um placeholder já se move via teclado (quad vermelho WASD, Rev.147)** — porém manipulado direto no `main.cpp`, fora do sistema `isysGObj`/`GObj`; W/S são eixos de mundo, não relativos à câmera.
- Câmera de debug (`pollCameraInput`) independente e funcional (deve coexistir com input do jogador).
- `GObjFactory`, `IsysGObjRuntime`, pools de GObj/ProcessNode + testes — ponte semântica do sistema de objetos validada.
- `boy.c` com `boy_hA`/`boy_hB`/`boy_hC` reconstruídos em C; `boy_hA.s` byte-exact e ~100+ `.s` de BoyAI.
- Infra de captura PCSX2 (logpoints, runtime-probe-analyzer) pronta para validar estados do BOY.

## Lacunas para "experiência minimamente jogável" (ordenadas por criticidade)

Interpretação adotada: "minimamente jogável" = personagem controlável com input que não atravessa piso/parede, num cômodo renderizado.

1. **Colisão mínima (Fase B) — o buraco real.** Não existe consulta de colisão nativa. Peça mais grossa: ponte semântica (`ClipBridge`) com cast AABB/plano contra a geometria de piso/parede já extraída do `.p2o` (somente superfícies relevantes do cômodo). Sem isso o placeholder atravessa tudo. Base de referência: comportamento documentado de `_Clip` em rev097 (não byte-exato).
2. **Encaixar o player no sistema de entidades (metade restante da Fase A).** Redirecionar o movimente do placeholder para um `GObj` alimentado pelo vetor de input via `IsysGObjRuntime`. Pequeno (as peças existem e são testadas), e é a base estrutural da Fase C.
3. **Lógica real do BOY (Fase C).** Portar `boy_hA/hB/hC` como semântica nativa consumindo o vetor de input (não leitura de controle PS2); substituir o placeholder pelo mesh do BOY se extraído (não extraído ainda). É o que transforma "cubo andando" em "ICO andando".
4. **Validação contra sessão PCSX2** dos estados do BOY (probes em endereços estáveis `boy_hA/B/C`), como já feito para GIF/render. Fase C só fecha com log comparado, não "parece certo".
5. **Manutenção menor:** renomear `pollCameraInput` → `pollDebugCameraInput`; purga do histórico git (`.p2o`/`.tm2` recuperáveis via `git show`) pendente desde Rev.143; `FileSystem` stub é lacuna conhecida não bloqueante (documentada).

## Caminho mínimo

- "Andar e não atravessar" (placeholder): itens 1 + 2.
- "Jogável de verdade" (lógica do BOY): tudo de 1 a 3, validado por 4.

A ordem do relatório (A→B→C, critério de saída por fase) é preservada e coerente com o padrão de ponte semântica do projeto (GObjFactory, WorldStateLoader, GirlBrainSemantic).

## O que é confirmado

- Duplicação de triângulo resolvida (Rev.147) — pré-requisito do relatório cumprido.
- `_Clip` não tem `.s` na árvore; só identidade localizada (rev097).
- Não há colisão nativa nem player-entity no runtime hoje.
- `boy_hA/B/C` existem como C semântico + `.s` byte-exact, mas não portados ao native.

## O que é provável

- O `ClipBridge` pode reutilizar a geometria já decodificada (`Ps2oMesh::positions`/`triangles`) para construir um plano de piso/paredes por material (verif. visual pendente).
- Modo mais barato de player-entity: um GObj de teste com um ProcessNode de update que consome o vetor de input — mesma máquina já exercitada nos testes `gobj_factory`/`isysgobj_runtime`.

## O que é possível

- A Fase A (input → placeholder via GObj) pode fechar sem a Fase B; a B sem a C. Crise de escopo: manter a ordem do relatório para que cada fase tenha critério de saída próprio.

## O que é desconhecido

- Escala/altura corretas do BOY no mundo do cômodo (cm PS2 vs world units) até o mesh ser extraído.
- O comportamento exato de bloqueio de `_Clip` (o que é piso vs parede) sem captura PCSX2 de colisão.

## O que é descartado

- Usar o "sistema de colisão" atual como base (não existe). Usar `pollCameraInput` como input de jogador (são sistemas distintos) — reforçado pelo relatório.

## Próximo mínimo teste (quando a implementação começar)

- Fase A: captura de tela mostrando placeholder controlável com câmera de debug independente.
- Fase B: vídeo/gif do placeholder colidindo com piso e ao menos uma parede, sem atravessar (build limpo + ctest antes de declarar pronto).

## Veredito conservador

O relatório é uma frente válida e bem sequenciada, com duas correções factuais necessárias (item 1 já resolvido; `_Clip` não decompilado). A maior lacuna de engenharia é a colisão (Fase B). Input real de jogador já existe parcialmente (marker WASD) mas fora do sistema de entidades; Fase C depende de extração/validação do BOY. Nenhuma fase deve fechar sem critério de saída documentado.