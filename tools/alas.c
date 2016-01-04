/*  =========================================================================
    Alas - command-line service request producer
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

#include <malamute.h>

#define MIN_ARGS_REQUIRED  0
#define DEFAULT_ENDPOINT   "tcp://127.0.0.1:9999"
#define DEFAULT_ADDRESS    "default"
#define DEFAULT_SUBJECT    "dump"
#define PRODUCER_NAME      "alas_producer"

#define D(a,b) a = a ? a : b

/* TODO use a proper structure, e.g. zeromq configuration */
static void
sevice_rq_send (const char *ep, const char *address, const char *subject, const char *content)
{
    mlm_client_t *client = mlm_client_new ();
    if (mlm_client_connect (client, ep, 1000, PRODUCER_NAME)) {
        zsys_error ("Could not connect to Malamute server");
        exit (EXIT_FAILURE);
    }
    mlm_client_sendforx (client, address, subject, content, NULL);
    mlm_client_destroy (&client);
}


static void
usage ()
{
    puts ("Usage:\n"
          "\talas [--help] [--endpoint <broker_endpoint>] [--address <service_name>] [--subject <subject>] < file_name\n"
          "Example:\n"
          "\t$ echo '{\"n\":1}' | alas\n"
          "\t$ for run in {1..10}; do echo \"{\\\"c\\\":${run}}\" | alas; done\n"
          "Options:\n"
          "\t-h, --help\n\t Prints this help\n"
          "\t-e <broker_endpoint>, --endpoint <broker_endpoint>\n\t Malamute endpoint. Defaults to '"
          DEFAULT_ENDPOINT "'\n"
          "\t-a <service_name>, --address <service_name>\n\t The name of the requested service. Defaults to '"
          DEFAULT_ADDRESS "'\n"
          "\t-s <subject>, --endpoint <subject>\n\t The service request subject. Defaults to '"
          DEFAULT_SUBJECT "'\n");
    exit (EXIT_FAILURE);
}

int
main (int argc, char *argv[])
{

    char *endpoint = NULL;
    char *subject = NULL;
    char *address = NULL;
    int option_index = 0;
    int c;

    static struct option long_options[] =
    {
        {"help", optional_argument, 0, 'h'},
        {"endpoint", optional_argument, 0, 'e'},
        {"address", optional_argument, 0, 'a'},
        {"subject", optional_argument, 0, 's'},
    };

    if (argc < MIN_ARGS_REQUIRED)
        usage ();

    while ((c = getopt_long (argc, argv, "he:a:s:",
            long_options, &option_index)) != -1) {
        switch (c) {
        case 'h':
            usage ();
        case 'e':
            endpoint = optarg;
            break;
        case 'a':
            address = optarg;
            break;
        case 's':
            subject = optarg;
            break;
        }
    }
    char *content = NULL;
    size_t len = 0;
    ssize_t read = getdelim (&content, &len, EOF, stdin);

    D (endpoint, DEFAULT_ENDPOINT);
    D (address, DEFAULT_ADDRESS);
    D (subject, DEFAULT_SUBJECT);
    printf ("Service RQ\n-Endpoint: %s\n-Address: %s\n-Subject: %s\n-Content:\n%s\n",
            endpoint, address, subject, content);

    sevice_rq_send (endpoint, address, subject, content);

    printf ("\n--\nSent!\n");

    free (content);
    return EXIT_SUCCESS;
}
