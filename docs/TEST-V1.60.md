# RUPTURE V1.60 — First-run formatting confirmation

## Problem

After holding SAVE for three seconds in the first-run wizard, the earlier result page was easy to miss and did not clearly distinguish verified formatting from a failed operation.

## Correction

- The three-second hold enters a **FORMATTING... / PLEASE DO NOT POWER OFF** UI state before the blocking storage operation starts.
- The result is latched until a new SEL press, with a minimum 1.5-second reading period.
- On successful initialization (`ETCH_STORE_OK`), the FM1 displays **FORMAT SUCCESSFUL**, a visible check mark, and **STORAGE READY**.
- When compatible storage already exists, the wizard does not erase and displays **STORAGE ALREADY READY / NO FORMAT NEEDED** instead.
- On unsuccessful or blocked formatting, the wizard displays **FORMAT FAILED / STORAGE NOT READY**, never a success message.
- The wizard still supports all NO paths including **NO :(**, CRT filter choice, and continuation without formatting.

## Tests

| Verification | Result |
|---|---|
| JieLi pi32v2 native application crossbuild | PASS |
| Native entrypoint and image validation | PASS |
| UI state-machine tests, early SEL rejection and explicit acknowledgement | PASS |
| Native 240×240 rendering of busy, success, already-ready and failure states | PASS |
| Global settings and storage NOR host regression | PASS |
| OTA host app-only positive/negative tests with new `.fwsc` | PASS |
| Trusted OTA loader and protected 16 KiB bootheader parity with V1.59 | PASS |
| Flash content outside application parity with V1.59 | PASS |
| Audio/DSP, USB, OTA, storage and sprite source byte-parity | PASS |
| WebInstaller and GitHub Pages `index.html` embedded firmware parity | PASS |
| Complete historical regression suite | NOT RUN |
| Real FM1 screen, flash-format result, audio DMA and stock V15 restore | NOT PHYSICALLY VERIFIED |

## Safety notes

A successful host build and NOR simulation cannot guarantee a successful format on physical hardware or cross-firmware recovery. Back up user data before flashing. No new audio DSP, ISR work, RAM framebuffer, flash partitions or OTA loader changes have been introduced.
