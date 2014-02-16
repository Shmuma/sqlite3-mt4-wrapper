#property strict

#include <sqlite.mqh>

int OnInit()
{
    if (!sqlite_init()) {
        return INIT_FAILED;
    }

    return INIT_SUCCEEDED;
}

void OnDeinit(const int reason)
{
    sqlite_finalize();
}

void OnStart ()
{
    string db = "test_quotes.db";

    if (!sqlite_table_exists (db, "quotes"))
        sqlite_exec (db, "create table quotes (date, open, high, low, close)");

    int count = iBars (NULL, 0);
    PrintFormat ("Count = %d", count);

    datetime start = TimeLocal ();

    for (int i = 0; i < count; i++) {
        string query = "insert into quotes (date, open, high, low, close) values ('" + 
                     TimeToStr (iTime (NULL, 0, i)) + "'," + 
                     DoubleToString(iOpen (NULL, 0, i), Digits) + "," +
                     DoubleToString(iHigh (NULL, 0, i), Digits) + "," +
                     DoubleToString(iLow (NULL, 0, i), Digits) + "," +
                     DoubleToString(iClose (NULL, 0, i), Digits) + ");";

        sqlite_exec (db, query);
    }

    datetime end = TimeLocal ();
    datetime elapsed = end - start;
    PrintFormat ("inserted %d rows in %u sec", count, elapsed);
}
