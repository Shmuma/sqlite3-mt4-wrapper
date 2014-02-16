sqlite3-mt4-wrapper
===================

Wrapper DLL for sqlite3 usage from MT4

## Howto

<p style="red">NOTICE: This instruction is only for MT4 build 600 or later!</p>

1. Download [zip of master](https://github.com/Shmuma/sqlite3-mt4-wrapper/archive/master.zip)
2. Extract it
3. Copy all contents under ``MQL4`` directory, to ``<TERMINAL_DATA_PATH>/MQL4``
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

## Sample

Many sample scripts in under ``MQL4/Scripts``.
