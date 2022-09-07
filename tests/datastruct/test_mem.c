#include "datastruct/mem.h"
#include "unity.h"

void test_MemHandleFromPtr_RepresentsMemory(void) {}
void test_MemAlloc_PlainAllocator_AllocatesMemory(void) {}
void test_MemAlloc_MemtblAllocator_AllocatesMemory(void) {}
void test_MemAlloc_MemtblAllocator_RestoresSigintHandler(void) {}
void test_MemAlloc_InvalidAllocator_ReturnsInvalidMemHandle(void) {}
void test_MemAllocClear_PlainAllocator_AllocatesClearMemory(void) {}
void test_MemAllocClear_MemtblAllocator_AllocatesClearMemory(void) {}
void test_MemAllocClear_InvalidAllocator_ReturnsInvalidMemHandle(void) {}
void test_MemRealloc_PlainAllocator_ReallocatesMemory(void) {}
void test_MemRealloc_MemtblAllocator_ReallocatesMemory(void) {}
void test_MemRealloc_MemtblAllocator_RestoresSigintHandler(void) {}
void test_MemRealloc_InvalidHandle_ReturnsInvalidHandle(void) {}
void test_MemP_FromPtr_ReturnsPointer(void) {}
void test_MemP_PlainAllocator_ReturnsPointer(void) {}
void test_MemP_MemtblAllocator_ReturnsPointer(void) {}
void test_MemP_InvalidHandle_ReturnsNullPointer(void) {}
void test_MemP_MemtblAllocatorFreedMemory_ReturnsNullPointer(void) {}
void test_MemDuplicate_PlainAllocatedHandle_ReturnsPlainDuplicate(void) {}
void test_MemDuplicate_MemtblAllocatedHandle_ReturnsMemtblDuplicate(void) {}
void test_MemDuplicateWithAllocator_FromPtr_ReturnsAllocatedHandle(void) {}
