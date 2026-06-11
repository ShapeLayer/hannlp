#include "exp.h"

typedef struct exp_rule {
  const char *name;
  const char *chars;
} exp_rule_t;

static int exp_vec_from_utf8(const char *text, codepoint_vec_t *out);
static int exp_add_change_from_parts(const codepoint_vec_t *front, const codepoint_vec_t *back, int front_tag_type, int back_tag_type, int phoneme, exp_change_t *changes, size_t *count, size_t max_count);

static const exp_rule_t exp_rules[] = {
  { "초성", "ᄀᄁᄂᄃᄄᄅᄆᄇᄈᄉᄊᄋᄌᄍᄎᄏᄐᄑᄒ" },
  { "종성", "ᆨᆩᆪᆫᆬᆭᆮᆯᆰᆱᆲᆳᆴᆵᆶᆷᆸᆹᆺᆻᆼᆽᆾᆿᇀᇁᇂ" },
  { "중성", "ᅡᅣᅥᅧᅩᅭᅮᅲᅳᅵᅢᅤᅦᅨᅬᅱᅴᅪᅯᅫᅰ" },
  { "음성모음", "ᅥᅮᅧᅲᅦᅯᅱᅨ" },
  { "양성모음", "ᅡᅩᅣᅢᅪᅬᅤ" },
  { "중성모음", "ᅳᅵ" },
  { "rule_것l", "" },
  { "rule_것", "ᄂᄆᄅᆫᆯᆸ" },
  { "rule_것r", "" },
  { "l11", "ᅡᅣᅥᅧᅩᅭᅮᅲᅳᅵᅢᅤᅦᅨᅬᅱᅴᅪᅯᅫᅰ" },
  { "11", " ᆫᆯᆷᆸᄂᄉ" },
  { "r11", "" },
  { "l11-1", "ᅡᅣᅥᅧᅩᅭᅮᅲᅳᅵᅢᅤᅦᅨᅬᅱᅴᅪᅯᅫᅰ" },
  { "11-1", "ᄂᄉ" },
  { "r11-1", "" },
  { "l12", "" },
  { "12", "ᅡᅥ" },
  { "r12", "" },
  { "l13", "" },
  { "13", "ᅡ" },
  { "r13", "" },
  { "l14", "" },
  { "14", "ᅥᅦᅧᅢ" },
  { "r14", "" },
  { "l21", "ᆯ" },
  { "21", "ᄋ" },
  { "r21", "ᅥᅡᅳ" },
  { "l22", "ᅡᅥᅮᅳᅵ" },
  { "22", "ᄋ" },
  { "r22", "ᅥᅡᅳ" },
  { "l23", "ᄋ" },
  { "23", "ᅮ" },
  { "r23", "" },
  { "l24", "ᄋ" },
  { "24", "ᅪ" },
  { "r24", "" },
  { "l25", "ᄋ" },
  { "25", "ᅯ" },
  { "r25", "" },
  { "l26", "ᄀᄃᄅᄆᄋ" },
  { "26", "ᅡᅣ" },
  { "r26", "" },
  { "l27", "ᄀᄃᄅᄆᄄᄋ" },
  { "27", "ᅢᅤ" },
  { "r27", "" },
  { "l28", "ᄀᄃᄅᄆᄄᄋ" },
  { "28", "ᅥ" },
  { "r28", "" },
  { "l29", "ᆯ" },
  { "29", "ᄅ" },
  { "r29", "ᅥᅡ" },
  { "l30", "ᅳ" },
  { "30", "ᄅ" },
  { "r30", "ᅥ" },
  { "l31", "ᄑ" },
  { "31", "ᅥ" },
  { "r31", "" },
  { "l32", "ᄒ" },
  { "32", "ᅡ" },
  { "r32", "ᄋ" },
  { "l33", "ᄒ" },
  { "33", "ᅢ" },
  { "r33", "" },
  { "l51", "" },
  { "51", "ᅪᅯ" },
  { "r51", "" },
  { "l52", "" },
  { "52", "ᅫ" },
  { "r52", "" },
  { "l53", "" },
  { "53", "ᅧ" },
  { "r53", "" },
  { "l54", "ᆯᅡᅣᅥᅧᅩᅭᅮᅲᅳᅵᅢᅤᅦᅨᅬᅱᅴᅪᅯᅫᅰ" },
  { "54", " ᆫᆯᆷᆸᄂᄅᄆᄉᄋ" },
  { "r54", "" }
};

static int HANNANUM_UNUSED
exp_pcheck_cp(unsigned int c, const char *rule)
{
  size_t i;
  for (i = 0; i < sizeof(exp_rules) / sizeof(exp_rules[0]); i++) {
    const unsigned char *p;
    if (strcmp(exp_rules[i].name, rule) != 0) {
      continue;
    }
    if (exp_rules[i].chars[0] == '\0') {
      return 1;
    }
    p = (const unsigned char *)exp_rules[i].chars;
    while (*p != '\0') {
      unsigned int rule_cp;
      size_t width;
      if (!utf8_decode_one(p, &rule_cp, &width)) {
        return 0;
      }
      if (rule_cp == c) {
        return 1;
      }
      p += width;
    }
    return 0;
  }
  return 0;
}

static int HANNANUM_UNUSED
exp_pcheck_vec(const codepoint_vec_t *base, size_t index, const char *rule)
{
  unsigned int c = index < base->count ? base->items[index] : 0;
  return exp_pcheck_cp(c, rule);
}

static int HANNANUM_UNUSED
exp_vec_insert(const codepoint_vec_t *src, size_t index, const codepoint_vec_t *insert, codepoint_vec_t *out)
{
  size_t i;
  memset(out, 0, sizeof(*out));
  if (src == NULL || insert == NULL || index > src->count) {
    return 0;
  }
  for (i = 0; i < index; i++) {
    if (!codepoint_vec_push(out, src->items[i])) {
      codepoint_vec_free(out);
      return 0;
    }
  }
  for (i = 0; i < insert->count; i++) {
    if (!codepoint_vec_push(out, insert->items[i])) {
      codepoint_vec_free(out);
      return 0;
    }
  }
  for (i = index; i < src->count; i++) {
    if (!codepoint_vec_push(out, src->items[i])) {
      codepoint_vec_free(out);
      return 0;
    }
  }
  return 1;
}

static int HANNANUM_UNUSED
exp_vec_replace(const codepoint_vec_t *src, size_t index, unsigned int replacement, codepoint_vec_t *out)
{
  size_t i;
  memset(out, 0, sizeof(*out));
  if (src == NULL || index >= src->count) {
    return 0;
  }
  for (i = 0; i < src->count; i++) {
    if (!codepoint_vec_push(out, i == index ? replacement : src->items[i])) {
      codepoint_vec_free(out);
      return 0;
    }
  }
  return 1;
}

static void HANNANUM_UNUSED
exp_change_free(exp_change_t *change)
{
  if (change == NULL) {
    return;
  }
  codepoint_vec_free(&change->front);
  codepoint_vec_free(&change->back);
  memset(change, 0, sizeof(*change));
}

static int HANNANUM_UNUSED
exp_vec_from_utf8(const char *text, codepoint_vec_t *out)
{
  const unsigned char *p = (const unsigned char *)text;
  memset(out, 0, sizeof(*out));
  while (*p != '\0') {
    unsigned int cp;
    size_t width;
    if (!utf8_decode_one(p, &cp, &width)) {
      codepoint_vec_free(out);
      return 0;
    }
    if (!codepoint_vec_push(out, cp)) {
      codepoint_vec_free(out);
      return 0;
    }
    p += width;
  }
  return 1;
}

static int HANNANUM_UNUSED
exp_vec_slice(const codepoint_vec_t *src, size_t start, size_t end, codepoint_vec_t *out)
{
  size_t i;
  memset(out, 0, sizeof(*out));
  if (src == NULL || start > end || end > src->count) {
    return 0;
  }
  for (i = start; i < end; i++) {
    if (!codepoint_vec_push(out, src->items[i])) {
      codepoint_vec_free(out);
      return 0;
    }
  }
  return 1;
}

static int HANNANUM_UNUSED
exp_vec_starts_with(const codepoint_vec_t *src, size_t index, const codepoint_vec_t *prefix)
{
  size_t i;
  if (src == NULL || prefix == NULL || index + prefix->count > src->count) {
    return 0;
  }
  for (i = 0; i < prefix->count; i++) {
    if (src->items[index + i] != prefix->items[i]) {
      return 0;
    }
  }
  return 1;
}

static int HANNANUM_UNUSED
exp_vec_equals_utf8(const codepoint_vec_t *src, const char *text)
{
  codepoint_vec_t expected;
  int equal = 0;
  size_t i;
  if (!exp_vec_from_utf8(text, &expected)) {
    return 0;
  }
  if (src->count == expected.count) {
    equal = 1;
    for (i = 0; i < src->count; i++) {
      if (src->items[i] != expected.items[i]) {
        equal = 0;
        break;
      }
    }
  }
  codepoint_vec_free(&expected);
  return equal;
}

static int HANNANUM_UNUSED
exp_vec_starts_with_utf8(const codepoint_vec_t *src, const char *text)
{
  codepoint_vec_t prefix;
  int ok;
  if (!exp_vec_from_utf8(text, &prefix)) {
    return 0;
  }
  ok = exp_vec_starts_with(src, 0, &prefix);
  codepoint_vec_free(&prefix);
  return ok;
}

static int HANNANUM_UNUSED
exp_add_change_from_utf8_parts(const char *front_text, const codepoint_vec_t *back, int front_tag_type, int back_tag_type, exp_change_t *changes, size_t *count, size_t max_count)
{
  codepoint_vec_t front;
  int ok;
  memset(&front, 0, sizeof(front));
  if (!exp_vec_from_utf8(front_text, &front)) {
    return 0;
  }
  ok = exp_add_change_from_parts(&front, back, front_tag_type, back_tag_type, 0, changes, count, max_count);
  codepoint_vec_free(&front);
  return ok;
}

static int
exp_add_change_from_insert(const codepoint_vec_t *str, size_t cur, const char *insert_text, exp_change_t *changes, size_t *count, size_t max_count)
{
  codepoint_vec_t insert;
  codepoint_vec_t new_str;
  if (*count >= max_count) {
    return 0;
  }
  memset(&insert, 0, sizeof(insert));
  memset(&new_str, 0, sizeof(new_str));
  if (!exp_vec_from_utf8(insert_text, &insert)) {
    return 0;
  }
  if (!exp_vec_insert(str, cur, &insert, &new_str)) {
    codepoint_vec_free(&insert);
    return 0;
  }
  codepoint_vec_free(&insert);
  if (!exp_vec_slice(&new_str, 0, cur, &changes[*count].front) || !exp_vec_slice(&new_str, cur, new_str.count, &changes[*count].back)) {
    codepoint_vec_free(&new_str);
    exp_change_free(&changes[*count]);
    return 0;
  }
  changes[*count].front_tag_type = EXP_TAG_TYPE_YONGS;
  changes[*count].back_tag_type = EXP_TAG_TYPE_EOMIES;
  changes[*count].phoneme = 0;
  (*count)++;
  codepoint_vec_free(&new_str);
  return 1;
}

static int
exp_add_change_from_replace(const codepoint_vec_t *str, size_t replace_at, unsigned int replacement, size_t split_at, exp_change_t *changes, size_t *count, size_t max_count)
{
  codepoint_vec_t new_str;
  if (*count >= max_count) {
    return 0;
  }
  memset(&new_str, 0, sizeof(new_str));
  if (!exp_vec_replace(str, replace_at, replacement, &new_str)) {
    return 0;
  }
  if (!exp_vec_slice(&new_str, 0, split_at, &changes[*count].front) || !exp_vec_slice(&new_str, split_at, new_str.count, &changes[*count].back)) {
    codepoint_vec_free(&new_str);
    exp_change_free(&changes[*count]);
    return 0;
  }
  changes[*count].front_tag_type = EXP_TAG_TYPE_YONGS;
  changes[*count].back_tag_type = EXP_TAG_TYPE_EOMIES;
  changes[*count].phoneme = 0;
  (*count)++;
  codepoint_vec_free(&new_str);
  return 1;
}

static int
exp_add_change_from_parts(const codepoint_vec_t *front, const codepoint_vec_t *back, int front_tag_type, int back_tag_type, int phoneme, exp_change_t *changes, size_t *count, size_t max_count)
{
  if (*count >= max_count) {
    return 0;
  }
  if (!exp_vec_slice(front, 0, front->count, &changes[*count].front) || !exp_vec_slice(back, 0, back->count, &changes[*count].back)) {
    exp_change_free(&changes[*count]);
    return 0;
  }
  changes[*count].front_tag_type = front_tag_type;
  changes[*count].back_tag_type = back_tag_type;
  changes[*count].phoneme = phoneme;
  (*count)++;
  return 1;
}

static size_t HANNANUM_UNUSED
exp_rule_eomi_u_generate(const codepoint_vec_t *str, size_t cur, exp_change_t *changes, size_t max_count)
{
  size_t count = 0;
  codepoint_vec_t bnida;
  codepoint_vec_t nda;
  if (str == NULL || changes == NULL || cur > str->count) {
    return 0;
  }
  memset(changes, 0, max_count * sizeof(changes[0]));
  if (cur > 0 && exp_pcheck_vec(str, cur - 1, "l54") && exp_pcheck_vec(str, cur, "54") && exp_pcheck_vec(str, cur + 1, "r54")) {
    exp_add_change_from_insert(str, cur, "으", changes, &count, max_count);
  }
  if (cur > 0 && exp_pcheck_vec(str, cur - 1, "l54") && exp_vec_from_utf8("ᆸ니", &bnida)) {
    if (exp_vec_starts_with(str, cur, &bnida)) {
      exp_add_change_from_insert(str, cur, "스", changes, &count, max_count);
    }
    codepoint_vec_free(&bnida);
  }
  if (cur > 0 && exp_pcheck_vec(str, cur - 1, "l54") && exp_vec_from_utf8("ᆫ다", &nda)) {
    if (exp_vec_starts_with(str, cur, &nda)) {
      exp_add_change_from_insert(str, cur, "느", changes, &count, max_count);
    }
    codepoint_vec_free(&nda);
  }
  return count;
}

static size_t HANNANUM_UNUSED
exp_rule_johwa_generate(const codepoint_vec_t *str, size_t cur, exp_change_t *changes, size_t max_count)
{
  size_t count = 0;
  unsigned int choseong_ieung;
  unsigned int jungseong_a;
  unsigned int jungseong_eo;
  size_t width;
  if (str == NULL || changes == NULL || cur > str->count) {
    return 0;
  }
  memset(changes, 0, max_count * sizeof(changes[0]));
  if (!utf8_decode_one((const unsigned char *)"ᄋ", &choseong_ieung, &width) || !utf8_decode_one((const unsigned char *)"ᅡ", &jungseong_a, &width) || !utf8_decode_one((const unsigned char *)"ᅥ", &jungseong_eo, &width)) {
    return 0;
  }
  if (cur > 0 && exp_pcheck_vec(str, cur - 1, "양성모음")) {
    if (cur + 2 < str->count && str->items[cur + 1] == choseong_ieung && str->items[cur + 2] == jungseong_a) {
      exp_add_change_from_replace(str, cur + 2, jungseong_eo, cur + 1, changes, &count, max_count);
    } else if (cur + 1 < str->count && str->items[cur] == choseong_ieung && str->items[cur + 1] == jungseong_a) {
      exp_add_change_from_replace(str, cur + 1, jungseong_eo, cur, changes, &count, max_count);
    }
  }
  return count;
}

static size_t HANNANUM_UNUSED
exp_rule_i_generate(const codepoint_vec_t *prev, const codepoint_vec_t *str, size_t cur, exp_change_t *changes, size_t max_count)
{
  size_t count = 0;
  codepoint_vec_t iyeo;
  codepoint_vec_t neun;
  codepoint_vec_t eun;
  codepoint_vec_t eum;
  codepoint_vec_t i_morph;
  codepoint_vec_t combined;
  unsigned int jungseong_eo;
  size_t width;
  if (prev == NULL || str == NULL || changes == NULL || cur + 2 > str->count) {
    return 0;
  }
  memset(changes, 0, max_count * sizeof(changes[0]));
  memset(&iyeo, 0, sizeof(iyeo));
  memset(&neun, 0, sizeof(neun));
  memset(&eun, 0, sizeof(eun));
  memset(&eum, 0, sizeof(eum));
  memset(&i_morph, 0, sizeof(i_morph));
  memset(&combined, 0, sizeof(combined));
  if (prev->count == 0 || cur != 0 || !exp_pcheck_vec(prev, prev->count - 1, "중성")) {
    return 0;
  }
  if (!exp_vec_from_utf8("여", &iyeo) || !exp_vec_from_utf8("는", &neun) || !exp_vec_from_utf8("은", &eun) || !exp_vec_from_utf8("음", &eum) || !exp_vec_from_utf8("이", &i_morph) || !utf8_decode_one((const unsigned char *)"ᅥ", &jungseong_eo, &width)) {
    goto done;
  }
  if (exp_vec_starts_with(str, 0, &iyeo)) {
    codepoint_vec_t replaced;
    codepoint_vec_t insert;
    codepoint_vec_t new_str;
    memset(&replaced, 0, sizeof(replaced));
    memset(&insert, 0, sizeof(insert));
    memset(&new_str, 0, sizeof(new_str));
    if (exp_vec_replace(str, cur + 1, jungseong_eo, &replaced) && exp_vec_from_utf8("ᅵᄋ", &insert) && exp_vec_insert(&replaced, cur + 1, &insert, &new_str) && max_count > 0) {
      if (exp_vec_slice(&new_str, 0, cur + 2, &changes[count].front) && exp_vec_slice(&new_str, cur + 2, new_str.count, &changes[count].back)) {
        changes[count].front_tag_type = EXP_TAG_TYPE_JP;
        changes[count].back_tag_type = EXP_TAG_TYPE_EOMIES;
        changes[count].phoneme = 0;
        count++;
      }
    }
    codepoint_vec_free(&replaced);
    codepoint_vec_free(&insert);
    codepoint_vec_free(&new_str);
  } else {
    if (exp_pcheck_vec(str, 0, "종성") || exp_vec_starts_with(str, 0, &neun) || exp_vec_starts_with(str, 0, &eun) || exp_vec_starts_with(str, 0, &eum) || exp_vec_starts_with(str, 2, &neun)) {
      goto done;
    }
    exp_add_change_from_parts(&i_morph, str, EXP_TAG_TYPE_JP, EXP_TAG_TYPE_EOMIES, 0, changes, &count, max_count);
    if (count < max_count && exp_vec_insert(str, 0, &i_morph, &combined)) {
      exp_change_t nested[4];
      size_t nested_count = exp_rule_eomi_u_generate(&combined, cur + 2, nested, 4);
      size_t i;
      for (i = 0; i < nested_count && count < max_count; i++) {
        changes[count++] = nested[i];
      }
      for (; i < nested_count; i++) {
        exp_change_free(&nested[i]);
      }
    }
  }
done:
  codepoint_vec_free(&iyeo);
  codepoint_vec_free(&neun);
  codepoint_vec_free(&eun);
  codepoint_vec_free(&eum);
  codepoint_vec_free(&i_morph);
  codepoint_vec_free(&combined);
  return count;
}

static size_t HANNANUM_UNUSED
exp_rule_gut_generate(const codepoint_vec_t *str, size_t cur, exp_change_t *changes, size_t max_count)
{
  size_t count = 0;
  codepoint_vec_t gut_prefix;
  codepoint_vec_t bnida;
  codepoint_vec_t llo;
  codepoint_vec_t si;
  codepoint_vec_t seu;
  unsigned int jong_b;
  unsigned int jong_l;
  unsigned int jong_n;
  unsigned int jong_s;
  size_t width;
  if (str == NULL || changes == NULL || cur >= str->count) {
    return 0;
  }
  memset(changes, 0, max_count * sizeof(changes[0]));
  memset(&gut_prefix, 0, sizeof(gut_prefix));
  memset(&bnida, 0, sizeof(bnida));
  memset(&llo, 0, sizeof(llo));
  memset(&si, 0, sizeof(si));
  memset(&seu, 0, sizeof(seu));
  if (!exp_vec_from_utf8("거", &gut_prefix) || !exp_vec_from_utf8("ᆸ니", &bnida) || !exp_vec_from_utf8("ᆯ로", &llo) || !exp_vec_from_utf8("ᆺ이", &si) || !exp_vec_from_utf8("ᆺ으", &seu) || !utf8_decode_one((const unsigned char *)"ᆸ", &jong_b, &width) || !utf8_decode_one((const unsigned char *)"ᆯ", &jong_l, &width) || !utf8_decode_one((const unsigned char *)"ᆫ", &jong_n, &width) || !utf8_decode_one((const unsigned char *)"ᆺ", &jong_s, &width)) {
    goto done;
  }
  if (!(cur > 1 && exp_vec_starts_with(str, cur - 2, &gut_prefix) && exp_pcheck_vec(str, cur, "rule_것"))) {
    goto done;
  }
  if (str->items[cur] == jong_b) {
    if (exp_vec_starts_with(str, cur, &bnida)) {
      codepoint_vec_t new_str;
      memset(&new_str, 0, sizeof(new_str));
      if (exp_vec_insert(str, cur, &si, &new_str) && count < max_count) {
        if (exp_vec_slice(&new_str, 0, cur + 1, &changes[count].front) && exp_vec_slice(&new_str, cur + 1, new_str.count, &changes[count].back)) {
          changes[count].front_tag_type = EXP_TAG_TYPE_NBNP;
          changes[count].back_tag_type = EXP_TAG_TYPE_JP;
          changes[count].phoneme = 0;
          count++;
        }
      }
      codepoint_vec_free(&new_str);
    }
  } else if (exp_vec_starts_with(str, cur, &llo)) {
    codepoint_vec_t replaced;
    codepoint_vec_t new_str;
    memset(&replaced, 0, sizeof(replaced));
    memset(&new_str, 0, sizeof(new_str));
    if (exp_vec_replace(str, cur, jong_s, &replaced) && exp_vec_insert(&replaced, cur + 1, &seu, &new_str) && count < max_count) {
      if (exp_vec_slice(&new_str, 0, cur + 1, &changes[count].front) && exp_vec_slice(&new_str, cur + 1, new_str.count, &changes[count].back)) {
        changes[count].front_tag_type = EXP_TAG_TYPE_NBNP;
        changes[count].back_tag_type = EXP_TAG_TYPE_JOSA;
        changes[count].phoneme = 0;
        count++;
      }
    }
    codepoint_vec_free(&replaced);
    codepoint_vec_free(&new_str);
  } else if (str->items[cur] == jong_l || str->items[cur] == jong_n) {
    if (str->count != cur + 1 && count < max_count) {
      codepoint_vec_t new_str;
      memset(&new_str, 0, sizeof(new_str));
      if (exp_vec_insert(str, cur, &si, &new_str)) {
        if (exp_vec_slice(&new_str, 0, cur + 1, &changes[count].front) && exp_vec_slice(&new_str, cur + 1, new_str.count, &changes[count].back)) {
          changes[count].front_tag_type = EXP_TAG_TYPE_NBNP;
          changes[count].back_tag_type = EXP_TAG_TYPE_JP;
          changes[count].phoneme = 0;
          count++;
        }
        codepoint_vec_free(&new_str);
      }
    }
    if (count < max_count) {
      codepoint_vec_t new_str;
      memset(&new_str, 0, sizeof(new_str));
      if (exp_vec_insert(str, cur, &seu, &new_str)) {
        if (exp_vec_slice(&new_str, 0, cur + 1, &changes[count].front) && exp_vec_slice(&new_str, cur + 1, new_str.count, &changes[count].back)) {
          changes[count].front_tag_type = EXP_TAG_TYPE_NBNP;
          changes[count].back_tag_type = EXP_TAG_TYPE_JOSA;
          changes[count].phoneme = 0;
          count++;
        }
        codepoint_vec_free(&new_str);
      }
    }
  } else if (count < max_count) {
    codepoint_vec_t new_str;
    memset(&new_str, 0, sizeof(new_str));
    if (exp_vec_insert(str, cur, &si, &new_str)) {
      if (exp_vec_slice(&new_str, 0, cur + 1, &changes[count].front) && exp_vec_slice(&new_str, cur + 1, new_str.count, &changes[count].back)) {
        changes[count].front_tag_type = EXP_TAG_TYPE_NBNP;
        changes[count].back_tag_type = EXP_TAG_TYPE_JP;
        changes[count].phoneme = 0;
        count++;
      }
      codepoint_vec_free(&new_str);
    }
  }
done:
  codepoint_vec_free(&gut_prefix);
  codepoint_vec_free(&bnida);
  codepoint_vec_free(&llo);
  codepoint_vec_free(&si);
  codepoint_vec_free(&seu);
  return count;
}

static size_t HANNANUM_UNUSED
exp_rule_shorten_generate(const codepoint_vec_t *str, size_t cur, exp_change_t *changes, size_t max_count)
{
  size_t count = 0;
  unsigned int jungseong_wa;
  unsigned int jungseong_wo;
  unsigned int jungseong_o;
  unsigned int jungseong_u;
  unsigned int jungseong_wae;
  unsigned int jungseong_oe;
  unsigned int jungseong_yeo;
  unsigned int jungseong_i;
  unsigned int choseong_ieung;
  size_t width;
  if (str == NULL || changes == NULL || cur >= str->count) {
    return 0;
  }
  memset(changes, 0, max_count * sizeof(changes[0]));
  if (!utf8_decode_one((const unsigned char *)"ᅪ", &jungseong_wa, &width) || !utf8_decode_one((const unsigned char *)"ᅯ", &jungseong_wo, &width) || !utf8_decode_one((const unsigned char *)"ᅩ", &jungseong_o, &width) || !utf8_decode_one((const unsigned char *)"ᅮ", &jungseong_u, &width) || !utf8_decode_one((const unsigned char *)"ᅫ", &jungseong_wae, &width) || !utf8_decode_one((const unsigned char *)"ᅬ", &jungseong_oe, &width) || !utf8_decode_one((const unsigned char *)"ᅧ", &jungseong_yeo, &width) || !utf8_decode_one((const unsigned char *)"ᅵ", &jungseong_i, &width) || !utf8_decode_one((const unsigned char *)"ᄋ", &choseong_ieung, &width)) {
    return 0;
  }
  if (cur > 0 && exp_pcheck_vec(str, cur - 1, "l51") && exp_pcheck_vec(str, cur, "51") && exp_pcheck_vec(str, cur + 1, "r51")) {
    exp_add_change_from_replace(str, cur, str->items[cur] == jungseong_wa ? jungseong_o : jungseong_u, cur + 1, changes, &count, max_count);
    if (count > 0) {
      codepoint_vec_t suffix;
      codepoint_vec_t with_eo;
      memset(&suffix, 0, sizeof(suffix));
      memset(&with_eo, 0, sizeof(with_eo));
      if (exp_vec_from_utf8("어", &suffix) && exp_vec_insert(&changes[count - 1].back, 0, &suffix, &with_eo)) {
        codepoint_vec_free(&changes[count - 1].back);
        changes[count - 1].back = with_eo;
      }
      codepoint_vec_free(&suffix);
    }
  }
  if (cur > 0 && exp_pcheck_vec(str, cur - 1, "l52") && exp_pcheck_vec(str, cur, "52") && exp_pcheck_vec(str, cur + 1, "r52")) {
    exp_add_change_from_replace(str, cur, jungseong_oe, cur + 1, changes, &count, max_count);
    if (count > 0) {
      codepoint_vec_t suffix;
      codepoint_vec_t with_eo;
      memset(&suffix, 0, sizeof(suffix));
      memset(&with_eo, 0, sizeof(with_eo));
      if (exp_vec_from_utf8("어", &suffix) && exp_vec_insert(&changes[count - 1].back, 0, &suffix, &with_eo)) {
        codepoint_vec_free(&changes[count - 1].back);
        changes[count - 1].back = with_eo;
      }
      codepoint_vec_free(&suffix);
    }
  }
  if (cur > 0 && ((cur > 1 || str->items[cur - 1] != choseong_ieung) && exp_pcheck_vec(str, cur - 1, "l53") && exp_pcheck_vec(str, cur, "53") && exp_pcheck_vec(str, cur + 1, "r53"))) {
    exp_add_change_from_replace(str, cur, jungseong_i, cur + 1, changes, &count, max_count);
    if (count > 0) {
      codepoint_vec_t suffix;
      codepoint_vec_t with_eo;
      memset(&suffix, 0, sizeof(suffix));
      memset(&with_eo, 0, sizeof(with_eo));
      if (exp_vec_from_utf8("어", &suffix) && exp_vec_insert(&changes[count - 1].back, 0, &suffix, &with_eo)) {
        codepoint_vec_free(&changes[count - 1].back);
        changes[count - 1].back = with_eo;
      }
      codepoint_vec_free(&suffix);
    }
  }
  (void)jungseong_wo;
  (void)jungseong_wae;
  (void)jungseong_yeo;
  return count;
}

static size_t HANNANUM_UNUSED
exp_rule_rem_generate(const codepoint_vec_t *str, size_t cur, exp_change_t *changes, size_t max_count)
{
  size_t count = 0;
  codepoint_vec_t inserted;
  codepoint_vec_t l_jong;
  codepoint_vec_t ieo;
  unsigned int jungseong_eo;
  size_t width;
  if (str == NULL || changes == NULL || cur >= str->count) {
    return 0;
  }
  memset(changes, 0, max_count * sizeof(changes[0]));
  memset(&inserted, 0, sizeof(inserted));
  memset(&l_jong, 0, sizeof(l_jong));
  memset(&ieo, 0, sizeof(ieo));
  exp_vec_from_utf8("ᆯ", &l_jong);
  exp_vec_from_utf8("어", &ieo);
  if (!utf8_decode_one((const unsigned char *)"ᅥ", &jungseong_eo, &width)) {
    codepoint_vec_free(&l_jong);
    codepoint_vec_free(&ieo);
    return 0;
  }
  if (cur > 0 && exp_pcheck_vec(str, cur - 1, "l11") && (exp_pcheck_vec(str, cur, "11") || exp_vec_starts_with(str, cur, &ieo)) && exp_pcheck_vec(str, cur + 1, "r11")) {
    if (exp_vec_insert(str, cur, &l_jong, &inserted)) {
      if (count < max_count && exp_vec_slice(&inserted, 0, cur + 1, &changes[count].front) && exp_vec_slice(&inserted, cur + 1, inserted.count, &changes[count].back)) {
        changes[count].front_tag_type = EXP_TAG_TYPE_YONGS;
        changes[count].back_tag_type = EXP_TAG_TYPE_EOMIES;
        changes[count].phoneme = 0;
        count++;
      }
      if (count < max_count && exp_vec_slice(&inserted, 0, cur, &changes[count].front) && exp_vec_slice(&inserted, cur + 1, inserted.count, &changes[count].back)) {
        changes[count].front_tag_type = EXP_TAG_TYPE_YONGS;
        changes[count].back_tag_type = EXP_TAG_TYPE_EOMIES;
        changes[count].phoneme = 0;
        count++;
      }
      if (count < max_count) {
        exp_change_t nested[4];
        size_t nested_count = exp_rule_eomi_u_generate(&inserted, cur + 1, nested, 4);
        size_t i;
        for (i = 0; i < nested_count && count < max_count; i++) {
          changes[count++] = nested[i];
        }
        for (; i < nested_count; i++) {
          exp_change_free(&nested[i]);
        }
      }
      codepoint_vec_free(&inserted);
    }
  }
  if (((cur > 0 && exp_pcheck_vec(str, cur - 1, "l12") && exp_pcheck_vec(str, cur, "12") && exp_pcheck_vec(str, cur + 1, "r12")) || (cur == 1 && str->items[cur] != 0x1161))) {
    codepoint_vec_t replaced;
    memset(&replaced, 0, sizeof(replaced));
    if (exp_vec_replace(str, cur, jungseong_eo, &replaced)) {
      codepoint_vec_t eu_ieung;
      codepoint_vec_t new_str;
      memset(&eu_ieung, 0, sizeof(eu_ieung));
      memset(&new_str, 0, sizeof(new_str));
      if (exp_vec_from_utf8("ᅳᄋ", &eu_ieung) && exp_vec_insert(&replaced, cur, &eu_ieung, &new_str) && count < max_count) {
        if (exp_vec_slice(&new_str, 0, cur + 1, &changes[count].front) && exp_vec_slice(&new_str, cur + 1, new_str.count, &changes[count].back)) {
          changes[count].front_tag_type = EXP_TAG_TYPE_YONGS;
          changes[count].back_tag_type = EXP_TAG_TYPE_EOMIES;
          changes[count].phoneme = 0;
          count++;
        }
      }
      codepoint_vec_free(&eu_ieung);
      codepoint_vec_free(&new_str);
      codepoint_vec_free(&replaced);
    }
  }
  if (cur > 0 && exp_pcheck_vec(str, cur - 1, "l13") && exp_pcheck_vec(str, cur, "13") && exp_pcheck_vec(str, cur + 1, "r13")) {
    exp_add_change_from_insert(str, cur + 1, "어", changes, &count, max_count);
    if (count > 0) {
      codepoint_vec_t new_str;
      memset(&new_str, 0, sizeof(new_str));
      if (exp_vec_insert(str, cur + 1, &ieo, &new_str)) {
        codepoint_vec_free(&changes[count - 1].front);
        codepoint_vec_free(&changes[count - 1].back);
        exp_vec_slice(&new_str, 0, cur + 1, &changes[count - 1].front);
        exp_vec_slice(&new_str, cur + 1, new_str.count, &changes[count - 1].back);
        codepoint_vec_free(&new_str);
      }
    }
  }
  if (cur > 0 && exp_pcheck_vec(str, cur - 1, "l14") && exp_pcheck_vec(str, cur, "14") && exp_pcheck_vec(str, cur + 1, "r14")) {
    if (count < max_count) {
      codepoint_vec_t new_str;
      memset(&new_str, 0, sizeof(new_str));
      if (exp_vec_insert(str, cur + 1, &ieo, &new_str)) {
        if (exp_vec_slice(&new_str, 0, cur + 1, &changes[count].front) && exp_vec_slice(&new_str, cur + 1, new_str.count, &changes[count].back)) {
          changes[count].front_tag_type = EXP_TAG_TYPE_YONGS;
          changes[count].back_tag_type = EXP_TAG_TYPE_EOMIES;
          changes[count].phoneme = 0;
          count++;
        }
        codepoint_vec_free(&new_str);
      }
    }
  }
  codepoint_vec_free(&l_jong);
  codepoint_vec_free(&ieo);
  return count;
}

static size_t HANNANUM_UNUSED
exp_rule_irr_word2_generate(const codepoint_vec_t *str, size_t cur, exp_change_t *changes, size_t max_count)
{
  size_t count = 0;
  unsigned int jungseong_u;
  unsigned int jungseong_a;
  unsigned int jungseong_eo;
  unsigned int jungseong_yeo;
  size_t width;
  if (str == NULL || changes == NULL || cur >= str->count) {
    return 0;
  }
  memset(changes, 0, max_count * sizeof(changes[0]));
  if (!utf8_decode_one((const unsigned char *)"ᅮ", &jungseong_u, &width) || !utf8_decode_one((const unsigned char *)"ᅡ", &jungseong_a, &width) || !utf8_decode_one((const unsigned char *)"ᅥ", &jungseong_eo, &width) || !utf8_decode_one((const unsigned char *)"ᅧ", &jungseong_yeo, &width)) {
    return 0;
  }
  if (cur > 0 && exp_pcheck_vec(str, cur - 1, "l31") && exp_pcheck_vec(str, cur, "31") && exp_pcheck_vec(str, cur + 1, "r31")) {
    exp_add_change_from_replace(str, cur, jungseong_u, cur + 1, changes, &count, max_count);
    if (count > 0) {
      codepoint_vec_t suffix;
      codepoint_vec_t with_eo;
      memset(&suffix, 0, sizeof(suffix));
      memset(&with_eo, 0, sizeof(with_eo));
      if (exp_vec_from_utf8("어", &suffix) && exp_vec_insert(&changes[count - 1].back, 0, &suffix, &with_eo)) {
        codepoint_vec_free(&changes[count - 1].back);
        changes[count - 1].back = with_eo;
      }
      codepoint_vec_free(&suffix);
    }
  }
  if (cur > 0 && cur + 2 < str->count && exp_pcheck_vec(str, cur - 1, "l32") && exp_pcheck_vec(str, cur, "32") && exp_pcheck_vec(str, cur + 1, "r32") && str->items[cur + 2] == jungseong_yeo) {
    exp_add_change_from_replace(str, cur + 2, jungseong_eo, cur + 1, changes, &count, max_count);
  }
  if (cur > 0 && exp_pcheck_vec(str, cur - 1, "l33") && exp_pcheck_vec(str, cur, "33") && exp_pcheck_vec(str, cur + 1, "r33")) {
    exp_add_change_from_replace(str, cur, jungseong_a, cur + 1, changes, &count, max_count);
    if (count > 0) {
      codepoint_vec_t suffix;
      codepoint_vec_t with_eo;
      memset(&suffix, 0, sizeof(suffix));
      memset(&with_eo, 0, sizeof(with_eo));
      if (exp_vec_from_utf8("어", &suffix) && exp_vec_insert(&changes[count - 1].back, 0, &suffix, &with_eo)) {
        codepoint_vec_free(&changes[count - 1].back);
        changes[count - 1].back = with_eo;
      }
      codepoint_vec_free(&suffix);
    }
  }
  return count;
}

static int
exp_add_change_from_replaced_with_phoneme(const codepoint_vec_t *str, size_t replace_at, unsigned int replacement, size_t split_at, int phoneme, exp_change_t *changes, size_t *count, size_t max_count)
{
  if (!exp_add_change_from_replace(str, replace_at, replacement, split_at, changes, count, max_count)) {
    return 0;
  }
  changes[*count - 1].phoneme = phoneme;
  return 1;
}

static int
exp_add_change_from_inserted_with_phoneme(const codepoint_vec_t *str, size_t insert_at, const codepoint_vec_t *insert, size_t split_at, int phoneme, exp_change_t *changes, size_t *count, size_t max_count)
{
  codepoint_vec_t new_str;
  memset(&new_str, 0, sizeof(new_str));
  if (*count >= max_count || !exp_vec_insert(str, insert_at, insert, &new_str)) {
    return 0;
  }
  if (!exp_vec_slice(&new_str, 0, split_at, &changes[*count].front) || !exp_vec_slice(&new_str, split_at, new_str.count, &changes[*count].back)) {
    codepoint_vec_free(&new_str);
    exp_change_free(&changes[*count]);
    return 0;
  }
  changes[*count].front_tag_type = EXP_TAG_TYPE_YONGS;
  changes[*count].back_tag_type = EXP_TAG_TYPE_EOMIES;
  changes[*count].phoneme = phoneme;
  (*count)++;
  codepoint_vec_free(&new_str);
  return 1;
}

static unsigned int
exp_to_choseong(unsigned int jong)
{
  static const unsigned int table[] = {
    0, 0x1100, 0x1101, 0, 0x1102, 0, 0, 0x1103, 0x1105, 0, 0, 0, 0, 0, 0, 0,
    0x1106, 0x1107, 0, 0x1109, 0x110a, 0x110b, 0x110c, 0x110e, 0x110f, 0x1110, 0x1111, 0x1112
  };
  if (jong >= 0x11a7 && jong < 0x11a7 + sizeof(table) / sizeof(table[0])) {
    return table[jong - 0x11a7];
  }
  return 0;
}

static size_t HANNANUM_UNUSED
exp_rule_irr_word_generate_basic(const codepoint_vec_t *str, size_t cur, const exp_irregular_ids_t *ids, exp_change_t *changes, size_t max_count)
{
  size_t count = 0;
  unsigned int jong_d;
  unsigned int jong_h;
  unsigned int jungseong_eu;
  unsigned int jungseong_eo;
  unsigned int jungseong_a;
  unsigned int jungseong_ya;
  unsigned int jungseong_ae;
  unsigned int jungseong_yeo;
  unsigned int choseong_ieung;
  codepoint_vec_t jong_s;
  codepoint_vec_t jong_b;
  codepoint_vec_t h_ieung_eu;
  codepoint_vec_t h_ieung_eo;
  codepoint_vec_t h_ieung;
  size_t width;
  if (str == NULL || ids == NULL || changes == NULL || cur > str->count) {
    return 0;
  }
  memset(changes, 0, max_count * sizeof(changes[0]));
  memset(&jong_s, 0, sizeof(jong_s));
  memset(&jong_b, 0, sizeof(jong_b));
  memset(&h_ieung_eu, 0, sizeof(h_ieung_eu));
  memset(&h_ieung_eo, 0, sizeof(h_ieung_eo));
  memset(&h_ieung, 0, sizeof(h_ieung));
  if (!utf8_decode_one((const unsigned char *)"ᆮ", &jong_d, &width) || !utf8_decode_one((const unsigned char *)"ᇂ", &jong_h, &width) || !utf8_decode_one((const unsigned char *)"ᅳ", &jungseong_eu, &width) || !utf8_decode_one((const unsigned char *)"ᅥ", &jungseong_eo, &width) || !utf8_decode_one((const unsigned char *)"ᅡ", &jungseong_a, &width) || !utf8_decode_one((const unsigned char *)"ᅣ", &jungseong_ya, &width) || !utf8_decode_one((const unsigned char *)"ᅢ", &jungseong_ae, &width) || !utf8_decode_one((const unsigned char *)"ᅧ", &jungseong_yeo, &width) || !utf8_decode_one((const unsigned char *)"ᄋ", &choseong_ieung, &width) || !exp_vec_from_utf8("ᆺ", &jong_s) || !exp_vec_from_utf8("ᆸ", &jong_b) || !exp_vec_from_utf8("ᇂ으", &h_ieung_eu) || !exp_vec_from_utf8("ᇂ어", &h_ieung_eo) || !exp_vec_from_utf8("ᇂᄋ", &h_ieung)) {
    goto done;
  }
  if (cur > 0 && cur <= str->count && exp_pcheck_vec(str, cur - 1, "l21") && exp_pcheck_vec(str, cur, "21") && exp_pcheck_vec(str, cur + 1, "r21")) {
    exp_add_change_from_replaced_with_phoneme(str, cur - 1, jong_d, cur, ids->type_d, changes, &count, max_count);
  }
  if (cur > 0 && cur < str->count && exp_pcheck_vec(str, cur - 1, "l22") && exp_pcheck_vec(str, cur, "22") && exp_pcheck_vec(str, cur + 1, "r22")) {
    exp_add_change_from_inserted_with_phoneme(str, cur, &jong_s, cur + 1, ids->type_s, changes, &count, max_count);
  }
  if (cur > 0 && cur <= str->count && exp_pcheck_vec(str, cur - 1, "l23") && exp_pcheck_vec(str, cur, "23") && exp_pcheck_vec(str, cur + 1, "r23")) {
    codepoint_vec_t replaced;
    memset(&replaced, 0, sizeof(replaced));
    if (exp_vec_replace(str, cur, jungseong_eu, &replaced)) {
      exp_add_change_from_inserted_with_phoneme(&replaced, cur - 1, &jong_b, cur, ids->type_b, changes, &count, max_count);
      codepoint_vec_free(&replaced);
    }
  }
  if (cur > 0 && cur <= str->count && exp_pcheck_vec(str, cur - 1, "l24") && exp_pcheck_vec(str, cur, "24") && exp_pcheck_vec(str, cur + 1, "r24")) {
    codepoint_vec_t replaced;
    memset(&replaced, 0, sizeof(replaced));
    if (exp_vec_replace(str, cur, jungseong_eo, &replaced)) {
      exp_add_change_from_inserted_with_phoneme(&replaced, cur - 1, &jong_b, cur, ids->type_b, changes, &count, max_count);
      codepoint_vec_free(&replaced);
    }
  }
  if (cur > 0 && cur <= str->count && exp_pcheck_vec(str, cur - 1, "l25") && exp_pcheck_vec(str, cur, "25") && exp_pcheck_vec(str, cur + 1, "r25")) {
    codepoint_vec_t replaced;
    memset(&replaced, 0, sizeof(replaced));
    if (exp_vec_replace(str, cur, jungseong_eo, &replaced)) {
      exp_add_change_from_inserted_with_phoneme(&replaced, cur - 1, &jong_b, cur, ids->type_b, changes, &count, max_count);
      codepoint_vec_free(&replaced);
    }
  }
  if (cur > 0 && cur + 1 < str->count && exp_pcheck_vec(str, cur - 1, "l26") && exp_pcheck_vec(str, cur, "26") && exp_pcheck_vec(str, cur + 1, "r26")) {
    exp_add_change_from_inserted_with_phoneme(str, cur + 1, &h_ieung_eu, cur + 2, ids->type_h, changes, &count, max_count);
  }
  if (cur > 0 && cur + 1 < str->count && exp_pcheck_vec(str, cur - 1, "l27") && exp_pcheck_vec(str, cur, "27") && exp_pcheck_vec(str, cur + 1, "r27")) {
    codepoint_vec_t replaced;
    memset(&replaced, 0, sizeof(replaced));
    if (exp_vec_replace(str, cur, str->items[cur] == jungseong_ae ? jungseong_a : jungseong_ya, &replaced)) {
      exp_add_change_from_inserted_with_phoneme(&replaced, cur + 1, &h_ieung_eo, cur + 2, ids->type_h, changes, &count, max_count);
      codepoint_vec_free(&replaced);
    }
    memset(&replaced, 0, sizeof(replaced));
    if (exp_vec_replace(str, cur, str->items[cur] == jungseong_ae ? jungseong_eo : jungseong_yeo, &replaced)) {
      exp_add_change_from_inserted_with_phoneme(&replaced, cur + 1, &h_ieung_eo, cur + 2, ids->type_h, changes, &count, max_count);
      codepoint_vec_free(&replaced);
    }
  }
  if (cur > 0 && cur + 1 < str->count && exp_pcheck_vec(str, cur - 1, "l28") && exp_pcheck_vec(str, cur, "28") && exp_pcheck_vec(str, cur + 1, "r28")) {
    codepoint_vec_t replaced;
    memset(&replaced, 0, sizeof(replaced));
    if (exp_vec_replace(str, cur, jungseong_eo, &replaced)) {
      exp_add_change_from_inserted_with_phoneme(&replaced, cur + 1, &h_ieung, cur + 2, ids->type_h, changes, &count, max_count);
      codepoint_vec_free(&replaced);
    }
  }
  if (cur > 0 && cur < str->count && exp_pcheck_vec(str, cur - 1, "l29") && exp_pcheck_vec(str, cur, "29") && exp_pcheck_vec(str, cur + 1, "r29")) {
    codepoint_vec_t replaced;
    codepoint_vec_t inserted;
    memset(&replaced, 0, sizeof(replaced));
    memset(&inserted, 0, sizeof(inserted));
    if (exp_vec_replace(str, cur, jungseong_eu, &replaced)) {
      if (cur + 1 < replaced.count && replaced.items[cur + 1] == jungseong_a) {
        replaced.items[cur + 1] = jungseong_eo;
      }
      {
        codepoint_vec_t ieung_vec;
        ieung_vec.items = &choseong_ieung;
        ieung_vec.count = 1;
        ieung_vec.capacity = 1;
        if (exp_vec_insert(&replaced, cur + 1, &ieung_vec, &inserted)) {
        unsigned int cho = exp_to_choseong(inserted.items[cur - 1]);
        if (cho != 0) {
          inserted.items[cur - 1] = cho;
          if (count < max_count && exp_vec_slice(&inserted, 0, cur + 1, &changes[count].front) && exp_vec_slice(&inserted, cur + 1, inserted.count, &changes[count].back)) {
            changes[count].front_tag_type = EXP_TAG_TYPE_YONGS;
            changes[count].back_tag_type = EXP_TAG_TYPE_EOMIES;
            changes[count].phoneme = ids->type_reu;
            count++;
          }
        }
        codepoint_vec_free(&inserted);
      }
      }
      codepoint_vec_free(&replaced);
    }
  }
  if (cur > 0 && cur <= str->count && exp_pcheck_vec(str, cur - 1, "l30") && exp_pcheck_vec(str, cur, "30") && exp_pcheck_vec(str, cur + 1, "r30") && cur >= 2 && str->items[cur - 2] == 0x1105) {
    exp_add_change_from_replaced_with_phoneme(str, cur, choseong_ieung, cur, ids->type_reo, changes, &count, max_count);
  }
done:
  codepoint_vec_free(&jong_s);
  codepoint_vec_free(&jong_b);
  codepoint_vec_free(&h_ieung_eu);
  codepoint_vec_free(&h_ieung_eo);
  codepoint_vec_free(&h_ieung);
  return count;
}

static size_t HANNANUM_UNUSED
exp_rule_np_generate(const codepoint_vec_t *str, exp_change_t *changes, size_t max_count)
{
  size_t count = 0;
  codepoint_vec_t back;
  unsigned int jong_n;
  unsigned int jong_l;
  size_t width;
  memset(changes, 0, max_count * sizeof(changes[0]));
  memset(&back, 0, sizeof(back));
  if (str == NULL || changes == NULL || !utf8_decode_one((const unsigned char *)"ᆫ", &jong_n, &width) || !utf8_decode_one((const unsigned char *)"ᆯ", &jong_l, &width)) {
    return 0;
  }
  if (exp_vec_starts_with_utf8(str, "내가")) {
    if (exp_vec_slice(str, 2, str->count, &back)) {
      exp_add_change_from_utf8_parts("나", &back, EXP_TAG_TYPE_NBNP, EXP_TAG_TYPE_JOSA, changes, &count, max_count);
      codepoint_vec_free(&back);
    }
  } else if (exp_vec_starts_with_utf8(str, "네가")) {
    if (exp_vec_slice(str, 2, str->count, &back)) {
      exp_add_change_from_utf8_parts("너", &back, EXP_TAG_TYPE_NBNP, EXP_TAG_TYPE_JOSA, changes, &count, max_count);
      codepoint_vec_free(&back);
    }
  } else if (exp_vec_starts_with_utf8(str, "제가")) {
    if (exp_vec_slice(str, 2, str->count, &back)) {
      exp_add_change_from_utf8_parts("저", &back, EXP_TAG_TYPE_NBNP, EXP_TAG_TYPE_JOSA, changes, &count, max_count);
      codepoint_vec_free(&back);
    }
  } else if (exp_vec_equals_utf8(str, "내")) {
    if (exp_vec_from_utf8("의", &back)) {
      exp_add_change_from_utf8_parts("나", &back, EXP_TAG_TYPE_NBNP, EXP_TAG_TYPE_JOSA, changes, &count, max_count);
      codepoint_vec_free(&back);
    }
  } else if (exp_vec_equals_utf8(str, "네")) {
    if (exp_vec_from_utf8("의", &back)) {
      exp_add_change_from_utf8_parts("너", &back, EXP_TAG_TYPE_NBNP, EXP_TAG_TYPE_JOSA, changes, &count, max_count);
      codepoint_vec_free(&back);
    }
  } else if (exp_vec_equals_utf8(str, "제")) {
    if (exp_vec_from_utf8("의", &back)) {
      exp_add_change_from_utf8_parts("저", &back, EXP_TAG_TYPE_NBNP, EXP_TAG_TYPE_JOSA, changes, &count, max_count);
      codepoint_vec_free(&back);
    }
  } else if (exp_vec_starts_with_utf8(str, "내게")) {
    codepoint_vec_t suffix;
    codepoint_vec_t e;
    memset(&suffix, 0, sizeof(suffix));
    memset(&e, 0, sizeof(e));
    if (exp_vec_slice(str, 2, str->count, &suffix) && exp_vec_from_utf8("에", &e) && exp_vec_insert(&suffix, 0, &e, &back)) {
      exp_add_change_from_utf8_parts("나", &back, EXP_TAG_TYPE_NBNP, EXP_TAG_TYPE_JOSA, changes, &count, max_count);
    }
    codepoint_vec_free(&suffix);
    codepoint_vec_free(&e);
    codepoint_vec_free(&back);
  } else if (exp_vec_starts_with_utf8(str, "네게")) {
    codepoint_vec_t suffix;
    codepoint_vec_t e;
    memset(&suffix, 0, sizeof(suffix));
    memset(&e, 0, sizeof(e));
    if (exp_vec_slice(str, 2, str->count, &suffix) && exp_vec_from_utf8("에", &e) && exp_vec_insert(&suffix, 0, &e, &back)) {
      exp_add_change_from_utf8_parts("너", &back, EXP_TAG_TYPE_NBNP, EXP_TAG_TYPE_JOSA, changes, &count, max_count);
    }
    codepoint_vec_free(&suffix);
    codepoint_vec_free(&e);
    codepoint_vec_free(&back);
  } else if (exp_vec_starts_with_utf8(str, "제게")) {
    codepoint_vec_t suffix;
    codepoint_vec_t e;
    memset(&suffix, 0, sizeof(suffix));
    memset(&e, 0, sizeof(e));
    if (exp_vec_slice(str, 2, str->count, &suffix) && exp_vec_from_utf8("에", &e) && exp_vec_insert(&suffix, 0, &e, &back)) {
      exp_add_change_from_utf8_parts("저", &back, EXP_TAG_TYPE_NBNP, EXP_TAG_TYPE_JOSA, changes, &count, max_count);
    }
    codepoint_vec_free(&suffix);
    codepoint_vec_free(&e);
    codepoint_vec_free(&back);
  } else if (exp_vec_starts_with_utf8(str, "나") && str->count == 3) {
    if (str->items[2] == jong_n && exp_vec_from_utf8("는", &back)) {
      exp_add_change_from_utf8_parts("나", &back, EXP_TAG_TYPE_NBNP, EXP_TAG_TYPE_JOSA, changes, &count, max_count);
    } else if (str->items[2] == jong_l && exp_vec_from_utf8("를", &back)) {
      exp_add_change_from_utf8_parts("나", &back, EXP_TAG_TYPE_NBNP, EXP_TAG_TYPE_JOSA, changes, &count, max_count);
    }
    codepoint_vec_free(&back);
  } else if (exp_vec_starts_with_utf8(str, "너") && str->count == 3) {
    if (str->items[2] == jong_n && exp_vec_from_utf8("는", &back)) {
      exp_add_change_from_utf8_parts("너", &back, EXP_TAG_TYPE_NBNP, EXP_TAG_TYPE_JOSA, changes, &count, max_count);
    } else if (str->items[2] == jong_l && exp_vec_from_utf8("를", &back)) {
      exp_add_change_from_utf8_parts("너", &back, EXP_TAG_TYPE_NBNP, EXP_TAG_TYPE_JOSA, changes, &count, max_count);
    }
    codepoint_vec_free(&back);
  } else if (exp_vec_starts_with_utf8(str, "누구") && str->count == 5) {
    if (str->items[4] == jong_n && exp_vec_from_utf8("는", &back)) {
      exp_add_change_from_utf8_parts("누구", &back, EXP_TAG_TYPE_NBNP, EXP_TAG_TYPE_JOSA, changes, &count, max_count);
    } else if (str->items[4] == jong_l && exp_vec_from_utf8("를", &back)) {
      exp_add_change_from_utf8_parts("누구", &back, EXP_TAG_TYPE_NBNP, EXP_TAG_TYPE_JOSA, changes, &count, max_count);
    }
    codepoint_vec_free(&back);
  } else if (exp_vec_equals_utf8(str, "무언가")) {
    if (exp_vec_from_utf8("인가", &back)) {
      exp_add_change_from_utf8_parts("무엇", &back, EXP_TAG_TYPE_NBNP, EXP_TAG_TYPE_JOSA, changes, &count, max_count);
      codepoint_vec_free(&back);
    }
  }
  return count;
}

static size_t HANNANUM_UNUSED
exp_prule_generate(const codepoint_vec_t *prev, const codepoint_vec_t *str, const exp_irregular_ids_t *ids, exp_change_t *changes, size_t max_count)
{
  size_t count = 0;
  exp_change_t local[16];
  size_t local_count;
  size_t i;
  size_t j;
  if (str == NULL || changes == NULL || ids == NULL) {
    return 0;
  }
  memset(changes, 0, max_count * sizeof(changes[0]));
  local_count = exp_rule_np_generate(str, local, 16);
  for (j = 0; j < local_count && count < max_count; j++) {
    changes[count++] = local[j];
  }
  for (; j < local_count; j++) {
    exp_change_free(&local[j]);
  }
  for (i = 0; i < str->count; i++) {
    local_count = exp_rule_rem_generate(str, i, local, 16);
    for (j = 0; j < local_count && count < max_count; j++) changes[count++] = local[j];
    for (; j < local_count; j++) exp_change_free(&local[j]);
    local_count = exp_rule_irr_word_generate_basic(str, i, ids, local, 16);
    for (j = 0; j < local_count && count < max_count; j++) changes[count++] = local[j];
    for (; j < local_count; j++) exp_change_free(&local[j]);
    local_count = exp_rule_irr_word2_generate(str, i, local, 16);
    for (j = 0; j < local_count && count < max_count; j++) changes[count++] = local[j];
    for (; j < local_count; j++) exp_change_free(&local[j]);
    local_count = exp_rule_shorten_generate(str, i, local, 16);
    for (j = 0; j < local_count && count < max_count; j++) changes[count++] = local[j];
    for (; j < local_count; j++) exp_change_free(&local[j]);
    local_count = exp_rule_eomi_u_generate(str, i, local, 16);
    for (j = 0; j < local_count && count < max_count; j++) changes[count++] = local[j];
    for (; j < local_count; j++) exp_change_free(&local[j]);
    local_count = exp_rule_johwa_generate(str, i, local, 16);
    for (j = 0; j < local_count && count < max_count; j++) changes[count++] = local[j];
    for (; j < local_count; j++) exp_change_free(&local[j]);
    local_count = exp_rule_i_generate(prev, str, i, local, 16);
    for (j = 0; j < local_count && count < max_count; j++) changes[count++] = local[j];
    for (; j < local_count; j++) exp_change_free(&local[j]);
    local_count = exp_rule_gut_generate(str, i, local, 16);
    for (j = 0; j < local_count && count < max_count; j++) changes[count++] = local[j];
    for (; j < local_count; j++) exp_change_free(&local[j]);
  }
  return count;
}

#ifdef HANNANUM_WITH_CHART_BRIDGE
static int HANNANUM_UNUSED
exp_apply_changes_to_chart(hannanum_t *h, morpheme_chart_t *chart, segment_position_t *sp, simti_t *simti, int from, const exp_change_t *changes, size_t count)
{
  size_t i;
  int added = 0;
  if (changes == NULL) {
    return 0;
  }
  for (i = 0; i < count; i++) {
    added += morpheme_chart_phoneme_change(h, chart, sp, simti, from, &changes[i].front, &changes[i].back, changes[i].front_tag_type, changes[i].back_tag_type, changes[i].phoneme);
  }
  return added;
}
#endif
