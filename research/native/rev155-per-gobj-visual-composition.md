# Rev.155 — Passo 1: per-GObj visual composition (GObjAttachmentStore) + render loop driven by active isysGObj lists

## Data
- 2026-09-09

## Objetivo

Fechar o "Passo 1" do plano de sequência do usuário: cada GObj criado pelo
`KanbanSceneLoader` (Rev.154, cena 0x0F) passa a **possuir** sua composição
visual (mesh path, texturas por material, transform) em um registry host-side,
e o loop de render passa a **iterar os GObjs ativos** e desenhar o que cada um
comanda — em vez de ler o `SceneAssetStore`/`stripBatches` global diretamente.

## Escopo

- `native/src/engine/GObjAttachment.{h,cpp}` (novo) — registry host-side.
- `native/src/game/KanbanSceneLoader.{h,cpp}` — `attachBoundAssetsToGObjs()`,
  `attachmentStore()`, `sceneGObjHandle()`.
- `native/src/main.cpp` — refactor do loop de render para dirigir-se por GObjs.
- `native/tests/gobj_attachment_test.cpp` (novo) — teste Passo 1.
- `GObjAttachmentKind` prevê `BoxMarker` para o avatar do Boy (fechado).

## Fontes

- Rev.154 (`research/native/rev154-verified-scene-tables.md`) — 68 descritores,
  3600 entries, 25 GObjs cena 0x0F.
- ABI canônica `src/core/gobj_abi.h` — `IcoGObj` 0x174, handle = slot+1.
- Árvore de código nativa (`native/`), demo `run_native_demo.sh`.

## Evidência

### Feed de verdade (Rev.154)

- `kVerifiedSceneDescriptors` = 68, descritores em 0x2A31B8 (stride 0x64).
- Entry table 0x2A4C48 striding 0x4C, run válido `descIdx<68` é idx 0..3590.
- Cena 0x0F = entries [847,876) = 29 payload rows; desc 44/54 são gate-0
  (skipped) → **25 host GObjs** criados por `initSceneGObj(0x0F)`.
- `stgst00a.manifest` (30 assets declarados, 28 .p2o existentes em disco).

### Implementação

`GObjAttachmentStore` — regras verificadas por teste:

| Operação | Semântica |
|----------|-----------|
| `attach(att)` | substitui registro com mesmo (handle, meshPath) |
| `detach(handle)` | remove TODOS os registros do handle |
| `detachMesh(handle, meshPath)` | remove o par (handle, meshPath) |
| `find(handle)` | primeiro registro do handle, ou nullptr |
| `countFor(handle)` | nº de registros do handle |
| `forEach(fn)` | itera todos os registros |

Decisão de design: um GObj pode ser dono de **vários** meshes (28 assets / 25
GObjs → handles de pareamento 0,1,2 têm 2 registros). O `attach()` não
sobrescreve por handle sozinho, apenas por (handle, meshPath).

`KanbanSceneLoader::attachBoundAssetsToGObjs(sceneId)` — pareia os assets
ligados da cena aos GObjs da última `initSceneGObj()` via **round-robin HOST**
(`m_sceneGObjs[i % size]`). Retorna nº de anexos (28). Emite aviso quando
assets > GObjs.

**Isso é uma heurística HOST, NÃO reconstrução byte-verified do vínculo
original GObj↔modelo.** Um capture futuro PCSX2 que ligue cada GObj ao seu
modelo substitui o round-robin.

`main.cpp` render loop (frase "renderer iterates active GObjs"):

```cpp
for (u8 listId = 0; listId < kPrimaryListCount; ++listId) {
    GObj* g = gobjRuntime.head(listId);
    while (g != nullptr) {
        GObjHandle h = gobjRuntime.pool().handleOf(*g);
        auto it = gobjDrawByHandle.find(h);
        if (it != end) { for sb in draw.batches: bridge.drawStrips(...); }
        g = (g->next != kNullGObjHandle) ? pool().get(g->next) : nullptr;
    }
}
```

Quando `gobjDrawByHandle` está vazio (viewer não ligado / mesh demo), cai no
`stripBatches` global — fallback preservado.

O boy: `BoyController::getGObj()` → handle → **`BoxMarker` attachment** na
`boyStore`. O transform do marker é escrito todo frame pelo BoyController
(`m->transform = translation(markerX, markerY, markerZ)`), e o box é desenhado
lendo o attachment (`drawBoxMarker(*m)`), não mais hardcoded em
`markerX/Y/Z`.

### Teste (gobj_attachment_test) — verificado

- runtime init 0x140/0x500; loader init; verified tables; requestScene(0x0F);
  initSceneGObj → **25 GObjs**; bindSceneAssets → 28 assets.
- `attachBoundAssetsToGObjs(0x0F)` → 28; `attachmentStore().count()==28`.
- owner de cada anexo vivo no pool; `kind==Mesh`; `active`; transform
  identidade (m[i]: i%5==0 → 1.0 senão 0.0).
- `meshPath(i) == boundAsset(i).meshPath` com owner `sceneGObjHandle(i%25)`.
- `doubleOwned==3` (handles 0,1,2 possuem 2); `countFor(handle0)==2`,
  `countFor(handle3)==1`.
- coesão find/detach: `detach(primeiro owner)` → count==28-ownedFirst,
  `find==nullptr`, `countFor==0`.
- rerun `initSceneGObj()` → limpa attachments (count==0).

### Demo (verificado)

`./run_native_demo.sh --shot /tmp/rev155_scene.ppm --angle -30 --frames 1`:

```
main: boy GObj 1 -> BoxMarker attachment (Passo 1)
main: scene 0x0F semantic load via requestScene/execute: ok (currentSceneId=0x0E/0x0F, 25 host GObjs)
main: Passo 1 per-GObj composition: 28 attachments for 25 host GObjs of scene 0x0F
main:   GObj 26 draws 1 labels, 1 strip batches
...
main:   GObj 3 draws 2 labels, 8 strip batches
main: wrote /tmp/rev155_scene.ppm
```

Pixel-check do PPM: quarto texturizado presente (15 buckets de cor grossos,
não só o fundo), +98 px vermelho-rosado consistentes com o BoxMarker do boy.

### CTest

`ctest` no build: **27/28 passam** — único falhando `opengl_backend` (segfault
headless pré-existente, documentado no AGENTS.md). `gobj_attachment_test`
passou. 96% pass rate, mesma linha de Rev.143/151/154.

## Confirmado

- Passo 1 ativo: 25 GObjs (cena 0x0F) cada um com composição visual mesh
  resolvida a partir de 28 assets, desenhada pelo loop que percorre as lists
  primárias do runtime isysGObj.
- BoxMarker do boy é GObj-ownered: transform atualizado pelo controller e
  lido pelo renderer — o renderer não hardcoda a posição do boy.
- Fallback preservado: `stripBatches` global continua usado quando não há
  store/composição GObj.
- 28 assets de `stgst00a.manifest` existentes em disco (nem todos os 30
  declarados; `piecePaths` só inclui os que `f.good()`).

## Provável

- A distribuição round-robin (28 assets → 25 GObjs) produzirá artefatos
  visuais errados no GObj-errado até um capture PCSX2 ligar GObj↔modelo: a
  cena 0x0F no PS2 quase certamente NÃO associa peças de geometria a GObjs de
  entidade 1:1. O objetivo é a *mecânica* (render dirigido por GObj), não a
  fidelidade do pareamento.

## Possível

- `kNullGObjHandle` na composição (bloco p-2) permanece não-anexado; p2 wall
  family continua sem discriminador próprio (Rev.151 open item).

## Desconhecido

- Mapeamento real GObj↔modelo (o round-robin é placeholder).
- Ordem de draws por GObj vs ordem de criação: mudar o pareamento pode mudar
  a ordem de desenho (estado alpha/estêncil ainda não é prioridade).

## Descartado

- Estender `IcoGObj` (0x174) com campos host: o ABI byte-exact não cresce. A
  composição visual vive no registry paralelo (desenho explícito, Rev.109).

## Próximo teste mínimo

- Capture PCSX2 da cena 0x0F ligando cada GObj ao seu modelo/init_fn para
  substituir o round-robin (Passo 2 do plano de sequência está na fila).

## Veredito conservador

- Passo 1 entregue: renderer dirigido por GObjs ativos, com composição
  visual por GObj e boy via BoxMarker. O pareamento GObj↔meshes é heurística
  host documentada, não reconstrução validada. 27/28 CTest.

## Arquivos

- `native/src/engine/GObjAttachment.{h,cpp}` (novo)
- `native/src/game/KanbanSceneLoader.{h,cpp}`
- `native/src/main.cpp`
- `native/tests/gobj_attachment_test.cpp` (novo)
- `native/CMakeLists.txt` (fonte + teste)