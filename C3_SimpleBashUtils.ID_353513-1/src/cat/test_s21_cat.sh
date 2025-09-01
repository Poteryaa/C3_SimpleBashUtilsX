#!/bin/bash

# Путь к s21_cat
S21_CAT="./s21_cat"
GNU_CAT="cat"
TEST_DIR="test_files"
mkdir -p $TEST_DIR

# Счетчик тестов
TEST_COUNT=0
SUCCESS_COUNT=0
FAIL_COUNT=0

# Функция для запуска теста
run_test() {
  local test_name="$1"
  local flags="$2"
  local input_file="$3"
  local expected_exit_code="$4"

  ((TEST_COUNT++))
  echo "Running Test $TEST_COUNT: $test_name"

  # Запускаем s21_cat и GNU cat
  $S21_CAT $flags "$input_file" > s21_output.txt 2> s21_error.txt
  s21_exit_code=$?
  $GNU_CAT $flags "$input_file" > gnu_output.txt 2> gnu_error.txt
  gnu_exit_code=$?

  # Проверяем код возврата
  if [ $s21_exit_code -ne $expected_exit_code ]; then
    echo "FAIL: Expected exit code $expected_exit_code, got $s21_exit_code"
    ((FAIL_COUNT++))
    return
  fi

  # Проверяем stdout - сравниваем с GNU cat напрямую
  if ! diff s21_output.txt gnu_output.txt > /dev/null; then
    echo "FAIL: s21_cat output differs from GNU cat"
    echo "s21_cat output:"
    cat -A s21_output.txt
    echo "GNU cat output:"
    cat -A gnu_output.txt
    ((FAIL_COUNT++))
    return
  fi

  # Проверяем stderr
  if ! diff s21_error.txt gnu_error.txt > /dev/null; then
    echo "FAIL: Error output differs"
    echo "s21_cat error:"
    cat s21_error.txt
    echo "GNU cat error:"
    cat gnu_error.txt
    ((FAIL_COUNT++))
    return
  fi

  echo "PASS"
  ((SUCCESS_COUNT++))
}

# Создание тестовых файлов
echo -e "Hello\n\nWorld\tTest\n" > $TEST_DIR/test1.txt
echo -e "\n\n\n" > $TEST_DIR/empty_lines.txt
touch $TEST_DIR/empty.txt
echo -e "Line1\nLine2\n\nLine4" > $TEST_DIR/mixed.txt

# Тесты
run_test "No flags, simple file" "" "$TEST_DIR/test1.txt" 0
run_test "-b (number non-empty)" "-b" "$TEST_DIR/test1.txt" 0
run_test "-n (number all)" "-n" "$TEST_DIR/test1.txt" 0
run_test "-s (squeeze blank)" "-s" "$TEST_DIR/empty_lines.txt" 0
run_test "-e (show ends)" "-e" "$TEST_DIR/test1.txt" 0
run_test "-t (show tabs)" "-t" "$TEST_DIR/test1.txt" 0
run_test "-s -n -e -t combined" "-s -n -e -t" "$TEST_DIR/test1.txt" 0
run_test "-b -n (mutually exclusive)" "-b -n" "$TEST_DIR/test1.txt" 0
run_test "Unknown option" "-x" "$TEST_DIR/test1.txt" 1
run_test "Non-existent file" "" "nonexistent.txt" 1
run_test "Empty file" "" "$TEST_DIR/empty.txt" 0
run_test "Mixed content with -b" "-b" "$TEST_DIR/mixed.txt" 0
run_test "Mixed content with -s" "-s" "$TEST_DIR/mixed.txt" 0

# Итоги
echo "--------------------------------"
echo "Total tests: $TEST_COUNT"
echo "Passed: $SUCCESS_COUNT"
echo "Failed: $FAIL_COUNT"

# Очистка
rm -f s21_output.txt gnu_output.txt s21_error.txt gnu_error.txt
rm -rf $TEST_DIR

