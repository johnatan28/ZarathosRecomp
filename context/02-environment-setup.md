# Guia de Setup do Ambiente (Windows)

Este documento reúne o ambiente configurado e verificado neste PC para engenharia reversa e recompilação do **Ghost Rider (PS2)**.

---

## 1. Ferramentas Instaladas e Verificadas

- **Git:** `C:\Program Files\Git\cmd\git.exe` (`core.longpaths = true` habilitado para submódulos Windows)
- **Node.js LTS:** `26.13.1` (`C:\Program Files\nodejs`) — exigido pelo `rea-agents@6.9.0`
- **JDK:** Eclipse Temurin **JDK 25.0.5+8 (64-bit)**
  - `JAVA_HOME`: `C:\Program Files\Eclipse Adoptium\jdk-25.0.5.8-hotspot`
- **Compilador C++ / CMake:** Visual Studio 2022 Build Tools (`MSVC 19.44` + CMake 3.31 + Ninja embutidos)
  - `cmake.exe`: `C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe`

---

## 2. Ghidra 12.2 + Extensão PS2 (Emotion Engine)

- **Instalação:** `C:\Users\Johnatan\tools\ghidra\ghidra_12.2_PUBLIC`
  - `GHIDRA_INSTALL_DIR`: `C:\Users\Johnatan\tools\ghidra\ghidra_12.2_PUBLIC`
- **Extensão PS2 instalada:** `ghidra-emotionengine-reloaded` (`v2.1.39` para Ghidra 12.2) em:
  - `C:\Users\Johnatan\tools\ghidra\ghidra_12.2_PUBLIC\Ghidra\Extensions\`
- **Como abrir o Ghidra (GUI):**
  ```powershell
  & "C:\Users\Johnatan\tools\ghidra\ghidra_12.2_PUBLIC\ghidraRun.bat"
  ```

---

## 3. REA (Reverse Engineer Anything — `rea-agents@6.9.0`)

- **Status:** Registrado como servidor MCP no Antigravity (`C:\Users\Johnatan\.gemini\config\mcp_config.json`) + skill `reverse-engineer-anything` instalada.
- **Provider verificado:** `ghidra` (`12.2`, `decompile.exe` x86_64, `JDK 25.0.5`) — `healthy: true`.
- **Importante:** Reinicie/recarregue o cliente Antigravity (ou abra a pasta do projeto como workspace) para que a nova conexão MCP do REA fique ativa nas ferramentas do agente.
- **Comando de diagnóstico:**
  ```powershell
  npx.cmd -y rea-agents@latest doctor --provider ghidra --json
  ```

---

## 4. PS2Recomp

- **Clone:** `tools/PS2Recomp` (com submódulos)
- **Diretório de build:** `tools/PS2Recomp/b` (CMake + Visual Studio 17 2022 x64 — compilado em `Release`)
- **Binários gerados:**
  - `tools/PS2Recomp/b/ps2xRecomp/Release/ps2_recomp.exe`
  - `tools/PS2Recomp/b/ps2xAnalyzer/Release/ps2_analyzer.exe`
  - `tools/PS2Recomp/b/ps2xRuntime/Release/ps2_runtime.lib`
  - `tools/PS2Recomp/b/ps2xStudio/Release/ps2xStudio.exe`
  - `tools/PS2Recomp/b/ps2xTest/Release/ps2x_tests.exe`
- **Script de exportação para Ghidra:**
  - `tools/PS2Recomp/ps2xRecomp/tools/ghidra/ExportPS2Functions.java`

---

## 5. Executável do Jogo (sua cópia legal)

Coloque o ELF principal extraído do seu disco de PS2 (normalmente `SLUS_213.06` na versão NTSC-U ou `SLES_...` na versão PAL) dentro da pasta `game/`.

> A pasta `game/` está listada no `.gitignore` para garantir que nenhum arquivo do jogo vá para o controle de versão.
