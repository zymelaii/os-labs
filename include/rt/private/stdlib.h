#pragma once

#include <stddef.h>

/*!
 * \brief get current system ticks
 */
size_t get_ticks();

/*!
 * \brief delay for about specified number of ticks
 * \param [in] ticks how many ticks to delay for
 * \return difference between requested ticks and actual delayed ticks
 *
 * \note for instance, if the expected delay is 100 ticks, and actual delay is
 * 105 ticks, then return 105 - 100 = 5
 */
ssize_t delay_ticks(size_t ticks);

/*!
 * \brief get pid of current process
 */
int get_pid();
