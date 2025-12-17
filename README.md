 ## I implemented MVCC in C, just to understand how MVCC works actually.
 ## I did this only for learning about MVCC.
 ## My Analogy to understand this:
 ```
 Imagine a magical notebook where:
 - When you erase something, the old writing is still there (invisible ink)
 - Different people looking at the same page see different versions
 - Nobody has to wait for others to finish writing
```
## Commands to run:
```
1. make        - Build the program
2. make clean  - Remove build files
3. make run    - Build and run
```
## Architecture:
```
┌─────────────────────────────────────────────────────────────┐
│                         USER / APPLICATION                  │
└────────────────────────────┬────────────────────────────────┘
                             │
                             ▼
┌─────────────────────────────────────────────────────────────┐
│                     mvcc_main.c                             │
│  • Entry point                                              │
│  • System initialization                                    │
│  • Test runner                                              │
└────────────────────────────┬────────────────────────────────┘
                             │
                             ▼
┌─────────────────────────────────────────────────────────────┐
│                     mvcc_tests.h                            │
│  • Test scenarios for MVCC features                         │
│  • Snapshot isolation demos                                 │
│  • Concurrent transaction examples                          │
└────────────────────────────┬────────────────────────────────┘
                             │
                             ▼
┌─────────────────────────────────────────────────────────────┐
│                     mvcc_table.h                            │
│  • Table storage management                                 │
│  • INSERT, UPDATE, DELETE, SELECT operations                │
│  • Version chain management                                 │
│  • VACUUM concept                                           │
└────────────┬───────────────┴────────────────┬───────────────┘
             │                                │
             ▼                                ▼
┌────────────────────────────┐  ┌────────────────────────────┐
│   mvcc_visibility.h        │  │ mvcc_transaction_manager.h │
│  • is_tuple_visible()      │  │  • begin_transaction()     │
│  • 7 visibility rules      │  │  • commit_transaction()    │
│  • Version chain traversal │  │  • abort_transaction()     │
│  • THE HEART OF MVCC       │  │     • Snapshot creation    │
└────────────┬───────────────┘  └────────────-┬──────────────┘
             │                                │
             └────────────┬───────────────────┘
                          │
                          ▼
             ┌────────────────────────────┐
             │     mvcc_types.h           │
             │  • TransactionId           │
             │  • Tuple structure         │
             │  • Transaction structure   │
             │  • TransactionStatus enum  │
             └────────────────────────────┘
           
 ```
