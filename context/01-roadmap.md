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
- [x] Extrair o executável principal (`SLUS_213.06`, NTSC-U v1.01) e `MODULES/` da cópia local do *Ghost Rider* (PS2) para `game/`.
- [x] Registrar região, nome do ELF, tamanho, seções e hash SHA-256 em `context/03-re-notes.md`.
- [x] Importar o ELF no Ghidra 12.2 com o processador **MIPS R5900 (`r5900:LE:32:default`)** e rodar a análise inicial (`8.454` funções, `10.319` strings).
- [x] Verificar strings de debug/engine: confirmado **RenderWare Graphics/Physics/Audio/FS 3.7** + **Bink Video** + **PS2 SDK 3.0.0 (`IOPRP300.IMG`)**.

## Fase 2 — Triage de Funções e Engenharia Reversa (Ghidra + PS2Recomp)
- [x] Mapear o ponto de entrada (`0x0025B718`, `_InitSys` em `0x0025B8A8`) e `_mips_gp0_value = 0x005B7E70`.
- [x] Identificar e rotular `334` funções do PS2 SDK (`89` mapeadas para stubs do `ps2xRuntime`, `320` aplicadas diretamente no projeto Ghidra).
- [x] Exportar mapa de funções do Ghidra (`re/ghostrider_ghidra_functions.csv`) via `re/scripts/ExportPS2Headless.java`.
- [ ] Revisar funções sinalizadas como self-modifying / jump tables faltantes e expandir cobertura de símbolos de jogo/RenderWare.

## Fase 3 — Recompilação Estática (`PS2Recomp`)
- [x] Criar arquivo de configuração `re/ghostrider.toml` combinando o mapa do Ghidra + stubs SCE SDK do `ps2_analyzer`.
- [x] Rodar `ps2_recomp.exe re/ghostrider.toml` sobre o ELF (`8.435` funções carregadas do Ghidra, `8.338` recompiladas, `97` stubs, `0` falhas de decode, `0` instruções não suportadas, `0` erros).
- [x] Criar projeto CMake (`CMakeLists.txt` + `runtime/src/main.cpp` + `runtime/src/ghostrider_overrides.cpp`) com Unity Build + PCH linkado com `ps2_runtime.lib`.
- [x] Compilar o executável nativo `build/Release/ghostcomp.exe` em Release sem erros de compilação ou linkagem.

## Fase 4 — Bring-up do Runtime / HLE
- [x] Conectar o código recompilado ao runtime base do `PS2Recomp` (`CMakeLists.txt` + `runtime/src/main.cpp` + `runtime/src/ghostrider_overrides.cpp`).
- [x] Recuperar ponteiros indiretos (`.ctors`, *vtables* C++, callbacks `lui+addiu` e *leaf functions* do RenderWare `PS2All.csl`) — total de **11.309 funções** (`11.211` recompiladas, `98` stubs, `0` erros).
- [x] Isolar o pool `guestMalloc` do `ps2xRuntime` em RDRAM alta (`0x01D00000`–`0x01EF0000`) para evitar colisão com o heap interno do jogo (`0x00604F70+`) durante `sceGsResetGraph`.
- [x] Carregar e executar todos os **8 módulos IOP `.IRX` originais** (`SIO2MAN`, `DBCMAN`, `SIO2D`, `PADMAN`, `LIBSD`, `SDRDRV`, `RTFSSIOP`, `RWA`) + alcançar a **primeira renderização real em tela cheia `640x448`** via GS/DMA/GIF!
- [ ] Avançar além da tela inicial / espera de `sceMc2` (`FUN_0036b870`) para vídeos Bink (`.BIK`), leitura de `GR.ARC`, menu principal e gameplay.

## Fase 5 — Subsistemas e Jogabilidade
- [ ] Entrada/controles (mapeamento DualShock 2 via SDL).
- [ ] Pipeline gráfico (DMA/GIF/GS e microprogramas VU — integração com o backend gráfico do runtime).
- [ ] Áudio (IOP/SPU2/ADPCM).
- [ ] Testes funcionais: menus → carregamento de fase → gameplay → estabilidade e performance.
