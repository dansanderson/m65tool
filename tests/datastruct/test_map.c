#include "datastruct/map.h"
#include "unity.h"

void test_MapGet_StrKey_FindsValue(void) {}
void test_MapGet_Uint32Key_FindsValue(void) {}
void test_MapGet_UnsetStrKey_ReturnsInvalidHandle(void) {}
void test_MapGet_UnsetUint32Key_ReturnsInvalidHandle(void) {}
void test_MapSet_ExistingKey_ReplacesValue(void) {}
void test_MapSet_17Keys_GrowsTable(void) {}
void test_MapDelete_ExistingStrKey_UnsetsKey(void) {}
void test_MapDelete_ExistingUint32Key_UnsetsKey(void) {}
void test_MapDelete_UnsetStrKey_DoesNothing(void) {}
void test_MapDelete_AfterGrowth_ShrinksTable(void) {}
void test_MapIter_NonEmptyMap_ReturnsAllValues(void) {}
void test_MapIter_EmptyMap_ReturnsDoneIteratorFirst(void) {}
