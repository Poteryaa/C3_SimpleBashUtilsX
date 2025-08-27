#!/bin/bash

# Путь к s21_grep
S21_GREP="grep/s21_grep"
GNU_GREP="grep"
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
  local pattern="$3"
  local input_file="$4"
  local expected_exit_code="$5"

  ((TEST_COUNT++))
  echo "Running Test $TEST_COUNT: $test_name"

  # Создаем команду
  local s21_cmd="$S21_GREP $flags '$pattern' $input_file"
  local gnu_cmd="$GNU_GREP $flags '$pattern' $input_file"

  # Запускаем s21_grep и GNU grep
  eval $s21_cmd > s21_output.txt 2> s21_error.txt
  s21_exit_code=$?
  eval $gnu_cmd > gnu_output.txt 2> gnu_error.txt
  gnu_exit_code=$?

  echo "Command: $s21_cmd"
  echo "s21 exit code: $s21_exit_code, gnu exit code: $gnu_exit_code"

  # Проверяем код возврата (разрешаем разные коды возврата для ошибок)
  if [ $expected_exit_code -eq 0 ] && [ $s21_exit_code -ne 0 ]; then
    echo "FAIL: Expected success (exit code 0), got $s21_exit_code"
    ((FAIL_COUNT++))
    return
  fi
  if [ $expected_exit_code -ne 0 ] && [ $s21_exit_code -eq 0 ]; then
    echo "FAIL: Expected failure (non-zero exit code), got $s21_exit_code"
    ((FAIL_COUNT++))
    return
  fi

  # Если ожидаем успех, сравниваем вывод
  if [ $expected_exit_code -eq 0 ]; then
    if ! diff s21_output.txt gnu_output.txt > /dev/null; then
      echo "FAIL: s21_grep output differs from GNU grep"
      echo "=== s21_grep output ==="
      cat s21_output.txt
      echo "=== GNU grep output ==="
      cat gnu_output.txt
      echo "=== Difference ==="
      diff s21_output.txt gnu_output.txt
      ((FAIL_COUNT++))
      return
    fi
  fi

  echo "PASS"
  ((SUCCESS_COUNT++))
}

# Создание тестовых файлов
echo -e "Hello\nhello\nWorld\nHELLO\ntest line" > $TEST_DIR/test1.txt
echo -e "Line1\nLine2\nLine3" > $TEST_DIR/test2.txt
echo -e "another\nfile\nhello" > $TEST_DIR/test3.txt
touch $TEST_DIR/empty.txt

# Базовые тесты
run_test "Simple pattern" "" "hello" "$TEST_DIR/test1.txt" 0
run_test "Pattern not found" "" "notfound" "$TEST_DIR/test1.txt" 1
run_test "Case sensitive (upper)" "" "HELLO" "$TEST_DIR/test1.txt" 0
run_test "Case sensitive (mixed)" "" "Hello" "$TEST_DIR/test1.txt" 0

# Тесты флагов
run_test "Flag -i (ignore case)" "-i" "hello" "$TEST_DIR/test1.txt" 0
run_test "Flag -v (invert match)" "-v" "hello" "$TEST_DIR/test1.txt" 0
run_test "Flag -c (count matches)" "-c" "hello" "$TEST_DIR/test1.txt" 0
run_test "Flag -n (line number)" "-n" "hello" "$TEST_DIR/test1.txt" 0

# Тесты с множественными файлами
run_test "Multiple files" "" "hello" "$TEST_DIR/test1.txt $TEST_DIR/test3.txt" 0
run_test "Multiple files with -l" "-l" "hello" "$TEST_DIR/test1.txt $TEST_DIR/test2.txt $TEST_DIR/test3.txt" 0
run_test "Multiple files with -c" "-c" "hello" "$TEST_DIR/test1.txt $TEST_DIR/test3.txt" 0

# Тесты с -e
run_test "Single -e pattern" "-e" "hello" "$TEST_DIR/test1.txt" 0

# Test 13 - особый случай с множественными -e
((TEST_COUNT++))
echo "Running Test $TEST_COUNT: Multiple -e patterns"
s21_cmd="$S21_GREP -e hello -e World $TEST_DIR/test1.txt"
gnu_cmd="$GNU_GREP -e hello -e World $TEST_DIR/test1.txt"
echo "Command: $s21_cmd"
eval $s21_cmd > s21_output.txt 2> s21_error.txt
s21_exit_code=$?
eval $gnu_cmd > gnu_output.txt 2> gnu_error.txt
gnu_exit_code=$?
echo "s21 exit code: $s21_exit_code, gnu exit code: $gnu_exit_code"
if [ $s21_exit_code -eq 0 ] && [ $gnu_exit_code -eq 0 ] && diff s21_output.txt gnu_output.txt > /dev/null; then
    echo "PASS"
    ((SUCCESS_COUNT++))
else
    echo "FAIL"
    if [ $s21_exit_code -ne 0 ] || [ $gnu_exit_code -ne 0 ]; then
        echo "Expected success (exit code 0), got s21: $s21_exit_code, gnu: $gnu_exit_code"
    elif ! diff s21_output.txt gnu_output.txt > /dev/null; then
        echo "s21_grep output differs from GNU grep"
        echo "=== s21_grep output ==="
        cat s21_output.txt
        echo "=== GNU grep output ==="
        cat gnu_output.txt
    fi
    ((FAIL_COUNT++))
fi

# Комбинированные флаги
run_test "Combine -i -n" "-i -n" "hello" "$TEST_DIR/test1.txt" 0
run_test "Combine -v -c" "-v -c" "hello" "$TEST_DIR/test1.txt" 0
run_test "Combine -i -v" "-i -v" "hello" "$TEST_DIR/test1.txt" 0

# Тесты ошибок
run_test "Non-existent file" "" "hello" "nonexistent.txt" 1
run_test "Empty file" "" "hello" "$TEST_DIR/empty.txt" 1

# Тесты с регулярными выражениями
run_test "Simple regex" "" "H.*o" "$TEST_DIR/test1.txt" 0
run_test "Regex with -i" "-i" "h.*o" "$TEST_DIR/test1.txt" 0

echo "--------------------------------"
echo "Total tests: $TEST_COUNT"
echo "Passed: $SUCCESS_COUNT"
echo "Failed: $FAIL_COUNT"

if [ $FAIL_COUNT -eq 0 ]; then
  echo "ALL TESTS PASSED!"
  exit 0
else
  echo "SOME TESTS FAILED!"
  exit 1
fi

# Очистка
rm -f s21_output.txt gnu_output.txt s21_error.txt gnu_error.txt
rm -rf $TEST_DIR