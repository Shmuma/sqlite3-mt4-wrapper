#include "sqlite.mqh"


int start ()
{
    string db = "experts/files/test.db";

    if (!sqlite_table_exists (db, "test")) {
        sqlite_exec (db, "create table test (name text)");
        sqlite_exec (db, "insert into test (name) values ('test1')");
        sqlite_exec (db, "insert into test (name) values ('test2')");
        sqlite_exec (db, "insert into test (name) values ('test3')");
        sqlite_exec (db, "insert into test (name) values ('test4')");
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
