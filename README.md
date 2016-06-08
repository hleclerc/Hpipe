What is Hpipe ?
==============

Hpipe is the acronym of "High Performance Incremental Parser Engine".

It allows to generate optimized lexical parsers, starting from a text description of the patterns to be found. As the data is coming, validated paths can trigger subroutines.

*Incremental* means that the crunching of the incoming data can be stopped/restarted at any point. It is a base requisite for data coming from the network or from huge files that do not fit into memory.

In most of the cases, even if some kind of backtracking is required, data has to be read only once (meaning for instance that in most of the cases, data has not to be saved). For specific cases where data has to be read again after a first pass (i.e. if not yet *validated* subroutines depend on data), Hpipe generate the code to handle the buffers in an optimal way.

The following code is an example of a "machine" that counts the number of foo, bar, and bar with a number.

```[python]
main = (
    ( 'bar' '0' .. '9' { bar += *data - '0'; } ) |
    ( 'bar' { bar += 1; } ) |
    ( 'foo' { foo += 1; } ) |
    any_utf8
)**
```

If the input is "bar5", `bar += 1` won't be executed: a code is executed only if the belonging path succeed. Furthermore, `|` handles the priority of the left hand side over the right one (if the lhs succeeds, the rhs is discarded... and codes are executed only when the ambiguities have disappeared).

If produces code like

```[C++]
l_2:
  if ( data + 3 > end_m1 ) goto l_1; // as in the Boyer-Moore algorithm, hpipe may decide to test several chars ahead if it leads to reduction of the overall execution time
  data += 3;
  if ( data[ 0 ] == 'a' ) goto l_19; // partition and ordering of tests depends on the result of "training" (see the Performance paragraph)
  if ( data[ 0 ] != 'o' ) goto l_20;
  if ( data[ -1 ] != 'o' ) goto l_21;
  if ( data[ -2 ] != 'f' ) goto l_2;
l_3:
  { foo += 1; }
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
  if ( last_buf ) goto l_11;
  sipe_data->inp_cont = &&e_7;
  return RET_CONT;
e_7:
  if ( data > end_m1 ) goto c_7;
  goto l_16;
```

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
```
'"' ( '\\' | '\"' | ( '"' -> out_string ) | any )** <- out_string
```
in place of
```
'"' ( '\\' | '\"' | any - '"' )** '"' <- out_string
```
when the end machine is written twice.
* `eof`: ok if end of file.
* `add_str[ "A" ]`: shortcut to write a code that will append the current char value to a string named A (that will be automatically declared in the generated HpipeData structure). `add_str` enables specific optimizations. For instance contiguous `add_str` may be transformed to an "add mark" and a "use mark" instructions, enabling effective zero copy.
* `clr_str[ "A" ]`: shortcut to write a code that will clear the string named A.


How to call and define machines ?
=================================

```[python]
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

They are defined in [src/Hpipe/Predef.sipe](https://github.com/hleclerc/Hpipe/blob/master/src/Hpipe/Predef.hpipe).


Performance
===========

Training
--------

Let's consider the following hpipe declaration:

```[python]
a = 'a' { cpt += 'a'; }
b = 'b' { cpt += 'b'; }
c = 'c' { cpt += 'c'; }
d = 'd' { cpt += 'd'; }
e = lf  { cpt +=  10; }

main = ( a | b | c | d | e )**

beg_training
    input
        aaaacdacaabda... (a majority of 'a')
    freq
        1
end_training
```

Training data is used by hpipe to transform the instruction graph before code generation. In particular, it enables to automatically decompose the tests to minimize the number of conditional branches for the average cases (+ provide some help for the Branch Prediction Units, ...).

|                            | Without training | With training  |speedup |
|----------------------------|:----------------:|:--------------:|:------:|
| exec time w/o profile opt  |     2.33148      |    1.1844      | 1.97x  |
| exec time with profile opt |     1.81193      |    0.806927    | 2.24x  |

(profile opt means use of `-fprofile-instr-generate` with g++)

Boyer-Moore like optimizations
------------------------------

Like in the Boyer-Moore algorithm it may be beneficial to test several chars ahead, in particular if the test leads most of the time to path where it is of no use to test the chars before.

With

```[python]
main = (
    ( 'bcdefghi' { cpt += 1; } ) |
    any
)**
```

we obtain the following result (g++, Skylake, ...):

|                  | Without BM       |  With BM       |speedup |
|------------------|:----------------:|:--------------:|:------:|
| execution time   |   0.937635       |    0.126285    | 7.42x  |

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


Why another parse engine ?
==========================

There is already a bunch of tool to generate code that perform pattern-matching on text (e.g. [Flex](http://flex.sourceforge.net/manual/index.html), [Ragel](http://www.complang.org/ragel)...).

To generate incremental parsers, Ragel may be a great help, at some point, but not if several paths with actions are possible (in this case ragel executes all the actions). It is counter-intuitive and leads to complex definitions: lot of work are left to the user.

Definition and use of priority enable hpipe definition to be more concise and less error prone than e.g. standard regular expressions.

Furthermore, hpipe has been designed to generate *fast* code, with specific optimizations that lead to significant speedups.
