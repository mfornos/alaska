/*  =========================================================================
    Alaska - command-line interface
    Copyright (c) the Contributors as noted in the AUTHORS file.
    This file is part of the Alaska Project
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
    =========================================================================
*/
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "worker.h"

#define PRODUCT         "Alaska Workers/0.0.1 (POC)"
#define COPYRIGHT       "Copyright (c) 2016 the Contributors"
#define NOWARRANTY \
"This Software is provided under the MPLv2 License on an \"as is\" basis,\n" \
"without warranty of any kind, either expressed, implied, or statutory."

static int version_flag;

static void
usage ()
{
    puts ("Usage:\n"
          "\talaska [--version] [--help] [--foreground] [--config <file_name>]\n"
          "Options:\n"
          "\t-h, --help\n\t Prints this help\n"
          "\t-c <file_name>, --config <file_name>\n\t The location of the Alaska configuration file\n"
          "\t-f, --foreground\n\t Forces foreground execution\n"
          "Notes:\n"
          "\tThe default configuration file is 'alaska.cfg'");
    exit (EXIT_FAILURE);
}

static void
version ()
{
    puts (PRODUCT "\n" COPYRIGHT "\n" NOWARRANTY "\n");
}

int
main (int argc, char *argv[])
{
    char *filename = NULL;
    int force_foreground = 0;
    int option_index = 0;
    int c;

    static struct option long_options[] =
    {
        {"version", no_argument, &version_flag, 1},
        {"help", optional_argument, 0, 'h'},
        {"config", optional_argument, 0, 'c'},
        {"foreground", optional_argument, 0, 'f'}
    };

    while ((c = getopt_long (argc, argv, "hfc:",
                             long_options, &option_index)) != -1) {
        switch (c) {
        case 'h':
            usage ();
        case 'f':
            force_foreground = 1;
            break;
        case 'c':
            filename = optarg;
            break;
        }
    }

    version ();
    if (version_flag)
        exit (EXIT_SUCCESS);

    alaska_worker_start (filename, force_foreground);

    return EXIT_SUCCESS;
}
