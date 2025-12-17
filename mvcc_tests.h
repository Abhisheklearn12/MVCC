/*----------------------------------------------------------------------
 * This file contains test scenarios to demonstrate MVCC in action.
 * Each test shows a different aspect of how MVCC works.
 * ---------------------------------------------------------------------
 */

#ifndef MVCC_TESTS_H
#define MVCC_TESTS_H

#include "mvcc_table.h"
#include <stdio.h>

// ----------------------------------------------------------------------------
// TEST 1: Basic Insert and Select
// ----------------------------------------------------------------------------
// The simplest test - insert some data and read it back
void test_basic_insert_select() {
    printf("\n");
    printf("========================================\n");
    printf("TEST 1: Basic Insert and Select\n");
    printf("========================================\n");

    // Start a transaction
    Transaction* tx1 = begin_transaction();
    printf("Transaction %lu started\n", tx1->xid);

    // Insert some data
    insert_tuple(tx1, 100);
    insert_tuple(tx1, 200);
    insert_tuple(tx1, 300);
    printf("Inserted 3 rows\n");

    // Select the data
    select_all(tx1);

    // Commit
    commit_transaction(tx1);
    printf("Transaction %lu committed\n", tx1->xid);
}

// ----------------------------------------------------------------------------
// TEST 2: Snapshot Isolation
// ----------------------------------------------------------------------------
// This shows how two transactions see different snapshots of the data!
void test_snapshot_isolation() {
    printf("\n");
    printf("========================================\n");
    printf("TEST 2: Snapshot Isolation\n");
    printf("========================================\n");
    printf("This shows the MAGIC of MVCC!\n");
    printf("Two transactions can see different data at the same time.\n\n");

    // Transaction 1 starts
    Transaction* tx1 = begin_transaction();
    printf("Transaction %lu started\n", tx1->xid);

    // Transaction 1 inserts data
    insert_tuple(tx1, 1000);
    printf("TX%lu: Inserted value 1000\n", tx1->xid);

    // Transaction 1 can see its own insert
    printf("TX%lu sees:\n", tx1->xid);
    select_all(tx1);

    // Transaction 2 starts BEFORE Transaction 1 commits
    Transaction* tx2 = begin_transaction();
    printf("Transaction %lu started (before TX%lu commits)\n",
           tx2->xid, tx1->xid);

    // Transaction 2 CANNOT see Transaction 1's data yet!
    printf("TX%lu sees (should NOT see TX%lu's insert):\n",
           tx2->xid, tx1->xid);
    select_all(tx2);

    // Now Transaction 1 commits
    commit_transaction(tx1);
    printf("Transaction %lu committed\n\n", tx1->xid);

    // Transaction 2 STILL can't see it (snapshot isolation)!
    printf("TX%lu sees (STILL doesn't see it - snapshot!):\n", tx2->xid);
    select_all(tx2);

    // Transaction 2 commits
    commit_transaction(tx2);

    // NEW transaction will see everything
    Transaction* tx3 = begin_transaction();
    printf("NEW Transaction %lu started\n", tx3->xid);
    printf("TX%lu sees (NOW it sees everything):\n", tx3->xid);
    select_all(tx3);
    commit_transaction(tx3);
}

// ----------------------------------------------------------------------------
// TEST 3: Update and Version Chains
// ----------------------------------------------------------------------------
// This shows how updates create new versions
void test_update_versions() {
    printf("\n");
    printf("========================================\n");
    printf("TEST 3: Update and Version Chains\n");
    printf("========================================\n");
    printf("Updates create NEW versions, old versions stay!\n\n");

    // TX1: Insert initial data
    Transaction* tx1 = begin_transaction();
    printf("TX%lu: Inserting value 42\n", tx1->xid);
    insert_tuple(tx1, 42);
    commit_transaction(tx1);

    // TX2: Update the value
    Transaction* tx2 = begin_transaction();
    printf("TX%lu: Updating value 42 -> 99\n", tx2->xid);
    update_tuple(tx2, 3, 99);  // Update index 3

    // TX2 sees the new value
    printf("TX%lu sees:\n", tx2->xid);
    select_all(tx2);

    // TX3 starts BEFORE TX2 commits
    Transaction* tx3 = begin_transaction();
    printf("TX%lu: Started before TX%lu commits\n", tx3->xid, tx2->xid);

    // TX3 sees the OLD value (snapshot isolation!)
    printf("TX%lu sees (OLD value):\n", tx3->xid);
    select_all(tx3);

    // Now TX2 commits
    commit_transaction(tx2);
    printf("TX%lu committed\n", tx2->xid);

    // TX3 STILL sees old value (it has a snapshot from before)
    printf("TX%lu STILL sees (OLD value - snapshot!):\n", tx3->xid);
    select_all(tx3);
    commit_transaction(tx3);

    // Show version chains
    printf("\n");
    vacuum_table();
}

// ----------------------------------------------------------------------------
// TEST 4: Delete and Visibility
// ----------------------------------------------------------------------------
// This shows how deletes work in MVCC
void test_delete() {
    printf("\n");
    printf("========================================\n");
    printf("TEST 4: Delete and Visibility\n");
    printf("========================================\n");
    printf("Deletes just mark rows, they don't remove them!\n\n");

    // TX1: Delete a row
    Transaction* tx1 = begin_transaction();
    printf("TX%lu: Deleting row at index 0\n", tx1->xid);
    delete_tuple(tx1, 0);

    // TX1 doesn't see the deleted row
    printf("TX%lu sees:\n", tx1->xid);
    select_all(tx1);

    // TX2 starts BEFORE TX1 commits
    Transaction* tx2 = begin_transaction();
    printf("TX%lu: Started before delete commits\n", tx2->xid);

    // TX2 STILL sees the row (snapshot isolation)
    printf("TX%lu sees (row still visible!):\n", tx2->xid);
    select_all(tx2);

    commit_transaction(tx1);
    commit_transaction(tx2);

    printf("\nBoth committed. Row is marked deleted but still in memory!\n");
    vacuum_table();
}

// ----------------------------------------------------------------------------
// TEST 5: Concurrent Transactions
// ----------------------------------------------------------------------------
// Multiple transactions working at the same time
void test_concurrent() {
    printf("\n");
    printf("========================================\n");
    printf("TEST 5: Concurrent Transactions\n");
    printf("========================================\n");
    printf("Multiple transactions working together!\n\n");

    // Clear table for clean test
    init_table();

    // TX1 inserts 1
    Transaction* tx1 = begin_transaction();
    insert_tuple(tx1, 1);
    printf("TX%lu: Inserted 1\n", tx1->xid);

    // TX2 inserts 2
    Transaction* tx2 = begin_transaction();
    insert_tuple(tx2, 2);
    printf("TX%lu: Inserted 2\n", tx2->xid);

    // TX3 starts and looks
    Transaction* tx3 = begin_transaction();
    printf("TX%lu: Looking at database\n", tx3->xid);
    printf("TX%lu sees (nothing yet!):\n", tx3->xid);
    select_all(tx3);

    // TX1 commits
    commit_transaction(tx1);
    printf("TX%lu committed\n", tx1->xid);

    // TX3 still doesn't see TX1's data (snapshot!)
    printf("TX%lu sees (STILL nothing - snapshot!):\n", tx3->xid);
    select_all(tx3);

    // TX2 commits
    commit_transaction(tx2);
    printf("TX%lu committed\n", tx2->xid);

    // TX3 STILL doesn't see anything
    printf("TX%lu sees (STILL nothing!):\n", tx3->xid);
    select_all(tx3);
    commit_transaction(tx3);

    // TX4 starts fresh and sees everything
    Transaction* tx4 = begin_transaction();
    printf("\nTX%lu: Fresh transaction\n", tx4->xid);
    printf("TX%lu sees (everything!):\n", tx4->xid);
    select_all(tx4);
    commit_transaction(tx4);
}

#endif
