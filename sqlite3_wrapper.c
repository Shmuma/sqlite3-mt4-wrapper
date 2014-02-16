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

// Error code
#define NO_ERROR 0
#define ERROR_INVALID_TERM_DATA_DIR 0x01

// MetaTrader4 TERMINAL_DATA_PATH
static wchar_t *terminal_data_path = NULL;

// how long wait when DB is busy
static int busy_timeout = 1000;

// pragma for journal mode
static char* journal_statement = NULL;


struct query_result {
    sqlite3 *s;
    sqlite3_stmt *stmt;
};

static BOOL directory_exists (wchar_t *path)
{
    DWORD attr = GetFileAttributesW(path);
    return (attr != INVALID_FILE_ATTRIBUTES &&
            (attr & FILE_ATTRIBUTE_DIRECTORY));
}

static const char *unicode_to_ansi_string (const wchar_t *unicode)
{
    const int ansi_bytes = WideCharToMultiByte(
        CP_ACP,
        WC_COMPOSITECHECK | WC_DISCARDNS | WC_SEPCHARS | WC_DEFAULTCHAR,
        unicode, -1, NULL, 0, NULL, NULL);

    if (ansi_bytes == 0) {
        return NULL;
    }

    const char *ansi_buf = HeapAlloc (GetProcessHeap(), 0, ansi_bytes);
    const int converted_bytes = WideCharToMultiByte(
        CP_ACP,
        WC_COMPOSITECHECK | WC_DISCARDNS | WC_SEPCHARS | WC_DEFAULTCHAR,
        unicode, -1, ansi_buf, ansi_bytes, NULL, NULL);

    if (converted_bytes == 0) {
        HeapFree (GetProcessHeap(), 0, ansi_buf);
        return NULL;
    }

    return ansi_buf;
}

static const wchar_t *ansi_to_unicode_string (const char *ansi)
{
    const int unicode_bytes = MultiByteToWideChar(
        CP_ACP,
        MB_COMPOSITE,
        ansi, -1, NULL, 0);

    if (unicode_bytes == 0) {
        return NULL;
    }

    const wchar_t *unicode_buf = HeapAlloc (GetProcessHeap(), 0, unicode_bytes);
    const int converted_bytes = MultiByteToWideChar(
        CP_ACP,
        MB_COMPOSITE,
        ansi, -1, unicode_buf, unicode_bytes);

    if (converted_bytes == 0) {
        HeapFree (GetProcessHeap(), 0, unicode_buf);
        return NULL;
    }

    return unicode_buf;
}

static int my_wcscat (wchar_t **dst, const wchar_t *src)
{
    int dst_buf_size = 0;

    if (*dst == NULL) {
        dst_buf_size = wcslen (src) + 1;
        *dst = HeapAlloc (GetProcessHeap (), 0, sizeof (wchar_t) * dst_buf_size);
        *dst[0] = L'\0';
    }
    else {
        dst_buf_size = wcslen (*dst) + wcslen (src) + 1;
        *dst = HeapReAlloc (GetProcessHeap (), 0, *dst, sizeof (wchar_t) * dst_buf_size);
    }

    return wcsncat (*dst, src, wcslen (src));
}

/* We assume that given file name is relative to MT Terminal Data Path. */
static wchar_t *build_db_path (const wchar_t *db_filename)
{
    wchar_t *buf[] = { NULL };
    HRESULT res;

    // if path is absolute, just return it, assuming it holds full db path
    if (!PathIsRelativeW (db_filename)) {
        if (my_wcscat (buf, db_filename) != 0) {
            return *buf;
        }
        else {
            return NULL;
        }
    }

    if (my_wcscat (buf, terminal_data_path) == 0) {
        return NULL;
    }

    if (my_wcscat (buf, L"\\MQL4\\Files\\SQLite") == 0) {
        return NULL;
    }

    if (!directory_exists (*buf)) {
        CreateDirectoryW (*buf, NULL);
    }

    if (my_wcscat (buf, L"\\") == 0) {
        return NULL;
    }

    if (my_wcscat (buf, db_filename) == 0) {
        return NULL;
    }

    return *buf;
}

static void tune_db_handler (sqlite3 *s)
{
    sqlite3_busy_timeout (s, busy_timeout);

    if (journal_statement)
        sqlite3_exec (s, journal_statement, NULL, NULL, NULL);

    RegisterExtensionFunctions (s);
}

static BOOL set_terminal_data_path(const wchar_t *path)
{
    if (!directory_exists (path)) {
        return FALSE;
    }

    if (terminal_data_path) {
        free (terminal_data_path);
    }

    terminal_data_path = _wcsdup (path);

    if (wcscpy (terminal_data_path, path) == 0) {
        return FALSE;
    }

    return TRUE;
}

int sqlite_initialize(const wchar_t *term_data_path)
{
    if (!set_terminal_data_path (term_data_path)) {
        return ERROR_INVALID_TERM_DATA_DIR;
    }

    return NO_ERROR;
}

void sqlite_finalize()
{
    if (terminal_data_path) {
        free (terminal_data_path);
        terminal_data_path = NULL;
    }
}

const wchar_t *__stdcall sqlite_get_fname (const wchar_t *db_filename)
{
    const wchar_t *db_path = build_db_path (db_filename);

    if (!db_path)
        return NULL;

    return db_path;
}


int __stdcall sqlite_exec (const wchar_t *db_filename, const wchar_t *sql)
{
    sqlite3 *s;
    int res;
    const wchar_t *db_path = build_db_path (db_filename);

    if (!db_path)
        return -1;

    const char *db_path_ansi = unicode_to_ansi_string (db_path);
    res = sqlite3_open (db_path_ansi, &s);
    HeapFree (GetProcessHeap (), 0, db_path);
    HeapFree (GetProcessHeap (), 0, db_path_ansi);

    if (res != SQLITE_OK)
        return res;

    tune_db_handler (s);

    const char* sql_ansi = unicode_to_ansi_string (sql);
    res = sqlite3_exec (s, sql_ansi, NULL, NULL, NULL);
    HeapFree(GetProcessHeap(), 0, sql_ansi);

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
int __stdcall sqlite_table_exists (const wchar_t *db_filename, const wchar_t *table_name)
{
    sqlite3 *s;
    sqlite3_stmt *stmt;
    char buf[256];
    int res, exists;
    const wchar_t *db_path = build_db_path (db_filename);

    if (!db_path)
        return -1;

    const char *db_path_ansi = unicode_to_ansi_string (db_path);
    res = sqlite3_open (db_path_ansi, &s);
    HeapFree (GetProcessHeap (), 0, db_path);
    HeapFree (GetProcessHeap (), 0, db_path_ansi);

    if (res != SQLITE_OK)
        return -res;

    tune_db_handler (s);

    const char *table_name_ansi = unicode_to_ansi_string (table_name);
    sprintf (buf, "select count(*) from sqlite_master where type='table' and name='%s'", table_name_ansi);
    res = sqlite3_prepare (s, buf, sizeof (buf), &stmt, NULL);
    HeapFree (GetProcessHeap (), 0, table_name_ansi);

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
int __stdcall sqlite_query (const wchar_t *db_filename, const wchar_t *sql, int* cols)
{
    sqlite3 *s;
    sqlite3_stmt *stmt;
    int res;
    struct query_result *result;
    const wchar_t* db_path = build_db_path (db_filename);

    if (!db_path)
        return -1;

    const char *db_path_ansi = unicode_to_ansi_string (db_path);
    res = sqlite3_open (db_path_ansi, &s);
    HeapFree (GetProcessHeap (), 0, db_path);
    HeapFree (GetProcessHeap (), 0, db_path_ansi);

    if (res != SQLITE_OK)
        return -res;

    tune_db_handler (s);

    const char* sql_ansi = unicode_to_ansi_string (sql);
    res = sqlite3_prepare (s, sql_ansi, strlen (sql_ansi), &stmt, NULL);
    HeapFree (GetProcessHeap (), 0, sql_ansi);

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

int __stdcall sqlite_bind_text (int handle, int col, const wchar_t* bind_value)
{
    struct query_result *res = (struct query_result*)handle;
    int ret;

    if (!res)
        return 0;

    const char *bind_value_ansi = unicode_to_ansi_string (bind_value);
    ret = sqlite3_bind_text (res->stmt, col, bind_value_ansi, -1, SQLITE_STATIC);
    HeapFree (GetProcessHeap (), 0, bind_value_ansi);

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



const wchar_t* __stdcall sqlite_get_col (int handle, int col)
{
    struct query_result *data = (struct query_result*)handle;

    if (!data)
        return NULL;

    return ansi_to_unicode_string (sqlite3_column_text (data->stmt, col));
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


void __stdcall sqlite_set_journal_mode (const wchar_t* mode)
{
    if (journal_statement) {
        free (journal_statement);
        journal_statement = NULL;
    }

    if (!mode)
        return;

    const char *mode_ansi = unicode_to_ansi_string (mode);

    static const char *format = "PRAGMA journal_mode=%s;";
    int len = strlen (format) + strlen (mode_ansi) + 1;

    journal_statement = (char*)malloc (len);
    sprintf (journal_statement, format, mode);
    HeapFree (GetProcessHeap (), 0, mode_ansi);
}
