#include <string.h>

int memcmp(const void *s1, const void *s2, size_t n)
{
	const unsigned char *str1 = s1;
	const unsigned char *str2 = s2;
	int		     res  = 0;
	while (n > 0 && res == 0) {
		res = *str1 - *str2;
		str1++;
		str2++;
		n--;
	}
	return res;
}

void *memcpy(void *restrict s1, const void *restrict s2, size_t n)
{
	char *restrict str1	  = s1;
	const char *restrict str2 = s2;
	while (n > 0) {
		*str1 = *str2;
		str1++;
		str2++;
		n--;
	}
	return s1;
}

void *memmove(void *s1, const void *s2, size_t n)
{
	char	   *str1 = s1;
	const char *str2 = s2;
	if (str1 < str2) {
		while (n > 0) {
			*str1 = *str2;
			str1++;
			str2++;
			n--;
		}
	} else {
		while (n > 0) {
			n--;
			*(str1 + n) = *(str2 + n);
		}
	}
	return s1;
}

void *memset(void *s, int c, size_t n)
{
	unsigned char *str = s;
	while (n > 0) {
		*str = (unsigned char)c;
		str++;
		n--;
	}
	return s;
}
