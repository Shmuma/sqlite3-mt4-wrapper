#include "sqlite.mqh"


int start ()
{
    string db = "experts/files/quotes.db";

    if (!sqlite_table_exists (db, "quotes"))
        sqlite_exec (db, "create table quotes (date, open, high, low, close)");

    int count = iBars (NULL, 0);
    Print ("Count = " + count);
    string query = "";
    for (int i = 0; i < count; i++) {
        string s = "insert into quotes (date, open, high, low, close) values ('" + 
                     TimeToStr (iTime (NULL, 0, i)) + "'," + 
                     iOpen (NULL, 0, i) + "," +
                     iHigh (NULL, 0, i) + "," +
                     iLow (NULL, 0, i) + "," +
                     iClose (NULL, 0, i) + ");";
        query = query + "\n" + s;
    }
    
    sqlite_exec (db, query);
}
