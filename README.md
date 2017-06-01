# What is Hpipe ?

Hpipe is the acronym of "High Performance Incremental Parser Engine".

It generates optimized code to evaluate regular expressions with embedded actions (arbitrary code, that can be defined by the user).

*Incremental* means that it works with streams: the crunching of the incoming data can be stopped/restarted at any point, with proper and automated handling of buffers (with reference counter or other techniques,depending on what the user wants). It is a prerequisite for different types of data coming from the network or for instance from huge files that do not fit into memory. It is the first difference compared to tools like for instance re2c which assumes that data come once, in a contiguous buffer.

Besides, all other things being equal,
* Hpipe is designed for performance. Hpipe supports training, anticipation and zero copy for most of the buffer styles, tries to read the data only once, etc...
* Hpipe's syntax and semantic focuses on *security* and *clarity* (over compactness). For instance, user codes are executed only if the paths fully succeed (not the case for ragel), escape sequences are avoided when possible, etc...

# Table of contents

<!-- TOC -->

- [What is Hpipe ?](#what-is-hpipe-)
- [Table of contents](#table-of-contents)
- [Show me an example](#show-me-an-example)
    - [What's the look of the produced code ?](#whats-the-look-of-the-produced-code-)
    - [Another simple example: reading floating point numbers](#another-simple-example-reading-floating-point-numbers)
    - [Buffer handling](#buffer-handling)
- [Installation](#installation)
- [A word on performance](#a-word-on-performance)
    - [Training](#training)
    - [Anticipation](#anticipation)
- [Syntax](#syntax)
    - [Operators](#operators)
    - [How to call and define machines ?](#how-to-call-and-define-machines-)
    - [Predefined machines](#predefined-machines)
    - [Aditional sections](#aditional-sections)
        - [Test](#test)
        - [Training data](#training-data)
        - [Methods](#methods)
        - [Flags](#flags)
    - [Operator precedence](#operator-precedence)
- [Important flags](#important-flags)
- [Output file](#output-file)
    - [Sections](#sections)
    - [Buffer styles](#buffer-styles)

<!-- /TOC -->

# Show me an example

## What's the look of the produced code ?

The following code is an example of a "machine" that sums the last digit of each 'bar[0-9]', and that counts the number of 'foo' and 'bar' not followed by a digit (which can be EOF or the beginning of something else...).

```python
main = (
    ( 'bar' '0' .. '9' { a += *data - '0'; } ) |
    ( 'bar' { b += 1; } ) |
    ( 'foo' { c += 1; } ) |
    any_utf8
)**
```

If the input is "bar5", `bar += 1` is not going to be executed: a user code is executed only if the belonging path _succeeds_ and it won't be the case for `( 'bar' { b += 1; } )` because there is a priority for `( 'bar' '0' .. '9' { a += *data - '0'; } )`.

It produces code like

```C++
l_2:
  // as in the Boyer-Moore algorithm, hpipe may decide to test several
  // chars ahead if it leads to reduction of the overall execution time
  if ( data + 3 > end_m1 ) goto l_1;
  data += 3;
  // partition and ordering of tests depend on the result of "training"
  // (see the paragraph on performance)
  if ( data[ 0 ] == 'a' ) goto l_19;
  if ( data[ 0 ] != 'o' ) goto l_20;
  if ( data[ -1 ] != 'o' ) goto l_21;
  if ( data[ -2 ] != 'f' ) goto l_2;
l_3:
  { c += 1; }
  goto l_2;
l_21:
  if ( data[ -1 ] != 'f' ) goto l_2;
l_6:
  // end of the buffer (different styles are possible)
  if ( data >= end_m1 ) goto c_2;
  ++data;
l_22:
  if ( data[ 0 ] == 'o' ) goto l_3;
...
c_7:
  // interruption of the data is fully handled (including in the cases
  // where one needs to go backward, ...)
  if ( last_buf ) goto l_11;
  hpipe_data->inp_cont = &&e_7;
  return RET_CONT;
e_7:
  if ( data > end_m1 ) goto c_7;
  goto l_16;
```

## Another simple example: reading floating point numbers

```python
# support function. res is a function argument
read_dec[ res ] = 
    ( digit { res =            *data - '0'; } )
    ( digit { res = 10 * res + *data - '0'; } )**

number_flt =
    read_dec[ "nfl" ]
    # in this example, '.' is mandatory for floating point numbers
    '.' { mul = 1; } ( digit { nfl += ( mul *= 0.1 ) * ( *data - '0' ); } )**
    # 'e' and 'E' are optional
    ( 'e' | 'E'
        ( '+'? read_dec[ "num" ] { nfl *= std::pow( 10.0,  num ); } ) |
        ( '-'  read_dec[ "num" ] { nfl *= std::pow( 10.0, -num ); } )
    )??
    { os << nfl << " "; }
```

## Buffer handling

In order to allow zero copy, hpipe proposes to take care of markers, for all the types of buffers (contiguous or not, interruptible or not, etc...).

For instance, a http parser can be written with 

```python
# read until `end` char, reference the result in `$name` if success
read_string[ name, end = 10 | 13 ] = beg_str_next[ name ] ( any - end )** end_str_next[ name ]
# read `size` bytes, reference the result in `$name` if success
read_ssized[ name, size ] = beg_str_next[ name ] skip[ size ] end_str_next[ name ]
# read a decimal number. Store the result in `$name`
read_number[ name ] = digit { name = *data - '0'; } ( digit { name = 10 * name + *data - '0'; } )**

# this http parser simply get url, content length and content
main = 'GET ' read_string[ 'url', ' ' ] (
    ( 10 'Content-Length: ' read_number[ 'content_length' ] ) |
    ( 10 eol read_ssized[ 'content', 'content_length' ] { os << "url:" << url << " cl:" << content_length << " content:" << content; } any** ) |
    any
)** 
```

For instance, for linked lists of buffers, the generated will then look like:

```cpp
    ...
    if ( __builtin_expect( data[ 0 ] == 'C', 0 ) ) goto l_10;
    if ( data[ 0 ] == 13 ) goto l_9;
  l_28:
    if ( data[ 0 ] != 10 ) goto l_6;
    // hpipe uses markers (in this example)
    HPIPE_DATA.__beg_content_buf = buf;
    HPIPE_DATA.__beg_content_data = data;
    if ( data + content_length <= end_m1 ) {
        data += content_length;
    } else {
        // if stream is interrupted
        HPIPE_DATA.__bytes_to_skip = content_length - ( buf->data + buf->used - data );
      t_9:
        if ( ! buf->next ) {
            if ( last_buf )
                goto l_38;
            // handling of buffer retention 
            HPIPE_DATA.pending_buf = buf;
            HPIPE_BUFF_T__INC_REF( buf );
            HPIPE_DATA.inp_cont = &&e_9;
            return RET_CONT;
          e_9:
            HPIPE_DATA.pending_buf->next = buf;
            HPIPE_DATA.pending_buf = buf;
            goto c_9;
        }
        buf = buf->next;
      c_9:
        if ( HPIPE_DATA.__bytes_to_skip >= buf->used ) {
            HPIPE_DATA.__bytes_to_skip -= buf->used;
            goto t_9;
        }
        data = buf->data + HPIPE_DATA.__bytes_to_skip;
        end_m1 = buf->data + buf->used - 1;
    }
    HPIPE_BUFF_T__INC_REF( buf );
    HPIPE_CB_STRING__ASSIGN_BEG_END( HPIPE_DATA.content, HPIPE_DATA.__beg_content_buf, HPIPE_DATA.__beg_content_data, buf, data + 1 );
    ...
```

# Installation

```bash
git clone https://github.com/hleclerc/Hpipe.git && cd Hpipe && mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=release .. && make -j8
sudo make install
```

# A word on performance

Hpipe is made from the ground for performance. Here are some illustrations:

## Training

Training data can be used by hpipe to transform the instruction graph before the code generation. It enables in particular to automatically decompose the tests, in order to minimize the average number of tests and branch mispredictions.

Let's consider the following declaration:

```python
a = 'a' { cpt += 'a'; }
b = 'b' { cpt += 'b'; }
c = 'c' { cpt += 'c'; }
d = 'd' { cpt += 'd'; }
l = lf  { cpt +=  10; } # lf = line feed

# we simply test presence of some given chars
main = ( a | b | c | d | l )**

# some training data
beg_training
    input
        aaaacdacaabda... (a majority of 'a')
    freq
        1
end_training
```

Launched on (very long) data similar to the training one, execution times are

|                  | Without training | With training  |speedup |
|------------------|:----------------:|:--------------:|:------:|
| w/o profile opt  |     2.27894s     |    1.11687s    | 2.04x  |
| with profile opt |     1.80447s     |    0.778634s   | 2.31x  |

("profile opt" means use of `g++ -fprofile-instr-generate` -- clang gives the same kind of results with or without profile optimizations)

Here is a comparison of the performance of a _basic_ js scanner, written with [re2c](http://re2c.org/), written with [ragel](http://www.colm.net/open-source/ragel/) or written using hpipe (see tests/re2c/js.re, tests/re2c/js.ragel and tests/hpipe/js.hpipe):

|                  |       re2c       |     re2c -g      |    ragel -G2     |     hpipe      |        speedup         |
|------------------|:----------------:|:----------------:|:----------------:|:--------------:|:----------------------:|
| w/o profile opt  |     6.06785s     |     3.26868s     |     4.09402s     |    2.15281s    | 2.49x **1.52x** 1.90x  |
| with profile opt |     6.36135s     |     3.26959s     |     2.42014s     |    1.82015s    | 3.49x 1.80x **1.32x**  |


re2c is not meant to work on interrupted streams, but the base techniques are roughly the same, hence the relevance of the test. By the way, languages like javascript are meant to be *easily* parsed, meaning that the most simple parsing codes should already been roughly optimized (i.e. this test is only about basic training, not about the other kinds of optimizations applicable for other kinds of input).

## Anticipation

Like in the Boyer-Moore algorithm it may be beneficial to test several chars ahead, in particular if the test leads most of the time to paths where it is of no use to test the chars before.

For instance, with

```python
main = (
    ( 'bcdefghi' { cpt += 1; } ) |
    any
)**
```

we obtain (on a very long random data set) the following result (using `--boyer-more` flag or not):

|                  | Without BM       |  With BM       |speedup |
|------------------|:----------------:|:--------------:|:------:|
| execution time   |   0.937635s      |    0.126285s   | 7.42x  |

# Syntax

## Operators

* `"foo"` or `'foo'`: look for the text "foo" in the incoming data. "f" must be the first character of the incoming data (looking for a substring can be expressed as `any* "foo"`).
* `104` (unsigned char value): look for a byte with specified (unsigned) numeric value. For encoding where characters can use more than one byte (UTFx...), concatenation can be used.
* `'a' .. 'v'` (range): look for a byte value between 'a' and 'v' inclusive. Range works also with numerical values (e.g. `'a' .. 105`).
* `A - B` (substraction): look for a byte value in `A` but not in `B`.
* `A B` (concatenation): test for `A`, and if there's a match for machine `A`, advance in the buffer and look for `B`.
* `{ something in the target language }` (execution): execute the code between the `{}` if the previous machines in the lane have a match and _if there is at least one ending path through the following machines_ (which can be guessed or tested). For example, if the machine is `'A' {foo();} 'B'` and the text is 'AC', `foo` won't be executed (`'B'` is not matched). Sometimes, it is not necessary to test the coming machines because there is at least a *sure* path (e.g. with *non mandatory* things like `A*`). If it is necessary to test the coming machines, Hpipe may add a marker to keep the data available until there is an "up to the end" path.
* `A | B` (priority or): test `A`; if `A` does not work, test `B` with the same input data. Of course, internally, it works as a state machine, and while `A` is tested, we keep an eye on `B` to avoid buffering of the incoming data, as much as possible. Nevertheless, sometimes, it can be mandatory, specifically when there are execution machines (`{...}`) which depend on the data and tests to be made (leading to "data dependent" postponed execution).
* `A*` (loop, not having the priority): test for 0 or any number of successive matches for `A`. If there is a following machine, it will have the priority on `A`.
 <!--Ex: with `( 'a' 'b' | 'c' {bar();} )* any 'c' {foo();}`, "ababac" will call `foo` two time and `bar` once because `any 'c'` have priority over the machine between the `()`.-->
* `A**` (priority loop): test for as much as possible (0 or any number of successive) matches for `A`. `A` has the priority over the eventual following machines.
 <!--In `( 'a' 'b' | 'c' {bar();} )** any 'c' {foo();}` with "ababac", `foo` would be called three times and `bar` zero.-->
* `A+` (one or more, not having the priority): test for 1 or more successive matches for `A`. If there is a following machine, it will have the priority on `A`.
* `A++` (one or more with priority): test for as much as possible matches for `A`, provided that there is at least one match.
* `A?`(option, not having the priority): test for zero or one match of `A`.  For example, the `eol` machine is equivalent to `cr? lf` to take into account optional dos end lines. If there is a following machine, the following machine have priority.
* `A??`(option with priority): test for zero or one match of `A`. If  `A` matches, it has the priority.
* `-> label ... <- label` (goto): `-> label` means "jump to label". `<- label` is to define the label. It allows notably to exit loops with complex priority patterns. For instance, we can scan C strings using
```python
'"' ( '\\' | '\"' | ( '"' -> out_string ) | any )** <- out_string
```
in place of
```python
'"' ( '\\' | '\"' | any - '"' )** '"'
```
where the end machine is written twice (please note that there is no escaping in hpipe strings).
* `eof`: ok if end of file.
* `beg_str[ "A" ]`: beginning of a string named `A`. If the corresponding `end_str` is in a succeeding path, `A` will contain (referenced) data between `beg_str` and `end_str`. This machine will register an attribute in `HpipeData`. Type of this attribute will depend on the buffer type.
* `beg_str_next[ "A" ]`: same thing than `beg_str[ "A" ]` but the actual first char will be after the current one.
* `end_str[ "A" ]`: end of a string named `A`. To be used in conjunction with beg_str.
* `end_str_next[ "A" ]`: end of a string named `A`, current char being be included in `A`.
* `skip[ "variable or value" ]`: skip some bytes (defined by an integer). At a given point, if the only active machine are skips, bytes are directly skipped.
<!--* `add_str[ "A" ]`: shortcut to write a code that will append the current char value to a string named A (that will be automatically declared in the generated `HpipeData` structure). `add_str` enables specific optimizations. For instance contiguous `add_str` may be transformed to an "add mark" and a "use mark" instructions, enabling effective zero copy.
* `clr_str[ "A" ]`: shortcut to write a code that will clear the string named A.-->

There are some internal predefined machine to help generate code:
* `add_variable[ type, name, optionnal_default_value ]` will add a variable declaration in the `HpipeData` structure (+use of `optionnal_default_value` in ctor).
* `add_include[ 'my_include.h' ]` will add `#include <my_include.h>` (only once) in the preliminary of the generated code. `<>` and `""` can be specified explicitely (as in `add_include[ '"my_include.h"' ]`)
* `add_prel[ 'some_code' ]` or `add_preliminary[ 'some_code' ]` will add `some_code` (only once) in the preliminary of the generated code
* `import my_file.hpipe` allows to import content from another hpipe file.

## How to call and define machines ?

```python
# Arguments values are machines. It may have default values.
my_machine[ val, num = "123", end = digit 'e' | 'E' ] =
    { res += num; } # substring with argument names are replaced by the values
    val any* end # variable can be used as normal machines
#
other[ foo = eol ] = ...

#
main =
    # arguments can be specified using their names
    my_machine[ any, num = "5" ]
    # if no argument of default values are available at every position, a machine can be called without []
    other
```

## Predefined machines

They are defined in [src/Hpipe/Predef.sipe](https://github.com/hleclerc/Hpipe/blob/master/src/Hpipe/predef.hpipe).

## Aditional sections

### Test

It is possible to define test(s) inside machine definition, using:

```
beg_test preproc
    input
        ...
    output
        ... status=...
end_test
```

`input` and `output` are shifted to the left (space at the beginning of the line are here to separate sections). `hpipe --test foo.hpipe` allows to launch the tests in `foo.hpipe` (one can use the flag `style` to test with different kinds of buffers).

### Training data

Training data uses a similar syntax:

```
beg_training
    input
        ...
    freq
        42
end_training
```

`freq` represent a frequency (of course relevant only if several training data are present).

### Methods

```
beg_methods
    void something_in_the_class();
    int an_attribute; 
    // ...
end_methods
```

will add some code inside the _declaration_ section (which may be in a class or not, depending on how the generated code is used).

### Flags

```
beg_flags
    --never-ending
    --...
end_flags
```

allows to inline some command line flags (for instance for machine intended to work only under some context, ...).

## Operator precedence

| group            |          operators         |
|------------------|:--------------------------:|
| 0                |    `[`                     |
| 1                |   `->` `<-`                |
| 2                |   `..`                     |
| 3                | `-`                        |
| 4                | `**` `++` `??` `*` `+` `?` |
| 5                |    `|`                     |
| 6                |    `,`                     |
| 7                |      `=`                   |

# Important flags

* `--never-ending` indicates that a machine is not going to fail due to the end of the stream. For instance in `{ some_code; } any any`, by default, one will have to read the two chars before executing `{ some_code; }`. If `--never-ending` is specified, `{ some_code; }` will be executed directly: one will consider that lack of data is not a failure reason.
* `--style buffer_style`: defines how the data are given (contiguous buffers or not, interruptible or not, ...). See [Buffer styles](#buffer-styles).

# Output file

## Sections

Hpipe generates only one output file, with sections that can be activated using defines (`#define HPIPE_DECLARATIONS`, `#define HPIPE_DEFINITIONS`, ...). A lot of flexibility is allowed using macros, users are encouraged to read them in the generated code.

## Buffer styles

By default (but there are other choices), hpipe generate a `parse` function with a signature like:

```C++
unsigned parse( HpipeData* hpipe_data, Hpipe::Buffer* buf, bool last_buf = false );
```
`Hpipe::Buffer` is basically a data chunk with a reference counter and a `next` field for strings made from several data chunks (`Hpipe::CbString` and `Hpipe::CbStringPtr` enable to handle those lists regular strings). A standard reading pattern using `Hpipe::Buffer` would be

```C++
Buffer *inp_buff = Buffer::New( Buffer::default_size );
HpipeData hpipe_data;
...
void input_data_event() {
    while ( true ) {
        // if not possible to reuse the old buffer, create a new one
        if ( inp_buff->cpt_use > 0 ) {
            --inp_buff->cpt_use;
            inp_buff = Buffer::New( Buffer::default_size );
        }

        // try to read some data
        inp_buff->used = recv( fd, inp_buff->data, inp_buff->size, ... );
        if ( int( inp_buff->used ) <= 0 ) ...;

        // parse
        parse( &hpipe_data, inp_buff );
    }
}
```

Of course, using this kind of buffer is not the taste of everyone. One can use the option `-s` or `--style` to choose the buffer style:
- `-s HPIPE_BUFFER` generates code as shown above.
- `-s HPIPE_CB_STRING_PTR` generates code that assumes that data comes once, in a chained list of buffer with a begin and an end offset (relative to the first buffer, the one given as argument).
- `-s BEG_END` will generate code for a unique (non interruptible) buffer defined using a `const unsigned char *` for the beginning and also for the end.
- `-s C_STR` will generate code for a unique (non interruptible) buffer defined using a `const unsigned char *` for the beginning, and ended by the value after the `--stop-char` cmd arg (0 by default).

For instance, `-s BEG_END` will generate a function with a signature like:

```C++
unsigned parse( HpipeData* hpipe_data, const unsigned char* data, const unsigned char* end_m1 ); // `end_m1` must point to the last char (e.g. for data.size == 0, end_m1 must be equal to data - 1).
```

