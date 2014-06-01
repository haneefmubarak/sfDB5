#include "lock.h"

// ===

// get an excl lock on the lock structure
static inline lock_al (arena_lock *lock) {
	int access = 0;

	// quick spinlock (ironically enough, to access the lock)
	while (!access) {
		access = __sync_bool_compare_and_swap (&(lock->access), (int) 0, (int) 1);
	}

	return access;
}

// force a memory fence and then unlock the structure
static inline unlock_al (arena_lock *lock) {
	__sync_synchronize ();
	return  __sync_bool_compare_and_swap (&(lock->access), (int) 1, (int) 0);
}

// ===

int lock_reader (arena_lock *lock) {
	// get an exclusive lock on the lock structure
	lock_al (lock);

	// if writer wants it, leave it be
	if (lock->wait.w) {
		unlock_al (lock);

		return 0;	// false
	}

	// see who is doing an op on it
	switch (lock->op) {
		case ARENA_OP_NONE:	// not in use
			lock->op = ARENA_OP_READER;
			lock->reader_count = 1;

			unlock_al (lock);

			return 1;	// true

		case ARENA_OP_READER:	// a writer is using it
			lock->wait.r = 1;	// notify the next writer that
						// another thread wants to read
			unlock_al (lock);

			return 0;	// false

		case ARENA_OP_WRITER:	// another reader is using it -- perfect
			// check for excessive readers
			if (lock->reader_count < ARENA_MAX_READER_COUNT) {
				lock->reader_count++;
				unlock_al (lock);

				return 1;	// true
			} else {
				unlock_al (lock);

				return 0;	// false
			}

		default:
			__builtin_unreachable ();
			unlock_al (lock);
			return 0;
	}

	__builtin_unreachable ();
	return 0;	// false
}

int lock_writer (arena_lock *lock) {
	// get an exclusive lock on the lock structure
	lock_al (lock);

	// see if a reader is waiting on it
	if (lock->wait.r) {
		unlock_al (lock);
		return 0;	// false
	}

	// see who is doing an op on it
	switch (lock->op) {
		case ARENA_OP_NONE:	// not in use
			lock->op = ARENA_OP_WRITER;

			unlock_al (lock);
			return 1;	// true

		case ARENA_OP_READER:	// a reader is using it
			lock->wait.w = 1;

			unlock_al (lock);
			return 0;	// false

		case ARENA_OP_WRITER:	// another writer is using it - ah well
			unlock_al (lock);
			return 0;	// false

		default:
			__builtin_unreachable ();
			unlock_al (lock);
			return 0;	// false
	}

	__builtin_unreachable ();
	return 0;	// false
}

// ===

int unlock_reader (arena_lock *lock) {
	// get an exclusive lock on the lock structure
	lock_al (lock);

	lock->reader_count--;

	// check if we are the last reader
	if (lock->reader_count < 1) {
		lock->op = ARENA_OP_NONE;
	}

	unlock_al (lock);
	return 0;	// success
}
