#!/bin/bash
# показываем, где интепретатор

# после названия файла принимается аргумент eval или diff для тестов

set +e # игнор ошибок

COLOR_RESET="\033[0m"
COLOR_PASS="\033[32m"
COLOR_FAIL="\033[31m"
COLOR_INFO="\033[34m"
COLOR_WARN="\033[33m"

if [[ "$1" != "diff" && "$1" != "eval" ]]; then
    echo -e "${COLOR_FAIL}Error: Unknown mode. Use 'diff' or 'eval' as first argument${COLOR_RESET}"
    exit 1
fi

MODE="$1"
EXP_DIR="./tests/Expressions/${MODE^}Expressions"
ANS_DIR="./tests/Answers/${MODE^}Answers"
OUT_DIR="./tests/Output/${MODE^}Output"
BIN="./tests/tests"

mkdir -p "$OUT_DIR"

pass=0
fail=0

echo -e "${COLOR_INFO}Running in $MODE mode${COLOR_RESET}"

for expr_file in "$EXP_DIR"/*.txt; do # проход по всем .txt в этой директории
    base=$(basename "$expr_file" .txt)
    ans_file="$ANS_DIR/$base.ans"
    out_file="$OUT_DIR/$base.out"

    if [[ ! -f "$ans_file" ]]; then # нет файла для ответа
        echo -e "${COLOR_WARN}Skipping $base — missing expected answer${COLOR_RESET}"
        continue
    fi

    $BIN $MODE "$expr_file" "$out_file"

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
