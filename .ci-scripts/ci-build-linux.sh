#!/bin/bash
set -e

CI_BRANCH="$1"
CI_JSDEC="$PWD"
CI_RZ_VERSION=$2

if [ "$CI_BRANCH" != "dev" ]; then
	# master branch always build against latest release of rizin
	CI_RZ_VERSION=$(curl -s GET https://api.github.com/repos/rizinorg/rizin/tags\?per_page\=1 | jq -r '.[].name')
else
	CI_RZ_VERSION="$CI_BRANCH"
fi

echo "CI_BRANCH:       $CI_BRANCH"
echo "CI_RZ_VERSION:   $CI_RZ_VERSION"
echo "CI_JSDEC:        $CI_JSDEC"

# avoid placing rizin in the same folder.
cd ..

# download rizin
if [ "$CI_BRANCH" == "dev" ]; then
	# dev branch always build against latest commit of rizin
	wget -O "rizin.tar.gz" "https://github.com/rizinorg/rizin/archive/refs/heads/dev.tar.gz"
	tar xf "rizin.tar.gz"
else
	# master branch always build against latest release of rizin
	wget -O "rizin.tar.xz" "https://github.com/rizinorg/rizin/releases/download/$CI_RZ_VERSION/rizin-src-$CI_RZ_VERSION.tar.xz"
	tar xf "rizin.tar.xz"
fi

# build rizin and install it.
cd "rizin-$CI_RZ_VERSION"
meson setup --buildtype=release -Denable_tests=false build
sudo ninja -C build install

# cleanup
cd ..
rm -rf "rizin-*"

# go back to the source folder of jsdec
cd "$CI_JSDEC"

# build jsdec and install in the rizin dir.
meson setup --buildtype=release -Dbuild_type=rizin --prefix=~/.local build
sudo ninja -C build install || sleep 0

# check if it was installed correctly and try to run it.
HAS_JSDEC=$(rizin -qc "Lc" | grep jsdec)
if [ -z "$HAS_JSDEC" ]; then
	echo "rizin failed to load jsdec."
	rizin -e log.level=0 -qc "Lc"
	rizin -hh
	exit 1
fi

OUTPUT=$(rizin -qc 'af ; pdd' /bin/ls)
CHECK=$(echo -e "$OUTPUT" | grep "jsdec pseudo code output")
echo -e "$OUTPUT"
if [ -z "$CHECK" ]; then
	echo "rizin failed to exec jsdec."
	exit 1
fi
