#include "mongo.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define ASSERT(x) \
    do{ \
        if(!(x)){ \
            printf("failed assert (%d): %s\n", __LINE__,  #x); \
            return 1; \
        }\
    }while(0)

static mongo_connection conn;
static mongo_connection_options opts;
static const char* db = "test";
static const char* ns = "test.c.error";

void force_error(){
    bson cmd;
    bson_buffer bb;

    bson_buffer_init(&bb);
    bson_append_int(&bb, "forceerror", 1);
    bson_from_buffer(&cmd, &bb);

    mongo_run_command(&conn, db, &cmd, NULL);
    
    bson_destroy(&cmd);
}

int main(){
    bson obj;

    strncpy(opts.host, TEST_SERVER, 255);
    opts.host[254] = '\0';
    opts.port = 27017;

    if (mongo_connect( &conn , &opts )){
        printf("failed to connect\n");
        exit(1);
    }


    /*********************/
    ASSERT(!mongo_cmd_get_prev_error(&conn, db, NULL));
    ASSERT(!mongo_cmd_get_last_error(&conn, db, NULL));

    ASSERT(!mongo_cmd_get_prev_error(&conn, db, &obj));
    bson_destroy(&obj);

    ASSERT(!mongo_cmd_get_last_error(&conn, db, &obj));
    bson_destroy(&obj);

    /*********************/
    force_error();

    ASSERT(mongo_cmd_get_prev_error(&conn, db, NULL));
    ASSERT(mongo_cmd_get_last_error(&conn, db, NULL));

    ASSERT(mongo_cmd_get_prev_error(&conn, db, &obj));
    bson_destroy(&obj);

    ASSERT(mongo_cmd_get_last_error(&conn, db, &obj));
    bson_destroy(&obj);

    /* should clear lasterror but not preverror */
    mongo_find_one(&conn, ns, bson_empty(&obj), bson_empty(&obj), NULL);

    ASSERT(mongo_cmd_get_prev_error(&conn, db, NULL));
    ASSERT(!mongo_cmd_get_last_error(&conn, db, NULL));

    ASSERT(mongo_cmd_get_prev_error(&conn, db, &obj));
    bson_destroy(&obj);

    ASSERT(!mongo_cmd_get_last_error(&conn, db, &obj));
    bson_destroy(&obj);

    /*********************/
    mongo_cmd_reset_error(&conn, db);

    ASSERT(!mongo_cmd_get_prev_error(&conn, db, NULL));
    ASSERT(!mongo_cmd_get_last_error(&conn, db, NULL));

    ASSERT(!mongo_cmd_get_prev_error(&conn, db, &obj));
    bson_destroy(&obj);

    ASSERT(!mongo_cmd_get_last_error(&conn, db, &obj));
    bson_destroy(&obj);
}
