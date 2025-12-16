/*-----------------------------------------------------------------
 * This is like a filing cabinet that stores all our rows.
 * Each row might have multiple versions because of MVCC!
 * -----------------------------------------------------------------
 */

#ifndef MVCC_TABLE_H
#define MVCC_TABLE_H

#include "mvcc_types.h"
#include "mvcc_visibility.h"
#include <stdlib.h>
#include <stdio.h>

// Maximum number of tuple versions we can store
#define MAX_TUPLES 1000

// ----------------------------------------------------------------------------
// TABLE STRUCTURE
// ----------------------------------------------------------------------------
// A table is just a collection of tuples (rows).
// Each tuple might have multiple versions linked together.

typedef struct {
    Tuple* tuples[MAX_TUPLES];  // Array of pointers to tuple chains
    int tuple_count;             // How many tuples do we have?
} Table;

// Global table (just one for simplicity)
Table global_table;

// ----------------------------------------------------------------------------
// INITIALIZE THE TABLE
// ----------------------------------------------------------------------------
// Set up an empty table (like getting a new empty notebook)
void init_table() {
    global_table.tuple_count = 0;
    for (int i = 0; i < MAX_TUPLES; i++) {
        global_table.tuples[i] = NULL;
    }
}

// ----------------------------------------------------------------------------
// INSERT A NEW ROW
// ----------------------------------------------------------------------------
// Add a brand new row to the table.
// This creates the FIRST version of this row.

bool insert_tuple(Transaction* tx, int32_t data) {
    // Check if table is full
    if (global_table.tuple_count >= MAX_TUPLES) {
        return false;  // No more room!
    }

    // Create a new tuple
    Tuple* new_tuple = (Tuple*)malloc(sizeof(Tuple));
    if (!new_tuple) {
        return false;  // Out of memory!
    }

    // Fill in the tuple's information
    new_tuple->xmin = tx->xid;        // I created this!
    new_tuple->xmax = INVALID_XID;    // Not deleted yet
    new_tuple->data = data;           // The actual data
    new_tuple->next_version = NULL;   // No older versions yet

    // Add it to the table
    global_table.tuples[global_table.tuple_count] = new_tuple;
    global_table.tuple_count++;

    return true;
}

// ----------------------------------------------------------------------------
// DELETE A ROW
// ----------------------------------------------------------------------------
// "Delete" in MVCC doesn't actually remove anything!
// We just mark it as deleted by setting xmax.
// Other transactions might still need to see the old version.

bool delete_tuple(Transaction* tx, int tuple_index) {
    // Check if index is valid
    if (tuple_index < 0 || tuple_index >= global_table.tuple_count) {
        return false;
    }

    Tuple* tuple = global_table.tuples[tuple_index];

    // Find the version we can see
    Tuple* visible = get_visible_version(tx, tuple);
    if (!visible) {
        return false;  // We can't see any version, so we can't delete it!
    }

    // Check if already deleted
    if (visible->xmax != INVALID_XID) {
        return false;  // Already deleted by someone
    }

    // Mark it as deleted by us
    visible->xmax = tx->xid;
    return true;
}

// ----------------------------------------------------------------------------
// UPDATE A ROW
// ----------------------------------------------------------------------------
// Update = Delete old version + Insert new version
// The old version stays around for transactions that started earlier!
// This is why MVCC is so powerful - no blocking!

bool update_tuple(Transaction* tx, int tuple_index, int32_t new_data) {
    // Check if index is valid
    if (tuple_index < 0 || tuple_index >= global_table.tuple_count) {
        return false;
    }

    Tuple* old_tuple = global_table.tuples[tuple_index];

    // Find the version we can see
    Tuple* visible = get_visible_version(tx, old_tuple);
    if (!visible) {
        return false;  // We can't see any version!
    }

    // Check if already updated/deleted
    if (visible->xmax != INVALID_XID) {
        return false;  // Someone else got here first
    }

    // Create a NEW version of this tuple
    Tuple* new_version = (Tuple*)malloc(sizeof(Tuple));
    if (!new_version) {
        return false;  // Out of memory
    }

    // Fill in the new version
    new_version->xmin = tx->xid;        // I created this version
    new_version->xmax = INVALID_XID;    // Not deleted yet
    new_version->data = new_data;       // The new data!
    new_version->next_version = NULL;   // End of chain

    // Mark the old version as "updated" (deleted by this transaction)
    visible->xmax = tx->xid;

    // Link the new version at the HEAD of the chain
    // (Newer versions go at the front, like a stack)
    new_version->next_version = old_tuple;
    global_table.tuples[tuple_index] = new_version;

    return true;
}

// ----------------------------------------------------------------------------
// SELECT ALL ROWS (VISIBLE TO THIS TRANSACTION)
// ----------------------------------------------------------------------------
// Read all rows that this transaction is allowed to see
void select_all(Transaction* tx) {
    printf("\n=== SELECT * (Transaction %lu) ===\n", tx->xid);
    printf("Index | Data\n");
    printf("------|-----\n");

    int visible_count = 0;
    for (int i = 0; i < global_table.tuple_count; i++) {
        Tuple* tuple = global_table.tuples[i];
        Tuple* visible = get_visible_version(tx, tuple);

        if (visible) {
            printf("  %3d | %4d\n", i, visible->data);
            visible_count++;
        }
    }

    if (visible_count == 0) {
        printf("  (no rows visible)\n");
    }
    printf("\n");
}

// ----------------------------------------------------------------------------
// CLEANUP OLD VERSIONS (VACUUM)
// ----------------------------------------------------------------------------
// In a real database, we eventually need to clean up old tuple versions
// that no transaction can see anymore. This is called "vacuuming".
// (We'll keep this simple for now - just show the concept)

void vacuum_table() {
    // In a full implementation, this would:
    // 1. Find tuple versions that no active transaction can see
    // 2. Free the memory for those old versions
    // 3. Compact the storage

    // For educational purposes, we'll just count how many versions exist
    int total_versions = 0;
    for (int i = 0; i < global_table.tuple_count; i++) {
        Tuple* current = global_table.tuples[i];
        while (current) {
            total_versions++;
            current = current->next_version;
        }
    }

    printf("VACUUM: %d total tuple versions in table\n", total_versions);
}

#endif
