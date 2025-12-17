/*--------------------------------------------------------------------------------
 * This is the entry point that runs all our tests and shows MVCC in action.
 *
 * WHAT IS MVCC? (Multi-Version Concurrency Control)
 *
 * Imagine a magical notebook where:
 * - When you erase something, the old writing is still there (invisible ink)
 * - Different people looking at the same page see different versions
 * - Nobody has to wait for others to finish writing
 *
 * That's MVCC, It's how PostgreSQL lets many people use the database at once
 * without blocking each other.
 * ---------------------------------------------------------------------------------
 */

#include "mvcc_types.h"
#include "mvcc_transaction_manager.h"
#include "mvcc_visibility.h"
#include "mvcc_table.h"
#include "mvcc_tests.h"
#include <stdio.h>

// ----------------------------------------------------------------------------
// EXPLAIN MVCC TO A 5-YEAR-OLD
// ----------------------------------------------------------------------------
void print_mvcc_explanation() {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║           MVCC: Multi-Version Concurrency Control             ║\n");
    printf("║              (Explained Simply!)                               ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    printf("Imagine a coloring book with MAGIC pages:\n");
    printf("\n");
    printf("  1. When you color something, your coloring stays on the page\n");
    printf("  2. But someone else can ALSO color the SAME page differently\n");
    printf("  3. Each person sees THEIR OWN version of the page\n");
    printf("  4. Nobody has to wait for others to finish!\n");
    printf("\n");
    printf("That's what MVCC does for databases:\n");
    printf("  ✓ Each transaction sees its own 'snapshot' of the data\n");
    printf("  ✓ Updates create NEW versions (old ones stay!)\n");
    printf("  ✓ No waiting = faster database!\n");
    printf("\n");
    printf("Key Concepts:\n");
    printf("  • TRANSACTION = One person's turn with the coloring book\n");
    printf("  • TUPLE = One row in a table (like one picture to color)\n");
    printf("  • VERSION = Different colored versions of the same picture\n");
    printf("  • SNAPSHOT = What you see when you start coloring\n");
    printf("\n");
    printf("Let's see it in action!\n");
    printf("\n");
}

// ----------------------------------------------------------------------------
// PRINT SYSTEM STATUS
// ----------------------------------------------------------------------------
void print_system_status() {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║                    SYSTEM STATUS                               ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    printf("Next Transaction ID: %lu\n", tx_manager.next_xid);
    printf("Active Transactions: %d\n", tx_manager.active_count);
    printf("Tuples in Table: %d\n", global_table.tuple_count);
    printf("\n");
}

// ----------------------------------------------------------------------------
// MAIN PROGRAM
// ----------------------------------------------------------------------------
int main() {
    printf("\n");
    printf("════════════════════════════════════════════════════════════════\n");
    printf("    PostgreSQL-Style MVCC Implementation in C11\n");
    printf("    A Complete, Working Example\n");
    printf("════════════════════════════════════════════════════════════════\n");

    // Step 1: Show explanation
    print_mvcc_explanation();

    // Step 2: Initialize the system
    printf("Initializing MVCC system...\n");
    init_transaction_manager();
    init_table();
    printf("✓ Transaction manager initialized\n");
    printf("✓ Table storage initialized\n");
    print_system_status();

    // Step 3: Run the tests
    printf("Press ENTER to start Test 1...\n");
    getchar();
    test_basic_insert_select();
    print_system_status();

    printf("\nPress ENTER for Test 2 (Snapshot Isolation)...\n");
    getchar();
    test_snapshot_isolation();
    print_system_status();

    printf("\nPress ENTER for Test 3 (Updates & Versions)...\n");
    getchar();
    test_update_versions();
    print_system_status();

    printf("\nPress ENTER for Test 4 (Deletes)...\n");
    getchar();
    test_delete();
    print_system_status();

    printf("\nPress ENTER for Test 5 (Concurrent Transactions)...\n");
    getchar();
    test_concurrent();
    print_system_status();

    // Step 4: Summary
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║                     TESTS COMPLETE!                            ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    printf("What We Learned:\n");
    printf("  ✓ MVCC allows multiple transactions to work simultaneously\n");
    printf("  ✓ Each transaction sees a consistent snapshot of data\n");
    printf("  ✓ Updates create new versions (no in-place modification)\n");
    printf("  ✓ Old versions stay around for concurrent transactions\n");
    printf("  ✓ No locks needed for reads (amazing performance!)\n");
    printf("\n");
    printf("This is how PostgreSQL achieves high concurrency!\n");
    printf("\n");
    printf("Files in this implementation:\n");
    printf("  1. mvcc_types.h               - Basic data structures\n");
    printf("  2. mvcc_transaction_manager.h - Transaction lifecycle\n");
    printf("  3. mvcc_visibility.h          - Visibility rules (MVCC core!)\n");
    printf("  4. mvcc_table.h               - Storage & operations\n");
    printf("  5. mvcc_tests.h               - Test scenarios\n");
    printf("  6. mvcc_main.c                - This main program\n");
    printf("\n");
    printf("To compile:\n");
    printf("  gcc -std=c11 -o mvcc_demo mvcc_main.c\n");
    printf("\n");

    return 0;
}

// ============================================================================
// BONUS: Quick Reference Guide
// ============================================================================
/*

MVCC QUICK REFERENCE:
--------------------

IMPORTANT RULES:
1. Each tuple has xmin (creator) and xmax (deleter)
2. Transactions see snapshots from when they start
3. Updates = Delete old + Insert new version
4. Visibility rules determine what each transaction sees

VISIBILITY ALGORITHM:
1. Can I see things I created? YES
2. Can I see things created after I started? NO
3. Can I see things being created by active transactions? NO
4. Can I see deleted things? DEPENDS on when deleted

FILE STRUCTURE:
mvcc_types.h               : Core data structures
mvcc_transaction_manager.h : Transaction control
mvcc_visibility.h          : Visibility rules (THE MAGIC!)
mvcc_table.h               : Storage and SQL operations
mvcc_tests.h               : Comprehensive test suite
mvcc_main.c                : Entry point and integration

*/
