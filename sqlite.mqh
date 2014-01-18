/*
 * SQLite interface for MT4
 */

#import "sqlite3_wrapper.dll"
int sqlite_exec (string db_fname, string sql);
int sqlite_table_exists (string db_fname, string table);
int sqlite_query (string db_fname, string sql, int& cols[]);
int sqlite_reset (int handle);
int sqlite_bind_int (int handle, int col, int bind_value);
int sqlite_bind_double (int handle, int col, double bind_value);
int sqlite_bind_text (int handle, int col, string bind_value);
int sqlite_bind_null (int handle, int col);
int sqlite_next_row (int handle);
string sqlite_get_col (int handle, int col);
int sqlite_get_col_int (int handle, int col);
double sqlite_get_col_double (int handle, int col);
int sqlite_free_query (int handle);
string sqlite_get_fname (string db_fname);
void sqlite_set_busy_timeout (int ms);
void sqlite_set_journal_mode (string mode);
#import
