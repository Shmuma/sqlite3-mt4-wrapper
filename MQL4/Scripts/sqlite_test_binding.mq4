#property strict

#include <sqlite.mqh>

bool do_check_table_exists (string db, string table)
{
    int res = sqlite_table_exists (db, table + "");

    if (res < 0) {
        PrintFormat ("Check for table existence failed with code %d", res);
        return (false);
    }

    return (res > 0);
}

void do_exec (string db, string exp)
{
    int res = sqlite_exec (db, exp + "");

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

    datetime start = TimeLocal ();
    for (int i = 0; i < count; i++) {
        sqlite_reset (handle);
        sqlite_bind_int64 (handle, 1, iTime (NULL, 0, i));
        sqlite_bind_text (handle, 2, Symbol ());
        sqlite_bind_double (handle, 3, NormalizeDouble (iOpen (NULL, 0, i), Digits));
        sqlite_bind_double (handle, 4, NormalizeDouble (iHigh (NULL, 0, i), Digits));
        sqlite_bind_double (handle, 5, NormalizeDouble (iLow (NULL, 0, i), Digits));
        sqlite_bind_double (handle, 6, NormalizeDouble (iClose (NULL, 0, i), Digits));
        sqlite_next_row (handle);
    }

    sqlite_free_query (handle);

    datetime end = TimeLocal ();
    datetime elapsed = end - start;
    PrintFormat ("insert %d rows in %u sec", IntegerToString(count), elapsed);
}

void OnDeinit (const int reason)
{

    string db = "test_binding.db";

    Print ("Fetching rows start");

    int cols[1];
    string query = "select * from quotes where symbol = ? order by date";
    int handle = sqlite_query (db, query, cols);
    if (handle < 0) {
        Print ("Preparing query failed; query=", query, ", error=", -handle);
        return;
    }

    sqlite_bind_text (handle, 1, Symbol ());

    int count = 0;
    
    // only print first 100 records
    while (sqlite_next_row (handle) == 1 && count < 100) {
        datetime date = (datetime) sqlite_get_col_int64 (handle, 0);
        string symbol = sqlite_get_col (handle, 1);
        double open = sqlite_get_col_double (handle, 2);
        double high = sqlite_get_col_double (handle, 3);
        double low = sqlite_get_col_double (handle, 4);
        double close = sqlite_get_col_double (handle, 5);

        PrintFormat ("date=%s, symbol=%s, open/high/low/close=%s/%s/%s/%s",
            TimeToString (date), Symbol (),
            DoubleToString (open, Digits),
            DoubleToString (high, Digits),
            DoubleToString (low, Digits),
            DoubleToString (close, Digits));

        count += 1;
    }
    
    Print ("fetching rows done");

    sqlite_free_query (handle);

    sqlite_finalize();
}
