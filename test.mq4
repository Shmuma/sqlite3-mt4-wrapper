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
    string db = "test.db";

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
    int handle = sqlite_query (db, "select * from test", cols);

    while (sqlite_next_row (handle) == 1) {
        for (int i = 0; i < cols[0]; i++)
            Print (sqlite_get_col (handle, i));
    }

    sqlite_free_query (handle);

    return (0);
}
