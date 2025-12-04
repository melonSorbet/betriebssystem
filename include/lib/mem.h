#include <stdint.h>
#include <stddef.h>

int memcmp(const void *s1, const void *s2, size_t n);

void *memcpy(void *restrict s1, const void *restrict s2, size_t n);

void *memmove(void *s1, const void *s2, size_t n);
void *memset(void *s, int c, size_t n);
