#include "datastruct/mem.h"
#include "datastruct/memtbl.h"
#include "datastruct/str.h"
#include "unity.h"

memtbl_handle mth;

void setUp(void) {
  mth = memtbl_create(MEM_ALLOCATOR_PLAIN);
}

void tearDown(void) {
  memtbl_destroy(mth);
}

void test_StrFromCstr_IsValid(void) {
  str val = str_from_cstr("Some string");
  TEST_ASSERT_TRUE(str_is_valid(val));
}

void test_StrFromCstr_HasLength(void) {
  str val = str_from_cstr("Some string");
  TEST_ASSERT_EQUAL(11, str_length(val));
}

void test_StrDuplicateCStrWithAllocator_IsValidHasChars(void) {
  char cstr[] = "Some string";
  str val = str_duplicate_cstr_with_allocator(cstr, MEM_ALLOCATOR_PLAIN);
  TEST_ASSERT_TRUE(str_is_valid(val));
  TEST_ASSERT_NOT_EQUAL(cstr, val.data);
  TEST_ASSERT_EQUAL_MEMORY(cstr, val.data, 11);
}

void test_StrDuplicateStr_IsValidHasChars(void) {
  str mystr = str_from_cstr("Some string");
  str val = str_duplicate_str(mystr);
  TEST_ASSERT_TRUE(str_is_valid(val));
  TEST_ASSERT_NOT_EQUAL(mystr.data, val.data);
  TEST_ASSERT_EQUAL_MEMORY(mystr.data, val.data, 11);
}

void test_StrDuplicateStrbuf_IsValidHasChars(void) {
  strbuf_handle bufhdl = strbuf_create(MEM_ALLOCATOR_PLAIN, 64);
  TEST_ASSERT_TRUE(strbuf_concatenate_cstr(bufhdl, "one"));
  str val = str_duplicate_strbuf(bufhdl);
  char *bufdatap = mem_p(((strbuf *)mem_p(bufhdl))->data);
  char *valp = mem_p(val);
  TEST_ASSERT_TRUE(str_is_valid(val));
  TEST_ASSERT_NOT_EQUAL(bufdatap, valp);
  TEST_ASSERT_EQUAL_MEMORY(bufdatap, valp, 11);
}

void test_StrDuplicateCstr_NullCStr_IsInvalid(void) {
  str val = str_duplicate_cstr_with_allocator((char *)0, MEM_ALLOCATOR_PLAIN);
  TEST_ASSERT_FALSE(str_is_valid(val));
}

void test_StrDuplicate_InvalidStr_IsInvalid(void) {
  str mystr = (str){0};
  str val = str_duplicate_str(mystr);
  TEST_ASSERT_FALSE(str_is_valid(val));
}

void test_StrDuplicate_InvalidStrbuf_IsInvalid(void) {
  str val = str_duplicate_strbuf((strbuf_handle){0});
  TEST_ASSERT_FALSE(str_is_valid(val));
}

void test_StrDestroy_InvalidStr_RemainsInvalid(void) {
  str val = (str){0};
  str_destroy(val);
  TEST_ASSERT_FALSE(str_is_valid(val));
}

void test_StrDestroy_NonAllocated_Ok(void) {
  str val = str_from_cstr("Some string");
  TEST_ASSERT_TRUE(str_is_valid(val));
  str_destroy(val);
}

void test_StrDestroy_Allocated_Ok(void) {
  str mystr = str_from_cstr("Some string");
  str val = str_duplicate_str(mystr);
  TEST_ASSERT_TRUE(str_is_valid(val));
  str_destroy(val);
}

void test_StrWriteCstrToBuf_SmallerThanBuf_WritesBufMakesStr(void) {
  // (19 X and a null terminator.)
  char buf[20] = "XXXXXXXXXXXXXXXXXXX";
  str mystr = str_from_cstr("Some string");
  str val = str_write_cstr_to_buf(mystr, buf, sizeof(buf));
  TEST_ASSERT_TRUE(str_is_valid(val));
  TEST_ASSERT_EQUAL_MEMORY(mystr.data, buf, 12);
  TEST_ASSERT_EQUAL_PTR(buf, val.data);
  TEST_ASSERT_EQUAL(str_length(mystr), str_length(val));
  TEST_ASSERT_EQUAL('\0', buf[11]);
}

void test_StrWriteCstrToBuf_LargerThanBuf_EndsWithNull(void) {
  // (9 X and a null terminator.)
  char buf[10] = "XXXXXXXXX";
  str mystr = str_from_cstr("Some string");
  str val = str_write_cstr_to_buf(mystr, buf, sizeof(buf));
  TEST_ASSERT_TRUE(str_is_valid(val));
  TEST_ASSERT_EQUAL_MEMORY(mystr.data, buf, 9);
  TEST_ASSERT_EQUAL_PTR(buf, val.data);
  TEST_ASSERT_EQUAL(9, str_length(val));
  TEST_ASSERT_EQUAL('\0', buf[9]);
}

void test_StrWriteCstrToBuf_InvalidStr_ReturnsInvalid() {
  char buf[20] = "XXXXXXXXXXXXXXXXXXX";
  str mystr = (str){0};
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

void test_StrCstr_StrInvalid_ReturnsNullPtr(void) {
  str val = (str){0};
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
  str haystack = (str){0};
  str needle = str_from_cstr("on");
  int pos = str_find(haystack, needle);
  TEST_ASSERT_EQUAL(-1, pos);

  haystack = str_from_cstr("one two three four");
  needle = (str){0};
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
  TEST_ASSERT_EQUAL(-1, str_compare((str){0}, str_from_cstr("compare")));
}

void test_StrCompare_SecondInvalid_Pos1(void) {
  TEST_ASSERT_EQUAL(1, str_compare(str_from_cstr("compare"), (str){0}));
}

void test_StrCompare_BothInvalid_Zero(void) {
  TEST_ASSERT_EQUAL(0, str_compare((str){0}, (str){0}));
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
  char *expected[] = {"one", "two", "three", "four"};
  int expected_count = 4;
  int expected_i = 0;
  str val = str_from_cstr("one,two,three,four");
  str delim = str_from_cstr(",");
  str part;

  while (str_is_valid(val)) {
    TEST_ASSERT_TRUE(expected_i < expected_count);
    val = str_split_pop(val, delim, &part);
    TEST_ASSERT_EQUAL(0,
                      str_compare(str_from_cstr(expected[expected_i]), part));
    ++expected_i;
  }
}

void assert_whitespace_pop(char *cstr, char **expected, int expected_count) {
  str val = str_from_cstr(cstr);
  int expected_i = 0;
  str part;
  while (str_is_valid(val)) {
    TEST_ASSERT_TRUE(expected_i < expected_count);
    val = str_split_whitespace_pop(val, &part);
    TEST_ASSERT_EQUAL(0,
                      str_compare(str_from_cstr(expected[expected_i]), part));
    ++expected_i;
  }
  TEST_ASSERT_EQUAL(expected_count, expected_i);
}

void test_StrSplitWhitespacePop_NoSpace_PopsOnce(void) {
  assert_whitespace_pop("one", (char *[]){"one"}, 1);
}

void test_StrSplitWhitespacePop_OneSpaceInMiddle_PopsTwice(void) {
  assert_whitespace_pop("one two", (char *[]){"one", "two"}, 2);
}

void test_StrSplitWhitespacePop_MultipleSpacesInMiddle_PopsTwice(void) {
  assert_whitespace_pop("one \t \n two", (char *[]){"one", "two"}, 2);
}

void test_StrSplitWhitespacePop_MultipleSpacesAtBeginning_PopsOnce(void) {
  assert_whitespace_pop(" \t \n one", (char *[]){"one"}, 1);
}

void test_StrSplitWhitespacePop_MultipleSpacesAtEnd_PopsOnce(void) {
  assert_whitespace_pop("one \t \n ", (char *[]){"one"}, 1);
}

void test_StrSplitWhitespacePop_OnlySpace_PopsEmptyString(void) {
  assert_whitespace_pop(" \t \n ", (char *[]){""}, 1);
}

void test_StrSplitWhitespacePop_LeadingInterTrailingSpace_ReturnsExpectedStrings(
    void) {
  assert_whitespace_pop("  one  \t\t \ntwo three\t\v \n ",
                        (char *[]){"one", "two", "three"}, 3);
}

void test_StrbufCreate_CreatesValid_DestroyOk(void) {
  strbuf_handle bufhdl = strbuf_create(MEM_ALLOCATOR_PLAIN, 64);
  TEST_ASSERT_TRUE(strbuf_is_valid(bufhdl));
  strbuf_destroy(bufhdl);
}

void test_StrbufDuplicate_Valid_CreatesValid(void) {
  strbuf_handle bufhdl = strbuf_create(MEM_ALLOCATOR_PLAIN, 64);
  strbuf_handle duphdl = strbuf_duplicate(bufhdl);
  TEST_ASSERT_TRUE(strbuf_is_valid(duphdl));
  strbuf_destroy(bufhdl);
  TEST_ASSERT_TRUE(strbuf_is_valid(duphdl));
  strbuf_destroy(duphdl);
}

void test_StrbufConcatenateChar(void) {
  strbuf_handle bufhdl = strbuf_create(MEM_ALLOCATOR_PLAIN, 64);
  TEST_ASSERT_TRUE(strbuf_is_valid(bufhdl));
  TEST_ASSERT_TRUE(strbuf_concatenate_char(bufhdl, 'a'));
  TEST_ASSERT_TRUE(strbuf_concatenate_char(bufhdl, 'b'));
  TEST_ASSERT_TRUE(strbuf_concatenate_char(bufhdl, 'c'));
  str result = strbuf_str(bufhdl);
  TEST_ASSERT_EQUAL_MEMORY("abc", mem_p(result), 3);
  TEST_ASSERT_EQUAL(3, str_length(result));
}

void test_StrbufConcatenateCstr(void) {
  strbuf_handle bufhdl = strbuf_create(MEM_ALLOCATOR_PLAIN, 64);
  TEST_ASSERT_TRUE(strbuf_concatenate_cstr(bufhdl, "one"));
  str result = strbuf_str(bufhdl);
  TEST_ASSERT_EQUAL_MEMORY("one", mem_p(result), 3);
  TEST_ASSERT_EQUAL(3, str_length(result));
  TEST_ASSERT_EQUAL(64, ((strbuf *)mem_p(bufhdl))->data.size);

  TEST_ASSERT_TRUE(strbuf_concatenate_cstr(bufhdl, "two"));
  TEST_ASSERT_TRUE(strbuf_concatenate_cstr(bufhdl, "three"));
  result = strbuf_str(bufhdl);
  TEST_ASSERT_EQUAL_MEMORY("onetwothree", mem_p(result), 11);
  TEST_ASSERT_EQUAL(11, str_length(result));
  TEST_ASSERT_EQUAL(64, ((strbuf *)mem_p(bufhdl))->data.size);
}

void test_StrbufConcatenateStr(void) {
  strbuf_handle bufhdl = strbuf_create(MEM_ALLOCATOR_PLAIN, 64);
  TEST_ASSERT_TRUE(strbuf_concatenate_str(bufhdl, str_from_cstr("one")));
  str result = strbuf_str(bufhdl);
  TEST_ASSERT_EQUAL_MEMORY("one", mem_p(result), 3);
  TEST_ASSERT_EQUAL(3, str_length(result));
  TEST_ASSERT_EQUAL(64, ((strbuf *)mem_p(bufhdl))->data.size);

  TEST_ASSERT_TRUE(strbuf_concatenate_str(bufhdl, str_from_cstr("two")));
  TEST_ASSERT_TRUE(strbuf_concatenate_str(bufhdl, str_from_cstr("three")));
  result = strbuf_str(bufhdl);
  TEST_ASSERT_EQUAL_MEMORY("onetwothree", mem_p(result), 11);
  TEST_ASSERT_EQUAL(11, str_length(result));
  TEST_ASSERT_EQUAL(64, ((strbuf *)mem_p(bufhdl))->data.size);
}

void test_StrbufConcatenateStrbuf(void) {
  strbuf_handle bufhdl = strbuf_create(MEM_ALLOCATOR_PLAIN, 64);
  TEST_ASSERT_TRUE(strbuf_concatenate_str(bufhdl, str_from_cstr("one")));

  strbuf_handle bufhdl2 = strbuf_create(MEM_ALLOCATOR_PLAIN, 64);
  TEST_ASSERT_TRUE(strbuf_concatenate_str(bufhdl, str_from_cstr("two")));

  TEST_ASSERT_TRUE(strbuf_concatenate_strbuf(bufhdl, bufhdl2));
  str result = strbuf_str(bufhdl);
  TEST_ASSERT_EQUAL_MEMORY("onetwo", mem_p(result), 6);
  TEST_ASSERT_EQUAL(6, str_length(result));
  TEST_ASSERT_EQUAL(64, ((strbuf *)mem_p(bufhdl))->data.size);
}

void test_StrbufConcatenate_Overflow_Grows(void) {
  strbuf_handle bufhdl = strbuf_create(MEM_ALLOCATOR_PLAIN, 16);
  str result = strbuf_str(bufhdl);
  TEST_ASSERT_EQUAL(0, str_length(result));
  TEST_ASSERT_EQUAL(16, ((strbuf *)mem_p(bufhdl))->data.size);
  // (16 X)
  TEST_ASSERT_TRUE(
      strbuf_concatenate_str(bufhdl, str_from_cstr("XXXXXXXXXXXXXXXX")));
  result = strbuf_str(bufhdl);
  TEST_ASSERT_EQUAL(16, str_length(result));
  TEST_ASSERT_EQUAL(16, ((strbuf *)mem_p(bufhdl))->data.size);
  TEST_ASSERT_TRUE(strbuf_concatenate_str(bufhdl, str_from_cstr("X")));
  result = strbuf_str(bufhdl);
  TEST_ASSERT_EQUAL(17, str_length(result));
  TEST_ASSERT_EQUAL(32, ((strbuf *)mem_p(bufhdl))->data.size);
}

void test_StrbufConcatenate_InvalidDestStrbuf_Fails(void) {
  TEST_ASSERT_FALSE(strbuf_concatenate_cstr((strbuf_handle){0}, "one"));
}

void test_StrbufConcatenate_NullCstr_Fails(void) {
  strbuf_handle bufhdl = strbuf_create(MEM_ALLOCATOR_PLAIN, 64);
  TEST_ASSERT_FALSE(strbuf_concatenate_cstr(bufhdl, (char *)0));
}

void test_StrbufConcatenate_InvalidStr_Fails(void) {
  strbuf_handle bufhdl = strbuf_create(MEM_ALLOCATOR_PLAIN, 64);
  TEST_ASSERT_FALSE(strbuf_concatenate_str(bufhdl, (str){0}));
}

void test_StrbufConcatenate_InvalidInputStrbuf_Fails(void) {
  strbuf_handle bufhdl = strbuf_create(MEM_ALLOCATOR_PLAIN, 64);
  TEST_ASSERT_FALSE(strbuf_concatenate_strbuf(bufhdl, (strbuf_handle){0}));
}

void test_StrbufConcatenatePrintf_Succeeds(void) {
  strbuf_handle bufhdl = strbuf_create(MEM_ALLOCATOR_PLAIN, 16);
  TEST_ASSERT_TRUE(strbuf_concatenate_printf(bufhdl, "This is a test: %d (%s)",
                                             123, "message"));
  str result = strbuf_str(bufhdl);
  TEST_ASSERT_EQUAL_MEMORY("This is a test: 123 (message)", mem_p(result), 29);
  TEST_ASSERT_EQUAL(29, str_length(result));
  TEST_ASSERT_EQUAL(32, ((strbuf *)mem_p(bufhdl))->data.size);
}

void test_StrbufReset_AllowsReuse(void) {
  strbuf_handle bufhdl = strbuf_create(MEM_ALLOCATOR_PLAIN, 64);
  TEST_ASSERT_TRUE(strbuf_concatenate_cstr(bufhdl, "one"));
  TEST_ASSERT_TRUE(strbuf_concatenate_cstr(bufhdl, "two"));
  TEST_ASSERT_TRUE(strbuf_concatenate_cstr(bufhdl, "three"));
  str result = strbuf_str(bufhdl);
  TEST_ASSERT_EQUAL_MEMORY("onetwothree", mem_p(result), 11);
  TEST_ASSERT_EQUAL(11, str_length(result));

  strbuf_reset(bufhdl);
  result = strbuf_str(bufhdl);
  TEST_ASSERT_EQUAL(0, str_length(result));
  TEST_ASSERT_TRUE(strbuf_concatenate_cstr(bufhdl, "four"));
  TEST_ASSERT_TRUE(strbuf_concatenate_cstr(bufhdl, "five"));
  result = strbuf_str(bufhdl);
  TEST_ASSERT_EQUAL_MEMORY("fourfive", mem_p(result), 8);
  TEST_ASSERT_EQUAL(8, str_length(result));
  TEST_ASSERT_TRUE(strbuf_concatenate_cstr(bufhdl, "six"));
  result = strbuf_str(bufhdl);
  TEST_ASSERT_EQUAL_MEMORY("fourfivesix", mem_p(result), 11);
  TEST_ASSERT_EQUAL(11, str_length(result));
}
