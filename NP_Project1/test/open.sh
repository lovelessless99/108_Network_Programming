#! /bin/sh
DELAY=30
SCRIPT_PATH=$( readlink -f $0 )
SCRIPT_DIR=$( dirname $SCRIPT_PATH )
SOURCE_DIR=$SCRIPT_DIR/src

chmod 755 -R $SOURCE_DIR/$1
sleep ${DELAY} && chmod 700 -R $SOURCE_DIR/$1 &
