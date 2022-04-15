#!/bin/sh

DIR=$(dirname "$0")
cd $DIR

VERSION="$(cat ./version.txt)"$'\n'

show $DIR/bg.png 
say "$VERSION"
confirm only