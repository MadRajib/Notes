### Build Steps
```bash
source ../../.venv/bin/activate
export ZEPHYR_BASE=$(HOME)/workspace/zypher_project_ws/zypher_madrajib/zephyr
west build -p always -b rpi_pico2/rp2350a/m33 -S cdc-acm-console $(HOME)/workspace/ch9120_test_app/ -- -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
```

### Run Build Test
```bash
west twister -p native_sim/native -T tests/drivers/build_all/ethernet -s net.ethernet.build.uart
```

### Befor Submittin
```bash
# on latest commit
scripts/checkpatch.pl --git HEAD

# on diff
git diff | scripts/checkpatch.pl -
```

Clang Format
```bash
# on File
clang-format -i drivers/ethernet/offload/eth_wch9120.c
```