What is Hpipe ?
==============

Hpipe is the acronym of "High Performance Incremental Parser Engine".

It enable to generate optimized code for regular expressions with actions, starting from a text description of the patterns to be found.

*Incremental* means that the crunching of the incoming data can be stopped/restarted at any point. It is a base requisite for data coming from the network or from huge files that do not fit into memory.

In most of the cases, even if some kind of rewinds are required, data has to be read only once (zero copy). For specific cases where data has to be read again after a first pass (i.e. if not yet *validated* subroutines depend on data), Hpipe generate the code to handle the buffers in an optimized way.

The following code is an example of a "machine" that sums the last digits of 'bar[0-9]', and that count the number of 'bar' not followed by a digit (which can be EOF or the beginning of something else...) and 'foo'.

```python
main = (
    ( 'bar' '0' .. '9' { a += *data - '0'; } ) |
    ( 'bar' { b += 1; } ) |
    ( 'foo' { c += 1; } ) |
    any_utf8
)**
```

If the input is "bar5", `bar += 1` won't be executed: a code is executed only if the belonging path succeeds; `|` handles the priority of the left hand side over the right one (if the lhs succeeds, the rhs is discarded... and codes are executed only when the ambiguities have disappeared).

If produces code like

```C++
l_2:
  // as in the Boyer-Moore algorithm, hpipe may decide to test several
  // chars ahead if it leads on average to reduction of the overall execution time
  if ( data + 3 > end_m1 ) goto l_1;
  data += 3;
  // partition and ordering of tests depends on the result of "training"
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
  if ( data >= end_m1 ) goto c_2;
  ++data;
l_22:
  if ( data[ 0 ] == 'o' ) goto l_3;
...
c_7:
  // interruption of the data is fully handled (including in the cases
  // where one needs to go backward, ...)
  if ( last_buf ) goto l_11;
  sipe_data->inp_cont = &&e_7;
  return RET_CONT;
e_7:
  if ( data > end_m1 ) goto c_7;
  goto l_16;
```

Here is another example, to display floating point numbers:

```python
read_dec[ res ] = # res is a function argument
    ( digit { res =            *data - '0'; } )
    ( digit { res = 10 * res + *data - '0'; } )**

number_flt =
    read_dec[ "nfl" ]
    # in this example, '.' is mandatory for floating point numbers
    '.' { mul = 1; } ( digit { nfl += ( mul *= 0.1 ) * ( *data - '0' );  } )**
    # 'e' and 'E' are optional
    ( 'e' | 'E'
        ( '+'? read_dec[ "num" ] { nfl *= std::pow( 10.0,  num ); } ) |
        ( '-'  read_dec[ "num" ] { nfl *= std::pow( 10.0, -num ); } )
    )??
    { os << nfl << " "; }
```


Performance
===========

Hpipe is made from the ground for performance. Here are some specific optimizations:

Training
--------

Training data is used by hpipe to transform the instruction graph before code generation. In particular, it enables to automatically decompose the tests to minimize the number of conditional branches for the average cases (+ provide some help for the Branch Prediction Units and so on).

Let's consider the following declaration:

```python
a = 'a' { cpt += 'a'; }
b = 'b' { cpt += 'b'; }
c = 'c' { cpt += 'c'; }
d = 'd' { cpt += 'd'; }
e = lf  { cpt +=  10; }

# we simply test for some given chars
main = ( a | b | c | d | e )**

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
| w/o profile opt  |     2.33148s     |    1.1844s     | 1.97x  |
| with profile opt |     1.81193s     |    0.806927s   | 2.24x  |

("profile opt" means use of `g++ -fprofile-instr-generate` -- clang gives the same kind of results with or without profile optimizations)

On some real-world cases (e.g. in a C++ parser...), the speedup can reach an order of magnitude.

Boyer-Moore like optimizations
------------------------------

Like in the Boyer-Moore algorithm it may be beneficial to test several chars ahead, in particular if the test leads most of the time to paths where it is of no use to test the chars before.

For instance. with

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


Syntax
======

Here are the main operators
* `"foo"` or `'foo'`: look for the text "foo" in the incoming data. "f" must be the first character of the incoming data (looking for a substring can be expressed as `any* "foo"`).
* `104` (unsigned char value): look for a byte with specified (unsigned) numeric value. For encoding where characters can use more than one byte (UTFx...), concatenation can be used.
* `'a' .. 'v'` (range): look for a byte value between 'a' and 'v' inclusive. Range works also with numerical values (e.g. `'a' .. 105`).
* `A - B` (sub): look for a byte value in `A` but not in `B`.
* `A B` (concatenation): test for `A`, and if there's a match for machine `A`, advance in the buffer and look for `B`.
* `{ something in C/C++ }` (execution): execute the code between the `{}` if the previous machines in the lane have a match and _if there is at least one ending path through the following machines_ (which can be guessed or tested). For example, if the machine is `'A' {foo();} 'B'` and the text is 'AC', `foo` won't be executed (`'B'` is not matched). Sometimes, it is not necessary to test the coming machines because there is at least a *sure* path (e.g. with *non mandatory* things like `A*`). If it is necessary to test the coming machines, Hpipe may add a marker to keep the data available until there is an "up to the end" path.
* `A | B` (priority or): test `A`; if `A` does not work, test `B` with the same input data. Of course, internally it works as a state machine, and while `A` is tested, we keep an eye on `B` to avoid buffering of the incoming data, as much as possible. Nevertheless, sometimes, it can be mandatory, specifically when there are execution machines (`{...}`) which depend on the data and tests to be made (leading to "data dependent" postponed execution).
* `A*` (loop, not having the priority): test for 0 or any number of successive matches for `A`. If there is a following machine, it will have the priority on `A`. Ex: with `( 'a' 'b' | 'c' {bar();} )* any 'c' {foo();}`, "ababac" will call `foo` two time and `bar` once because `any 'c'` have priority over the machine between the `()`.
* `A**` (priority loop): test for as much as possible (0 or any number of successive) matches for `A`. `A` has the priority over the eventual following machines. In the preceding example, `foo` would be called three times and `bar` zero.
* `A+` (one or more, not having the priority): test for 1 or more successive matches for `A`. If there is a following machine, it will have the priority on `A`.
* `A++` (one or more with priority): test for as much as possible matches for `A`, provided that there is at least one match.
* `A?`(option, not having the priority): test for zero or one match of `A`.  For example, the `eol` machine is equivalent to `cr? lf` to take into account optional dos end lines. If there is a following machine, the following machine have priority.
* `A??`(option with priority): test for zero or one match of `A`. If  `A` matches, it has the priority.
* `-> label ... <- label` (goto): `-> label` means "jump to label". `<- label` is to define the label. It allows notably to exit loops with complex priority patterns. For instance, we can find C strings using
```python
'"' ( '\\' | '\"' | ( '"' -> out_string ) | any )** <- out_string
```
in place of
```python
'"' ( '\\' | '\"' | any - '"' )** '"' <- out_string
```
when the end machine is written twice.
* `eof`: ok if end of file.
* `add_str[ "A" ]`: shortcut to write a code that will append the current char value to a string named A (that will be automatically declared in the generated HpipeData structure). `add_str` enables specific optimizations. For instance contiguous `add_str` may be transformed to an "add mark" and a "use mark" instructions, enabling effective zero copy.
* `clr_str[ "A" ]`: shortcut to write a code that will clear the string named A.

There are some internal predefined machine to help generate code:
* `add_include[ 'my_include.h' ]` will add `#include <my_include.h>` (only once) in the preliminary of the generated code
* `add_prel[ 'some_code' ]` or `add_preliminary[ 'some_code' ]` will add `some_code` (only once) in the preliminary of the generated code
* `add_attr[ 'some_code' ]` will add `some_code` (only once) in the attributes of the generated class


Buffer style
============

By default, hpipe generate a `parse` function with a signature like:

```C++
unsigned parse( HpipeData* hpipe_data, Hpipe::Buffer* buf, bool last_buf = false )
```
`Hpipe::Buffer` is basically a data chunk with a reference counter and a `next` field for strings made from several data chunks (`Hpipe::CbString` and `Hpipe::CbStringPtr` enable to handle those lists as more or less regular strings). A standard reading pattern using `Hpipe::Buffer` would be

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

For more conventional buffer styles (e.g. if data is given as a whole), one can use the option `-s BEG_END` which generates a function with the following signature:

```C++
unsigned parse( HpipeData* sipe_data, const unsigned char* data, const unsigned char* end_m1 );
```

`end_m1` must point to the last char (e.g. for data.size == 0, end_m1 must be equal to data - 1).

It is also possible to use `-s C_STR` which generates a function for zero ended strings.

Additionally, `-s BUFFER_IN_CLASS` will generate the parse function without `HpipeData* hpipe_data, `, assuming that data accessible via an attribute or a global variable.

How to call and define machines ?
=================================

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

Predefined machines
===================

They are defined in [src/Hpipe/Predef.sipe](https://github.com/hleclerc/Hpipe/blob/master/src/Hpipe/predef.hpipe).


Operator precedence
===================

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
