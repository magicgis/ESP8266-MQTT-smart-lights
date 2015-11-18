#ifndef INCLUDE_DRIVER_MNHS_GPIO_H_
#define INCLUDE_DRIVER_MNHS_GPIO_H_

#include "c_types.h"
void initialize_gpio(void);
bool io_toggle(uint8 pin, uint8 value);


#endif /* INCLUDE_DRIVER_MNHS_GPIO_H_ */


