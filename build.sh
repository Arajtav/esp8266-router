#!/bin/bash

# keys (idk if that works or not i am bad at https)
mkdir -p keys
if [ ! -f ./keys/private.key ] && [ ! -f ./keys/server.crt  ]; then
    echo "Regenerating keys"
    openssl genpkey -algorithm RSA -out ./keys/private.key -pkeyopt rsa_keygen_bits:2048
    openssl req -new -x509 -key ./keys/private.key -out ./keys/server.crt -days 365 -subj "/CN=192.168.0.1" -addext "subjectAltName=IP:192.168.0.1"
fi

# generate headers
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

# actual build
# webUI html
cd webUI &&
npm install &&
npm run build &&
mkdir -p ../data/webUI/ &&
mv dist/in/* ../data/webUI/ &&
cd .. &&
# keys
mkdir -p ./data/private &&
cp ./keys/* data/private/ &&
# actual code
pio run &&
pio run -t compiledb && # clang lsp
pio run -t uploadfs && # TODO: this overwrites files like log.txt
pio run -t upload &&
pio run -t monitor
