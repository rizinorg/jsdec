#!/bin/bash
set -e

CI_BRANCH="$1"

echo "Branch: $CI_BRANCH"
make --no-print-directory testbin -C p
ERRORED=$?
if [ "$ERRORED" == "1" ]; then
	exit $ERRORED
fi

## jstest-test
cd ..
CI_WORKDIR=$(pwd)
echo "Work dir: $CI_WORKDIR"

#rm -rf jstest-test >/dev/null 2>&1 || echo "no need to clean.."
#git clone --branch "$CI_BRANCH" --depth 1 https://github.com/rizinorg/jstest-test || git clone --depth 1 https://github.com/rizinorg/jstest-test
cd jstest-test
chmod +x testall.sh
./testall.sh "$CI_WORKDIR/jsdec" travis
ERRORED=$?
cd ..
