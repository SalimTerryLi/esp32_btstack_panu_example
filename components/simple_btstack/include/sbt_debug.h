#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_err.h"
#include <stdbool.h>
#include "bluetooth.h"

/*
 * This file serves internal event logging and debugging
 */

/*
 * TestOnly
 */
esp_err_t sbt_debugging_init();

#ifdef __cplusplus
}
#endif
