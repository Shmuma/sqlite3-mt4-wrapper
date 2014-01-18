#include <sqlite.mqh>

bool do_check_table_exists (string db, string table)
{
    int res = sqlite_table_exists (db, table);

    if (res < 0) {
        Print ("Check for table existence failed with code " + res);
        return (false);
    }

    return (res > 0);
}

void do_exec (string db, string exp)
{
    int res = sqlite_exec (db, exp);

    if (res != 0)
        Print ("Expression '" + exp + "' failed with code " + res);
}


int start ()
{
    string db = "test_binding.db";

    if (!do_check_table_exists (db, "quotes"))
        do_exec (db,
            "create table quotes (" +
            " date integer," +
            " symbol text," +
            " open real," +
            " high real," +
            " low real," +
            " close real)");

    int count = iBars (NULL, 0);
    Print ("Count = " + count);

    string query = "insert into quotes (date, symbol, open, high, low, close) values (?, ?, ?, ?, ?, ?)";
    int cols[1];

    int handle = sqlite_query (db, query, cols);
    if (handle < 0) {
        Print ("Preparing query failed; query=", query, ", error=", -handle);
        return (1);
    }

    for (int i = 0; i < count; i++) {
        sqlite_reset (handle);
        sqlite_bind_int (handle, 1, iTime (NULL, 0, i));
        sqlite_bind_text (handle, 2, Symbol ());
        sqlite_bind_double (handle, 3, iOpen (NULL, 0, i));
        sqlite_bind_double (handle, 4, iHigh (NULL, 0, i));
        sqlite_bind_double (handle, 5, iLow (NULL, 0, i));
        sqlite_bind_double (handle, 6, iClose (NULL, 0, i));
        sqlite_next_row (handle);
    }

    sqlite_free_query (handle);
    return (0);
}

int deinit ()
{
    string db = "test_binding.db";

    int count = iBars (NULL, 0);
    Print ("Count = " + count);

    int cols[1];
    string query = "select * from quotes where symbol = ? order by date";
    int handle = sqlite_query (db, query, cols);
    if (handle < 0) {
        Print ("Preparing query failed; query=", query, ", error=", -handle);
        return (1);
    }

    sqlite_bind_text (handle, 1, Symbol ());

    while (sqlite_next_row (handle) == 1) {
        datetime date = sqlite_get_col_int (handle, 0);
        string symbol = sqlite_get_col (handle, 1);
        double open = sqlite_get_col_double (handle, 2);
        double high = sqlite_get_col_double (handle, 3);
        double low = sqlite_get_col_double (handle, 4);
        double close = sqlite_get_col_double (handle, 5);

        Print ("date=", TimeToStr(date), ", symbol=", symbol, ", open/high/low/close=", open, "/", high, "/", low, "/", close);
    }

    sqlite_free_query (handle);

    return (0);
}
