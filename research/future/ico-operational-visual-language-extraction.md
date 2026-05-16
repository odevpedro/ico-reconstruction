# ICO — Operational Visual Language Extraction

---

## ENGLISH

---

## Status

STAND BY / FUTURE TRACK

This front is not active. It is a planning document for a future phase of the ico-reconstruction project. It must not compete with the current active fronts: decompilation, runtime analysis, splat tooling, compiler matching, descriptor/callback modeling, and cross-reference with ICO-decomp.

## Executive summary

This document defines a future research front for extracting the operational visual language of ICO. When the project reaches sufficient maturity in decompilation, asset understanding, environment cataloging, and texture extraction, this front may be activated to systematically document how the game world looks, feels, and communicates through visual design. The goal is not to produce isolated prompts for image generation, but to reconstruct a formal aesthetic grammar grounded in evidence extracted from the game itself.

## Why this front must remain on stand by

This front requires conditions that do not yet exist in the project:

- The project is still consolidating internal subsystems (dispatcher model, cloth domain, ROPE callback chain, compiler matching). These are the current priority.
- Asset extraction is explicitly excluded from the current phase. DATA.DF, textures, models, and graphical formats are on the "what not to investigate yet" list in AGENTS.md.
- The aesthetic analysis would be stronger and more rigorous if built on top of organized evidence: known file formats, named assets, mapped environments, extracted textures. Without this foundation, the analysis risks being speculative.
- The current priority is incremental decompilation and runtime validation. Starting a visual front now would split focus and delay the technical reconstruction.
- When this front is activated, it will be supported by technical knowledge from the decompilation work, making the aesthetic claims auditable rather than impressionistic.

## What "operational visual language" means

Operational visual language is a formal system of rules, patterns, descriptions, and visual principles that explain how ICO world looks, breathes, and organizes itself. It is broader than color palettes or texture libraries.

It includes:

- architectural rhythm: how towers, walls, bridges, and halls repeat and vary;
- scale perception: how the player character relates to doorways, columns, rooms;
- visual silence: areas with minimal detail, empty space, long corridors;
- light behavior: direction, falloff, color temperature, shadow type;
- ruin geometry: how broken walls, collapsed ceilings, and eroded surfaces are constructed;
- material language: how stone ages, how metal rusts, how water pools;
- proportion system: room height, corridor width, stair depth, window placement;
- ornament grammar: what decorative elements exist, where they appear, what they signify;
- emptiness as a design tool: the use of negative space to create atmosphere.

The goal is to extract not just "what textures exist" but "why the castle looks the way it does" as a reproducible visual system.

## Difference between visual prompt and aesthetic grammar

**Visual prompt**: A textual description designed to generate a single image. It is narrow, output-oriented, and disposable. It answers "what does this scene contain?"

**Aesthetic grammar**: A structured set of rules and descriptions that explains how a visual system operates across all instances. It is analytical, reusable, and evidence-based. It answers "what are the rules that make any scene in this world feel coherent?"

This front pursues the second. Prompts may be derived from the grammar, but the grammar itself is the primary deliverable.

## Future objectives

When activated, this front will aim to:

1. Catalog the visual aesthetics of ICO castle across all accessible areas.
2. Write dense environmental descriptions grounded in extracted evidence.
3. Build category-specific visual descriptions (by area, material, lighting condition).
4. Support possible visual experiments (studies, reconstructions, comparative analysis) without violating the original aesthetic.
5. Create a foundation for aesthetically coherent expansion or reinterpretation.
6. Document the visual language as part of the project digital archaeology output, alongside the technical research notes.

## Minimum conditions to start

This front must not begin until a substantial subset of the following exists:

- A catalog of in-game screenshots organized by area and lighting condition.
- Texture dumps from known extraction pipelines (DATA.DF partially understood, TM2 format handled).
- Named or partially classified assets (models, textures, materials).
- Understanding of relevant file formats enough to extract and view visual assets reliably.
- Maps or environments located and documented in the binary.
- Materials organized by region (entrance, interior, courtyard, tower, bridge).
- A reliable pipeline that separates evidence from speculation: no aesthetic claim should be made without supporting extraction or screenshot.
- Partial decompilation (estimated 40-50%) to provide context for room loading, camera placement, and rendering logic.

## Future sources of visual evidence

- In-game screenshots captured from PCSX2 at consistent resolution and lighting.
- Extracted textures (TM2, clut, palette data) from DATA.DF or overlay assets.
- Model data (geometry, UVs, material assignments) from extracted mesh formats.
- Environment maps or room layout data from the binary.
- Asset dumps from known file ranges.
- Gameplay video captures for studying camera composition and lighting transitions.
- Comparison between in-game render and extracted assets to verify fidelity.
- Internal asset names from symbols or data tables, when available.
- Binary evidence such as material IDs, light parameters, or room references.

## Categories of aesthetic analysis

Each category represents a dimension of visual analysis to be developed when the front is active:

- **Exterior architecture**: castle silhouette, tower shapes, wall construction, bridge design, main entrance, outer defenses.
- **Interiors**: throne room, halls, corridors, save rooms, ceremonial spaces, prison areas.
- **Stone**: primary stone material, cut patterns, joint types, surface wear, color variation.
- **Wood**: beam construction, door planks, furniture, decay patterns.
- **Metal**: gates, grilles, chains, handles, fittings, oxidation.
- **Fabric**: curtains, banners, character clothing, flags, decay.
- **Chains**: link types, size, placement, suspension mechanics.
- **Doors**: types (wooden, metal, barred, ceremonial), framing, scale.
- **Bridges**: construction (stone, rope), width, railing, span, condition.
- **Stairs**: spiral, straight, narrow, wide, worn steps, riser height.
- **Windows**: shape (arched, narrow, round), placement, light admission, bars.
- **Courtyards**: open spaces, fountain areas, garden remnants, boundary walls.
- **Water**: pools, channels, reflection, color, movement.
- **Vegetation**: vines, moss, trees, overgrowth, integration with ruins.
- **Light**: directional, ambient, volumetric, color temperature, falloff.
- **Shadow**: shape, density, movement, hard vs soft.
- **Fog**: color, density, distance, layering.
- **Scale**: room height relative to character, doorway proportions, column size.
- **Void**: empty space as compositional element, negative area, silence.
- **Ruin**: collapse patterns, broken edges, fallen debris, erosion.
- **Composition**: framing, sight lines, verticality, depth, leading elements.

## How to transform visual analysis into reverse prompts

When the visual grammar is sufficiently documented, reverse prompts can be constructed. Each prompt should emerge from cataloged evidence, not intuition.

A reverse prompt for the operational visual language framework should contain:

- **Environment category**: which area type the prompt describes (castle exterior, interior corridor, ceremonial hall, courtyard ruin, bridge approach).
- **Materials**: specific stone type, wood condition, metal state, fabric presence.
- **Lighting**: direction, quality (hard/diffuse), color, source, shadow behavior.
- **Composition**: framing, depth layers, vertical/horizontal emphasis, size of empty space.
- **Scale**: room proportions, character-to-structure ratio.
- **Texture**: surface detail level, wear, crack density, overgrowth.
- **Atmosphere**: silence level, solitude weight, age feeling, mystery intensity.
- **Constraints**: elements that must not appear (fantasy exaggeration, gothic excess, clean surfaces).
- **Coherence markers**: how this scene relates to the overall castle visual system.

Example future prompt categories (not to be written now):

- ICO castle exterior prompt
- ICO interior corridor prompt
- ICO stone texture prompt
- ICO ceremonial hall prompt
- ICO abandoned courtyard prompt
- ICO bridge and tower prompt
- ICO shadowed stairway prompt

These categories are placeholders. Actual prompts must be written from evidence, not from memory or intuition.

## How to use this for a coherent expansion

If the project or a related effort ever considers an expansion, fan study, experimental reconstruction, or inspired prototype, the operational visual language would serve as the constraint system.

The grammar defines rules that keep new work aesthetically coherent with the original:

- Spaces must feel large and silent.
- Architecture must be monumental but emptied of function.
- Materials must look old, maintained only by neglect.
- Lighting must be soft, natural, and directional.
- Verticality must be present: stairs, towers, raised halls.
- The character must feel small relative to structures.
- The atmosphere must balance mystery, age, and a quiet sense of purpose.
- Narrative elements must remain minimal; the environment tells the story.

This is not about copying ICO. It is about understanding the rules that make its world coherent, so that any extension respects the same visual logic.

## Risks and limits

- Confusing inspired design with direct copying. The goal is to understand rules, not to reproduce assets.
- Generating generic "fantasy castle" aesthetics that lose ICO specific visual identity.
- Over-ornamenting. ICO is minimal; decorative excess would break coherence.
- Losing the minimalism that defines the game atmosphere.
- Transforming ICO into generic dark fantasy. ICO is not dark fantasy; it has its own register.
- Inventing assets or details that do not exist in the game.
- Using AI visual generation as a substitute for analytical work.
- Treating prompts as the deliverable instead of the grammar itself.
- Violating authorial coherence: the visual language should not be used to create content that contradicts the original tone.

## What not to do

- Do not activate this front before the technical prerequisites are met.
- Do not open DATA.DF solely for aesthetic curiosity.
- Do not mix this front with Yorda, capture, menu, or narrative analysis.
- Do not generate images during the planning or stand-by phase.
- Do not write final prompts without extracted evidence or organized screenshots.
- Do not reduce ICO to a generic medieval castle aesthetic.
- Do not deprioritize decompilation, runtime validation, or callback modeling for visual work.
- Do not treat this document as an active task list.

## Next steps when the front is activated

When the conditions are met and the decision is made to activate this front, the following steps should be taken in order:

1. Create a catalog of in-game screenshots organized by area, lighting, and category.
2. Classify each accessible environment by type and visual character.
3. Extract and categorize textures from DATA.DF or relevant asset containers.
4. Build a controlled visual vocabulary: terms for materials, lighting, scale, composition.
5. Write analysis documents for each aesthetic category (listed above).
6. Create experimental reverse prompts from cataloged evidence.
7. Test prompts visually (if visual generation tools are part of the experiment) and compare results against original game captures.
8. Adjust the grammar iteratively until the output is visually coherent with ICO.
9. Document the final operational visual language as a standalone deliverable of the project.
10. Archive all evidence, intermediate prompts, and adjustment logs for auditable traceability.

## Verdict

The operational visual language extraction is a valuable future front for the ico-reconstruction project. It aligns with the digital archaeology philosophy: to understand ICO completely, not just as code but as a constructed world.

However, it must remain in stand-by until the project has sufficient maturity in asset understanding, environment mapping, texture extraction, and partial decompilation. Starting earlier would risk speculation over evidence and dilute the current technical priorities.

When activated, this front should follow the same rigor as the technical work: evidence-based, auditable, conservative in claims, and clearly separated into confirmed observations, interpretations, and hypotheses.

Until then, this document serves as a placeholder — a recognition that the visual dimension of ICO is part of the project long-term scope, to be addressed when the technical foundation is solid enough to support it.

---

## PORTUGUÊS (PT-BR)

---

## Status

STAND BY / FUTURE TRACK

Esta frente não está ativa. Este é um documento de planejamento para uma fase futura do projeto ico-reconstruction. Ela não deve competir com as frentes atuais: descompilação, análise runtime, ferramentas splat, compiler matching, modelagem de descriptors/callbacks e cross-reference com o ICO-decomp.

## Resumo executivo

Este documento define uma frente futura de pesquisa para extrair a linguagem visual operacional do ICO. Quando o projeto atingir maturidade suficiente em descompilação, entendimento de assets, catalogação de ambientes e extração de texturas, esta frente poderá ser ativada para documentar sistematicamente como o mundo do jogo parece, respira e se comunica através do design visual. O objetivo não é produzir prompts isolados para geração de imagens, mas reconstruir uma gramática estética formal baseada em evidências extraídas do próprio jogo.

## Por que esta frente deve ficar em stand by

Esta frente requer condições que ainda não existem no projeto:

- O projeto ainda está consolidando subsistemas internos (modelo do dispatcher, domínio cloth, cadeia de callbacks ROPE, compiler matching). Estas são a prioridade atual.
- A extração de assets está explicitamente excluída da fase atual. DATA.DF, texturas, modelos e formatos gráficos estão na lista de "o que não investigar ainda" no AGENTS.md.
- A análise estética será mais forte e rigorosa se construída sobre evidências organizadas: formatos de arquivo conhecidos, assets nomeados, ambientes mapeados, texturas extraídas. Sem essa base, a análise arrisca ser especulativa.
- A prioridade atual é descompilação incremental e validação runtime. Iniciar uma frente visual agora dividiria o foco e atrasaria a reconstrução técnica.
- Quando esta frente for ativada, ela será apoiada pelo conhecimento técnico do trabalho de descompilação, tornando as afirmações estéticas auditáveis ao invés de impressionistas.

## O que significa "linguagem estética operacional"

Linguagem estética operacional é um sistema formal de regras, padrões, descrições e princípios visuais que explicam como o mundo do ICO parece, respira e se organiza. É mais amplo que paletas de cor ou bibliotecas de textura.

Inclui:

- ritmo arquitetônico: como torres, paredes, pontes e salões se repetem e variam;
- percepção de escala: como o personagem se relaciona com portas, colunas, salas;
- silêncio visual: áreas com detalhamento mínimo, espaço vazio, corredores longos;
- comportamento da luz: direção, queda, temperatura de cor, tipo de sombra;
- geometria da ruína: como paredes quebradas, tetos colapsados e superfícies erodidas são construídas;
- linguagem dos materiais: como a pedra envelhece, como o metal enferruja, como a água se acumula;
- sistema de proporções: altura do teto, largura do corredor, profundidade da escada, posição da janela;
- gramática dos ornamentos: quais elementos decorativos existem, onde aparecem, o que significam;
- vazio como ferramenta de design: o uso do espaço negativo para criar atmosfera.

O objetivo é extrair não apenas "quais texturas existem" mas "por que o castelo parece o que parece" como um sistema visual reproduzível.

## Diferença entre prompt visual e gramática estética

**Prompt visual**: Uma descrição textual projetada para gerar uma imagem específica. É estreito, orientado a saída e descartável. Responde "o que esta cena contém?"

**Gramática estética**: Um conjunto estruturado de regras e descrições que explica como um sistema visual opera em todas as instâncias. É analítico, reutilizável e baseado em evidências. Responde "quais são as regras que fazem qualquer cena neste mundo parecer coerente?"

Esta frente persegue o segundo. Prompts podem ser derivados da gramática, mas a gramática em si é o entregável principal.

## Objetivo futuro da frente

Quando ativada, esta frente visará:

1. Catalogar a estética visual do castelo de ICO em todas as áreas acessíveis.
2. Escrever descrições ambientais densas baseadas em evidências extraídas.
3. Construir descrições visuais por categoria (por área, material, condição de iluminação).
4. Apoiar possíveis experimentos visuais (estudos, reconstruções, análise comparativa) sem violar a estética original.
5. Criar uma base para expansão ou reinterpretação esteticamente coerente.
6. Documentar a linguagem visual como parte da produção de arqueologia digital do projeto, junto com as notas técnicas de pesquisa.

## Condições mínimas para iniciar

Esta frente não deve começar até que um subconjunto substancial do seguinte exista:

- Um catálogo de screenshots in-game organizado por área e condição de iluminação.
- Dumps de textura de pipelines de extração conhecidos (DATA.DF parcialmente entendido, formato TM2 tratado).
- Assets nomeados ou parcialmente classificados (modelos, texturas, materiais).
- Entendimento de formatos de arquivo relevantes o suficiente para extrair e visualizar assets confiavelmente.
- Mapas ou ambientes localizados e documentados no binário.
- Materiais organizados por região (entrada, interior, pátio, torre, ponte).
- Um pipeline confiável que separa evidência de especulação: nenhuma afirmação estética deve ser feita sem extração ou screenshot de suporte.
- Descompilação parcial (estimada 40-50%) para fornecer contexto para carregamento de salas, posicionamento de câmera e lógica de renderização.

## Fontes futuras de evidência visual

- Screenshots in-game capturadas do PCSX2 em resolução e iluminação consistentes.
- Texturas extraídas (TM2, clut, dados de paleta) do DATA.DF ou overlays.
- Dados de modelo (geometria, UVs, atribuições de material) de formatos de malha extraídos.
- Mapas de ambiente ou dados de layout de sala do binário.
- Dumps de assets de faixas de arquivo conhecidas.
- Capturas de vídeo de gameplay para estudar composição de câmera e transições de iluminação.
- Comparação entre render in-game e assets extraídos para verificar fidelidade.
- Nomes internos de assets de símbolos ou tabelas de dados, quando disponíveis.
- Evidências do binário como IDs de material, parâmetros de luz ou referências de sala.

## Categorias de análise estética

Cada categoria representa uma dimensão de análise visual a ser desenvolvida quando a frente estiver ativa:

- **Arquitetura externa**: silhueta do castelo, formas das torres, construção de muros, design de pontes, entrada principal, defesas externas.
- **Interiores**: salão do trono, salões, corredores, salas de save, espaços cerimoniais, áreas de prisão.
- **Pedra**: material de pedra primário, padrões de corte, tipos de junta, desgaste de superfície, variação de cor.
- **Madeira**: construção de vigas, tábuas de porta, mobiliário, padrões de deterioração.
- **Metal**: portões, grades, correntes, maçanetas, ferragens, oxidação.
- **Tecidos**: cortinas, bandeiras, roupa do personagem, bandeirolas, deterioração.
- **Correntes**: tipos de elo, tamanho, posicionamento, mecânica de suspensão.
- **Portas**: tipos (madeira, metal, gradeadas, cerimoniais), moldura, escala.
- **Pontes**: construção (pedra, corda), largura, corrimão, vão, condição.
- **Escadas**: espiral, reta, estreita, larga, degraus desgastados, altura do espelho.
- **Janelas**: formato (arcadas, estreitas, redondas), posicionamento, entrada de luz, grades.
- **Pátios**: espaços abertos, áreas de fonte, remanescentes de jardim, muros de contorno.
- **Água**: poças, canais, reflexão, cor, movimento.
- **Vegetação**: vinhas, musgo, árvores, crescimento excessivo, integração com ruínas.
- **Luz**: direcional, ambiente, volumétrica, temperatura de cor, queda.
- **Sombra**: formato, densidade, movimento, dura vs suave.
- **Névoa**: cor, densidade, distância, camadas.
- **Escala**: altura da sala relativa ao personagem, proporções de portal, tamanho de coluna.
- **Vazio**: espaço vazio como elemento composicional, área negativa, silêncio.
- **Ruína**: padrões de colapso, bordas quebradas, detritos caídos, erosão.
- **Composição**: enquadramento, linhas de visão, verticalidade, profundidade, elementos de condução.

## Como transformar análise visual em reverse prompts

Quando a gramática visual estiver suficientemente documentada, reverse prompts podem ser construídos. Cada prompt deve surgir de evidência catalogada, não de intuição.

Um reverse prompt para o framework de linguagem visual operacional deve conter:

- **Categoria de ambiente**: qual tipo de área o prompt descreve (exterior do castelo, corredor interno, salão cerimonial, pátio em ruínas, aproximação de ponte).
- **Materiais**: tipo específico de pedra, condição da madeira, estado do metal, presença de tecido.
- **Iluminação**: direção, qualidade (dura/difusa), cor, fonte, comportamento da sombra.
- **Composição**: enquadramento, camadas de profundidade, ênfase vertical/horizontal, tamanho do espaço vazio.
- **Escala**: proporções da sala, relação personagem-estrutura.
- **Textura**: nível de detalhe de superfície, desgaste, densidade de trincas, crescimento vegetal.
- **Atmosfera**: nível de silêncio, peso da solidão, sensação de idade, intensidade de mistério.
- **Restrições**: elementos que não devem aparecer (exagero fantástico, excesso gótico, superfícies limpas).
- **Marcadores de coerência**: como esta cena se relaciona com o sistema visual geral do castelo.

Exemplos de categorias futuras de prompt (não escrever agora):

- ICO castle exterior prompt
- ICO interior corridor prompt
- ICO stone texture prompt
- ICO ceremonial hall prompt
- ICO abandoned courtyard prompt
- ICO bridge and tower prompt
- ICO shadowed stairway prompt

Estas categorias são placeholders. Prompts reais devem ser escritos a partir de evidências, não de memória ou intuição.

## Como usar isso em uma expansão coerente

Se o projeto ou um esforço relacionado considerar uma expansão, estudo de fã, reconstrução experimental ou protótipo inspirado, a linguagem visual operacional serviria como sistema de restrição.

A gramática define regras que mantêm um trabalho novo esteticamente coerente com o original:

- Espaços devem parecer grandes e silenciosos.
- A arquitetura deve ser monumental mas esvaziada de função.
- Materiais devem parecer velhos, mantidos apenas pelo abandono.
- A iluminação deve ser suave, natural e direcional.
- Verticalidade deve estar presente: escadas, torres, salões elevados.
- O personagem deve parecer pequeno em relação às estruturas.
- A atmosfera deve equilibrar mistério, idade e um senso quieto de propósito.
- Elementos narrativos devem permanecer mínimos; o ambiente conta a história.

Isto não é sobre copiar ICO. É sobre entender as regras que tornam seu mundo coerente, para que qualquer extensão respeite a mesma lógica visual.

## Riscos e limites

- Confundir design inspirado com cópia direta. O objetivo é entender regras, não reproduzir assets.
- Gerar estética genérica de "castelo de fantasia" que perde a identidade visual específica do ICO.
- Exagerar ornamentos. ICO é minimalista; excesso decorativo quebraria a coerência.
- Perder o minimalismo que define a atmosfera do jogo.
- Transformar ICO em dark fantasy genérica. ICO não é dark fantasy; tem seu próprio registro.
- Inventar assets ou detalhes que não existem no jogo.
- Usar geração visual por IA como substituto de trabalho analítico.
- Tratar prompts como o entregável ao invés da gramática em si.
- Violar a coerência autoral: a linguagem visual não deve ser usada para criar conteúdo que contradiz o tom original.

## O que não fazer

- Não ativar esta frente antes dos pré-requisitos técnicos serem atendidos.
- Não abrir DATA.DF apenas por curiosidade estética.
- Não misturar esta frente com Yorda, captura, menu ou análise narrativa.
- Não gerar imagens durante a fase de planejamento ou stand by.
- Não escrever prompts finais sem evidências extraídas ou screenshots organizados.
- Não reduzir ICO a uma estética genérica de castelo medieval.
- Não despriorizar descompilação, validação runtime ou modelagem de callbacks por trabalho visual.
- Não tratar este documento como uma lista de tarefas ativa.

## Próximos passos quando a frente for ativada

Quando as condições forem atendidas e a decisão de ativar esta frente for tomada, os seguintes passos devem ser seguidos em ordem:

1. Criar um catálogo de screenshots in-game organizado por área, iluminação e categoria.
2. Classificar cada ambiente acessível por tipo e caráter visual.
3. Extrair e categorizar texturas do DATA.DF ou containers de asset relevantes.
4. Construir um vocabulário visual controlado: termos para materiais, iluminação, escala, composição.
5. Escrever documentos de análise para cada categoria estética (listadas acima).
6. Criar reverse prompts experimentais a partir de evidências catalogadas.
7. Testar prompts visualmente (se ferramentas de geração visual fizerem parte do experimento) e comparar resultados com capturas originais do jogo.
8. Ajustar a gramática iterativamente até que a saída seja visualmente coerente com ICO.
9. Documentar a linguagem visual operacional final como um entregável independente do projeto.
10. Arquivar todas as evidências, prompts intermediários e logs de ajuste para rastreabilidade auditável.

## Veredito

A extração de linguagem estética operacional é uma frente futura valiosa para o projeto ico-reconstruction. Está alinhada com a filosofia de arqueologia digital: entender ICO completamente, não apenas como código, mas como um mundo construído.

No entanto, deve permanecer em stand by até que o projeto tenha maturidade suficiente em entendimento de assets, mapeamento de ambientes, extração de texturas e descompilação parcial. Começar antes arriscaria especulação sobre evidência e diluiria as prioridades técnicas atuais.

Quando ativada, esta frente deve seguir o mesmo rigor do trabalho técnico: baseado em evidências, auditável, conservador em afirmações e claramente separado entre observações confirmadas, interpretações e hipóteses.

Até lá, este documento serve como um placeholder — um reconhecimento de que a dimensão visual do ICO faz parte do escopo de longo prazo do projeto, a ser abordada quando a fundação técnica for sólida o suficiente para sustentá-la.
