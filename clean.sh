#!/bin/bash
# doesn't remove ./keys because the whole purpose of having that folder instead of using ./data/keys is so it will be preserved
rm -rf .cache .pio data include/build_info.h webUI/dist webUI/node_modules compile_commands.json
git restore data
