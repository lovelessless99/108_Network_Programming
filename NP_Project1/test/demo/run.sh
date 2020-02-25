#! /bin/sh
trap 'echo -ne "\r"' INT

PROJECT_NAME=npdemo1
CLEANUP_DELAY=3600

info() {
  echo "[0;33m[Info] $1[m"
}

error() {
  echo "[0;31m[Error] $1[m"
}

success() {
  echo "[0;32m[Success] $1[m"
}

if [ -f ~/.$PROJECT_NAME ]; then
  source ~/.$PROJECT_NAME
  info "Removing previous tmp directory ..."
  rm -rf $TMP_DIR
fi

NP_SCRIPT_PATH=$( readlink -f $0 )
NP_SCRIPT_DIR=$( dirname $NP_SCRIPT_PATH )
NP_WORK_DIR=$NP_SCRIPT_DIR/work_dir
NP_PROJECT_DIR=$( dirname $NP_SCRIPT_DIR )
NP_SOURCE_DIR=$NP_PROJECT_DIR/src
NP_TESTCASE_DIR=$NP_SCRIPT_DIR/data/testcase
NP_ANSWER_DIR=$NP_SCRIPT_DIR/data/answer

TMP_DIR=$( mktemp -d -p /tmp ${PROJECT_NAME}.XXXX )
SOURCE_DIR=$TMP_DIR/src
NPSHELL_PATH=$SOURCE_DIR/npshell
WORK_DIR=$TMP_DIR/work_dir
OUTPUT_DIR=$TMP_DIR/output

STUDENT_ID=$( ldapsearch -LLLx "uid=$USER" csid | tail -n 2 | head -n 1 | cut -d " " -f 2 )
[ -n "$1" ] && STUDENT_ID=$1

TEST_CASE_START=1
TEST_CASE_END=15

[ -n "$2" ] && TEST_CASE_START=$2

cat > ~/.npdemo1 <<EOF
TMP_DIR=$TMP_DIR
EOF

chmod 700 $TMP_DIR

cp -r $NP_SOURCE_DIR/$STUDENT_ID $SOURCE_DIR
mkdir -p $OUTPUT_DIR

info "Compiling npshell ..."
if make -s -C $SOURCE_DIR ; then
  success "Compilation completed"
else
  error "Failed to compile with make"
  exit
fi

if [ ! -x $NPSHELL_PATH ] ; then
  error "npshell not found"
  exit
fi

for i in $( seq $TEST_CASE_START $TEST_CASE_END ); do
  echo "[1;34m===== Test Case $i ===== [m"
  rm -rf $WORK_DIR
  cp -r $NP_WORK_DIR $WORK_DIR
  pushd $WORK_DIR > /dev/null
  env -i stdbuf -o 0 -e 0 $NPSHELL_PATH < $NP_TESTCASE_DIR/$i > $OUTPUT_DIR/$i 2>&1
  popd > /dev/null
  echo -n "Your answer is "
  if diff -w $OUTPUT_DIR/$i $NP_ANSWER_DIR/$i > /dev/null ; then
    echo "[0;32mcorrect[m"
    correct_cases="$correct_cases $i"
  else
    echo "[0;31mwrong[m"
    wrong_cases="$wrong_cases $i"
  fi
done

echo "[1;34m======= Summary =======[m"
[ -n "$correct_cases" ] && echo "[0;32m[Correct][m$correct_cases"
[ -n "$wrong_cases" ] && echo "[0;31m[ Wrong ][m$wrong_cases"
echo "[1;34m=======================[m"

tmux new -d "sleep $CLEANUP_DELAY; rm -rf $TMP_DIR"
