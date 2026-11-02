# RUPTURE DAM V3 - WATER OVER ROAD

20 separate PNG sprites, each exactly 224 x 30 px, true 1-bit black/white.

## Fix
Water now visibly spills FROM the damaged dam and ACROSS the road, flooding over and locally hiding the white lane dashes. The dashes remain aligned and intact outside the wet area.

## Animation
- OPEN: frames 01-10
- LOOP WHILE PLAYING: 11-13 (repeat three frames)
- CLOSE AFTER NOTE OFF: 14-20
- IDLE: 01. 20 matches 01 exactly for a seamless restart.

Higher octaves -> faster 11-13 loop. See animation.json.

The reference storyboard did not have cells for 15 and 16; those two are reconstructed as intermediate recession states.

Every frame is ready to use as an image asset at native resolution. The firmware was not rebuilt or changed by this ZIP.
