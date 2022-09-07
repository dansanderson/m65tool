# datastruct

This is a collection of data structure types and routines based around a memory
management system. This system can allocate memory using a _memory table_, an
object that keeps track of all allocations and can free un-freed allocations
all at once. The same memory system can be used for regular heap allocations
and non-allocated memory, so data structures can refer to any memory management
style.

This module has no dependencies and performs no I/O. It can be used as a non-mock library in tests.

## Concepts

A **memory handle** refers to a region of memory. It stores the starting
address, size, and whether it was allocated dynamically by this library. It
also keeps track of how it was allocated. A handle is an immutable value
similar to a pointer, and should be discarded when memory is reallocated or
freed.

A **memory allocator** describes how a region of memory is managed. It can be
managed by a memory table, allocated directly on the heap, or represent memory
managed outside of the system (such as literals in the program's data region).
A memory handle remembers its allocator, so features can use the same allocator
when creating related data objects.

A data object has **validity**. datastruct functions fail gracefully when given
an invalid object, and return an invalid object when there is a problem. For
example, if a memory allocation fails, it returns an invalid memory handle.
Functions are available to test objects for validity. It is always possible to
construct an invalid object by initializing a struct with all zeroes (such as
`(mem_handle){0}`).

## Managing memory

## Memory tables

## Maps

## Strings and string buffers
