/*
 * SQLite interface for MT4
 */

#import "sqlite3_wrapper.dll"
int sqlite_initialize (string terminal_data_path);
void sqlite_finalize ();

// Warning: These two routines are affected by MT4 (build 610) bug,
// which causes wrong argument order passed to DLL, when both arguments are from variables.
// The simplest workaround of this, is to add empty string to SECOND argument on call.
// See example sqlite_test.mq4.
int sqlite_exec (string db_fname, string sql);
int sqlite_table_exists (string db_fname, string table);

int sqlite_query (string db_fname, string sql, int& cols[]);
int sqlite_reset (int handle);
int sqlite_bind_int (int handle, int col, int bind_value);
int sqlite_bind_int64 (int handle, int col, long bind_value);
int sqlite_bind_double (int handle, int col, double bind_value);
int sqlite_bind_text (int handle, int col, string bind_value);
int sqlite_bind_null (int handle, int col);
int sqlite_next_row (int handle);
string sqlite_get_col (int handle, int col);
int sqlite_get_col_int (int handle, int col);
long sqlite_get_col_int64 (int handle, int col);
double sqlite_get_col_double (int handle, int col);
int sqlite_free_query (int handle);
string sqlite_get_fname (string db_fname);
void sqlite_set_busy_timeout (int ms);
void sqlite_set_journal_mode (string mode);
#import

bool sqlite_init()
{
    int error = sqlite_initialize(TerminalInfoString(TERMINAL_DATA_PATH));
    if (error == 0) {
        Print("sqlite initialization succeeded");
        return true;
    }
    else {
        Alert("ERROR: sqlite initialization failed, error=" + IntegerToString(error));
        return false;
    }
}
