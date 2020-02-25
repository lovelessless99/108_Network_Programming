#! /bin/sh
PROJECT_NAME=npdemo1
NP_SCRIPT_PATH=$( readlink -f $0 )
NP_SCRIPT_DIR=$( dirname $NP_SCRIPT_PATH )
NP_ANSWER_DIR=$NP_SCRIPT_DIR/data/answer
source ~/.$PROJECT_NAME
vimdiff -c "set diffopt+=iwhite" -c "map <F8> :qa!<CR>" $TMP_DIR/output/$1 $NP_ANSWER_DIR/$1
