#!/bin/bash
set -e

CI_BRANCH="$1"
CI_JSDEC="$PWD"
CI_RZ_VERSION=$(curl -s GET https://api.github.com/repos/rizinorg/rizin/tags\?per_page\=1 | jq -r '.[].name')

echo "CI_BRANCH:       $CI_BRANCH"
echo "CI_RZ_VERSION:   $CI_RZ_VERSION"
echo "CI_JSDEC:        $CI_JSDEC"

# avoid placing rizin in the same folder.
cd ..

# download the latest tagged rizin version and install it
wget -O "rizin.tar.xz" "https://github.com/rizinorg/rizin/releases/download/$CI_RZ_VERSION/rizin-src-$CI_RZ_VERSION.tar.xz"
tar xf "rizin.tar.xz"
cd "rizin-$CI_RZ_VERSION"

meson --buildtype=release -Denable_tests=false --prefix=~/.local build
sudo ninja -C build install

# cleanup
cd ..
rm -rf "rizin-*"

# go back to the source folder of jsdec
cd "$CI_JSDEC"

# build jsdec and install in the rizin dir.
meson --buildtype=release -Dstandalone=false build
sudo ninja -v -C build install

# check if it was installed correctly and try to run it.
HAS_JSDEC=$(rizin -Qc "Lc" | grep jsdec)
if [ -z "$HAS_JSDEC" ]; then
	echo "rizin failed to load jsdec."
	rizin -e log.level=2 -Qc "Lc" | grep jsdec || sleep 0
	exit 1
fi

OUTPUT=$(rizin -Qc 'af ; pdd' /bin/ls)
CHECK=$(echo -e "$OUTPUT" | grep "jsdec pseudo code output")
echo -e "$OUTPUT"
if [ -z "$CHECK" ]; then
	exit 1
fi
