#!/bin/bash

set -e

builddir=$(mktemp -d build_XXXXXX)
srcdir=$(pwd)

mkdir -p _ccache
export CCACHE_BASEDIR="$(pwd)"
export CCACHE_DIR="${CCACHE_BASEDIR}/_ccache"

ccache --zero-stats
ccache --show-stats

# Disable ccache while running Meson, to avoid cached compiler tests
export CCACHE_DISABLE=true
meson ${builddir} ${srcdir} || exit $?
unset CCACHE_DISABLE

cd ${builddir}

ninja || exit $?

cd ..

rm -rf ${builddir}

ccache --show-stats
