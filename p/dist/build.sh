#!/bin/sh
[ -z "${VERSION}" ] && VERSION=4.0.0
(
	RV=${VERSION}
	RA=amd64
	echo "[*] Downloading r2rizin-${RV}-${RA}"
	wget -c https://github.com/rizinorg/rizin/releases/download/${RV}/rizin_${RV}_${RA}.deb
	wget -c https://github.com/rizinorg/rizin/releases/download/${RV}/rizin-dev_${RV}_${RA}.deb
	#sudo apt update -y
	#sudo apt upgrade -y
	sudo apt install -y libssl-dev # why
	echo "[*] Installing rizin-${RV}-${RA}"
	sudo dpkg -i rizin_${RV}_${RA}.deb
	sudo dpkg -i rizin-dev_${RV}_${RA}.deb
)

# install NodeJS LTS
(
	NV=v10.15.1
	NA=linux-x64
	echo "[*] Downloading NodeJS"
	wget -c https://nodejs.org/dist/${NV}/node-${NV}-${NA}.tar.xz
	cd /work
	echo "[*] Installing NodeJS"
	tar xJf node-${NV}-${NA}.tar.xz -C /tmp
	export PATH=/tmp/node-${NV}-${NA}/bin:$PATH
	ls /tmp
	node --version || exit 1
	npm --version || exit 1
)
export PATH=/tmp/node-${NV}-${NA}/bin:$PATH
[ -z "${DESTDIR}" ] && DESTDIR=/
[ -z "${R2_LIBR_PLUGINS}" ] && R2_LIBR_PLUGINS=/usr/lib/rizin/last
make R2_PLUGDIR=${R2_LIBR_PLUGINS} DESTDIR=${DESTDIR}
