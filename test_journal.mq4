#include "sqlite.mqh"

bool do_check_table_exists (string db, string table)
{
    int res = sqlite_table_exists (db, table);

    if (res < 0) {
        Print ("Check for table existence failed with code " + res);
        return (false);
    }

    return (res > 0);
}


void benchmark (string db, string mode)
{
    sqlite_set_journal_mode (mode);

    datetime start = TimeCurrent ();

    sqlite_exec (db, "delete from bench;");

    for (int i = 0; i < 100000; i++)
        sqlite_exec (db, "insert into bench (" + i + ");");

    Alert ("Benchmark for mode " + mode + " took " + (TimeCurrent()-start) + " seconds");
}


int start ()
{
    string db = "test.db";

    string path = sqlite_get_fname (db);
    Print ("Dest DB path: " + path);

    if (!do_check_table_exists (db, "test")) {
        Print ("DB not exists, create schema");
        sqlite_exec (db, "create table bench (id integer)");
    }

    Print ("Start benchmarks");

    benchmark (db, "DELETE");
    benchmark (db, "WAL");
    benchmark (db, "MEMORY");
    benchmark (db, "OFF");

    return (0);
}
