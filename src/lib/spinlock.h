//
// Created by nero on 5/3/2021.
//

#ifndef OPENGROK_SPINLOCK_H
#define OPENGROK_SPINLOCK_H

/*
 * Spinlock implementation
 */

typedef bool spinner_t;

volatile static void spinlock_lock(spinner_t *spinner) {
    while (*spinner) {}
    *spinner = true;
}

static bool spinlock_try_lock(spinner_t *spinner) {
    if (*spinner) {
        return false;
    }

    *spinner = true;

    return true;
}

static bool spinlock_try_unlock(spinner_t *spinner) {
    if (*spinner) {
        *spinner = false;

        return true;
    }

    return false;
}

static void spinlock_unlock(spinner_t *spinner) {
    if (!spinlock_try_unlock(spinner)) {
        abort();
    }
}

#endif //OPENGROK_SPINLOCK_H
