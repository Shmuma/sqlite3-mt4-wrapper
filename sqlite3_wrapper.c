#include "sqlite3.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WINVER 0x0501
#define _WIN32_WINNT 0x0501
#include <windef.h>
#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>

#define APPDATA_PATHS 1

//static char buf[1024];

// how long wait when DB is busy
static int busy_timeout = 1000;

// pragma for journal mode
static char* journal_statement = NULL;


struct query_result {
    sqlite3 *s;
    sqlite3_stmt *stmt;
};



/* We assume that given file name is relative to MT installation directory. We
 * obtain this path prefix by query process's module name. */
static char* build_db_fname (const char* db)
{
    // if path is absolute, just return it, assuming it holds full db path
    if (!PathIsRelative (db))
        return strdup (db);
#if APPDATA_PATHS
    TCHAR buf[MAX_PATH];
    HRESULT res;

    res = SHGetFolderPathAndSubDir (0, CSIDL_APPDATA, NULL, 0, "MT-Sqlite", buf);
    if (res != S_OK) {
        SHGetFolderPath (0, CSIDL_APPDATA, NULL, 0, buf);
        strcat (buf, "/MT-Sqlite");
        CreateDirectory (buf, NULL);
    }
    
    strcat (buf, "/");
    strcat (buf, db);
    return strdup (buf);
#else
    unsigned int len, s;
    char* res;

    len = GetModuleFileNameA (NULL, buf, sizeof (buf));

    if (len <= 0)
        return NULL;

    while (len > 0 && buf[len-1] != '\\')
        len--;
    if (!len)
        return NULL;
    buf[len-1] = 0;

    s = len + 1 + strlen (db);
    res = malloc (s);
    if (!res)
        return NULL;

    snprintf (res, s, "%s\\%s", buf, db);
    return res;
#endif
}


static void tune_db_handler (sqlite3 *s)
{
    sqlite3_busy_timeout (s, busy_timeout);

    if (journal_statement)
        sqlite3_exec (s, journal_statement, NULL, NULL, NULL);

    RegisterExtensionFunctions (s);
}


const char* __stdcall sqlite_get_fname (const char* db)
{
    static char buf[MAX_PATH];
    char* p = build_db_fname (db);

    if (!p)
        return NULL;

    strcpy (buf, p);
    free (p);
    return buf;
}


int __stdcall sqlite_exec (const char *db, const char *sql)
{
    sqlite3 *s;
    int res;
    char* name = build_db_fname (db);

    if (!name)
        return -1;

    res = sqlite3_open (name, &s);
    free (name);
    if (res != SQLITE_OK)
        return res;

    tune_db_handler (s);

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
    char *name = build_db_fname (db);

    if (!name)
        return -1;

    res = sqlite3_open (name, &s);
    free (name);
    if (res != SQLITE_OK)
        return -res;

    tune_db_handler (s);

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
    char* name = build_db_fname (db);

    if (!name)
        return -1;

    res = sqlite3_open (name, &s);
    free (name);
    if (res != SQLITE_OK)
        return -res;

    tune_db_handler (s);

    res = sqlite3_prepare (s, sql, strlen (sql), &stmt, NULL);
    if (res != SQLITE_OK) {
        sqlite3_close (s);
        return -res;
    }

    result = (struct query_result*)malloc (sizeof (struct query_result));
    result->s = s;
    result->stmt = stmt;
    *cols = sqlite3_column_count (stmt);
    return (int)result;
}

int __stdcall sqlite_reset (int handle)
{
    struct query_result *res = (struct query_result*)handle;
    int ret;

    if (!res)
        return 0;

    ret = sqlite3_reset (res->stmt);

    return ret == SQLITE_OK ? 1 : 0;
}

int __stdcall sqlite_bind_int (int handle, int col, int bind_value)
{
    struct query_result *res = (struct query_result*)handle;
    int ret;

    if (!res)
        return 0;

    ret = sqlite3_bind_int (res->stmt, col, bind_value);

    return ret == SQLITE_OK ? 1 : 0;
}

int __stdcall sqlite_bind_double (int handle, int col, double bind_value)
{
    struct query_result *res = (struct query_result*)handle;
    int ret;

    if (!res)
        return 0;

    ret = sqlite3_bind_double (res->stmt, col, bind_value);

    return ret == SQLITE_OK ? 1 : 0;
}

int __stdcall sqlite_bind_text (int handle, int col, const char* bind_value)
{
    struct query_result *res = (struct query_result*)handle;
    int ret;

    if (!res)
        return 0;

    ret = sqlite3_bind_text (res->stmt, col, bind_value, -1, SQLITE_STATIC);

    return ret == SQLITE_OK ? 1 : 0;
}

int __stdcall sqlite_bind_null (int handle, int col)
{
    struct query_result *res = (struct query_result*)handle;
    int ret;

    if (!res)
        return 0;

    ret = sqlite3_bind_null (res->stmt, col);

    return ret == SQLITE_OK ? 1 : 0;
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

int __stdcall sqlite_get_col_int (int handle, int col)
{
    struct query_result *data = (struct query_result*)handle;

    if (!data)
        return 0;

    return sqlite3_column_int (data->stmt, col);
}

double __stdcall sqlite_get_col_double (int handle, int col)
{
    struct query_result *data = (struct query_result*)handle;

    if (!data)
        return 0;

    return sqlite3_column_double (data->stmt, col);
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


void __stdcall sqlite_set_busy_timeout (int ms)
{
    busy_timeout = ms;
}


void __stdcall sqlite_set_journal_mode (const char* mode)
{
    if (journal_statement) {
        free (journal_statement);
        journal_statement = NULL;
    }

    if (!mode)
        return;

    static const char* format = "PRAGMA journal_mode=%s;";
    int len = strlen (format) + strlen (mode) + 1;

    journal_statement = (char*)malloc (len);
    sprintf (journal_statement, format, mode);
}
