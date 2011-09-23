#pragma once

// Fuang.Cao <cavan.cfa@gmail.com> Mon Jul  4 10:32:22 CST 2011

char *get_bracket_pair(const char *formula, const char *formula_end);
int check_bracket_match_pair(const char *formula, const char *formula_end);
int __simple_calculator(const char *formula, const char *formula_end, double *result_last);
int simple_calculator(const char *formula, double *result_last);
int text2double(const char *text, const char *text_end, double *result_last);

