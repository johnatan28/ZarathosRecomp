# Roadmap — Ghost Rider Recomp (PS2)

Checklist vivo do projeto. Atualize conforme cada marco for concluído.

---

## Fase 0 — Preparação do Ambiente
- [ ] Definir `ghostrider-recomp` como workspace ativo no Antigravity (e recarregar para ativar o MCP do REA).
- [x] Verificar/instalar **Git** (`core.longpaths`), **Node.js LTS 26.13.1**, **CMake/Visual Studio 2022 Build Tools** e **Eclipse Temurin JDK 25.0.5**.
- [x] Instalar o **Ghidra 12.2** e a extensão **ghidra-emotionengine-reloaded v2.1.39** (suporte MIPS R5900 / MMI / VU0).
- [x] Instalar, registrar no Antigravity e validar o **REA (`rea-agents@6.9.0`)** conectado ao Ghidra (`doctor --provider ghidra`: `healthy: true`).
- [x] Clonar e compilar o **PS2Recomp** em `tools/PS2Recomp` (`ps2_recomp.exe`, `ps2_analyzer.exe`, `ps2_runtime.lib`, `ps2xStudio.exe` compilados em Release).

## Fase 1 — Binário Alvo e Análise Inicial no Ghidra
- [ ] Extrair o executável principal (`SLUS_...` ou `SLES_...`) da sua cópia legal do *Ghost Rider* (PS2) e colocá-lo em `game/`.
- [ ] Registrar região, nome do ELF, tamanho, seções e hash SHA-256 em `context/03-re-notes.md`.
- [ ] Importar o ELF no Ghidra com o processador **MIPS R5900 (Emotion Engine)** e rodar a análise inicial.
- [ ] Verificar se existem símbolos STABS/DWARF residuais, strings de debug, nomes de arquivos fonte (`.cpp`/`.c`) ou RTTI.

## Fase 2 — Triage de Funções e Engenharia Reversa (Ghidra + REA)
- [ ] Mapear o ponto de entrada (`entry` / `_start` → `main`) e o loop principal.
- [ ] Identificar e rotular funções da biblioteca padrão C/C++ e do PS2 SDK (`sce*`, `sif*`, `gs*`, `dma*`, `cdvd*`, `pad*`, threads/semáforos).
- [ ] Usar o **REA** para acelerar a identificação de subsistemas (engine, gerenciador de memória, sistema de arquivos, áudio, gráficos, input).
- [ ] Corrigir limites de função, *jump tables* e chamadas indiretas (ponteiros de função / vtables) que o analisador automático não resolveu.
- [ ] Rodar `ExportPS2Functions.java` (do `PS2Recomp`) para gerar o mapa de funções em `re/`.

## Fase 3 — Recompilação Estática (`PS2Recomp`)
- [ ] Criar arquivo de configuração `.toml` específico do *Ghost Rider* em `re/`.
- [ ] Rodar `ps2xAnalyzer` e `ps2xRecomp` sobre o ELF + mapa de funções exportado do Ghidra.
- [ ] Resolver eventuais instruções R5900/MMI/VU não cobertas ou funções problemáticas (via stubs ou ajustes no mapa).
- [ ] Obter geração limpa do código C++ recompilado e compilar o projeto CMake sem erros de sintaxe/linkagem.

## Fase 4 — Bring-up do Runtime / HLE
- [ ] Conectar o código recompilado ao runtime base do `PS2Recomp` em `runtime/`.
- [ ] Implementar/ajustar stubs de syscalls do kernel EE, alocação de memória e leitura de arquivos (redirecionando leitura de DVD/ISO para pasta local).
- [ ] Alcançar a execução das rotinas de boot até o loop principal sem crash imediato.
- [ ] Depurar chamadas indiretas faltantes e registradores divergentes (usando logs e comparação com PCSX2 quando necessário).

## Fase 5 — Subsistemas e Jogabilidade
- [ ] Entrada/controles (mapeamento DualShock 2 via SDL).
- [ ] Pipeline gráfico (DMA/GIF/GS e microprogramas VU — integração com o backend gráfico do runtime).
- [ ] Áudio (IOP/SPU2/ADPCM).
- [ ] Testes funcionais: menus → carregamento de fase → gameplay → estabilidade e performance.
