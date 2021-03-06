/*  =========================================================================
    Alaska - worker implementation
    Copyright (c) the Contributors as noted in the AUTHORS file.
    This file is part of the Alaska Project
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
    =========================================================================
*/
#include <stdlib.h>
#include <sys/utsname.h>
#include <malamute.h>

#include "thpool.h"

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#define DEFAULT_TIMEOUT          "1000"
#define DEFAULT_POOL_SIZE        "4"
#define DEFAULT_SCRIPTS_DIR      "services"
#define DEFAULT_ERR_MAILBOX      "aw.err"
#define SCRIPT_CONTEXT_NAME      "alaskaContext"

#define LUA_SENDTO_ERR \
"Lua sendTo function needs at least 2 arguments:\n"\
"sendTo(subject, content, [mailbox], [attachment], [sender])"

#define R(k,v) zconfig_resolve (config, k, v)
#define P(k,v) zconfig_put (config, k, v)
#define Rint(k,v) atoi (R(k,v))
#define LUAP(an, a, b) \
    if (n > an) a = lua_tostring (L, an+1);\
    else a = b

static zconfig_t *config;
static const char *scripts_dir;
static int report_errors;

/* Worker message */
typedef struct wk_msg_t {
    const char *sender;
    const char *subject;
    const char *content;
    /* used for mailbox and stream name */
    const char *topic;
    const char *attachment;
} wk_msg;

static void
wk_sendx (wk_msg *m);

static void
wk_sendtox (wk_msg *m);

static void
wk_sendtoerr (wk_msg *m);

typedef void wk_send(wk_msg *m);

static int
lua_send (lua_State *L,  wk_send send)
{
    int n = lua_gettop (L);
    if (n < 2) {
        lua_pushstring(L, LUA_SENDTO_ERR);
        lua_error(L);
        return 0;
    }
    
    wk_msg m;
    m.subject = lua_tostring (L, 1);
    m.content = lua_tostring (L, 2);
    LUAP (2, m.topic, "aw.res");
    LUAP (3, m.attachment, NULL);
    LUAP (4, m.sender, "LU");

    send (&m);

    /* return true */
    lua_pushnumber (L, 0);

    /* number of return values*/
    return 1;
}

static int 
lua_sendtox (lua_State *L)
{
    return lua_send (L, wk_sendtox);
}

static int 
lua_sendx (lua_State *L)
{
    return lua_send (L, wk_sendx);
}

static void
th_lua_handler (char *argv[])
{
    const char *subject = argv[0];
    const char *context = argv[1];
    char filename[6 + strlen (scripts_dir) + strlen (subject)];
    sprintf (filename, "%s/%s%s", scripts_dir, subject, ".lua");

    lua_State *L = luaL_newstate ();
    luaL_openlibs (L);

    /* Set global context var */
    lua_pushstring (L, context);
    lua_setglobal (L, SCRIPT_CONTEXT_NAME);
    
    /* Register functions */
    lua_register (L, "send", lua_sendx);
    lua_register (L, "sendTo", lua_sendtox);

    /* Execute script */
    if (luaL_dofile (L, filename)) {
        const char *error_msg = lua_tostring (L, -1);
        zsys_error (error_msg);
        wk_msg m;
        m.subject = subject;
        m.content = error_msg;
        m.attachment = context;
        wk_sendtoerr (&m);
    }

    lua_close (L);
}

static void
wk_name (char **name, const char *prefix)
{
    struct utsname udata;
    uname (&udata);
    char identity[14 + strlen (udata.nodename) + strlen (prefix)];
    sprintf (identity, "%s::%s::%i-%04X", prefix, udata.nodename,
             getpid (), randof (0x10000));
    *name = (char *)malloc (strlen (identity) * sizeof (char));
    strcpy (*name, identity);
}

static void
wk_connect (mlm_client_t *c, const char *prefix)
{
    char *wname;
    wk_name (&wname, prefix);

    const char *username = R ("worker/auth/plain/username", NULL);
    const char *password = R ("worker/auth/plain/password", NULL);
    if (username && password && !mlm_client_set_plain_auth (c, username, password)) {
        zsys_error ("Error authenticating '%s'", username);
        mlm_client_destroy (&c);
        exit (EXIT_FAILURE);
    }
    const char *endpoint = R ("worker/broker/endpoint", NULL);
    int timeout = Rint ("worker/broker/timeout", DEFAULT_TIMEOUT);
    if (mlm_client_connect (c, endpoint, timeout, wname)) {
        zsys_error ("Could not connect to Malamute server [%s]", endpoint);
        mlm_client_destroy (&c);
        exit (EXIT_FAILURE);
    } else {
        zsys_info ("[CONN]( %s )", wname);
    }

    free (wname);
}

static void
wk_sendtox (wk_msg *m)
{
    mlm_client_t *writer = mlm_client_new ();
    wk_connect (writer, m->sender);
    zsys_info ("[MBOX]( subject: '%s',  mailbox: '%s' )\n>>\n%s\n-",
        m->subject, m->topic, m->content);
    mlm_client_sendtox (writer, m->topic, m->subject, 
        m->content, m->attachment, NULL);
    mlm_client_destroy (&writer);
}

static void
wk_sendx (wk_msg *m)
{
    mlm_client_t *writer = mlm_client_new ();
    wk_connect (writer, m->sender);
    mlm_client_set_producer (writer, m->topic);
    zsys_info ("[STRM]( subject: '%s', topic: '%s' )\n>>\n%s\n-",
        m->subject, m->topic, m->content);
    mlm_client_sendx (writer, m->subject, m->content, NULL);
    mlm_client_destroy (&writer);
}

static void
wk_sendtoerr (wk_msg *m)
{
    if (!report_errors)
        return;

    m->topic = R ("worker/errors/mailbox", DEFAULT_ERR_MAILBOX);
    m->sender = "EP";

    wk_sendtox (m);
}

static void
wk_free_buffers (int siz, char **a, char **b)
{
    int i;
    for (i = 0; i < siz; i++) {
        zstr_free (&a[i]);
        zstr_free (&b[i]);
    }
}

static void
wk_init_config (const char *filename)
{
    zsys_info ("Loading configuration from '%s'", filename);

    config = zconfig_load (filename);
    if (!config) {
        zsys_info ("'%s' is missing, creating with defaults:", filename);
        config = zconfig_new ("root", NULL);
        P ("worker/background", "0");
        P ("worker/workdir", ".");
        P ("worker/verbose", "0");
        P ("worker/syslog", "0");
        P ("worker/scripts/dir", DEFAULT_SCRIPTS_DIR);
        P ("worker/pool/size", DEFAULT_POOL_SIZE);
        P ("worker/broker/endpoint", MLM_DEFAULT_ENDPOINT);
        P ("worker/broker/timeout", DEFAULT_TIMEOUT);
        P ("worker/service/address", "default");
        P ("worker/service/filter", ".*");
        P ("worker/errors/notify", "1");
        P ("worker/errors/mailbox", DEFAULT_ERR_MAILBOX);
        zconfig_print (config);
        zconfig_save (config, filename);
    }
    zsys_set_logsystem (R ("worker/syslog", "0"));
    scripts_dir = R ("worker/scripts/dir", DEFAULT_SCRIPTS_DIR);
    report_errors = Rint ("worker/errors/notify", "1");
}

static void
wk_start (void)
{
    mlm_client_t *client = mlm_client_new ();
    wk_connect (client, "AW");

    mlm_client_set_worker (client, R ("worker/service/address", NULL), R ("worker/service/filter", NULL));

    int pool_size = Rint ("worker/pool/size", DEFAULT_POOL_SIZE);
    threadpool thpool = thpool_init (pool_size);

    zsys_info ("Alaska worker ready (pool:%i)", pool_size);

    char *subject[pool_size], *content[pool_size];
    int wip = 0;

    /* Event loop */
    while (!zsys_interrupted) {
        if (wip == pool_size) {
            thpool_wait (thpool);
            wk_free_buffers (pool_size, subject, content);
            wip = 0;
        }
        mlm_client_recvx (client, &subject[wip], &content[wip], NULL);
        const char *command = mlm_client_command (client);

        if (command) {
            zsys_info ("[SREQ]( address: %s, subject: %s, sender: %s, command: %s )\n<<\n%s\n-",
                       mlm_client_address (client), subject[wip], mlm_client_sender (client),
                       command, content[wip]);
            thpool_add_work (thpool, (void *)th_lua_handler, (char *[]){
                subject[wip], content[wip]
            });
            wip++;
        }
    }

    puts ("interrupted");

    thpool_wait (thpool);
    wk_free_buffers (wip, subject, content);

    mlm_client_destroy (&client);
    thpool_destroy (thpool);
}

static void
wk_set_runmode (int force_foreground)
{
    /* Do we want to run worker in the background? */
    if (force_foreground)
        puts ("Forcing foreground execution...");
    int as_daemon = !force_foreground && Rint ("worker/background", "0");
    const char *workdir = R ("worker/workdir", ".");
    if (as_daemon) {
        zsys_info ("Switching worker to background...");
        if (zsys_daemonize (workdir))
            exit (EXIT_FAILURE);
    }
    /* Switch to user/group to run process under, if any */
    if (zsys_run_as (R ("worker/lockfile", NULL),
                     R ("worker/group", NULL),
                     R ("worker/user", NULL)))
        exit (EXIT_FAILURE);
}

void
alaska_worker_start (const char *filename, int force_foreground)
{
    /* Init seed */
    struct timeval t;
    gettimeofday (&t, NULL);
    srandom (t.tv_usec * t.tv_sec);

    /* Initialise the worker */
    wk_init_config (filename ? filename : "alaska.cfg");
    wk_set_runmode (force_foreground);

    /* Kick-off the worker */
    wk_start ();

    /* Free resources */
    zconfig_destroy (&config);
}
