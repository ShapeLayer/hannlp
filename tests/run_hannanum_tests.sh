#!/usr/bin/env sh
set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
PROJECT_DIR=$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)
ROOT_DIR=$(CDPATH= cd -- "$PROJECT_DIR/.." && pwd)
JHANNANUM_DIR="$ROOT_DIR/JHanNanum-0.8.4-en/JHanNanum"
DATA_DIR="$JHANNANUM_DIR/data"
BUILD_DIR=${1:-"$PROJECT_DIR/build-tests"}
C_RUNTIME="$BUILD_DIR/hannanum"
EXPECTED_DIR="$SCRIPT_DIR/expected"

require_command() {
  if ! command -v "$1" >/dev/null 2>&1; then
    printf '%s\n' "missing required command: $1" >&2
    exit 77
  fi
}

run_case() {
  name=$1
  input=$2
  mode=$3
  expected=$4
  c_out="$BUILD_DIR/${name}.c.out"
  c_norm="$BUILD_DIR/${name}.c.norm"
  expected_norm="$EXPECTED_DIR/${name}.expected"

  "$C_RUNTIME" --data-dir "$DATA_DIR" "$input" >"$c_out"

  if [ "$mode" = "contains" ]; then
    if grep -F "$expected" "$c_out" >/dev/null 2>&1; then
      printf '%s\n' "PASS smoke: $name"
    else
      printf '%s\n' "FAIL smoke: $name" >&2
      printf '%s\n' "expected substring: $expected" >&2
      printf '%s\n' "actual C output:" >&2
      cat "$c_out" >&2
      return 1
    fi
    return 0
  fi

  awk 'NF { for (i = 1; i <= n; i++) print blank[i]; n = 0; print; next } { blank[++n] = $0 }' "$c_out" >"$c_norm"

  if cmp -s "$expected_norm" "$c_norm"; then
    printf '%s\n' "PASS parity: $name"
  else
    printf '%s\n' "FAIL parity: $name" >&2
    printf '%s\n' "This proves the current C port is not yet fully equivalent to JHanNanum for this case." >&2
    printf '%s\n' "--- Expected fixture" >&2
    cat "$expected_norm" >&2
    printf '%s\n' "--- C port" >&2
    cat "$c_norm" >&2
    return 1
  fi
}

check_case() {
  if ! run_case "$@"; then
    failures=$((failures + 1))
  fi
}

run_mode_case() {
  name=$1
  input=$2
  c_mode=$3
  c_out="$BUILD_DIR/${name}.c.out"
  c_norm="$BUILD_DIR/${name}.c.norm"
  expected_norm="$EXPECTED_DIR/${name}.expected"

  "$C_RUNTIME" --data-dir "$DATA_DIR" "$c_mode" "$input" >"$c_out"

  awk 'NF { for (i = 1; i <= n; i++) print blank[i]; n = 0; print; next } { blank[++n] = $0 }' "$c_out" >"$c_norm"

  if cmp -s "$expected_norm" "$c_norm"; then
    printf '%s\n' "PASS parity: $name"
  else
    printf '%s\n' "FAIL parity: $name" >&2
    printf '%s\n' "--- Expected fixture" >&2
    cat "$expected_norm" >&2
    printf '%s\n' "--- C port" >&2
    cat "$c_norm" >&2
    return 1
  fi
}

check_mode_case() {
  if ! run_mode_case "$@"; then
    failures=$((failures + 1))
  fi
}

require_command cmake

cmake -S "$PROJECT_DIR" -B "$BUILD_DIR" -DHANNANUM_BUILD_RUNTIME=ON >/dev/null
cmake --build "$BUILD_DIR" >/dev/null

failures=0

check_case sample_smoke "학교에서조차도 그 사실을 모르고 있었다." contains "학교/ncn+에서/jca+조차/jxc+도/jxc"
check_case analyzed_dictionary_parity "사실을" parity ""
check_case demo_sentence_parity "학교에서조차도 그 사실을 모르고 있었다." parity ""
check_case workflow_demo_parity "프로젝트 전체 회의." parity ""
check_case date_demo_parity "日時: 2010년 7월 30일 오후 1시" parity ""
check_case decimal_parity "12.42" parity ""
check_case abbreviation_parity "U.S." parity ""
check_case analyzed_verb_parity "갑자기 갔다." parity ""
check_case short_sentence_parity "나는 학교에 간다." parity ""
check_case schedule_sentence_parity "회의 일정은 다음과 같습니다." parity ""
check_case progressive_sentence_parity "공부를 하고 있다." parity ""
check_case adjective_past_parity "같았다." parity ""
check_case copula_sentence_parity "것이다." parity ""
check_case parenthesized_company_parity "(주)대우" parity ""
check_case english_phrase_parity "Coex Conference Room" parity ""
check_case incident_number_parity "10.26사태" parity ""
check_case connective_sentence_parity "가지고 있다." parity ""
check_case modifier_sentence_parity "같은 것이다." parity ""
check_case quote_symbol_parity "\"학교\"" parity ""
check_case ellipsis_parity "그래..." parity ""
check_case repeated_punctuation_parity "뭐?!" parity ""
check_case negative_number_parity "-12.5" parity ""
check_case comma_number_parity "1,234" parity ""
check_case korean_number_unit_parity "삼십일" parity ""
check_case email_like_parity "hudoni@gmail.com" parity ""
check_case polite_ending_parity "겁니다." parity ""
check_case nominal_particle_parity "것으로" parity ""
check_case copula_question_parity "것인가?" parity ""
check_case user_dictionary_parity "그분들 당신들" parity ""
check_case percent_symbol_parity "100%" parity ""
check_case parenthesis_symbol_parity "( 학교 )" parity ""
check_case single_quote_symbol_parity "'학교'" parity ""
check_case month_day_unit_parity "12월 31일" parity ""
check_case signed_integer_parity "+42" parity ""
check_case long_ellipsis_parity "그래......" parity ""
check_case double_exclamation_parity "야!!" parity ""
check_case plus_decimal_parity "+12.5" parity ""
check_case comma_decimal_parity "1,234.56" parity ""
check_case sentence_remainder_parity "가라.그리고" parity ""
check_case closing_quote_eos_parity '간다."' parity ""
check_case postprocess_positive_vowel_parity "보았다." parity ""
check_case postprocess_o_vowel_parity "왔다." parity ""
check_case postprocess_ha_exception_parity "했다." parity ""
check_case postprocess_eu_elision_parity "가진" parity ""
check_case informal_repeat_parity "ㅋㅋㅋㅋㅋㅋ" parity ""
check_case informal_dot_repeat_parity "그래......." parity ""
check_mode_case simple_pos_09_parity "학교에서조차도 그 사실을 모르고 있었다." --simple-pos-09
check_mode_case simple_pos_22_parity "학교에서조차도 그 사실을 모르고 있었다." --simple-pos-22
check_mode_case noun_extractor_parity "학교에서조차도 그 사실을 모르고 있었다." --nouns
check_mode_case morph_analyzer_parity "사실을" --morph
check_mode_case simple_ma_09_parity "사실을" --simple-ma-09
check_mode_case simple_ma_22_parity "사실을" --simple-ma-22

if [ "$failures" -ne 0 ]; then
  printf '%s\n' "$failures parity test(s) failed" >&2
  exit 1
fi
