# RUPTURE V2.0.13 — FM1 hardware checklist

1. Boot on a known-good FM1 with a current backup and recovery path.
2. Open SETTINGS > PRESETS KNOB. Select CXS and check the saved CXS
   assignment is unchanged, including after reboot.
3. In the same chooser, select GROWL and check the initial assignment
   is EXPRESSION in GROWL PERFORMANCE. Switch MAIN to GROWL using ALGORITHM.
4. Hold a long note with reverb and both SUB oscillators. Turn PRESETS
   slowly from 50 to 0 to 100 and back several times. Check the evolving
   growl, stable note pitch, no clicks, and no increased DMA LATE counter.
5. Confirm exact midpoint 50 returns to the same base GROWL sound.
6. Assign a different GROWL target in SETTINGS, save, reboot, and check that
   the CXS assignment remains independent. Restore GROWL > EXPRESSION.
7. Save and load projects, check the two assignments and previous patches.
8. Hold SEQ while rotating PRESETS: rotate steps, not the Expression macro.
9. Stress sustained notes with Channel EQ at minimum frequencies, HALL
   reverb, multiple FX, and both SUB oscillators. Check DMA LATE and audio.
10. Check OTA to another firmware and recovery before public release.

Software tests check expression range/headroom, backward-compatible journal
storage, A/B compaction and the cross-compiled FM1 application. These tests
do not replace step 4/9/10 on real FM1 hardware.
