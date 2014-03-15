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

void OnDeinit(const int reason)
{
    sqlite_finalize();
}

void OnStart ()
{
    string db = "test_extra.db";

    string path = sqlite_get_fname (db);
    Print ("Dest DB path: " + path);

    if (!do_check_table_exists (db, "test")) {
        Print ("DB not exists, create schema");
        do_exec (db, "create table test (name text)");
        do_exec (db, "insert into test (name) values ('test1')");
        do_exec (db, "insert into test (name) values ('test2')");
        do_exec (db, "insert into test (name) values ('test3')");
        do_exec (db, "insert into test (name) values ('test4')");
    }

    int cols[1];
    int handle = sqlite_query (db, "select cos(radians(45))", cols);

    PrintFormat ("Handle value: %d", handle);

    while (sqlite_next_row (handle) == 1) {
        for (int i = 0; i < cols[0]; i++)
            Print (sqlite_get_col (handle, i));
    }

    sqlite_free_query (handle);
}
