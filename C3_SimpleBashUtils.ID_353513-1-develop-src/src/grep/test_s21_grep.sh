#!/bin/bash

# Путь к s21_grep
S21_GREP="./s21_grep"
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
  
  # Проверяем код возврата
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
    if diff -q s21_output.txt gnu_output.txt > /dev/null; then
      echo "PASS"
      ((SUCCESS_COUNT++))
    else
      echo "FAIL: Output differs"
      echo "=== S21_GREP OUTPUT ==="
      cat s21_output.txt
      echo "=== GNU GREP OUTPUT ==="
      cat gnu_output.txt
      echo "===================="
      ((FAIL_COUNT++))
    fi
  else
    echo "PASS"
    ((SUCCESS_COUNT++))
  fi
}

# Функция для тестов с -f флагом
run_test_with_file() {
  local test_name="$1"
  local flags="$2"
  local pattern_file="$3"
  local input_file="$4"
  local expected_exit_code="$5"
  
  ((TEST_COUNT++))
  echo "Running Test $TEST_COUNT: $test_name"
  
  # Создаем команду с -f
  local s21_cmd="$S21_GREP $flags -f '$pattern_file' $input_file"
  local gnu_cmd="$GNU_GREP $flags -f '$pattern_file' $input_file"
  
  # Запускаем s21_grep и GNU grep
  eval $s21_cmd > s21_output.txt 2> s21_error.txt
  s21_exit_code=$?
  eval $gnu_cmd > gnu_output.txt 2> gnu_error.txt
  gnu_exit_code=$?
  
  echo "Command: $s21_cmd"
  echo "s21 exit code: $s21_exit_code, gnu exit code: $gnu_exit_code"
  
  # Аналогичная проверка как в run_test
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
  
  if [ $expected_exit_code -eq 0 ]; then
    if diff -q s21_output.txt gnu_output.txt > /dev/null; then
      echo "PASS"
      ((SUCCESS_COUNT++))
    else
      echo "FAIL: Output differs"
      ((FAIL_COUNT++))
    fi
  else
    echo "PASS"
    ((SUCCESS_COUNT++))
  fi
}

# Создание тестовых файлов
echo -e "hello world\nHELLO WORLD\nHello World\ntest line\nworld test" > "$TEST_DIR/test1.txt"
echo -e "foo bar\nbaz qux\nhello" > "$TEST_DIR/test2.txt"
echo -e "HELLO\nWORLD\ntest" > "$TEST_DIR/test3.txt"
echo "" > "$TEST_DIR/empty.txt"
echo -e "line1\nline2\n\nline4\n\n\nline7" > "$TEST_DIR/multiline.txt"

# PART 2 TESTS (базовые тесты)
echo "=== PART 2 TESTS (BASIC FUNCTIONALITY) ==="

run_test "Simple pattern" "" "hello" "$TEST_DIR/test1.txt" 0
run_test "Pattern not found" "" "notfound" "$TEST_DIR/test1.txt" 1
run_test "Case sensitive (upper)" "" "HELLO" "$TEST_DIR/test1.txt" 0
run_test "Case sensitive (mixed)" "" "Hello" "$TEST_DIR/test1.txt" 0
run_test "Flag -i (ignore case)" "-i" "hello" "$TEST_DIR/test1.txt" 0
run_test "Flag -v (invert match)" "-v" "hello" "$TEST_DIR/test1.txt" 0
run_test "Flag -c (count matches)" "-c" "hello" "$TEST_DIR/test1.txt" 0
run_test "Flag -n (line number)" "-n" "hello" "$TEST_DIR/test1.txt" 0
run_test "Multiple files" "" "hello" "$TEST_DIR/test1.txt $TEST_DIR/test3.txt" 0
run_test "Multiple files with -l" "-l" "hello" "$TEST_DIR/test1.txt $TEST_DIR/test2.txt $TEST_DIR/test3.txt" 0
run_test "Multiple files with -c" "-c" "hello" "$TEST_DIR/test1.txt $TEST_DIR/test3.txt" 0
run_test "Single -e pattern" "-e" "hello" "$TEST_DIR/test1.txt" 0

# ИСПРАВЛЕНИЕ: Специальный тест для множественных -e паттернов
((TEST_COUNT++))
echo "Running Test $TEST_COUNT: Multiple -e patterns"
$S21_GREP -e hello -e World "$TEST_DIR/test1.txt" > s21_output.txt 2> s21_error.txt
s21_exit_code=$?
$GNU_GREP -e hello -e World "$TEST_DIR/test1.txt" > gnu_output.txt 2> gnu_error.txt
gnu_exit_code=$?
echo "Command: $S21_GREP -e hello -e World $TEST_DIR/test1.txt"
echo "s21 exit code: $s21_exit_code, gnu exit code: $gnu_exit_code"
if [ $s21_exit_code -eq $gnu_exit_code ] && diff -q s21_output.txt gnu_output.txt > /dev/null; then
  echo "PASS"
  ((SUCCESS_COUNT++))
else
  echo "FAIL: Multiple -e patterns test failed"
  ((FAIL_COUNT++))
fi

run_test "Combine -i -n" "-i -n" "hello" "$TEST_DIR/test1.txt" 0
run_test "Combine -v -c" "-v -c" "hello" "$TEST_DIR/test1.txt" 0
run_test "Combine -i -v" "-i -v" "hello" "$TEST_DIR/test1.txt" 0
run_test "Non-existent file" "" "hello" "nonexistent.txt" 2
run_test "Empty file" "" "hello" "$TEST_DIR/empty.txt" 1
run_test "Simple regex" "" "H.*o" "$TEST_DIR/test1.txt" 0
run_test "Regex with -i" "-i" "h.*o" "$TEST_DIR/test1.txt" 0

echo ""
echo "=== PART 3 TESTS (ADDITIONAL FLAGS) ==="

# Тесты для флага -h (подавление имен файлов)
run_test "Flag -h with single file" "-h" "hello" "$TEST_DIR/test1.txt" 0
run_test "Flag -h with multiple files" "-h" "hello" "$TEST_DIR/test1.txt $TEST_DIR/test2.txt" 0
run_test "Flag -h with -n" "-h -n" "hello" "$TEST_DIR/test1.txt $TEST_DIR/test2.txt" 0
run_test "Flag -h with -c" "-h -c" "hello" "$TEST_DIR/test1.txt $TEST_DIR/test2.txt" 0

# Тесты для флага -s (подавление ошибок)
echo "Testing -s flag (error suppression)..."
$S21_GREP -s "hello" "nonexistent_file.txt" > s21_output.txt 2> s21_error.txt
s21_exit_code=$?
$GNU_GREP -s "hello" "nonexistent_file.txt" > gnu_output.txt 2> gnu_error.txt
gnu_exit_code=$?

((TEST_COUNT++))
if [ $s21_exit_code -eq $gnu_exit_code ] && [ ! -s s21_error.txt ]; then
  echo "PASS: Test $TEST_COUNT: Flag -s suppresses errors"
  ((SUCCESS_COUNT++))
else
  echo "FAIL: Test $TEST_COUNT: Flag -s should suppress errors"
  ((FAIL_COUNT++))
fi

# Создаем файлы для тестов -f
echo -e "hello\nworld\ntest" > "$TEST_DIR/patterns1.txt"
echo -e "Hello\nWORLD" > "$TEST_DIR/patterns2.txt"
echo -e "hello\n\nworld" > "$TEST_DIR/patterns3.txt"

# Тесты для флага -f (паттерны из файла)
run_test_with_file "Flag -f basic" "" "$TEST_DIR/patterns1.txt" "$TEST_DIR/test1.txt" 0
run_test_with_file "Flag -f with -i" "-i" "$TEST_DIR/patterns2.txt" "$TEST_DIR/test1.txt" 0
run_test_with_file "Flag -f with empty lines" "" "$TEST_DIR/patterns3.txt" "$TEST_DIR/test1.txt" 0
run_test_with_file "Flag -f with -n" "-n" "$TEST_DIR/patterns1.txt" "$TEST_DIR/test1.txt" 0
run_test_with_file "Flag -f with -c" "-c" "$TEST_DIR/patterns1.txt" "$TEST_DIR/test1.txt" 0

# Тест -f с несуществующим файлом паттернов
echo "Testing -f with non-existent pattern file..."
$S21_GREP -f "nonexistent_patterns.txt" "$TEST_DIR/test1.txt" > s21_output.txt 2> s21_error.txt
s21_exit_code=$?
$GNU_GREP -f "nonexistent_patterns.txt" "$TEST_DIR/test1.txt" > gnu_output.txt 2> gnu_error.txt
gnu_exit_code=$?

((TEST_COUNT++))
if [ $s21_exit_code -eq $gnu_exit_code ]; then
  echo "PASS: Test $TEST_COUNT: Flag -f with non-existent file"
  ((SUCCESS_COUNT++))
else
  echo "FAIL: Test $TEST_COUNT: Flag -f with non-existent file"
  ((FAIL_COUNT++))
fi

# Тесты для флага -o (только совпадающие части)
run_test "Flag -o basic" "-o" "hello" "$TEST_DIR/test1.txt" 0
run_test "Flag -o multiple matches" "-o" "l" "$TEST_DIR/test1.txt" 0
run_test "Flag -o with -n" "-o -n" "hello" "$TEST_DIR/test1.txt" 0
run_test "Flag -o with multiple files" "-o" "hello" "$TEST_DIR/test1.txt $TEST_DIR/test2.txt" 0
run_test "Flag -o with -h" "-o -h" "hello" "$TEST_DIR/test1.txt $TEST_DIR/test2.txt" 0
run_test "Flag -o with regex" "-o" "H.*o" "$TEST_DIR/test1.txt" 0

# ИСПРАВЛЕНИЕ: Специальный тест для -o -v (должен не выводить ничего)
((TEST_COUNT++))
echo "Running Test $TEST_COUNT: Flag -o with -v (should output nothing)"
$S21_GREP -o -v "hello" "$TEST_DIR/test1.txt" > s21_output.txt 2> s21_error.txt
s21_exit_code=$?
$GNU_GREP -o -v "hello" "$TEST_DIR/test1.txt" > gnu_output.txt 2> gnu_error.txt
gnu_exit_code=$?
echo "Command: $S21_GREP -o -v hello $TEST_DIR/test1.txt"
echo "s21 exit code: $s21_exit_code, gnu exit code: $gnu_exit_code"
if [ $s21_exit_code -eq $gnu_exit_code ] && diff -q s21_output.txt gnu_output.txt > /dev/null; then
  echo "PASS"
  ((SUCCESS_COUNT++))
else
  echo "FAIL: -o -v combination test failed"
  echo "=== S21_GREP OUTPUT ==="
  cat s21_output.txt
  echo "=== GNU GREP OUTPUT ==="
  cat gnu_output.txt
  echo "===================="
  ((FAIL_COUNT++))
fi

echo ""
echo "=== COMPLEX COMBINATION TESTS ==="

# Сложные комбинации флагов
run_test "Combine -i -n -v" "-i -n -v" "hello" "$TEST_DIR/test1.txt" 0
run_test "Combine -h -l -i" "-h -l -i" "hello" "$TEST_DIR/test1.txt $TEST_DIR/test2.txt $TEST_DIR/test3.txt" 0

# ИСПРАВЛЕНИЕ: Специальный тест для -s -v -c с несуществующим файлом
((TEST_COUNT++))
echo "Running Test $TEST_COUNT: Combine -s -v -c with non-existent file"
$S21_GREP -s -v -c "hello" "$TEST_DIR/test1.txt" nonexistent.txt > s21_output.txt 2> s21_error.txt
s21_exit_code=$?
$GNU_GREP -s -v -c "hello" "$TEST_DIR/test1.txt" nonexistent.txt > gnu_output.txt 2> gnu_error.txt
gnu_exit_code=$?
echo "Command: $S21_GREP -s -v -c hello $TEST_DIR/test1.txt nonexistent.txt"
echo "s21 exit code: $s21_exit_code, gnu exit code: $gnu_exit_code"
if [ $s21_exit_code -eq $gnu_exit_code ] && diff -q s21_output.txt gnu_output.txt > /dev/null; then
  echo "PASS"
  ((SUCCESS_COUNT++))
else
  echo "FAIL: -s -v -c combination test failed"
  ((FAIL_COUNT++))
fi

run_test_with_file "Combine -f -i -n" "-i -n" "$TEST_DIR/patterns1.txt" "$TEST_DIR/test1.txt" 0
run_test_with_file "Combine -f -h -v" "-h -v" "$TEST_DIR/patterns1.txt" "$TEST_DIR/test1.txt $TEST_DIR/test2.txt" 0

echo ""
echo "=== EDGE CASES ==="

# Граничные случаи
run_test "Empty pattern" "" "" "$TEST_DIR/test1.txt" 0
run_test "Pattern with spaces" "" "hello world" "$TEST_DIR/test1.txt" 0
run_test "Very long line match" "-o" "hello.*world" "$TEST_DIR/test1.txt" 0

# Результаты
echo "--------------------------------"
echo "Total tests: $TEST_COUNT"
echo "Passed: $SUCCESS_COUNT"
echo "Failed: $FAIL_COUNT"

if [ $FAIL_COUNT -eq 0 ]; then
  echo "ALL TESTS PASSED!"
  exit_code=0
else
  echo "SOME TESTS FAILED!"
  exit_code=1
fi

# Очистка
rm -f s21_output.txt gnu_output.txt s21_error.txt gnu_error.txt
rm -f "$TEST_DIR/patterns1.txt" "$TEST_DIR/patterns2.txt" "$TEST_DIR/patterns3.txt"
rm -rf $TEST_DIR

exit $exit_code
