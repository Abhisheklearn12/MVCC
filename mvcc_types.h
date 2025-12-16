/* ---------------------------------------------------------------------------
 * This file defines the fundamental data types our database needs.
 * Think of this like defining what a "toy block" looks like before building.
 * ---------------------------------------------------------------------------
 */

#ifndef MVCC_TYPES_H
#define MVCC_TYPES_H

#include <stdint.h>
#include <stdbool.h>

// ----------------------------------------------------------------------------
// TRANSACTION ID (XID)
// ----------------------------------------------------------------------------
// Every transaction gets a unique number, like a ticket number at a bakery.
// Smaller numbers = older transactions (came first in line)
typedef uint64_t TransactionId;

#define INVALID_XID 0       // This means "no transaction" (like ticket #0)
#define FIRST_NORMAL_XID 1  // Real transactions start at 1

// ----------------------------------------------------------------------------
// TUPLE (ROW) STRUCTURE
// ----------------------------------------------------------------------------
// A tuple is one row of data in our database table.
// But in MVCC, we keep multiple versions of the same row!
//
// Example: If you update a row, we don't delete the old version.
// We create a NEW version and mark when it was created/deleted.

typedef struct Tuple {
    // Who created this version of the row?
    TransactionId xmin;  // "Transaction that INSERTED this row"

    // Who deleted/updated this version? (0 if still alive)
    TransactionId xmax;  // "Transaction that DELETED this row"

    // The actual data (we'll keep it simple: just one integer)
    int32_t data;

    // Link to next version of this row (like a chain of beads)
    struct Tuple* next_version;

} Tuple;

// ----------------------------------------------------------------------------
// TRANSACTION STATUS
// ----------------------------------------------------------------------------
// Every transaction can be in one of these states:
typedef enum {
    TX_IN_PROGRESS,  // Still running (like a game in progress)
    TX_COMMITTED,    // Finished successfully (saved the game)
    TX_ABORTED       // Failed/cancelled (threw away the changes)
} TransactionStatus;

// ----------------------------------------------------------------------------
// TRANSACTION INFO
// ----------------------------------------------------------------------------
// Information about one transaction
typedef struct Transaction {
    TransactionId xid;           // This transaction's ID number
    TransactionStatus status;    // Is it running, done, or cancelled?
    TransactionId snapshot_xmin; // Oldest transaction I can see
    TransactionId snapshot_xmax; // Newest transaction I can see
} Transaction;

#endif
