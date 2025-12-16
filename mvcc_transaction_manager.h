/*----------------------------------------------------------------------------
 * This manages all transactions - like a teacher managing students in class.
 * It hands out ID numbers and tracks who's doing what.
 * ---------------------------------------------------------------------------
 */

#ifndef MVCC_TRANSACTION_MANAGER_H
#define MVCC_TRANSACTION_MANAGER_H

#include "mvcc_types.h"
#include <stdlib.h>
#include <string.h>

// Maximum number of transactions that can run at once
#define MAX_TRANSACTIONS 100

// ----------------------------------------------------------------------------
// TRANSACTION MANAGER
// ----------------------------------------------------------------------------
// This is the "boss" that keeps track of all transactions
typedef struct {
    // Next transaction ID to hand out (increases by 1 each time)
    TransactionId next_xid;

    // Array storing info about all transactions
    Transaction transactions[MAX_TRANSACTIONS];

    // How many transactions are currently active?
    int active_count;

} TransactionManager;

// Global transaction manager (only one exists)
TransactionManager tx_manager;

// ----------------------------------------------------------------------------
// INITIALIZE THE TRANSACTION MANAGER
// ----------------------------------------------------------------------------
// Call this once at startup to set everything up
void init_transaction_manager() {
    tx_manager.next_xid = FIRST_NORMAL_XID;
    tx_manager.active_count = 0;

    // Clear all transaction slots
    for (int i = 0; i < MAX_TRANSACTIONS; i++) {
        tx_manager.transactions[i].xid = INVALID_XID;
        tx_manager.transactions[i].status = TX_ABORTED;
    }
}

// ----------------------------------------------------------------------------
// START A NEW TRANSACTION
// ----------------------------------------------------------------------------
// Like getting a ticket number at the DMV
Transaction* begin_transaction() {
    // Find an empty slot
    int slot = -1;
    for (int i = 0; i < MAX_TRANSACTIONS; i++) {
        if (tx_manager.transactions[i].xid == INVALID_XID) {
            slot = i;
            break;
        }
    }

    if (slot == -1) {
        return NULL;  // No room! (all slots taken)
    }

    // Create the new transaction
    Transaction* tx = &tx_manager.transactions[slot];
    tx->xid = tx_manager.next_xid++;
    tx->status = TX_IN_PROGRESS;

    // SNAPSHOT ISOLATION: What can this transaction see?
    // It can see all transactions that finished BEFORE it started.
    // snapshot_xmin = oldest active transaction
    // snapshot_xmax = this transaction's ID

    tx->snapshot_xmin = tx->xid;  // Start with our own ID
    tx->snapshot_xmax = tx->xid;

    // Find the oldest transaction that's still running
    for (int i = 0; i < MAX_TRANSACTIONS; i++) {
        Transaction* other = &tx_manager.transactions[i];
        if (other->xid != INVALID_XID &&
            other->status == TX_IN_PROGRESS &&
            other->xid < tx->snapshot_xmin) {
            tx->snapshot_xmin = other->xid;
        }
    }

    tx_manager.active_count++;
    return tx;
}

// ----------------------------------------------------------------------------
// COMMIT A TRANSACTION
// ----------------------------------------------------------------------------
// Save all changes permanently (like clicking "Save" in a video game)
void commit_transaction(Transaction* tx) {
    if (tx && tx->status == TX_IN_PROGRESS) {
        tx->status = TX_COMMITTED;
        tx_manager.active_count--;
    }
}

// ----------------------------------------------------------------------------
// ABORT A TRANSACTION
// ----------------------------------------------------------------------------
// Throw away all changes (like clicking "Don't Save")
void abort_transaction(Transaction* tx) {
    if (tx && tx->status == TX_IN_PROGRESS) {
        tx->status = TX_ABORTED;
        tx_manager.active_count--;
    }
}

// ----------------------------------------------------------------------------
// GET TRANSACTION STATUS
// ----------------------------------------------------------------------------
// Check if a transaction is done, running, or cancelled
TransactionStatus get_transaction_status(TransactionId xid) {
    for (int i = 0; i < MAX_TRANSACTIONS; i++) {
        if (tx_manager.transactions[i].xid == xid) {
            return tx_manager.transactions[i].status;
        }
    }
    return TX_ABORTED;  // If we can't find it, assume it's old and done
}

#endif
