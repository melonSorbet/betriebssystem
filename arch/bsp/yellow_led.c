#include <arch/bsp/yellow_led.h>
#include <stddef.h>

static constexpr unsigned int GPIO_BASE		   = 0x7E200000 - 0x3F000000;
static constexpr unsigned int YELLOW_LED	   = 7u;
static constexpr unsigned int GPF_BITS		   = 3u;
static constexpr unsigned int GPF_MASK		   = 0b111u;
static constexpr unsigned int YELLOW_LED_GPF_SHIFT = YELLOW_LED * GPF_BITS;

enum gpio_func {
	gpio_input  = 0x0,
	gpio_output = 0x1,
};

struct gpio {
	unsigned int func[6];
	unsigned int unused0;
	unsigned int set[2];
	unsigned int unused1;
	unsigned int clr[2];
};
static_assert(offsetof(struct gpio, func) == 0x00);
static_assert(offsetof(struct gpio, set) == 0x1C);
static_assert(offsetof(struct gpio, clr) == 0x28);

static volatile struct gpio *const gpio_port = (struct gpio *)GPIO_BASE;

void init_yellow(void)
{
	gpio_port->func[0] = (gpio_port->func[0] & ~(GPF_MASK << YELLOW_LED_GPF_SHIFT)) |
			     (gpio_output << YELLOW_LED_GPF_SHIFT);
}

void yellow_on(void)
{
	gpio_port->set[0] = 1 << YELLOW_LED;
}

void yellow_off(void)
{
	gpio_port->clr[0] = 1 << YELLOW_LED;
}
