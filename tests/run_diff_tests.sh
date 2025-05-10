#!/bin/bash
# показываем, где интепретатор

set +e # игнор ошибок

COLOR_RESET="\033[0m"
COLOR_PASS="\033[32m"
COLOR_FAIL="\033[31m"
COLOR_INFO="\033[34m"
COLOR_WARN="\033[33m"

EXP_DIR="./tests/EvalExpressions"
ANS_DIR="./tests/EvalAnswers"
OUT_DIR="./tests/EvalOutput"
BIN="./tests/tests"

mkdir -p "$OUT_DIR"

pass=0
fail=0

for expr_file in "$EXP_DIR"/*.txt; do # проход по всем .txt в этой директории
    base=$(basename "$expr_file" .txt)
    ans_file="$ANS_DIR/$base.ans"
    out_file="$OUT_DIR/$base.out"

    if [[ ! -f "$ans_file" ]]; then # нет файла для ответа
        echo -e "${COLOR_WARN}Skipping $base — missing expected answer${COLOR_RESET}"
        continue
    fi

    $BIN eval "$expr_file" "$out_file"

    expected=$(<"$ans_file")
    actual=$(<"$out_file")

    actual_rounded=$(printf "%.1f" "$actual")
    expected_rounded=$(printf "%.1f" "$expected")

    if [[ "$actual_rounded" == "$expected_rounded" ]]; then
        echo -e "${COLOR_PASS}PASS: $base${COLOR_RESET}"
        ((pass++))
    else
        echo -e "${COLOR_FAIL}FAIL: $base${COLOR_RESET}"
        echo -e "${COLOR_INFO}Expected: $expected_rounded${COLOR_RESET}"
        echo -e "${COLOR_INFO}Got     : $actual_rounded${COLOR_RESET}"
        ((fail++))
    fi
done

echo -e "${COLOR_INFO}Total: $((pass + fail)) | ${COLOR_PASS}Passed: $pass | ${COLOR_FAIL}Failed: $fail${COLOR_RESET}"
