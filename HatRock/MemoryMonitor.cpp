#include "MemoryMonitor.h"

#include <atomic> // for std::atomic, which is used for thread-safe operations on shared variables
#include <cstddef> // for std::size_t, which is used for size-related types
#include <cstdlib> 
#include <new>
#include <fstream>
#include <malloc.h> // _aligned_malloc / _aligned_free on Windows, posix_memalign on Linux

namespace
{
	constexpr std::size_t MAX_TRACKED_ALLOCATIONS = 100000; // Instead of storing allocations in a vector/unordered map, since those take up memory as well;

	struct AllocationRecord {
		void* address = nullptr; // address in memory;
		std::size_t size = 0; // how many bytes were allocated
		std::size_t allocationID = 0; // #1, #2, etc - helps track which ones are which
	};

	// Static storage: no dynamic allocatio needed for the tracking table;
	AllocationRecord allocations[MAX_TRACKED_ALLOCATIONS]{}; // uses the maxed number from above, this actually starts up the database
	std::size_t allocationCount = 0; // how many entries are currently used;
	std::size_t usedMemoryTotal = 0; // how many bytes are currently allocated;
	std::size_t totalAllocations = 0; // how many allocations have been made in total;
	std::size_t totalFrees = 0; // how many frees have been made in total;
	std::size_t peakMemory = 0; // counts how many bytes were allocated max at the same time;
	std::size_t peakAllocations = 0; // counts how many allocations were active max at the same time;
	std::size_t nextAllocationID = 1; // ID counter;
	std::size_t untrackedAllocations = 0;

	// Protet the table and counters if allocations happen on multiple threads at the same time;
	std::atomic_flag trackerLock = ATOMIC_FLAG_INIT; // atomic_flag is a simple lock that can be set and cleared atomically. It is used to protect shared data from concurrent access.

	class LockGuard {
	public:
		LockGuard() noexcept {
			while (trackerLock.test_and_set(std::memory_order_acquire)) { // spin-wait (busy wait) until the lock is available

			}
		}

		~LockGuard() noexcept {
			trackerLock.clear(std::memory_order_release); // release the lock
		}

		LockGuard(const LockGuard&) = delete; // Delete copy constructor
		LockGuard& operator=(const LockGuard&) = delete; // Delete copy assignment operator
	};

	void recordAllocation(void* address, std::size_t size) noexcept
{
    LockGuard lock; // Lock the tracker for thread safety

    ++totalAllocations;

    if (allocationCount < MAX_TRACKED_ALLOCATIONS) {
        allocations[allocationCount++] = { address, size, nextAllocationID++ };

        usedMemoryTotal += size;

        if (usedMemoryTotal > peakMemory) {
            peakMemory = usedMemoryTotal;
        }

        if (allocationCount > peakAllocations) {
            peakAllocations = allocationCount;
        }
    }
    
	else {
        ++untrackedAllocations;
    }
}

	void recordFree(void* address) noexcept {
		LockGuard lock;

		for (std::size_t i = 0; i < allocationCount; i++) {
			if (allocations[i].address == address) {
				// Use the recorded size, not the sized-delete argument, to avoid issues with mismatched sizes;
				usedMemoryTotal -= allocations[i].size;
				totalFrees++;

				// Replace this entry with the last one, and decrement the count;
				allocations[i] = allocations[allocationCount - 1];
				allocations[allocationCount - 1] = {}; // Clear the last entry
				allocationCount--;
				return;
			}

			// It may be an allocation made before tracking started, or one that could not be recorded because the table was full
		}
	}

	void* allocate(std::size_t size) {
		// malloc(0) is allowed to return null; new must still succeed or throw, so request at least one byte
		void* memory = std::malloc(size == 0 ? 1 : size);

		if (!memory)
			throw std::bad_alloc(); // Throw an exception if memory allocation fails

		recordAllocation(memory, size);
		return memory;
	}

	void* allocateAligned(std::size_t size, std::size_t alignment) {
		void* memory = _aligned_malloc(size == 0 ? 1 : size, alignment);

		if (!memory)
			throw std::bad_alloc(); // Throw an exception if memory allocation fails

		recordAllocation(memory, size);
		return memory;
	}
}
	

// Ordinary scalar and array allocations
void* operator new(std::size_t size) {
	return allocate(size);
}

void* operator new[](std::size_t size) {
	return allocate(size);
}

// Ordinary scalar and array deallocations
void operator delete(void* memory) noexcept {
	if (!memory) 
		return;

	recordFree(memory);
    std::free(memory);
}

void operator delete[](void* memory) noexcept {
	if (!memory)
		return;

	recordFree(memory);
	std::free(memory);
}

// Sized deallocation overloads
void operator delete(void* memory, std::size_t) noexcept {
	::operator delete(memory);
}

void operator delete[](void* memory, std::size_t) noexcept {
	::operator delete[](memory);
}

// Over-aligned allocations (useful for C++17 and later)
void* operator new(std::size_t size, std::align_val_t alignment) {
	return allocateAligned(size, static_cast<std::size_t>(alignment));
}

void* operator new[](std::size_t size, std::align_val_t alignment) {
	return allocateAligned(size, static_cast<std::size_t>(alignment));
}

void operator delete(void* memory, std::align_val_t) noexcept {
	if (!memory)
		return;

	recordFree(memory);
	_aligned_free(memory);
}

void operator delete[](void* memory, std::align_val_t) noexcept
{
	if (!memory)
		return;

	recordFree(memory);
	_aligned_free(memory);
}

// Sized, over-aligned deallocation overloads.
void operator delete(
	void* memory, std::size_t, std::align_val_t alignment) noexcept
{
	::operator delete(memory, alignment);
}

void operator delete[](
	void* memory, std::size_t, std::align_val_t alignment) noexcept
{
	::operator delete[](memory, alignment);
}

void writeMemoryLog()
{
	// Snapshot the tracker before using streams, which may allocate.
	std::size_t currentBytes;
	std::size_t currentCount;
	std::size_t allocationsMade;
	std::size_t freesMade;
	std::size_t peakBytes;
	std::size_t peakCount;
	std::size_t untracked;

	{
		LockGuard lock;
		currentBytes = usedMemoryTotal;
		currentCount = allocationCount;
		allocationsMade = totalAllocations;
		freesMade = totalFrees;
		peakBytes = peakMemory;
		peakCount = peakAllocations;
		untracked = untrackedAllocations;
	}

	std::ofstream logFile("MemoryMonitorLog.txt");

	if (!logFile)
		return;

	logFile << "HatRock Memory Monitor\n";
	logFile << "======================\n";
	logFile << "Total allocations: " << allocationsMade << '\n';
	logFile << "Total frees: " << freesMade << '\n';
	logFile << "Peak memory: " << peakBytes << " bytes\n";
	logFile << "Peak active allocations: " << peakCount << '\n';
	logFile << "Current tracked memory: " << currentBytes << " bytes\n";
	logFile << "Current tracked allocations: " << currentCount << '\n';
	logFile << "Allocations not recorded (table full): "
		<< untracked << '\n';

	logFile << "\nActive allocations at snapshot:\n";

	{
		LockGuard lock;

		for (std::size_t i = 0; i < allocationCount; ++i) {
			logFile << "ID: " << allocations[i].allocationID
				<< ", Address: " << allocations[i].address
				<< ", Size: " << allocations[i].size << " bytes\n";
		}
	}
}