void kprintf(const char *input, ...);

void send_string(const char *string);

void send_char(int asci_encoding);

void send_int(int integer);

void send_unsigned_int(unsigned int unsigned_integer);

void send_pointer_as_hex(void *pointer);

void send_percent_sign();

void send_unsinged_int_as_hex(unsigned int unsigned_integer);

static void print_padding(int width, char pad_char, int content_len);
