sqlite3-mt4-wrapper
===================

Wrapper DLL for sqlite3 usage from MT4

## Howto

**NOTICE: This instruction is only for MT4 build 600 or later!**
For pre-600 builds use dll and header from tag https://github.com/Shmuma/sqlite3-mt4-wrapper/tree/pre-600

1. Download [zip of master](https://github.com/Shmuma/sqlite3-mt4-wrapper/archive/master.zip)
2. Extract it
3. Copy all contents under ``MQL4`` directory, to ``<TERMINAL_DATA_PATH>/MQL4``
    * See the "Terminal data path" section below
4. In your EA/Indicator/Script, add following include

    ```cpp
    #include <sqlite.mqh>
    ```
5. Add the following code

   ```cpp
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
    ```
6. sqlite wrapper functions

## Database file

Database file is by default stored to ``<TERMINAL_DATA_PATH>\MQL4\Files\SQLite``.

If you specify a full path as database filename, it's used.

## Terminal data path

TERMINAL_DATA_PATH can be known by the following instruction.

1. Open MT4
2. Open [File] menu
3. Click "Open Data Folder"

## Sample

Many sample scripts in under ``MQL4/Scripts``.

## Precautions
### Argument mess

MT4 build 610 has a weird bug when dll function with two string arguments gets corrupted when both values are variables. In that case, inside dll, second argument is the same as the first. In sqlite-wrapper two routines are affected: sqlite_exec and sqlite_table_exists. The simple temparary workaround of this (I hope it will be fixed in latest MT4) is to add empty string to a second argument, for examlpe:
```
bool do_check_table_exists (string db, string table)
{
    int res = sqlite_table_exists (db, table + "");
```
