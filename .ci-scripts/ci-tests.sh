#!/bin/bash
set -e

sudo apt update > /dev/null
sudo apt -y install meson ninja-build

CI_BRANCH="$1"
CI_JSDEC="$PWD"

echo "CI_BRANCH: $CI_BRANCH"
echo "CI_JSDEC:  $CI_JSDEC"

rm -rf tests >/dev/null 2>&1 || echo "no need to clean.."
git clone --branch "$CI_BRANCH" --depth 1 https://github.com/rizinorg/jsdec-test tests || git clone --depth 1 https://github.com/rizinorg/jsdec-test tests
cd tests
chmod +x testall.sh
./testall.sh "$CI_JSDEC" travis
