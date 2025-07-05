#!/usr/bin/env bash

set -euo pipefail

function show_help() {
  echo "Usage: $ME"
  echo ""
  echo "Prepare a .tar.gz archive of the sources at \$SOURCE_DIR,"
  echo "and create a development setup in \$PKG_DIR for testing "
  echo "the voidstar Conan recipe."
  echo ""
  echo "Environment variables:"
  echo "  SOURCE_DIR  Repository root to package"
  echo "                = $SOURCE_DIR"
  echo "  PKG_DIR     Location for recipe testing setup"
  echo "                = $DIR_DIR"
}

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
SCRIPT_DIR=$( realpath -- "$SCRIPT_DIR" )
ME=$( basename -- "${BASH_SOURCE[0]}" )

: "${SOURCE_DIR:="$SCRIPT_DIR/../.."}"
PKG_DIR=$(
  realpath --canonicalize-missing -- \
    "${PKG_DIR:-"$SOURCE_DIR/build/packaging/conan"}"
)

if [ $# -ne 0 ]; then
  show_help
  exit 1
fi

set -x

mkdir -p "$PKG_DIR"

# Create an archive
cd "$SOURCE_DIR"
tar -czf "$PKG_DIR/voidstar-dev.tar.gz" \
  --exclude-vcs-ignores \
  --exclude-vcs \
  --directory=.. "$( basename -- "$PWD" )"

cd "$PKG_DIR"

# Recipe testing environment: symlink to recipe and test_package
function symlink() {
  ln --symbolic --force "$SCRIPT_DIR/voidstar/all/$1" "$1"
}
symlink conanfile.py
symlink test_package

# Recipe testing environment: create a dummy conandata.yml
PKG_URL="file://$PKG_DIR/voidstar-dev.tar.gz"
echo >conandata.yml "sources: {dev: {url: ${PKG_URL@Q}}}"
