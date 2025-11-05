#include <stdint.h>
#include <stddef.h>
#include <arch/bsp/yellow_led.h>
#include <config.h>

#define halt()                      \
	do {                        \
		asm("cpsid if");    \
		while (true) {      \
			asm("wfi"); \
		}                   \
	} while (0)

void kprintf [[gnu::format(printf, 1, 2)]] (const char *format, ...);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsuggest-attribute=noreturn"
void kprintf [[gnu::weak, gnu::format(printf, 1, 2)]] (const char *, ...)
{
	init_yellow();
	while (true) {
		for (volatile unsigned int i = 0; i < BUSY_WAIT_COUNTER; i++) {
		}
		yellow_on();
		for (volatile unsigned int i = 0; i < BUSY_WAIT_COUNTER; i++) {
		}
		yellow_off();
	}
}
#pragma GCC diagnostic pop

// structs copyed form the linux kernel
// https://github.com/torvalds/linux/blob/master/lib/ubsan.h

struct source_location {
	const char    *file_name;
	const uint32_t line;
	const uint32_t column;
};

struct type_descriptor {
	const uint16_t type_kind;
	const uint16_t type_info;
	const char     type_name[1];
	const char     unused[3];
};

struct type_mismatch_data_v1 {
	const struct source_location  location;
	const struct type_descriptor *type;
	const unsigned char	      log_alignment;
	const unsigned char	      type_check_kind;
	const char		      unused[2];
};

struct out_of_bounds_data {
	struct source_location	loc;
	struct type_descriptor *array_type;
	struct type_descriptor *index_type;
};

static_assert(sizeof(unsigned int) == sizeof(long));

static void print_source_location(const struct source_location *location)
{
	kprintf("%s:%u:%u", location->file_name, (unsigned int)location->line,
		(unsigned int)location->column);
}

#define DEF_UBSAN_HANDLER(name)                                                           \
	void __ubsan_handle_##name [[noreturn]] (const struct source_location *location); \
	void __ubsan_handle_##name [[noreturn]] (const struct source_location *location)  \
	{                                                                                 \
		kprintf("Undefined behavior detected: " #name " at ");                    \
		print_source_location(location);                                          \
		kprintf("\n");                                                            \
		halt();                                                                   \
	}

DEF_UBSAN_HANDLER(add_overflow)
DEF_UBSAN_HANDLER(add_overflow_abort)
DEF_UBSAN_HANDLER(alignment_assumption)
DEF_UBSAN_HANDLER(builtin_unreachable)
DEF_UBSAN_HANDLER(alignment_assumption_abort)
DEF_UBSAN_HANDLER(cfi_check_fail)
DEF_UBSAN_HANDLER(cfi_bad_type)
DEF_UBSAN_HANDLER(divrem_overflow)
DEF_UBSAN_HANDLER(cfi_check_fail_abort)
DEF_UBSAN_HANDLER(dynamic_type_cache_miss)
DEF_UBSAN_HANDLER(divrem_overflow_abort)
DEF_UBSAN_HANDLER(float_cast_overflow)
DEF_UBSAN_HANDLER(dynamic_type_cache_miss_abort)
DEF_UBSAN_HANDLER(function_type_mismatch_v1)
DEF_UBSAN_HANDLER(float_cast_overflow_abort)
DEF_UBSAN_HANDLER(implicit_conversion)
DEF_UBSAN_HANDLER(function_type_mismatch_v1_abort)
DEF_UBSAN_HANDLER(invalid_builtin)
DEF_UBSAN_HANDLER(implicit_conversion_abort)
DEF_UBSAN_HANDLER(invalid_objc_cast)
DEF_UBSAN_HANDLER(invalid_builtin_abort)
DEF_UBSAN_HANDLER(load_invalid_value)
DEF_UBSAN_HANDLER(invalid_objc_cast_abort)
DEF_UBSAN_HANDLER(missing_return)
DEF_UBSAN_HANDLER(load_invalid_value_abort)
DEF_UBSAN_HANDLER(mul_overflow_abort)
DEF_UBSAN_HANDLER(mul_overflow)
DEF_UBSAN_HANDLER(negate_overflow_abort)
DEF_UBSAN_HANDLER(negate_overflow)
DEF_UBSAN_HANDLER(nonnull_arg_abort)
DEF_UBSAN_HANDLER(nonnull_arg)
DEF_UBSAN_HANDLER(nonnull_return_v1_abort)
DEF_UBSAN_HANDLER(nonnull_return_v1)
DEF_UBSAN_HANDLER(nullability_arg_abort)
DEF_UBSAN_HANDLER(nullability_arg)
DEF_UBSAN_HANDLER(nullability_return_v1_abort)
DEF_UBSAN_HANDLER(nullability_return_v1)
DEF_UBSAN_HANDLER(out_of_bounds_abort)
DEF_UBSAN_HANDLER(pointer_overflow_abort)
DEF_UBSAN_HANDLER(pointer_overflow)
DEF_UBSAN_HANDLER(shift_out_of_bounds_abort)
DEF_UBSAN_HANDLER(shift_out_of_bounds)
DEF_UBSAN_HANDLER(sub_overflow_abort)
DEF_UBSAN_HANDLER(sub_overflow)
DEF_UBSAN_HANDLER(type_mismatch_v1_abort)
DEF_UBSAN_HANDLER(vla_bound_not_positive_abort)
DEF_UBSAN_HANDLER(vla_bound_not_positive)

void __ubsan_handle_out_of_bounds
	[[noreturn]] (struct out_of_bounds_data *data, unsigned long index);
void __ubsan_handle_out_of_bounds
	[[noreturn]] (struct out_of_bounds_data *data, unsigned long index)
{
	kprintf("Undefined behavior detected: out of bounds access index %u at ",
		(unsigned int)index);
	print_source_location(&data->loc);
	kprintf("\n");
	halt();
}

void __ubsan_handle_type_mismatch_v1 [[noreturn]] (struct type_mismatch_data_v1 *data_v1, int ptr);
void __ubsan_handle_type_mismatch_v1 [[noreturn]] (struct type_mismatch_data_v1 *data_v1, int ptr)
{
	if (ptr == (uint32_t)NULL) {
		kprintf("Undefined behavior detected: null pointer access at ");
	} else if (data_v1->log_alignment && (ptr & (data_v1->log_alignment - 1))) {
		kprintf("Undefined behavior detected: unaligned access at ");
	} else {
		kprintf("Undefined behavior detected: type mismatch at ");
	}

	print_source_location(&data_v1->location);
	kprintf("\n");
	halt();
}
