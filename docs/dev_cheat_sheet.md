# m65tool developer cheat sheet

## Build commands

```text
autoreconf --install

python3 scripts/makemake.py

./configure
./configure --enable-silent-rules

make

make check
make check TESTS='tests/runners/runner_test_scoreboard'

./configure --enable-coverage
make check-code-coverage

make distcheck

make clean
make distclean
python3 scripts/superclean.py

python3 scripts/newmod.py {modname}
python3 scripts/newmod.py --program {modname}

python3 scripts/build.py
python3 scripts/build.py --debugbuild
python3 scripts/build.py --windows
```

## VSCode

[VSCode and C++](https://code.visualstudio.com/docs/cpp/introvideos-cpp).

- Cmd+Shift+F : Format file
- F5 : Start Debugging
  - F5 : Continue
  - F10 : Step Over
  - F11 : Step Into
  - Shift+F11 : Step Out Of
  - Cmd+Shift+F5 : Restart
  - Shift+F5 : Stop

## Unity Test

[Reference](https://github.com/ThrowTheSwitch/Unity/blob/master/docs/UnityAssertionsReference.md).

To force a test result:

```c
TEST_FAIL ();
TEST_FAIL_MESSAGE ( message );

TEST_PASS ();
TEST_PASS_MESSAGE ( message );

TEST_IGNORE ();
TEST_IGNORE_MESSAGE ( message );
```

To just print a message:

```c
TEST_MESSAGE ( message );
```

All assertion forms take an optional message:

```c
TEST_ASSERT_X ( {modifiers}, {expected}, actual, {size/count} );
TEST_ASSERT_X_MESSAGE ( {modifiers}, {expected}, actual, {size/count}, message );

// For example:
TEST_ASSERT_EQUAL_UINT16_MESSAGE(0xff01, myval, "I/O mask must enable ZMODE");
```

Boolean assertions:

```c
TEST_ASSERT ( condition );
TEST_ASSERT_TRUE ( condition );
TEST_ASSERT_FALSE ( condition );
TEST_ASSERT_UNLESS ( condition );      // synonym for ASSERT_FALSE
TEST_ASSERT_NULL ( pointer );          // pointer is NULL
TEST_ASSERT_NOT_NULL ( pointer );
TEST_ASSERT_EMPTY ( pointer );         // *pointer is NULL
TEST_ASSERT_NOT_EMPTY ( pointer );
```

Integer value assertions have a type in the name:

- Signed integer types: `INT`, `INT8`, `INT16`, `INT32`, `INT64`
- Unsigned integer types: `UINT`, `UINT8`, `UINT16`, `UINT32`, `UINT64`
- Unsigned integer types, hexadecimal output: `HEX`, `HEX8`, `HEX16`,
  `HEX32`, `HEX64`
- Unsigned 8-bit integer, character output: `CHAR`

Equality / inequality assertions:

```c
// Exact integer equality
TEST_ASSERT_EQUAL_X ( expected, actual );

// Exact integer inequality
TEST_ASSERT_GREATER_THAN_X ( threshold, actual );
TEST_ASSERT_GREATER_OR_EQUAL_X ( threshold, actual );
TEST_ASSERT_LESS_THAN_X ( threshold, actual );
TEST_ASSERT_LESS_OR_EQUAL_X ( threshold, actual );
TEST_ASSERT_NOT_EQUAL_X ( threshold, actual );

// Inexact integer equality
TEST_ASSERT_X_WITHIN ( delta, expected, actual );
TEST_ASSERT_X_NOT_WITHIN ( delta, expected, actual );
```

Bit pattern assertions:

```c
TEST_ASSERT_BITS ( mask, expected, actual );
TEST_ASSERT_BITS_HIGH ( mask, actual );
TEST_ASSERT_BITS_LOW ( mask, actual );
TEST_ASSERT_BIT_HIGH ( bit, actual );
TEST_ASSERT_BIT_LOW ( bit, actual );
```

Structs and strings:

```c
TEST_ASSERT_EQUAL_PTR ( expected, actual );
TEST_ASSERT_EQUAL_STRING ( expected, actual );   // assumes NULL terminated
TEST_ASSERT_EQUAL_MEMORY ( expected, actual, len );
```

Array equality / inequality assertions:

```c
// Exact integer equality of two arrays
TEST_ASSERT_EQUAL_X_ARRAY ( expected_array, actual_array, num_elements );

// ... Array of memory takes a (single) memory length:
TEST_ASSERT_EQUAL_MEMORY_ARRAY ( expected_array, actual_array, element_len, num_elements );

// Exact integer inequality of two arrays
TEST_ASSERT_GREATER_THAN_X_ARRAY ( threshold_array, actual_array, num_elements );
TEST_ASSERT_GREATER_OR_EQUAL_X_ARRAY ( threshold_array, actual_array, num_elements );
TEST_ASSERT_LESS_THAN_X_ARRAY ( threshold_array, actual_array, num_elements );
TEST_ASSERT_LESS_OR_EQUAL_X_ARRAY ( threshold_array, actual_array, num_elements );
TEST_ASSERT_NOT_EQUAL_X_ARRAY ( threshold_array, actual_array, num_elements );

// Inexact integer equality of two arrays
TEST_ASSERT_X_ARRAY_WITHIN ( delta, expected_array, actual_array );

// Exact integer equality of each element of an array and a single value
TEST_ASSERT_EACH_EQUAL_X ( expected_array, actual_element, num_elements );

// ... Array of memory takes a (single) memory length:
TEST_ASSERT_EACH_EQUAL_MEMORY ( expected_array, actual_element, element_len, num_elements );
```

Floats and doubles assertions:

- `FLOAT` and `DOUBLE` work as types in all assertion forms.
- Equality is never exact, and uses dynamic delta based on the order of
  magnitude of the expected value, equivalent to a single bit of error.
- Use `WITHIN` forms to select a custom fixed delta.

```c
TEST_ASSERT_X_IS_INF ( actual );
TEST_ASSERT_X_IS_NEG_INF ( actual );
TEST_ASSERT_X_IS_NAN ( actual );
TEST_ASSERT_X_IS_DETERMINATE ( actual );  // not inf, -inf, or NaN

TEST_ASSERT_X_IS_NOT_INF ( actual );
TEST_ASSERT_X_IS_NOT_NEG_INF ( actual );
TEST_ASSERT_X_IS_NOT_NAN ( actual );
TEST_ASSERT_X_IS_NOT_DETERMINATE ( actual );  // inf, -inf, or NaN
```

## CMock

[Reference](https://github.com/ThrowTheSwitch/CMock/blob/master/docs/CMock_Summary.md).

Every function call from the module under test to a mocked dependency must have
an expectation set first, so the mock knows how to respond and how to report a
missed expectation. CMock generates expectation functions for every mocked
dependency function based on its arguments and return values.

To expect a function named `func` to be called:

```c
// void func(...)
func_Expect();
func_Expect(expected_params);
func_ExpectAnyArgs();

// retval func(...)
func_ExpectAndReturn(retval_to_return);
func_ExpectAndReturn(expected_params, retval_to_return);
func_ExpectAnyArgsAndReturn(retval_to_return);
```

Call `AndReturn` functions more than once to enqueue multiple return values.

To set expectations for some arguments but ignore others, call an `Expect` first,
then call `IgnoreArg` followed by the name of the parameter. This is especially
useful for pointer parameters whose values are sometimes unpredictable during
tests.

```c
func_IgnoreArg_param();
```

If `func` has a pointer parameter, `WithArray` expectation functions take
the expected pointer and a number of elements to test the parameter value as an
array.

```c
func_ExpectWithArray(ptr* param, int param_depth, other);
func_ExpectWithArrayAndReturn(other, ptr* param, int param_depth, retval_to_return)
```

To allow but otherwise ignore calls to a function, optionally setting a return value:

```c
func_Ignore();
func_IgnoreAndReturn(retval_to_return);
```

To cancel an Ignore directive for the next part of the test, you can call an
Expect, or `StopIgnore`:

```c
func_StopIgnore();
```

If `func` accepts a pointer parameter for the purpose of returning a value
through the pointer, use `ReturnThruPtr` to cause the mock to set that value.
As with `AndReturn`, you can call this more than once to enqueue return values.
Note that you usually want to ignore arg values (`ExpectAnyArgs`, `IgnoreArg`)
in this case because they will need to be actual writable addresses during the
test.

```c
func_ReturnThruPtr_param(val_to_return);
func_ReturnArrayThruPtr_param(val_to_return, len);
func_ReturnMemThruPtr_param(val_to_return, size);
```

To have the mock call a custom callback function, define the callback function
with the same signature as the function being mocked plus an additional `int`
argument that takes the number of times the mock has been called. Register the
callback with either:

- `func_AddCallback(callback)` : perform all Expect logic then call the callback
- `func_Stub(callback)` : skip Expects and just call the callback

If the function being mocked can throw CExceptions (a facility from the makers
of CMock and Unity Test), the mock can be set up to throw an exception:

- `func_ExpectAndThrow(expected_params..., value_to_throw)`
