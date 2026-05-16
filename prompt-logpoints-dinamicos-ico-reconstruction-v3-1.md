# Prompt Técnico v3.1 — Decisão e POC para Logpoints Dinâmicos no PCSX2 / ICO Reconstruction

## Objetivo

Você é um engenheiro de sistemas, pesquisador de reverse engineering e/ou desenvolvedor com experiência em emuladores, MIPS/PS2, PCSX2, instrumentação dinâmica e pipelines de decompilação.

Sua tarefa é avaliar e propor o **menor experimento implementável** para substituir breakpoints manuais no PCSX2 por **logpoints dinâmicos confiáveis** no projeto **ICO Reconstruction**.

O foco desta avaliação não é discutir todas as possibilidades de forma abstrata. O foco é decidir:

1. Qual mecanismo deve ser usado como captura primária.
2. Quais mecanismos devem ser apenas auxiliares.
3. Quais mecanismos devem ser descartados para este objetivo.
4. Qual é o menor patch/experimento possível.
5. Como transformar a POC em uma ferramenta reutilizável depois.

---

## Contexto do projeto

O projeto **ICO Reconstruction** busca reconstruir e documentar o executável de ICO, PlayStation 2, sem redistribuir assets ou binários proprietários.

O projeto já possui:

- Indexação de disco ISO9660/BIN/CUE
- Parsing de ELF32 MIPS do PS2
- Scan de aproximadamente 3991 prologues MIPS detectados
- Call graph estático parcial
- Integração com Ghidra
- Integração com splat/splat64
- Funções promovidas a arquivos `.asm`
- Identificação do compilador:
  - EE GCC 2.9-991111-01
  - Sony fork
  - `-march=r5900`
  - `-mips3`
  - `-mgp64`
  - `-mabi=eabi`
  - `-msingle-float`
  - `-G0`
  - `-O2`

A dor atual é que várias funções precisam de contexto de runtime. O fluxo manual atual é:

1. Abrir PCSX2
2. Carregar ICO
3. Navegar até uma cena relevante
4. Setar breakpoint manual
5. Pausar em uma função
6. Inspecionar registradores
7. Copiar valores como `a0`, `a1`, `ra`, `sp`
8. Voltar ao Ghidra
9. Repetir para cada função

Esse processo não escala.

---

## Caso concreto

A análise estática travou no subsistema de cloth/rope physics.

Já foi identificado:

- Dispatcher em `0x001d37c8`
- Jump table em `0x00618fb0`
- Cadeia de callbacks envolvendo record `.data` associado a `ROPE`
- Função inicializadora/candidata em `0x001d27a8`
- A função `0x001d27a8` depende de um valor em `a1`
- O acesso crítico é algo como `[a1 + 0x30]`
- A origem de `a1` é determinada em runtime
- A análise estática não conseguiu resolver esse ponteiro com segurança

A pergunta prática é:

> Como capturar automaticamente, de forma confiável, o valor de `a1`, `ra` e o contexto de memória relevante quando o EE/MIPS entra em `0x001d27a8`, sem depender de breakpoint manual?

---

## Premissas importantes

### 1. O alvo inicial é um único endereço

A primeira POC deve mirar apenas:

```text
0x001d27a8
```

Não tente resolver o pipeline inteiro antes de provar que é possível capturar um evento confiável nesse endereço.

---

### 2. A captura precisa ser síncrona

Para campos derivados de ponteiros, como:

```text
[a1 + 0x30]
```

considere que a captura só é evidência forte se ocorrer **sincronamente ao evento de entrada da função**, antes que o estado do jogo avance.

Se o hook captura `a1`, mas um processo externo usa PINE depois para ler `[a1 + 0x30]`, a memória pode já ter mudado.

Portanto, avalie explicitamente se o dump de memória precisa ocorrer dentro do próprio hook/logpoint interno.

---

### 3. PINE provavelmente não deve ser o mecanismo primário

Assuma como hipótese inicial:

> PINE é inadequado para detecção confiável de entrada exata em função, salvo se você demonstrar tecnicamente o contrário.

PINE pode ser útil para:

- validar conexão com PCSX2
- ler memória manualmente
- automatizar inspeções auxiliares
- aplicar patches pontuais
- comparar estado antes/depois
- scripts de suporte

Mas não trate polling de PC via PINE como solução primária sem justificar rigorosamente.

---

### 4. Savestate deve ser considerado parte da estratégia

O processo manual de navegar até uma cena com cloth ativo pode levar 10–20 minutos.

Avalie usar um **savestate imediatamente antes de uma cena com cloth ativo** como ponto de entrada reproduzível para a POC.

Responda:

- O hook interno funciona após carregar savestate?
- BREAK/injeção funciona após carregar savestate?
- PINE funciona após carregar savestate?
- Existe risco de o cloth já estar inicializado e a função `0x001d27a8` não ser chamada novamente?
- O savestate é pré-condição prática para concluir a POC em menos de 4 horas?

#### Ponto crítico sobre EE Interpreter + savestate

A POC pode depender de carregar um savestate e rodar o PCSX2 com EE em modo Interpreter.

Avalie explicitamente:

- Ao carregar um savestate no PCSX2 com EE Interpreter, o estado de registradores, memória, PC, stack e contexto necessário do EE é restaurado fielmente?
- Existem diferenças relevantes entre carregar o savestate em Recompiler e depois alternar para Interpreter?
- O Interpreter precisa de algum estado de inicialização que o savestate não preserve completamente?
- Uma falha em observar `0x001d27a8` após savestate poderia significar:
  - hook errado;
  - endereço errado;
  - função já inicializada antes do savestate;
  - diferença Interpreter/Recompiler;
  - ou savestate restaurado em ponto inadequado?
- Qual teste mínimo separa essas causas?

---

### 5. Âncoras reais do PCSX2 para a POC

Para evitar uma resposta genérica sobre “procurar o loop da CPU”, considere como hipótese inicial de implementação:

- Arquivo candidato para EE Interpreter:
  - `pcsx2/Interpreter.cpp`
- Função candidata do loop de execução do EE:
  - `intExecute()`
- Estrutura global de registradores do EE:
  - `cpuRegs`
- Acesso conceitual aos registradores GPR:
  - `cpuRegs.GPR.n.a0.UL[0]`
  - `cpuRegs.GPR.n.a1.UL[0]`
  - `cpuRegs.GPR.n.a2.UL[0]`
  - `cpuRegs.GPR.n.a3.UL[0]`
  - `cpuRegs.GPR.n.sp.UL[0]`
  - `cpuRegs.GPR.n.ra.UL[0]`
- PC atual:
  - `cpuRegs.pc`

A avaliação deve verificar se essas âncoras ainda são corretas para a versão moderna do PCSX2, mas não deve responder de forma vaga. Se algum nome mudou, indique como localizar o equivalente.

---

### 6. Hipótese sobre MTVU/VU1

O risco de MTVU/VU1 deve ser tratado com contexto.

Hipótese razoável:

> `0x001d27a8` foi encontrado no ELF MIPS principal de ICO, portanto é código EE/R5900. O VU1 executa microcode separado, não o ELF MIPS. Assim, um hook no EE deve capturar a entrada em `0x001d27a8` independentemente de MTVU, desde que a função seja realmente executada.

Avalie essa hipótese.

Responda:

- O MTVU pode afetar a captura do endereço EE `0x001d27a8`?
- Ou o MTVU só seria relevante para partes do cloth executadas em VU1/microcode?
- Para o problema imediato — capturar `a1` entrando em uma função EE — o hook EE é suficiente?
- Em que cenário seria necessário instrumentar VU1 também?

---

## Abordagens a avaliar

Avalie as abordagens abaixo, mas não trate todas como igualmente boas.

Ao final, você deve escolher uma abordagem primária.

---

### Abordagem A — Polling externo via PINE

Um processo Python lê periodicamente o PC via PINE:

```python
while True:
    pc = pine.read_pc()

    if pc == 0x001d27a8:
        regs = pine.read_registers(["a0", "a1", "a2", "a3", "sp", "ra"])
        write_event(pc, regs)
```

Avalie:

1. Isso consegue capturar entrada exata em função MIPS?
2. Qual seria o risco de miss?
3. A latência de IPC/socket torna a abordagem estruturalmente frágil?
4. Essa abordagem serve apenas como baseline/controle negativo?
5. Em que contexto PINE ainda tem valor real?

Veredito esperado:

- Viável como primário
- Viável apenas como auxiliar
- Inviável como primário

---

### Abordagem B — Injeção de BREAK/SYSCALL via PINE

Fluxo:

```text
1. Ler instrução original em 0x001d27a8
2. Escrever BREAK/SYSCALL no endereço
3. Esperar PCSX2 pausar/trapar
4. Ler registradores
5. Restaurar instrução original
6. Continuar execução
```

Avalie:

1. O PCSX2 expõe evento externo confiável de breakpoint hit?
2. PINE permite detectar o trap de forma limpa?
3. Escrever em código recompilado invalida blocos JIT?
4. BREAK/restore causaria degradação ou instabilidade?
5. Isso exige EE Interpreter?
6. É aceitável para 1 endereço crítico?
7. É inadequado para dezenas/centenas de chamadas por frame?

Veredito esperado:

- Viável como primário
- Viável apenas para casos pontuais
- Inviável
- Não recomendado para este objetivo

---

### Abordagem C — Hook/logpoint interno mínimo no PCSX2

Criar um patch mínimo no PCSX2 para emitir evento quando o EE/MIPS entrar no endereço alvo.

Para a POC, o patch pode ser hardcoded:

```cpp
if (cpuRegs.pc == 0x001d27a8) {
    dump_registers_and_memory_to_file();
}
```

Âncoras de implementação para avaliar:

```text
Arquivo candidato: pcsx2/Interpreter.cpp
Loop candidato: intExecute()
PC atual: cpuRegs.pc
Registradores: cpuRegs.GPR.n.<reg>.UL[0]
```

Avalie:

1. Essas âncoras estão corretas na versão moderna do PCSX2?
2. Se estiverem desatualizadas, onde procurar o equivalente?
3. Esse caminho elimina o problema de polling miss?
4. O hook deve entrar no EE Interpreter, no recompilador, ou em ambos?
5. ICO é navegável em EE Interpreter até a cena relevante?
6. Se precisar de recompilador, onde inserir o hook?
7. O custo de checar `pc == 0x001d27a8` é aceitável?
8. O log deve ser arquivo, stdout, ring buffer, fila assíncrona ou socket local?
9. Como evitar travar a emulação ao escrever log?
10. O dump de memória deve acontecer dentro do hook?
11. Esse patch pode ser mantido como diff isolado?

Veredito esperado:

- Recomendado como primário
- Viável com ressalvas
- Inviável
- Recomendado apenas para POC

---

### Abordagem D — Híbrido PINE + Hook interno

Arquitetura possível:

```text
Hook interno:
  - detecta entrada exata em função
  - captura registradores
  - captura memória contextual síncrona
  - emite JSONL/CSV bruto

PINE:
  - inspeção auxiliar
  - leitura manual de memória fora do momento crítico
  - scripts de apoio
  - validação de estado
  - patches pontuais

Analyzer Python:
  - importa JSONL/CSV
  - converte para SQLite
  - resolve ra → caller
  - cruza dumps de memória
  - gera relatório Markdown
```

Avalie:

1. Essa é a melhor divisão de responsabilidades?
2. O hook deve ser a fonte primária de eventos?
3. PINE deve ser apenas auxiliar?
4. O Analyzer Python deve ser responsável por transformar logs em evidência humana?
5. Isso evita um fork complexo de longo prazo?

Veredito esperado:

- Arquitetura recomendada
- Viável com ressalvas
- Complexa demais para POC
- Não recomendada

---

## Dados mínimos que a POC deve capturar

Para o primeiro evento em `0x001d27a8`, capture no mínimo:

```json
{
  "pc": "0x001d27a8",
  "a0": "...",
  "a1": "...",
  "a2": "...",
  "a3": "...",
  "sp": "...",
  "ra": "...",
  "timestamp": "...",
  "frame": "se disponível"
}
```

Mas avalie se a POC também deve capturar memória contextual sincronamente:

```json
{
  "memory_captures": [
    {
      "base": "a1",
      "offset": 0,
      "size": 128,
      "purpose": "dump da struct candidata apontada por a1"
    },
    {
      "base": "a1",
      "offset": 48,
      "deref": true,
      "size": 64,
      "purpose": "conteúdo apontado por [a1 + 0x30]"
    },
    {
      "base": "sp",
      "offset": 0,
      "size": 64,
      "purpose": "contexto de stack"
    }
  ]
}
```

Responda:

1. Capturar só `a1` é suficiente?
2. É obrigatório capturar `[a1 + 0x30]` no mesmo momento?
3. Faz sentido dumpar 64/128 bytes ao redor de `a1`?
4. Faz sentido dumpar stack?
5. Faz sentido capturar instruções ao redor de `ra`?
6. O dump de memória deve ser configurável por alvo em uma versão posterior?

---

## Correlação de `ra` e delay slot

No MIPS, após um `jal`, o `ra` normalmente aponta para:

```text
jal_addr + 8
```

por causa do delay slot.

Avalie:

1. Como calcular corretamente o caller a partir de `ra`?
2. O analyzer deve considerar `ra - 8` como ponto provável do `jal`?
3. Deve registrar instruções em:
   - `ra - 8`
   - `ra - 4`
   - `ra`
4. Tail calls, `jalr`, callbacks indiretos ou wrappers podem quebrar a correlação?
5. Mesmo com ressalvas, `ra` provavelmente distingue qual caller chamou uma função com múltiplos callers estáticos?

---

## POC vs ferramenta final

Diferencie claramente três fases:

---

### Fase 1 — POC hardcoded

Objetivo:

```text
Provar que é possível capturar um evento confiável em 0x001d27a8.
```

Características:

- 1 endereço hardcoded
- log simples
- sem SQLite obrigatório
- sem `probe_targets.json`
- sem UI
- sem generalização
- usar savestate se possível

---

### Fase 2 — Versão inicial reutilizável

Objetivo:

```text
Capturar múltiplos alvos do cluster cloth.
```

Características:

- `probe_targets.json`
- labels por função
- captura configurável por alvo
- JSONL bruto
- importador Python para SQLite
- tabela separada para dumps de memória

---

### Fase 3 — Infraestrutura de pesquisa

Objetivo:

```text
Gerar evidência documentável para o projeto.
```

Características:

- relatórios Markdown
- correlação `ra → caller`
- classificação de regiões de memória
- comparação entre chamadas
- estabilidade de offsets
- output citável em `research/elf/ghidra-revXXX-*.md`

---

## Persistência recomendada

Avalie se este desenho é adequado para a fase 2/3.

### Tabela principal

```sql
CREATE TABLE probe_events (
  id INTEGER PRIMARY KEY,
  session_id TEXT NOT NULL,
  ts_ns INTEGER NOT NULL,
  frame INTEGER,
  pc INTEGER NOT NULL,
  fn_addr INTEGER NOT NULL,
  fn_label TEXT,
  subsystem TEXT,

  a0 INTEGER,
  a1 INTEGER,
  a2 INTEGER,
  a3 INTEGER,
  sp INTEGER,
  ra INTEGER,

  caller_fn_guess INTEGER,
  caller_offset INTEGER,

  a1_region TEXT,
  a1_plus_30_value INTEGER,

  raw_json TEXT
);

CREATE INDEX idx_probe_fn ON probe_events(fn_addr);
CREATE INDEX idx_probe_session_time ON probe_events(session_id, ts_ns);
CREATE INDEX idx_probe_caller ON probe_events(caller_fn_guess);
```

### Tabela de dumps

```sql
CREATE TABLE probe_memory_dumps (
  id INTEGER PRIMARY KEY,
  event_id INTEGER NOT NULL REFERENCES probe_events(id),
  base_reg TEXT,
  base_addr INTEGER NOT NULL,
  offset INTEGER NOT NULL DEFAULT 0,
  size INTEGER NOT NULL,
  data BLOB NOT NULL,
  purpose TEXT
);
```

Avalie:

1. SQLite é adequado?
2. JSONL bruto também deve existir?
3. Dumps devem ir para tabela separada?
4. Inserção deve ser feita em batch?
5. WAL mode é recomendado?
6. Esse formato facilita gerar Markdown depois?

---

## Riscos obrigatórios a analisar

Analise explicitamente:

1. Build do PCSX2 no Linux/Ubuntu 24
2. Diferença entre EE Interpreter e Recompiler
3. Possível necessidade de hook também no recompilador
4. Overhead de logging
5. Escrita síncrona travando a emulação
6. Endereços virtuais vs físicos
7. JIT/cache invalidation se usar BREAK
8. Ponteiros para heap sem struct conhecida
9. Necessidade de classificar regiões de memória
10. Correlação imprecisa de caller
11. Delay slots
12. `jalr`/callbacks indiretos
13. Savestate não disparar inicialização novamente
14. Fidelidade do savestate ao alternar ou usar EE Interpreter
15. Endereços específicos de ICO USA / `SCUS_971.13`
16. MTVU/VU1:
    - hipótese: `0x001d27a8` é código EE/R5900 vindo do ELF MIPS principal;
    - VU1 executa microcode separado;
    - portanto, o hook EE deve capturar o problema imediato de `a1`;
    - VU1 só vira problema se o alvo futuro for microcode ou estado gerado exclusivamente no VU1.

---

## Saída obrigatória

Sua resposta deve ter exatamente estas seções:

---

## 1. Decisão técnica

Diga claramente:

```text
Mecanismo primário recomendado:
Mecanismos auxiliares:
Mecanismos descartados:
Motivo da decisão:
```

Não responda que todas as abordagens são igualmente viáveis.

---

## 2. Avaliação das abordagens

Para cada abordagem A, B, C e D, responda:

```text
Veredito:
Justificativa técnica:
Risco principal:
Recomendação:
```

---

## 3. Menor POC possível

Descreva o menor experimento em menos de 4 horas.

Inclua:

- usar ou não savestate
- como testar se o savestate restaura corretamente sob EE Interpreter
- endereço hardcoded
- registradores mínimos
- memória mínima
- formato mínimo do log
- como validar se funcionou
- como saber se falhou

---

## 4. Menor patch conceitual no PCSX2

Use obrigatoriamente como ponto de partida estas âncoras:

```text
Arquivo candidato: pcsx2/Interpreter.cpp
Loop candidato: intExecute()
PC atual: cpuRegs.pc
Registradores: cpuRegs.GPR.n.<reg>.UL[0]
```

Descreva:

- se essas âncoras ainda são corretas;
- arquivo(s) reais/prováveis a investigar;
- ponto provável do loop de execução onde inserir o hook;
- se deve mirar Interpreter primeiro;
- se precisa considerar Recompiler depois;
- pseudocódigo do patch;
- como ler memória EE de forma segura no hook;
- estratégia de logging sem travar a emulação.

Não precisa fornecer patch real compilável, mas precisa ser específico o suficiente para orientar um desenvolvedor.

---

## 5. Dados a capturar

Diga exatamente o que capturar na POC:

- registradores
- memória
- instruções ao redor de `ra`
- timestamp/frame
- qualquer outro campo essencial

---

## 6. Como transformar em ferramenta reutilizável

Explique a evolução:

```text
POC hardcoded → probe_targets.json → JSONL → SQLite → relatório Markdown
```

Inclua recomendações sobre schema e organização no repo.

---

## 7. Riscos restantes

Liste o que ainda pode inviabilizar ou limitar a proposta.

---

## 8. Veredito final

Responda de forma direta:

```text
Eu implementaria primeiro: ...
Eu evitaria primeiro: ...
O sucesso mínimo seria: ...
O próximo passo após sucesso seria: ...
```

---

## Critério de sucesso da POC

A POC é bem-sucedida se gerar automaticamente uma evidência semelhante a:

```json
{
  "pc": "0x001d27a8",
  "label": "cloth_initializer_candidate",
  "a0": "0x....",
  "a1": "0x....",
  "a2": "0x....",
  "a3": "0x....",
  "sp": "0x....",
  "ra": "0x....",
  "caller_probe": {
    "ra_minus_8": "0x....",
    "ra_minus_4": "0x....",
    "ra": "0x...."
  },
  "memory": {
    "a1_dump_128": "...",
    "a1_plus_30_value": "0x....",
    "a1_plus_30_deref_dump_64": "..."
  },
  "frame": 12345,
  "timestamp": "..."
}
```

E permitir responder:

1. Quem chamou `0x001d27a8` em runtime?
2. Qual valor real chegou em `a1`?
3. O que havia em `[a1 + 0x30]` naquele momento?
4. O valor parece estável entre chamadas?
5. O dado é forte o suficiente para voltar ao Ghidra e documentar a struct?
6. O pipeline pode ser expandido para os ~22 alvos do cluster cloth?

---

## Tom esperado

Responda como uma avaliação técnica honesta, orientada à implementação.

Evite:

- generalidades
- “depende”
- respostas diplomáticas
- tratar todas as opções como equivalentes
- propor arquitetura grande antes da POC

Prefira:

- decisão clara
- tradeoffs explícitos
- menor experimento possível
- recomendação prática
- riscos reais
- caminho incremental
