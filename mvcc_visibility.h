/*-----------------------------------------------------------------------------------
 * This is where MVCC gets interesting! Different transactions see different
 * versions of the same row. It's like each transaction has special glasses
 * that only let them see certain versions.
 * -----------------------------------------------------------------------------------
 */

#ifndef MVCC_VISIBILITY_H
#define MVCC_VISIBILITY_H

#include "mvcc_types.h"
#include "mvcc_transaction_manager.h"

// ----------------------------------------------------------------------------
// IS THIS TUPLE VISIBLE TO ME?
// ----------------------------------------------------------------------------
// This is the CORE of MVCC. It answers: "Can this transaction see this row?"
//
// Think of it like this:
// - You can only see things that existed BEFORE you started
// - You can't see things that were deleted BEFORE you started
// - You can't see things created by transactions still running
//
// It's like watching a movie - you can only see scenes that were filmed
// before you started watching!

bool is_tuple_visible(Transaction* tx, Tuple* tuple) {
    TransactionId xmin = tuple->xmin;  // Who created this row?
    TransactionId xmax = tuple->xmax;  // Who deleted this row?

    // ========================================================================
    // RULE 1: Was this row created by ME in this transaction?
    // ========================================================================
    // If I created it, I can definitely see it!
    if (xmin == tx->xid) {
        // But wait - did I also delete it in this same transaction?
        if (xmax == tx->xid) {
            return false;  // I deleted it, so I shouldn't see it now
        }
        return true;  // I created it and haven't deleted it yet
    }

    // ========================================================================
    // RULE 2: Was this row created AFTER I started?
    // ========================================================================
    // I can't see things that didn't exist when I began!
    // (Like you can't see a movie scene that wasn't filmed yet)
    if (xmin >= tx->snapshot_xmax) {
        return false;  // Too new! Created after my snapshot
    }

    // ========================================================================
    // RULE 3: Was the creator transaction still running when I started?
    // ========================================================================
    // If the transaction that created this row was still working when I began,
    // I don't know if they'll commit or abort, so I can't see it yet.
    if (xmin >= tx->snapshot_xmin) {
        TransactionStatus status = get_transaction_status(xmin);
        if (status != TX_COMMITTED) {
            return false;  // Creator didn't finish yet (or cancelled)
        }
    }

    // ========================================================================
    // RULE 4: Was this row deleted?
    // ========================================================================
    // If nobody deleted it, we're good!
    if (xmax == INVALID_XID) {
        return true;  // Still alive!
    }

    // ========================================================================
    // RULE 5: Did I delete this row myself?
    // ========================================================================
    if (xmax == tx->xid) {
        return false;  // I deleted it, so I shouldn't see it
    }

    // ========================================================================
    // RULE 6: Was it deleted AFTER I started?
    // ========================================================================
    // If it was deleted after my snapshot, I should still see it
    // (I'm looking at an older version of the world)
    if (xmax >= tx->snapshot_xmax) {
        return true;  // Deleted in the future (relative to me)
    }

    // ========================================================================
    // RULE 7: Was the deleter transaction still running when I started?
    // ========================================================================
    if (xmax >= tx->snapshot_xmin) {
        TransactionStatus status = get_transaction_status(xmax);
        if (status != TX_COMMITTED) {
            return true;  // Deleter didn't finish, so row is still alive to me
        }
    }

    // If we got here, the row was deleted before I started and the
    // deleter committed, so I shouldn't see it
    return false;
}

// ----------------------------------------------------------------------------
// GET THE CORRECT VERSION OF A TUPLE
// ----------------------------------------------------------------------------
// Sometimes multiple versions of a row exist (a chain of updates).
// This finds the RIGHT version for this transaction to see.
//
// It's like finding the right frame in a film strip!

Tuple* get_visible_version(Transaction* tx, Tuple* tuple) {
    // Walk through the chain of versions
    Tuple* current = tuple;

    while (current != NULL) {
        // Is this version visible to me?
        if (is_tuple_visible(tx, current)) {
            return current;  // Found it!
        }

        // Try the next version
        current = current->next_version;
    }

    return NULL;  // No visible version found
}

#endif
