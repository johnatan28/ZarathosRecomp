# Contexto Mestre do Projeto — Ghost Rider Recomp

## 1. Objetivo

Construir **`ZarathosRecomp`**, uma recompilação estática nativa para PC de **Ghost Rider (2007)** (Climax Studios / 2K Games), convertendo o executável original do console para C/C++ compilável nativamente, apoiado por análise no **Ghidra** e automação via **REA (Reverse Engineer Anything)**.

- **Nome do repositório GitHub:** `ZarathosRecomp` (`https://github.com/johnatan28/ZarathosRecomp`)
- **Licença:** GNU GPL v3.0 (alinhada ao `PS2Recomp` e ao padrão da comunidade de recompilação estática)

## 2. Escolha da Plataforma: Por que PlayStation 2 (PS2)?

- **Por que não Xbox?** A versão de Xbox original de *Ghost Rider (2007)* foi **cancelada** antes do lançamento comercial. As únicas versões lançadas oficialmente foram **PS2**, **PSP** e **GBA**.
- **Por que PS2 é a melhor opção hoje:**
  1. É a versão principal de console de mesa (completa em conteúdo e qualidade gráfica).
  2. Possui ecossistema maduro e aberto para engenharia reversa e recompilação estática:
     - **CPU alvo:** MIPS R5900 (Emotion Engine, little-endian, 64/128-bit MMI + VU0/VU1).
     - **Análise estática:** **Ghidra** + extensão [`chaoticgd/ghidra-emotionengine-reloaded`](https://github.com/chaoticgd/ghidra-emotionengine-reloaded).
     - **Automação com IA:** **REA (`morluto/rea` / `rea-agents`)** via MCP, operando sobre o backend local do Ghidra para consulta de funções, strings, referências cruzadas e pseudocódigo baseado em evidências.
     - **Recompilador estático:** [`ran-j/PS2Recomp`](https://github.com/ran-j/PS2Recomp) (`ps2xAnalyzer` + `ps2xRecomp` + runtime base + script de exportação `ExportPS2Functions.java` para Ghidra).

## 3. Fluxo Técnico de Alto Nível

```text
[Sua cópia legal do jogo (PS2)]
          │
          ▼  (extração do executável principal SLUS/SLES ELF)
[Ghidra + ghidra-emotionengine-reloaded]  ◄──►  [REA (rea-agents MCP)]
          │                                       • Busca strings / RTTI / SDK
          │                                       • Identifica limites de função
          │                                       • Resolve jump tables / vtables
          ▼  (ExportPS2Functions.java → .toml / .csv)
[PS2Recomp (ps2xAnalyzer + ps2xRecomp)]
          │
          ▼  (gera C++ nativo correspondente ao MIPS R5900)
[Runtime / HLE específico do Ghost Rider (C++ / CMake)]
          │
          ▼  (syscalls, memória, filesystem, gráficos, áudio, SDL input)
[Executável nativo PC]
```

## 4. Princípios do Projeto

- **Clean-room & legalidade:** O repositório nunca armazena ISO, ELF ou assets do jogo. Apenas código próprio, configurações, offsets/símbolos e scripts.
- **Evidência antes de suposição:** Toda função renomeada, estrutura mapeada ou stub implementado deve ser confirmado no disassembly/pseudocódigo do Ghidra (diretamente ou via REA).
- **Iteração incremental:** Primeiro alcançar *build* limpo do código recompilado e primeiros logs/chamadas de inicialização (*bring-up*), antes de perseguir renderização completa ou otimizações.
