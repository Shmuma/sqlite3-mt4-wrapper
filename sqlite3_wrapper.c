#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct query_result {
    sqlite3 *s;
    sqlite3_stmt *stmt;
};



int __stdcall sqlite_exec (const char *db, const char *sql)
{
    sqlite3 *s;
    int res;

    res = sqlite3_open (db, &s);
    if (res != SQLITE_OK)
        return res;

    res = sqlite3_exec (s, sql, NULL, NULL, NULL);
    if (res != SQLITE_OK) {
        sqlite3_close (s);
        return res;
    }

    sqlite3_close (s);
    return SQLITE_OK;
}



/*
 * return 1 if table exists in database, 0 oterwise. -ERROR returned on error.
 */
int __stdcall sqlite_table_exists (const char *db, const char *table)
{
    sqlite3 *s;
    sqlite3_stmt *stmt;
    char buf[256];
    int res, exists;

    res = sqlite3_open (db, &s);
    if (res != SQLITE_OK)
        return -res;

    sprintf (buf, "select count(*) from sqlite_master where type='table' and name='%s'", table);
    res = sqlite3_prepare (s, buf, sizeof (buf), &stmt, NULL);
    if (res != SQLITE_OK) {
        sqlite3_close (s);
        return -res;
    }

    res = sqlite3_step (stmt);
    if (res != SQLITE_ROW) {
        sqlite3_finalize (stmt);
        sqlite3_close (s);
        return -res;
    }

    exists = sqlite3_column_int (stmt, 0);
    sqlite3_finalize (stmt);
    sqlite3_close (s);
    return exists > 0 ? 1 : 0;
}



/*
 * Perform query and pack results in internal structure. Routine returns amount of data fetched and
 * integer handle which can be used to sqlite_get_data. On error, return -SQLITE_ERROR.
 */
int __stdcall sqlite_query (const char *db, const char *sql, int* cols)
{
    sqlite3 *s;
    sqlite3_stmt *stmt;
    int res;
    struct query_result *result;

    res = sqlite3_open (db, &s);
    if (res != SQLITE_OK)
        return 0;
    res = sqlite3_prepare (s, sql, strlen (sql), &stmt, NULL);
    if (res != SQLITE_OK) {
        sqlite3_close (s);
        return 0;
    }

    result = (struct query_result*)malloc (sizeof (struct query_result));
    result->s = s;
    result->stmt = stmt;
    *cols = sqlite3_column_count (stmt);
    return (int)result;
}


/*
 * Return 1 if next row fetched, 0 if end of resultset reached
 */
int __stdcall sqlite_next_row (int handle)
{
    struct query_result *res = (struct query_result*)handle;
    int ret;

    if (!res)
        return 0;

    ret = sqlite3_step (res->stmt);

    return ret == SQLITE_ROW ? 1 : 0;
}



const char* __stdcall sqlite_get_col (int handle, int col)
{
    struct query_result *data = (struct query_result*)handle;

    if (!data)
        return NULL;

    return sqlite3_column_text (data->stmt, col);
}



int __stdcall sqlite_free_query (int handle)
{
    struct query_result *data = (struct query_result*)handle;

    if (!data)
        return 0;

    if (data->stmt)
        sqlite3_finalize (data->stmt);
    if (data->s)
        sqlite3_close (data->s);
    free (data);
    return 1;
}
