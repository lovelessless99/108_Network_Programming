#! /bin/sh

NPSHELL_PATH=./npshell
NP_TESTCASE_DIR=demo/data/testcase/
OUTPUT_DIR=demo/data/output/
NP_ANSWER_DIR=demo/data/answer/
WORK_DIR=demo/workdir/
TEST_CASE_START=1
TEST_CASE_END=15

for i in $( seq $TEST_CASE_START $TEST_CASE_END ); do
  echo "[1;34m===== Test Case $i ===== [m"
  env -i stdbuf -o 0 -e 0 $NPSHELL_PATH < $NP_TESTCASE_DIR/$i > $OUTPUT_DIR/$i 2>&1
  
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

