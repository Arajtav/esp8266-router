#!/bin/bash
set -e

mkdir -p .cache # used by lsp, but also for storing outputs of things here

echo -e "\033[36m--------generating headers--------\033[0m"
mkdir -p include

GIT_BRANCH=$(git rev-parse --abbrev-ref HEAD)
GIT_COMMIT=$(git rev-parse --short HEAD)
GIT_DATE=$(date -u +"%Y-%m-%dT%H:%M:%SZ")
GIT_AUTHOR=$(git log -1 --pretty=format:'%an <%ae>')
GIT_COMMIT_MSG=$(git log -1 --pretty=%s)
GIT_DIRTY=$(git diff-index --quiet HEAD -- || echo "+dirty")

GIT_COMMIT_MSG_ESCAPED=$(echo "$GIT_COMMIT_MSG" | sed 's/\\/\\\\/g' | sed 's/"/\\"/g')
GIT_STRING="$GIT_BRANCH@$GIT_COMMIT ($GIT_DATE) $GIT_AUTHOR: \\\"$GIT_COMMIT_MSG_ESCAPED\\\" $GIT_DIRTY"

rm ./include/build_info.h -f
cat << EOF > ./include/build_info.h
#ifndef BUILD_INFO_H
#define BUILD_INFO_H

#define VERSION "$GIT_STRING"

#endif // BUILD_INFO_H
EOF

echo -e "\033[36m--------generating webUI files--------\033[0m"
mkdir -p data/webUI/

cd webUI
if [ ! -d node_modules ]; then
    npm install
fi
node build.mjs > /dev/null
mv dist/in/* ../data/webUI/
cd ..

echo -e "\033[36m--------copying keys--------\033[0m"
mkdir -p keys
if [ ! -f ./keys/private.key ] && [ ! -f ./keys/server.crt  ]; then
    echo -e "\033[36m--------generating new private key and certificate--------\033[0m"
    openssl genpkey -algorithm RSA -out ./keys/private.key -pkeyopt rsa_keygen_bits:2048
    openssl req -new -x509 -key ./keys/private.key -out ./keys/server.crt -days 365 -subj "/CN=192.168.0.1" -addext "subjectAltName=IP:192.168.0.1"
fi
mkdir -p ./data/private
cp ./keys/private.key ./keys/server.crt data/private/

echo -e "\033[36m--------compiling--------\033[0m"
pio run > .cache/compile_log

pio run -t compiledb # clang lsp

echo -e "\033[36m--------uploading fs and code--------\033[0m"
# TODO: this overwrites existing files
pio run -t uploadfs > .cache/uploadfs_log
pio run -t upload > .cache/upload_log

echo -e "\033[36m--------ready--------\033[0m"
pio run -t monitor
