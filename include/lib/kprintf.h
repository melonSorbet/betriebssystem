void kprintf(const char *input, ...);

void print_padding(int width, char pad_char, int content_len);

int int_to_str(int value, char *buffer);

int uint_to_str(unsigned int value, char *buffer);

int uint_to_hex_str(unsigned int value, char *buffer);

void send_int_width(int value, int width, char pad_char);

void send_unsigned_width(unsigned int value, int width, char pad_char, bool hex);

void send_pointer(void *ptr);
