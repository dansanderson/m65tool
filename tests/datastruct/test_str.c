#include "datastruct/memlist.h"
#include "datastruct/str.h"
#include "unity.h"

static str STR_INVALID = (str){0};
static strbuf STRBUF_INVALID = (strbuf){0};
static memlist MEMLIST_INVALID = (memlist){0};

memlist ml;

void setUp(void) {
  ml = memlist_create();
}

void tearDown(void) {
  memlist_destroy(&ml);
}

void test_StrFromCstr_IsValid(void) {
  str val = str_from_cstr("Some string");
  TEST_ASSERT_TRUE(str_is_valid(val));
}

void test_StrFromCstr_HasLength(void) {
  str val = str_from_cstr("Some string");
  TEST_ASSERT_EQUAL(11, str_length(val));
}

void test_StrFromBytes_IsValid(void) {
  char buf[] = "Some\0string";
  str val = str_from_bytes(buf, sizeof(buf));
  TEST_ASSERT_TRUE(str_is_valid(val));
}

void test_StrFromBytes_HasLength(void) {
  char buf[] = "Some\0string";
  str val = str_from_bytes(buf, sizeof(buf));
  // 11 chars plus the C string null terminator is 12
  TEST_ASSERT_EQUAL(12, str_length(val));
}

void test_StrDuplicate_CStr_IsValidHasChars(void) {
  char cstr[] = "Some string";
  str val = str_duplicate(cstr);
  TEST_ASSERT_TRUE(str_is_valid(val));
  TEST_ASSERT_NOT_EQUAL(cstr, val.value);
  TEST_ASSERT_EQUAL_MEMORY(cstr, val.value, 11);
}

void test_StrDuplicate_Str_IsValidHasChars(void) {
  str mystr = str_from_cstr("Some string");
  str val = str_duplicate(mystr);
  TEST_ASSERT_TRUE(str_is_valid(val));
  TEST_ASSERT_NOT_EQUAL(mystr.value, val.value);
  TEST_ASSERT_EQUAL_MEMORY(mystr.value, val.value, 11);
}

void test_StrDuplicate_Strbuf_IsValidHasChars(void) {
  strbuf buf = strbuf_create(64);
  TEST_ASSERT_TRUE(strbuf_concatenate(&buf, "one"));
  str val = str_duplicate(buf);
  TEST_ASSERT_TRUE(str_is_valid(val));
  TEST_ASSERT_NOT_EQUAL(buf.value, val.value);
  TEST_ASSERT_EQUAL_MEMORY(buf.value, val.value, 11);
}

void test_StrDuplicate_NullCStr_IsInvalid(void) {
  str val = str_duplicate((char *)0);
  TEST_ASSERT_FALSE(str_is_valid(val));
}

void test_StrDuplicate_InvalidStr_IsInvalid(void) {
  str mystr = STR_INVALID;
  str val = str_duplicate(mystr);
  TEST_ASSERT_FALSE(str_is_valid(val));
}

void test_StrDuplicate_InvalidStrbuf_IsInvalid(void) {
  str val = str_duplicate(STRBUF_INVALID);
  TEST_ASSERT_FALSE(str_is_valid(val));
}

void test_StrDuplicateToMemlist_CstrValidMemlist_UsesMemlist(void) {
  char cstr[] = "Some string";
  str val = str_duplicate_to_memlist(cstr, &ml);
  TEST_ASSERT_EQUAL(1, ml.next_index);
  TEST_ASSERT_NOT_EQUAL(cstr, ml.ptrlist[0]);
  TEST_ASSERT_EQUAL_MEMORY(cstr, ml.ptrlist[0], 11);
}

void test_StrDuplicateToMemlist_InvalidMemlist_IsInvalid(void) {
  char cstr[] = "Some string";
  str val = str_duplicate_to_memlist(cstr, &MEMLIST_INVALID);
  TEST_ASSERT_FALSE(str_is_valid(val));
}

void test_StrDestroy_InvalidStr_RemainsInvalid(void) {
  str val = STR_INVALID;
  str_destroy(&val);
  TEST_ASSERT_FALSE(str_is_valid(val));
}

void test_StrDestroy_NonAllocated_MakesInvalid(void) {
  str val = str_from_cstr("Some string");
  TEST_ASSERT_TRUE(str_is_valid(val));
  str_destroy(&val);
  TEST_ASSERT_FALSE(str_is_valid(val));
}

void test_StrDestroy_Allocated_MakesInvalid(void) {
  str mystr = str_from_cstr("Some string");
  str val = str_duplicate(mystr);
  TEST_ASSERT_TRUE(str_is_valid(val));
  str_destroy(&val);
  TEST_ASSERT_FALSE(str_is_valid(val));
}

void test_StrWriteCstrToBuf_SmallerThanBuf_WritesBufMakesStr(void) {
  // (19 X and a null terminator.)
  char buf[20] = "XXXXXXXXXXXXXXXXXXX";
  str mystr = str_from_cstr("Some string");
  str val = str_write_cstr_to_buf(mystr, buf, sizeof(buf));
  TEST_ASSERT_TRUE(str_is_valid(val));
  TEST_ASSERT_EQUAL_MEMORY(mystr.value, buf, 12);
  TEST_ASSERT_EQUAL_PTR(buf, val.value);
  TEST_ASSERT_EQUAL(str_length(mystr), str_length(val));
  TEST_ASSERT_EQUAL('\0', buf[11]);
}

void test_StrWriteCstrToBuf_LargerThanBuf_EndsWithNull(void) {
  // (9 X and a null terminator.)
  char buf[10] = "XXXXXXXXX";
  str mystr = str_from_cstr("Some string");
  str val = str_write_cstr_to_buf(mystr, buf, sizeof(buf));
  TEST_ASSERT_TRUE(str_is_valid(val));
  TEST_ASSERT_EQUAL_MEMORY(mystr.value, buf, 9);
  TEST_ASSERT_EQUAL_PTR(buf, val.value);
  TEST_ASSERT_EQUAL(9, str_length(val));
  TEST_ASSERT_EQUAL('\0', buf[9]);
}

void test_StrWriteCstrToBuf_InvalidStr_ReturnsInvalid() {
  char buf[20] = "XXXXXXXXXXXXXXXXXXX";
  str mystr = STR_INVALID;
  str val = str_write_cstr_to_buf(mystr, buf, sizeof(buf));
  TEST_ASSERT_FALSE(str_is_valid(val));
}

void test_StrWriteCstrToBuf_NullPtr_ReturnsInvalid() {
  str mystr = str_from_cstr("Some string");
  str val = str_write_cstr_to_buf(mystr, (void *)0, 0);
  TEST_ASSERT_FALSE(str_is_valid(val));
}

void test_StrCstr_ValidStr_ReturnsPtrToString(void) {
  char cstr[] = "Some string";
  str val = str_from_cstr(cstr);
  char *result = str_cstr(val);
  TEST_ASSERT_NOT_EQUAL(cstr, result);
  TEST_ASSERT_EQUAL_MEMORY(cstr, result, 12);
}

void test_StrCSTR_INVALIDStr_ReturnsNullPtr(void) {
  str val = STR_INVALID;
  char *result = str_cstr(val);
  TEST_ASSERT_NULL(result);
}

void test_StrFind_SubstringInStringBeginning_Found(void) {
  str haystack = str_from_cstr("one two three four");
  str needle = str_from_cstr("on");
  int pos = str_find(haystack, needle);
  TEST_ASSERT_EQUAL(0, pos);
}

void test_StrFind_SubstringInStringMiddle_Found(void) {
  str haystack = str_from_cstr("one two three four");
  str needle = str_from_cstr("wo th");
  int pos = str_find(haystack, needle);
  TEST_ASSERT_EQUAL(5, pos);
}

void test_StrFind_SubstringInStringEnd_Found(void) {
  str haystack = str_from_cstr("one two three four");
  str needle = str_from_cstr("our");
  int pos = str_find(haystack, needle);
  TEST_ASSERT_EQUAL(15, pos);
}

void test_StrFind_SubstringInStringTwice_FindsLeftmost(void) {
  str haystack = str_from_cstr("one two one two");
  str needle = str_from_cstr("e t");
  int pos = str_find(haystack, needle);
  TEST_ASSERT_EQUAL(2, pos);
}

void test_StrFind_SubstringNotInString_NotFound(void) {
  str haystack = str_from_cstr("one two three four");
  str needle = str_from_cstr("threz");
  int pos = str_find(haystack, needle);
  TEST_ASSERT_EQUAL(-1, pos);
}

void test_StrFind_InvalidInputs_NotFound(void) {
  str haystack = STR_INVALID;
  str needle = str_from_cstr("on");
  int pos = str_find(haystack, needle);
  TEST_ASSERT_EQUAL(-1, pos);

  haystack = str_from_cstr("one two three four");
  needle = STR_INVALID;
  pos = str_find(haystack, needle);
  TEST_ASSERT_EQUAL(-1, pos);
}

void test_StrFind_DelimLongerThanStr_NotFound(void) {
  str haystack = str_from_cstr("thr");
  str needle = str_from_cstr("three");
  int pos = str_find(haystack, needle);
  TEST_ASSERT_EQUAL(-1, pos);
}

void test_StrCompare_FirstLessThanSecond_Neg1(void) {
  TEST_ASSERT_EQUAL(
      -1, str_compare(str_from_cstr("camper"), str_from_cstr("compare")));
}

void test_StrCompare_FirstGreaterThanSecond_Pos1(void) {
  TEST_ASSERT_EQUAL(
      1, str_compare(str_from_cstr("compare"), str_from_cstr("camper")));
}

void test_StrCompare_FirstEqualToSecond_Zero(void) {
  TEST_ASSERT_EQUAL(
      0, str_compare(str_from_cstr("compare"), str_from_cstr("compare")));
}

void test_StrCompare_FirstShorterThanSecond_Neg1(void) {
  TEST_ASSERT_EQUAL(
      -1, str_compare(str_from_cstr("comp"), str_from_cstr("compare")));
}

void test_StrCompare_FirstLongerThanSecond_Pos1(void) {
  TEST_ASSERT_EQUAL(
      1, str_compare(str_from_cstr("compare"), str_from_cstr("comp")));
}

void test_StrCompare_FirstInvalid_Neg1(void) {
  TEST_ASSERT_EQUAL(-1, str_compare(STR_INVALID, str_from_cstr("compare")));
}

void test_StrCompare_SecondInvalid_Pos1(void) {
  TEST_ASSERT_EQUAL(1, str_compare(str_from_cstr("compare"), STR_INVALID));
}

void test_StrCompare_BothInvalid_Zero(void) {
  TEST_ASSERT_EQUAL(0, str_compare(STR_INVALID, STR_INVALID));
}

void test_StrSplitPop_OnceInMiddle_PopsTwice(void) {
  str val = str_from_cstr("oneDELIMtwo");
  str delim = str_from_cstr("DELIM");
  str part;

  str first_pop = str_split_pop(val, delim, &part);
  TEST_ASSERT_TRUE(str_is_valid(first_pop));
  TEST_ASSERT_TRUE(str_is_valid(part));
  TEST_ASSERT_EQUAL(0, str_compare(str_from_cstr("one"), part));
  TEST_ASSERT_EQUAL(0, str_compare(str_from_cstr("two"), first_pop));

  str second_pop = str_split_pop(first_pop, delim, &part);
  TEST_ASSERT_FALSE(str_is_valid(second_pop));
  TEST_ASSERT_TRUE(str_is_valid(part));
  TEST_ASSERT_EQUAL(0, str_compare(str_from_cstr("two"), part));
}

void test_StrSplitPop_OnceAtBeginning_PopsTwiceFirstEmpty(void) {
  str val = str_from_cstr("DELIMtwo");
  str delim = str_from_cstr("DELIM");
  str part;

  str first_pop = str_split_pop(val, delim, &part);
  TEST_ASSERT_TRUE(str_is_valid(first_pop));
  TEST_ASSERT_TRUE(str_is_valid(part));
  TEST_ASSERT_EQUAL(0, str_compare(str_from_cstr(""), part));
  TEST_ASSERT_EQUAL(0, str_compare(str_from_cstr("two"), first_pop));

  str second_pop = str_split_pop(first_pop, delim, &part);
  TEST_ASSERT_FALSE(str_is_valid(second_pop));
  TEST_ASSERT_TRUE(str_is_valid(part));
  TEST_ASSERT_EQUAL(0, str_compare(str_from_cstr("two"), part));
}

void test_StrSplitPop_OnceAtEnd_PopsTwiceLastEmpty(void) {
  str val = str_from_cstr("oneDELIM");
  str delim = str_from_cstr("DELIM");
  str part;

  str first_pop = str_split_pop(val, delim, &part);
  TEST_ASSERT_TRUE(str_is_valid(first_pop));
  TEST_ASSERT_TRUE(str_is_valid(part));
  TEST_ASSERT_EQUAL(0, str_compare(str_from_cstr("one"), part));
  TEST_ASSERT_EQUAL(0, str_compare(str_from_cstr(""), first_pop));

  str second_pop = str_split_pop(first_pop, delim, &part);
  TEST_ASSERT_FALSE(str_is_valid(second_pop));
  TEST_ASSERT_TRUE(str_is_valid(part));
  TEST_ASSERT_EQUAL(0, str_compare(str_from_cstr(""), part));
}

void test_StrSplitPop_DelimNotFound_PopsOnce(void) {
  str val = str_from_cstr("onetwo");
  str delim = str_from_cstr("DELIM");
  str part;

  str first_pop = str_split_pop(val, delim, &part);
  TEST_ASSERT_FALSE(str_is_valid(first_pop));
  TEST_ASSERT_TRUE(str_is_valid(part));
  TEST_ASSERT_EQUAL(0, str_compare(str_from_cstr("onetwo"), part));
}

void test_StrSplitPop_ThreeDelim_PopsFourTimes(void) {
  char *expected[4] = {"one", "two", "three", "four"};
  int expected_i = 0;
  str val = str_from_cstr("one,two,three,four");
  str delim = str_from_cstr(",");
  str part;

  while (str_is_valid(val)) {
    TEST_ASSERT_TRUE(expected_i < 4);
    val = str_split_pop(val, delim, &part);
    TEST_ASSERT_EQUAL(0,
                      str_compare(str_from_cstr(expected[expected_i]), part));
    ++expected_i;
  }
}

void test_StrbufCreate_CreatesValid_DestroyMakesInvalid(void) {
  strbuf val = strbuf_create(64);
  TEST_ASSERT_TRUE(strbuf_is_valid(val));
  strbuf_destroy(&val);
  TEST_ASSERT_FALSE(strbuf_is_valid(val));
}

void test_StrbufDuplicate_Valid_CreatesValid(void) {
  strbuf val = strbuf_create(64);
  strbuf dup = strbuf_duplicate(val);
  TEST_ASSERT_TRUE(strbuf_is_valid(dup));
  strbuf_destroy(&val);
  TEST_ASSERT_TRUE(strbuf_is_valid(dup));
  strbuf_destroy(&dup);
  TEST_ASSERT_FALSE(strbuf_is_valid(dup));
}

void test_StrbufConcatenate_Cstr(void) {
  strbuf buf = strbuf_create(64);
  TEST_ASSERT_TRUE(strbuf_concatenate(&buf, "one"));
  TEST_ASSERT_EQUAL_MEMORY("one", buf.value, 3);
  TEST_ASSERT_EQUAL(3, buf.length);
  TEST_ASSERT_EQUAL(64, buf.bufsize);

  TEST_ASSERT_TRUE(strbuf_concatenate(&buf, "two"));
  TEST_ASSERT_TRUE(strbuf_concatenate(&buf, "three"));
  TEST_ASSERT_EQUAL_MEMORY("onetwothree", buf.value, 11);
  TEST_ASSERT_EQUAL(11, buf.length);
  TEST_ASSERT_EQUAL(64, buf.bufsize);
}

void test_StrbufConcatenate_Str(void) {
  strbuf buf = strbuf_create(64);
  TEST_ASSERT_TRUE(strbuf_concatenate(&buf, str_from_cstr("one")));
  TEST_ASSERT_EQUAL_MEMORY("one", buf.value, 3);
  TEST_ASSERT_EQUAL(3, buf.length);
  TEST_ASSERT_EQUAL(64, buf.bufsize);

  TEST_ASSERT_TRUE(strbuf_concatenate(&buf, str_from_cstr("two")));
  TEST_ASSERT_TRUE(strbuf_concatenate(&buf, str_from_cstr("three")));
  TEST_ASSERT_EQUAL_MEMORY("onetwothree", buf.value, 11);
  TEST_ASSERT_EQUAL(11, buf.length);
  TEST_ASSERT_EQUAL(64, buf.bufsize);
}

void test_StrbufConcatenate_Strbuf(void) {
  strbuf buf = strbuf_create(64);
  TEST_ASSERT_TRUE(strbuf_concatenate(&buf, "one"));

  strbuf bufval = strbuf_create(64);
  TEST_ASSERT_TRUE(strbuf_concatenate(&bufval, "two"));

  TEST_ASSERT_TRUE(strbuf_concatenate(&buf, bufval));
  TEST_ASSERT_EQUAL_MEMORY("onetwo", buf.value, 6);
  TEST_ASSERT_EQUAL(6, buf.length);
  TEST_ASSERT_EQUAL(64, buf.bufsize);
}

void test_StrbufConcatenate_Overflow_Grows(void) {
  strbuf buf = strbuf_create(16);
  TEST_ASSERT_EQUAL(0, buf.length);
  TEST_ASSERT_EQUAL(16, buf.bufsize);
  // (16 X)
  TEST_ASSERT_TRUE(strbuf_concatenate(&buf, str_from_cstr("XXXXXXXXXXXXXXXX")));
  TEST_ASSERT_EQUAL(16, buf.length);
  TEST_ASSERT_EQUAL(16, buf.bufsize);
  TEST_ASSERT_TRUE(strbuf_concatenate(&buf, str_from_cstr("X")));
  TEST_ASSERT_EQUAL(17, buf.length);
  TEST_ASSERT_EQUAL(32, buf.bufsize);
}

void test_StrbufConcatenate_InvalidDestStrbuf_Fails(void) {
  strbuf buf = strbuf_create(64);
  strbuf_destroy(&buf);
  TEST_ASSERT_FALSE(strbuf_concatenate(&buf, "one"));
}

void test_StrbufConcatenate_NullCstr_Fails(void) {
  strbuf buf = strbuf_create(64);
  TEST_ASSERT_FALSE(strbuf_concatenate(&buf, (char *)0));
}

void test_StrbufConcatenate_InvalidStr_Fails(void) {
  strbuf buf = strbuf_create(64);
  TEST_ASSERT_FALSE(strbuf_concatenate(&buf, STR_INVALID));
}

void test_StrbufConcatenate_InvalidInputStrbuf_Fails(void) {
  strbuf buf = strbuf_create(64);
  TEST_ASSERT_FALSE(strbuf_concatenate(&buf, STRBUF_INVALID));
}

void test_StrbufConcatenatePrintf_Succeeds(void) {
  strbuf buf = strbuf_create(16);
  TEST_ASSERT_TRUE(strbuf_concatenate_printf(&buf, "This is a test: %d (%s)",
                                             123, "message"));
  TEST_ASSERT_EQUAL_MEMORY("This is a test: 123 (message)", buf.value, 29);
  TEST_ASSERT_EQUAL(29, buf.length);
  TEST_ASSERT_EQUAL(32, buf.bufsize);
}
