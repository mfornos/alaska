/*  =========================================================================
    Alaska - worker public interface
    Copyright (c) the Contributors as noted in the AUTHORS file.
    This file is part of the Alaska Project
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
    =========================================================================
*/
#ifndef _ALKWK_
#define _ALKWK_

/**
 * @brief  Starts a worker process
 * 
 * Starts a worker process. 
 * This function blocks until the process is interrupted.
 * 
 * @param filename  The configuration file. 
 *                  If NULL is passed the worker will create a default configuration.
 * @param force_foreground  Forces the process to run in foreground.
 */
void alaska_worker_start (const char *filename, int force_foreground);

#endif