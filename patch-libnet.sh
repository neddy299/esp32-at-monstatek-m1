#!/bin/bash

export PFILE_ORIG=./esp-idf/components/esp_wifi/lib/esp32c6/libnet80211.a
export PFILE_PATCHED=./esp-idf/components/esp_wifi/lib/esp32c6/libnet80211.a.patched
export PFILE_OLD=./esp-idf/components/esp_wifi/lib/esp32c6/libnet80211.a.old

ls -al $PFILE_ORIG
ls -al $PFILE_PATCHED

pio pkg exec -p toolchain-riscv32-esp -- riscv32-esp-elf-objcopy  --weaken-symbol=ieee80211_raw_frame_sanity_check $PFILE_ORIG $PFILE_PATCHED
mv $PFILE_ORIG $PFILE_OLD
mv $PFILE_PATCHED $PFILE_ORIG
touch $PFILE_ORIG

ls -al $PFILE_OLD
sha256sum $PFILE_OLD
sha256sum $PFILE_ORIG