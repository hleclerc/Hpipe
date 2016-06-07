What is SIPE ?
==============

Hpipe stands for the "Simple Incremental Parser Engine".

It allows to generate optimized lexical parsers, starting from a text description of the patterns to be found. As the data is coming, matched patterns can trigger subroutines, conditionally or not.

"Incremental" means that the crunching of the incoming data can be stopped/restarted at any point, and interleaved with analysis of data from other sources. It is a basic requisite when the data come from a lot of different sources (e.g. for a server which receives messages from a lot of different clients) or from huge files that do not fit into memory.

In most of the cases, even if some kind of backtracking is required, data do not have to be saved (i.e. a fixed size buffer is enough). For specific cases where data has to be retrieved after a first pass (i.e. if not yet validated subroutines depend on data), Hpipe manages the buffers.

Benchmarks
==============

```[python]
a = 'a' { cpt += 'a'; }
b = 'b' { cpt += 'b'; }
c = 'c' { cpt += 'c'; }
d = 'd' { cpt += 'd'; }
e = lf  { cpt +=  10; }

main = ( a | b | c | d | e )** { return cpt; }
```

Majority of d
Without training
-> 2.33148
-> 1.81193 (profile optimization)
With training
-> 1.1844
-> 0.806927 (profile optimization) (2.24x)

Boyer-Moore

'bcdefghi' ->
  without:      0.976072 vs 0.219639 (4.44)
  with training 0.937635 vs 0.126285 (7.42)

An example
==========

For a *simplified* HTTP parser, one can write:
```[python]
get = 'GET '
    ( '/url_1' { do_something_now(); } ) |
    ( '/url_2' { do_another_thing(); } ) |
    { error_404(); }

content_length = 'Content-Length: ' uint[ length ]

put = 'PUT /url_with_queries?s=' uint[ v ] '&p=' uint[ u ] ' ' (
        content_length |
        ( eol eol -> end_header ) |
        any
    )** <- end_header { use( data, length ); }

main = get | put | { error_400(); }
```

`sipe my_file` with `my_file` containing the previous data, will give something like

```[C++]
struct HpipeData {
    HpipeData() {
        v = 0;
        u = 0;
        length = 0;
    }
    ...
    unsigned long v;
    unsigned long u;
    unsigned long length;
};
int parse( HpipeData *sipe_data, SIPE_CHARP data, SIPE_CHARP end ) {
     SIPE_CHARP beg_data = data;
     if ( sipe_data->_inp_cont )
         goto *sipe_data->_inp_cont;
     #define INCR( N ) if ( ++data >= end ) goto p_##N; c_##N:

     if ( *data != 'G' ) goto l_0;
     INCR( 0 )
     if ( *data == 'E' ) goto l_1;
l_3: { error_400(); }
l_2: sipe_data->_inp_cont = &&l_2;
     return 1;
l_1: INCR( 1 )
     if ( *data != 'T' ) goto l_3;
     INCR( 2 )
     ...
     if ( *data >= '1' and *data <= '2' ) goto l_5;
l_4: { error_404(); }
     goto l_2;
l_5: if ( *data != '1' ) goto l_6;
     { do_something_now(); }
     goto l_2;
l_6: { do_another_thing(); }
     goto l_2;
l_0: if ( *data != 'P' ) goto l_3;
     INCR( 9 )
     if ( *data != 'U' ) goto l_3;
     INCR( 10 )
     ...
```


The generator looks for the `main` "machine", which here contains several possible sub-machines, specified in priority order. If the `get` machine works (or will work) for the analyzed text, nothing related to the `put` will be executed (and of course `error_404` won't be called).



Syntax
======

Here are the main operators
* `"foo"` or `'foo'`: look for the text "foo" in the incoming data. "f" must be the first character of the incoming data (looking for a substring can be expressed as `any* "foo"`).
* `104` (unsigned char value): look for a byte with specified (unsigned) numeric value. For encoding where characters can use more than one byte (UTFx...), concatenation can be used.
* `'a' .. 'v'` (range): look for a byte between 'a' and 'v' included. Range works also with numerical values (e.g. `100 .. 105`).
* `A B` (concatenation): test for `A`, and if there's a match for machine `A`, advance in the buffer and look for `B`.
* `{ something in C/C++ }` (execution): execute the code between the `{}` if the previous machines in the lane have a match and _if there is at least one ending path through the following machines_ (which can be guessed or tested). For example, if the machine is `'A' {foo();} 'B'` and the text is 'AC', `foo` won't be executed (`'B'` is not matched). Sometimes, it is not necessary to test the coming machines because there is at least a *sure* path (e.g. with *non mandatory* things like `A*`). If it is necessary to test the coming machines, Hpipe may add a marker to keep the data available until there is an "up to the end" path.
* `A | B` (priority or): test `A`; if `A` does not work, test `B` with the same input data. Of course, internally it works as a state machine, and while `A` is tested, we keep an eye on `B` to avoid as much as possible buffering of the incoming data. Nevertheless, sometimes, it can be mandatory, specifically when there are execution machines (`{...}`) which depend on the data and tests to be made (leading to "data dependent" postponed execution).
* `A*` (loop, not having the priority): test for 0 or any number of successive matches for `A`. If there is a following machine, it will have the priority on `A`. Ex: with `( 'a' 'b' | 'c' {bar();} )* any 'c' {foo();}`, "ababac" will call `foo` two time and `bar` once because `any 'c'` have priority over the machine between the `()`.
* `A**` (priority loop): test for as much as possible (0 or any number of successive) matches for `A`. `A` has the priority over the eventual following machines. In the preceding example, `foo` would be called three times and `bar` zero.
* `A+` (one or more, not having the priority): test for 1 or more successive matches for `A`. If there is a following machine, it will have the priority on `A`.
* `A++` (one or more with priority): test for as much as possible matches for `A`, provided that there is at least one match.
* `A?`(option, not having the priority): test for zero or one match of `A`.  For example, the `eol` machine is equivalent to `cr? lf` to take into account optional dos end lines. If there is a following machine, the following machine have priority.
* `A??`(option with priority): test for zero or one match of `A`. If  `A` matches, the match have priority.
* `-> label ... <- label` (goto): `-> label` means "jump to label". `<- label` is to express where is the label. It allows to exit potentially infinite loops (like in the `put` machine in the simplified http example).
* eof ...

Predefined machines
===================

They are defined in [src/Hpipe/Predef.sipe](https://github.com/hleclerc/Hpipe/blob/master/src/Hpipe/Predef.sipe).

Beware: some of them require that the std header `string` is included (this is not the best choice for performance but we are working on this).

Operator precedence
===================

"["

"->"
"<-"

".."

"-"

"**"
"++"
"??"
"*"
"+"
"?"


"|"

","

"="



How to call and define machines ?
=================================

Machine may have arguments, with or without default values. Arguments are used as variables or maps for text replacement (e.g. in "..." or '...' string or in C/C++ code).

For example, `uint[ name, type = 'unsigned long' ] = ` allows to define a machine named `uint` with one mandatory argument, and one optional with a default value. It can be called with e.g. `uint[ name, unsigned ]`. Between the arguments the `,` is optional.

Arguments can be specified along with their names. For example, one can call `uint` with `uint[ type = unsigned, name = foo ]`

If there are no argument, the `[]` are optional.

It an argument contains a space or a `[]`, one can use `"..."` or `'...'`. Ex: `uint[ 'unsigned short' ]`.

Why another parse engine ?
==========================

There is already a bunch of tool to generate code that perform pattern-matching on text (e.g. [Flex](http://flex.sourceforge.net/manual/index.html), ...).

The specificity of Hpipe is that is allows to generate incremental parsers, i.e. parsers which can deal with potentially interrupted data.

Of course, there is the great [Ragel](http://www.complang.org/ragel) program.

Actually the goals are very close to this one... but with
* a simplified syntax
* conditionnal execution (`{...}` are launched only if the machine will surely match up to the end)
* priorities (`A|B` means that `A` and `B` will work, but `A` is prefered over `B`)
* execution postponing

Broadly speaking, Hpipe is a kind of Ragel including the management of observed most common needs inside the generated state machine.

TODO
====

Training !!
-----------

There are a lot of choices to be made when the code is generated. It will be significantly beneficial to feed the code generator with representative input data.

For example, if at some point a specific character at the beginning of a machine appears 99% of the time, it would be a good idea to test specifically for this character with a forward jump if not the case, (and not to use lookup tables for example).

Corrections
-----------

There are still some cases where Hpipe is stuck in an infinite loop (during the construction of the state machine) because it considers that there is an infinity of possible states.

Please send me your sipe files if such cases are encoutered.


Recursive machines
------------------

...would be beneficial to generate lexers for some programming languages.

More than one character at a time
---------------------------------

Currently, Hpipe works one character at a time.

Since the stream may be interrupted at any time, it is necessary to be able to do this.

Nevertheless, "cuts" do not occur so often, and in most of the cases, data come in chuncks of size greater than 1 byte.

For optimization, managing larger sequences at each iteration will allow things like
* more discriminating tests, as in Boyer-Moore like algorithms where characters are not tested in order.
* comparisons with several bytes at a time (with bitwise ands, ...) which is some cases may divide the execution time by 8 or more (in case of aligned data, and 32/64 bits or SIMD instruction)
