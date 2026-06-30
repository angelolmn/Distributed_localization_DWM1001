/**
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef CONSENSUS_H
#define CONSENSUS_H

#include "sd_protocol.h"
#include "uwb_comm.h"

extern int32_t clock_time;
extern int32_t control;
extern int32_t neighbordiff_curr_time;
extern int32_t neighbordiff_last_time;
extern int32_t cont;


extern THD_FUNCTION(CONSENSUS, arg);

#endif /* CONSENSUS_H */