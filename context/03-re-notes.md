# Caderno de Engenharia Reversa — Ghost Rider (PS2)

Registro de descobertas verificadas a partir da análise estática (`SLUS_213.06`, Ghidra 12.2 + `ghidra-emotionengine-reloaded` + `ps2_analyzer`).

---

## 1. Metadados do Binário Alvo

- **Região / Serial:** NTSC-U — `SLUS_213.06` (`SYSTEM.CNF`: `BOOT2 = cdrom0:\SLUS_213.06;1`, `VER = 1.01`, `VMODE = NTSC`)
- **Nome do arquivo ELF:** `SLUS_213.06`
- **Tamanho (bytes):** `4.936.852` bytes (~4,71 MiB)
- **SHA-256:** `1C7FA300B9AAA700F13E8D334068937B7C96B494A011071E1C3F8489906DAF8E`
- **Processador Ghidra:** `r5900:LE:32:default` (MIPS R5900 / Emotion Engine)
- **Image Base:** `0x00100000`
- **Ponto de entrada (`entry`):** `0x0025B718` (`sub_0025B718`)
- **GP0 (`_mips_gp0_value`):** `0x005B7E70`
- **Engine & Middleware identificados (evidência de strings + disco):**
  - **Engine principal:** **RenderWare Graphics 3.7** (`c:/daily/rwsdk/plugin/pds/sky2/...`, `PS2All.csl`, `PS2Mesh.csl`, `PS2Im3DFastTransform.csl`) + **RenderWare Studio** (`RWS::RWSGUID*`)
  - **Física:** **RenderWare Physics 3.7** (`//Physics/Rwp37Active/src/...`, `RwpBox.c`, `RwpRagdollDef.c`, `RwpGjk.c`)
  - **Áudio:** **RenderWare Audio** (`MODULES/RWS/RWA.IRX`, trilhas `.RWS` em `DATA/MUSIC/`)
  - **Filesystem:** **RenderWare FileSystem** (`MODULES/RWS/RTFSSIOP.IRX`) + arquivo principal `GR.ARC` + `start.cfg`
  - **Vídeos:** **Bink Video** (`DATA/MOVIES/*.BIK`, strings `Not a Bink file.`, `Error reading Bink header.`)
  - **Imagem:** **libjpeg** (`JFIF APP0 marker`)
  - **Estruturas Climax:** `ClimaxT1DictionarySchema`, `ClimaxP1DictionarySchema`, `CSceneObject`, `CStyledSceneObject`
  - **PS2 IOP SDK:** `MODULES/IOPRP300.IMG` (SDK 3.0.0) + `SIO2MAN.IRX`, `DBCMAN.IRX`, `SIO2D.IRX`, `PADMAN.IRX`, `LIBSD.IRX`, `SDRDRV.IRX`, `MC2_D.IRX`

---

## 2. Mapa de Seções ELF (Ghidra 12.2 / R5900)

| Seção | Início | Fim | Tamanho (bytes) | Perm | Init | Observações |
|-------|--------|-----|-----------------|------|------|-------------|
| `.text` | `0x00100000` | `0x003A27E3` | `2.762.724` | RWX | sim | Código EE principal |
| `.vutext` | `0x003A27F0` | `0x003A540F` | `11.296` | RWX | sim | Microprogramas VU0/VU1 (RenderWare PDS) |
| `.data` | `0x003A5480` | `0x00577F27` | `1.911.464` | RWX | sim | Dados inicializados |
| `.vudata` | `0x00577F30` | `0x00577FEF` | `192` | RWX | sim | Dados VU |
| `.rodata` | `0x00578000` | `0x005AFC07` | `228.360` | RWX | sim | Strings / constantes |
| `.gcc_except_table` | `0x005AFC80` | `0x005AFE4F` | `464` | RWX | sim | Tabelas de exceção C++ |
| `.lit4` | `0x005AFE80` | `0x005B067B` | `2.044` | RWX | sim | Literais 32-bit |
| `.sdata` | `0x005B0680` | `0x005B10CF` | `2.640` | RWX | sim | Small data (relativo a `$gp`) |
| `.sbss` | `0x005B1100` | `0x005B1497` | `920` | RWX | não | Small BSS |
| `.bss` | `0x005B1500` | `0x00604F63` | `342.628` | RWX | não | Dados zerados |

*(Além de 10 blocos `.DVP.overlay.*` correspondentes aos overlays VU1/DVP do RenderWare.)*

---

## 3. Estatísticas da Análise Inicial

- **Ghidra 12.2 (`r5900:LE:32:default`):** `8.454` funções analisadas (`320` renomeadas via banco SCE SDK), `10.319` strings definidas -> exportado em `re/ghostrider_ghidra_functions.csv`.
- **`ps2_analyzer.exe`:** `9.612` funções extraídas, `334` matches no banco SCE SDK (`89` stubs com handler pronto no `ps2xRuntime`, `234` funções de biblioteca sem handler ainda).

---

## 4. Funções-Chave Descobertas

| Endereço | Nome | Categoria | Evidência | Status |
|----------|------|-----------|-----------|--------|
| `0x0025B718` | `_start` (`sub_0025B718`) | Boot | ELF Header entry point | Mapeado |
| `0x0025B8A8` | `_InitSys` (callsite) | Boot | `ps2_analyzer` entry trace | Mapeado |
| `0x0025BA10` | `exit` | libc | SCE SDK DB (`stubs`) | Stubbed |
| `0x003168A0` | `scePadInit` | Input | SCE SDK DB (`stubs`) | Stubbed |
| `0x00316F90` | `scePadRead` | Input | SCE SDK DB (`stubs`) | Stubbed |
| `0x003175C8` | `sceGsResetGraph` | Gráficos | SCE SDK DB (`stubs`) | Stubbed |
| `0x00318880` | `sceGsSwapDBuffDc` | Gráficos | SCE SDK DB (`stubs`) | Stubbed |
| `0x00318EC8` | `sceCdInit` | Disco/CDVD | SCE SDK DB (`stubs`) | Stubbed |
| `0x00319708` | `sceDmaSend` | DMA | SCE SDK DB (`stubs`) | Stubbed |
| `0x0039AC90` | `InitThread` | Kernel | SCE SDK DB (`stubs`) | Stubbed |
| `0x0039CFB0` | `sceSifInitRpc` | SIF/IOP | SCE SDK DB (`stubs`) | Stubbed |
| `0x0039E3F8` | `sceOpen` | FileIO | SCE SDK DB (`stubs`) | Stubbed |
| `0x0039F450` | `sceSifRebootIop` | SIF/IOP | SCE SDK DB (`stubs`) | Stubbed |
| `0x003A0D78` | `InitTLB` | Kernel | SCE SDK DB (`stubs`) | Stubbed |

---

## 5. Bibliotecas / SDK Identificados

- [x] libc / libm / libgcc (`exit`, `memcmp`, `rand`, `strrchr`, `read`, `write`, `__divdi3`, soft-float)
- [x] PS2 SDK (EE Kernel / Threads / Alarm / TLB / Deci2)
- [x] SIF / IOP / CDVD (`sceSif*`, `sceCd*`, `sceOpen/Read/Close`, `IOPRP300.IMG`)
- [x] DMA / GIF / GS (`sceDma*`, `sceGs*`)
- [x] VU0 / VU1 (RenderWare PDS `.vutext` + `.DVP.overlay.*`)
- [x] Pad / Memory Card / DBC (`libpad.a`, `sceMc2`, `libdbc.a`)
- [x] Áudio & Vídeo (RenderWare Audio `RWA.IRX` + `LIBSD.IRX` + `SDRDRV.IRX`, Bink Video `.BIK`)

---

## 6. Pontos de Atenção para Próximas Fases

- **Progresso de Bring-up (`ZarathosRecomp.exe`) — 2026-11-20:**
  - **Boot #1–#3:** Recuperamos `559` funções indiretas (`.ctors` + *vtables*), `+937` de referências `lui+addiu` e buracos no `.text`, e `+1.376` *leaf functions* + `363` fronteiras de ponteiros de dados via `re/scripts/fill_csv_gaps.py` (total: **11.309 funções**). O RenderWare inicializou GS + DMA + VIF1 + VU1 (`.vudata` em `0x577f30`).
  - **Boot #4:** Todos os **8 drivers IOP `.IRX`** carregaram e rodaram código R3000A no `ps2xIOP` (`SDR driver version 4.0.1`, `RTFSSIOP.IRX`, `IOP RwaRPCOpen`).
  - **Boot #5–#6 (Diagnóstico de colisão de heap):** O loop infinito em `FUN_00269c60` foi causado pelo `guestMalloc` do `ps2xRuntime` (em `sceGsResetGraph`) alocando 128 bytes em `0x00604F70` — sobrescrevendo as sentinelas STL alocadas pelo `malloc` interno do jogo (`0x00604F70`/`0x00604F80`).
  - **Boot #7 (PRIMEIRA IMAGEM REAL `640x448`!):** Com o isolamento do pool host-side em `0x01D00000`–`0x01EF0000` (`runtime/src/ghostrider_overrides.cpp`), o jogo passou limpo por `FUN_00269c60`, subiu 8 faixas RGBA (`640x63` + `640x7` = **`640x448`**) para VRAM e desenhou a **primeira tela cheia texturizada** (`fbp=140`, `activeThreads=2`, `dma=75`, `gif=18`, `0` crashes)! Em seguida, aguarda até 600 VSyncs em `FUN_0036b870` (`sceGsSyncV(0)`, inicialização assíncrona de `sceMc2`/IOP).
- **Código sinalizado como self-modifying pelo `ps2_analyzer`:** `0x001985D8`, `0x00199A88`, `0x0019B330`, `0x001B1DE8`, `0x001C22D8`, `0x0023F740` — revisar no Ghidra se são falsos positivos (ex.: escrita em buffers DMA/VIF/VU) ou código dinâmico real.
- **Módulos IOP customizados (RenderWare):** `MODULES/RWS/RTFSSIOP.IRX` e `MODULES/RWS/RWA.IRX` serão cruciais na Fase 4 (avaliar execução via `ps2xIOP` vs. HLE de filesystem para `GR.ARC`).
- **Vídeos Bink (`.BIK`):** decodificados em software no EE (não dependem de IPU/MPEG PS2 para cutscenes `.BIK`).
