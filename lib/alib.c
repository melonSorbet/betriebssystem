// converter functions
#include <stdint.h>
int *int_to_binary(int number, int *buffer, int length)
{
	if (length <= 0 || !buffer)
		return buffer;

	int index = 0;

	if (number == 0) {
		buffer[0] = 0;
		return buffer;
	}

	while (number > 0 && index < length) {
		buffer[index++] = number % 2;
		number /= 2;
	}
	return buffer;
}
char *int_to_base_above_ten(unsigned int number, char *buffer, int length)
{
	if (length < 2)
		return buffer; // Need space for at least one digit + '\0'

	if (number == 0) {
		buffer[0] = '0';
		buffer[1] = '\0';
		return buffer;
	}

	int index = 0;
	while (number > 0 && index < length - 1) {
		int temp = number % 16;
		if (temp < 10)
			buffer[index] = '0' + temp;
		else
			buffer[index] = 'a' + (temp - 10);
		number /= 16;
		index++;
	}

	// Null-terminate
	buffer[index] = '\0';

	// Reverse the string in-place because digits are currently backwards
	for (int i = 0; i < index / 2; i++) {
		char tmp	      = buffer[i];
		buffer[i]	      = buffer[index - 1 - i];
		buffer[index - 1 - i] = tmp;
	}

	return buffer;
}

char *pointer_to_array(void *pointer, char *buffer, int buffer_length)
{
	if (!buffer || buffer_length < 11)
		return buffer;
	int	  i	  = buffer_length - 2;
	uintptr_t address = (uintptr_t)pointer;
	buffer[0]	  = '0';
	buffer[1]	  = 'x';

	for (int count = 0; count < 8 && i >= 2; count++) {
		int right_most_digit = address & 0xF; //get the last 4 bits
		if (right_most_digit < 10) {
			buffer[i--] = '0' + right_most_digit;
		} else {
			buffer[i--] = 'a' + (right_most_digit - 10);
		}
		address >>= 4;
	}
	buffer[buffer_length - 1] = '\0';
	return buffer;
}

char *reverse_char_array(char *array, char *buffer, int length)
{
	if (!array || !buffer || length < 0)
		return buffer;

	for (int i = 0; i < length; i++) {
		buffer[i] = array[length - 1 - i];
	}
	buffer[length] = '\0';
	return buffer;
}
