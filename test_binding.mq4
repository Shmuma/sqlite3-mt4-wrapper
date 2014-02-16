#property strict

#include <sqlite.mqh>

bool do_check_table_exists (string db, string table)
{
    int res = sqlite_table_exists (db, table);

    if (res < 0) {
        PrintFormat ("Check for table existence failed with code %d", res);
        return (false);
    }

    return (res > 0);
}

void do_exec (string db, string exp)
{
    int res = sqlite_exec (db, exp);

    if (res != 0)
        PrintFormat ("Expression '%s' failed with code %d", exp, res);
}

int OnInit()
{
    if (!sqlite_init()) {
        return INIT_FAILED;
    }

    return INIT_SUCCEEDED;
}

void OnStart ()
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
    PrintFormat ("Count = %d", count);

    string query = "insert into quotes (date, symbol, open, high, low, close) values (?, ?, ?, ?, ?, ?)";
    int cols[1];

    int handle = sqlite_query (db, query, cols);
    if (handle < 0) {
        Print ("Preparing query failed; query=", query, ", error=", -handle);
        return;
    }

    for (int i = 0; i < count; i++) {
        sqlite_reset (handle);
        sqlite_bind_int (handle, 1, (int)iTime (NULL, 0, i));
        sqlite_bind_text (handle, 2, Symbol ());
        sqlite_bind_double (handle, 3, iOpen (NULL, 0, i));
        sqlite_bind_double (handle, 4, iHigh (NULL, 0, i));
        sqlite_bind_double (handle, 5, iLow (NULL, 0, i));
        sqlite_bind_double (handle, 6, iClose (NULL, 0, i));
        sqlite_next_row (handle);
    }

    sqlite_free_query (handle);
}

void OnDeinit (const int reason)
{
    /*
    string db = "test_binding.db";

    int count = iBars (NULL, 0);
    PrintFormat ("Count = %d", count);

    int cols[1];
    string query = "select * from quotes where symbol = ? order by date";
    int handle = sqlite_query (db, query, cols);
    if (handle < 0) {
        Print ("Preparing query failed; query=", query, ", error=", -handle);
        return;
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
    */
    sqlite_finalize();
}
