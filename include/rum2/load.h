/* $Id: load.h,v 1.1 2005/04/22 09:12:20 jirka Exp $ */
/*
 * Code for counting various loads.
 * Copyright (C) 2005 Jiri Denemark
 *
 * This file is part of RUM2.
 *
 * RUM2 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/** @file
 * Code for counting various loads.
 * @author Jiri Denemark
 * @date 2005
 * @version $Revision: 1.1 $ $Date: 2005/04/22 09:12:20 $
 */

#ifndef LOAD_H
#define LOAD_H

#if HAVE_STDINT_H
# include <stdint.h>
#endif
#if HAVE_PTHREAD_H
# include <pthread.h>
#endif
#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#include "limits.h"


/** History for counting loads. */
struct load {
    /** Data history for the last @c RUM_LOAD_MAX mins. */
    uint64_t data[RUM_LOAD_MAX];
    /** Index to a most recent minute in data array. */
    int current;
    /** Beginning of the last (i.e. current) minute. */
    time_t last;
    /** If nonzero, all operations are thread-safe. */
    int lock;
    /** Mutex for thread-safe operations. */
    pthread_mutex_t mutex;
};


/** Initialize load structure.
 *
 * @param load
 *      pointer to a load structure to be initialized.
 *
 * @param thrsafe
 *      make all operations on this structure thread-safe (i.e. use locking).
 *
 * @return
 *      zero on success, nonzero otherwise. This function will never fail if
 *      @c thrsafe is zero.
 */
extern int load_init(struct load *load, int thrsafe);


/** Free all resources allocated by load_init().
 *
 * @param load
 *      pointer to a load structure.
 *
 * @return
 *      nothing.
 */
extern void load_clean(struct load *load);


/** Update load structure by incerement.
 *
 * @param load
 *      pointer to a load structure to be updated.
 *
 * @param increment
 *      add this value to a current minute's history.
 *
 * @return
 *      nothing.
 */
extern void load_update_inc(struct load *load, uint64_t increment);


/** Update load structure by computing a maximum value.
 *
 * @param load
 *      pointer to a load structure to be updated.
 *
 * @param value
 *      set a current minute's history to be a maximum of its current value
 *      and a value of this parameter.
 *
 * @return
 *      nothing.
 */
extern void load_update_max(struct load *load, uint64_t value);


/** Compute load for a given minutes as an average of their values.
 * Sums values for the last @c minutes minutes and devides the result by
 * <tt>minutes * by</tt>. The current minute is counted proportionally to a
 * number of seconds since its beginning.
 *
 * @param load
 *      pointer to a load structure.
 *
 * @param minutes
 *      how many minutes has to be included in the computation.
 *
 * @param by
 *      number of units corresponding to one minute.
 *
 * @return
 *      load.
 */
extern long double load_get_avg(struct load *load, int minutes, int by);


/** Compute load for a given minutes as a maximum of their values.
 * Computes maximum value for the last @c minutes minutes.
 *
 * @param load
 *      pointer to a load structure.
 *
 * @param minutes
 *      how many minutes has to be included in the computation.
 *
 * @return
 *      load.
 */
extern uint64_t load_get_max(struct load *load, int minutes);


/** Compute load for a given minutes as a median of their values.
 * Computes a median of the last @minutes values.
 *
 * @param load
 *      pointer to a load structure.
 *
 * @param minutes
 *      how many minutes has to be included in the computation.
 *
 * @return
 *      load.
 */
extern uint64_t load_get_med(struct load *load, int minutes);

#endif

