#include "number_dic.h"

static const unsigned char number_dic_automata[13][7] = {
  { 0, 0, 0, 0, 0, 0, 0 },
  { 0, 9, 9, 0, 0, 2, 0 },
  { 1, 0, 0, 11, 5, 3, 0 },
  { 1, 0, 0, 11, 5, 4, 0 },
  { 1, 0, 0, 11, 5, 10, 0 },
  { 0, 0, 0, 0, 0, 6, 0 },
  { 0, 0, 0, 0, 0, 7, 0 },
  { 0, 0, 0, 0, 0, 8, 0 },
  { 1, 0, 0, 0, 5, 0, 0 },
  { 0, 0, 0, 0, 0, 10, 0 },
  { 1, 0, 0, 11, 0, 10, 0 },
  { 1, 0, 0, 0, 0, 12, 0 },
  { 1, 0, 0, 0, 0, 12, 0 }
};

static int HANNANUM_UNUSED
number_dic_node_look(unsigned int c, int state)
{
  int input;
  if (state < 0 || state >= 13) {
    return 0;
  }
  switch (c) {
  case '+':
    input = 1;
    break;
  case '-':
    input = 2;
    break;
  case '.':
    input = 3;
    break;
  case ',':
    input = 4;
    break;
  default:
    input = isdigit((unsigned char)c) ? 5 : 6;
    break;
  }
  return number_dic_automata[state][input];
}

static int HANNANUM_UNUSED
number_dic_is_num(int state)
{
  if (state < 0 || state >= 13) {
    return 0;
  }
  return number_dic_automata[state][0] == 1;
}
