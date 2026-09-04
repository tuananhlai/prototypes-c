#ifndef DYNAMIC_STRING_H
#define DYNAMIC_STRING_H

#include <stddef.h>

typedef struct {
  char* data;
  size_t len;
  size_t cap;
} String;

String s_new(void);
String s_init(const char* val, size_t len);
void s_set(String* s, const char* val, size_t len);
void s_copy(String* dest, String* src);
void s_appendc(String* s, char ch);
void s_append(String* s, const char* val);
void s_trim(String* s);
void s_clear(String* s);
void s_substr(String* s, size_t start, size_t end, String* sub_str);
void s_trim_suffix(String* s, const char* suffix);
void s_destroy(String* s);

#endif  // DYNAMIC_STRING_H
