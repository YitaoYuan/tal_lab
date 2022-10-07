#!/bin/bash
DIR=`cd $(dirname $0); pwd`
SRC=$DIR/src
#echo $SRC
set -x
g++ $SRC/test.cc $SRC/tal_socket.cc $SRC/ring.cc -o test -lpthread -g3
