# Caderno de Engenharia Reversa — Ghost Rider (PS2)

Use este arquivo para registrar descobertas verificadas no Ghidra / REA.

---

## 1. Metadados do Binário Alvo

- **Região / Serial:** *(ex.: NTSC-U `SLUS_213.06` ou PAL `SLES_...` — preencher após extração)*
- **Nome do arquivo ELF:** *(preencher)*
- **Tamanho (bytes):** *(preencher)*
- **SHA-256:** *(preencher)*
- **Ponto de entrada (`entry`):** *(preencher)*
- **Engine identificada:** *(verificar strings/RTTI no Ghidra — ex.: RenderWare ou engine proprietária da Climax)*

---

## 2. Mapa de Seções ELF

| Seção | Endereço Virtual | Tamanho | Permissões | Observações |
|-------|------------------|---------|------------|-------------|
| `.text` | — | — | RX | Código principal |
| `.rodata` | — | — | R | Strings / tabelas constantes |
| `.data` | — | — | RW | Dados inicializados |
| `.bss` | — | — | RW | Dados zerados |

---

## 3. Funções-Chave Descobertas

| Endereço | Nome proposto | Categoria | Evidência (Ghidra/REA) | Status |
|----------|---------------|-----------|------------------------|--------|
| — | `_start` / `entry` | Boot | — | Pendente |
| — | `main` | Boot | — | Pendente |

---

## 4. Bibliotecas / SDK Identificados

- [ ] libc / libm / libgcc
- [ ] PS2 SDK (EE Kernel / Syscalls)
- [ ] SIF / IOP / CDVD (leitura de disco/arquivos)
- [ ] DMA / GIF / GS (gráficos)
- [ ] VU0 / VU1 (unidades vetoriais)
- [ ] Pad / Memory Card
- [ ] Áudio (SPU2 / middleware de áudio)

---

## 5. Problemas Conhecidos e Anotações

- *(Registrar aqui jump tables problemáticas, chamadas indiretas, código auto-modificável/overlays ou instruções específicas que precisarem de atenção especial no PS2Recomp.)*
