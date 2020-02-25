#! /bin/sh
PROJECT_NAME=npdemo1
NP_SCRIPT_PATH=$( readlink -f $0 )
NP_SCRIPT_DIR=$( dirname $NP_SCRIPT_PATH )
NP_TESTCASE_DIR=$NP_SCRIPT_DIR/data/testcase
vim -c "map <F8> :qa!<CR>" $NP_TESTCASE_DIR/$1
