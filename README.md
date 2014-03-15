sqlite3-mt4-wrapper
===================

Wrapper DLL for sqlite3 usage from MT4

## Howto

**NOTICE: This instruction is only for MT4 build 600 or later!**

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
