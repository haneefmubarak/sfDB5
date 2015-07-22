sfDB5
=====

A NoSQL Schemaless Relational Key-Structure Store

---

Have a look at the [transcribed talk](https://goo.gl/m6spq5).

---

sfDB5 has three main inspirations:

 - Inspiration I: Data Dependent Queries

   Solve recursion. In other words, instead of you recursing
   client-side and fetching data over and over again, you can
   just send the server a path string (similar to C's memory access
   syntax) and it'll do the recursion **within the cluster**. This is
   not only faster but is also (IMHO) easier to do.

 - Inspiration II: Ease of Use

   Three of the top NoSQL databases today aren't **as easy
   to use as a database should be**. One isn't exactly trivial
   to administer, another requires you to learn a query language
   (isn't that part of what we wanted to escape with NoSQL?), and
   the final one requires you to do manual parsing.

   sfDB5 should have everything necessary in the box. Cluster
   administration is just adding and removing servers - sfDB5
   will take care of the rest. There's no fancy query language,
   just simple path strings and standard functions. Parsing isn't
   a thing either. sfDB5 has its own efficient internal representation,
   but it'll just spit out a **string, blob, integer or float** when you
   ask it for data.

 - Inspiration III: Atomic Operations

   Serverside atomic operations like basic arithmetic, bitwise,
   and shift operations are quite useful, especially in synchronization.
   While **all sfDB5 operations are atomic**, these additional functions
   can perform as basic synchronization primitives, especially seeing as
   that sfDB5 does not support external locking.

Dependencies
------------

You will need the following:

 - a Linux or OSX box (after networking is implemented, only Linux will work)
 - GNU `coreutils >= 8.9`
 - one of the following compilers which support GNU C:
   - `gcc >= 4.8.0` (preferable)
   - `clang >= 3.5.0`
 - a working copy of [`git >= 1.8.5`](http://git-scm.com/downloads)
 - GNU `make >= 3.8`
 - [`premake4`](http://industriousone.com/premake/download)

Building
--------

Here's how you go about it; please ensure that your output looks somewhat
similar to the example output listed below (your results may be
slightly different, depending on various factors):

```bash
$ git clone https://github.com/haneefmubarak/sfDB5.git
Cloning into 'sfDB5'...
remote: Counting objects: 245, done.
remote: Compressing objects: 100% (15/15), done.
remote: Total 245 (delta 4), reused 0 (delta 0)
Receiving objects: 100% (245/245), 38.35 KiB | 0 bytes/s, done.
Resolving deltas: 100% (117/117), done.
Checking connectivity... done.
$ cd sfDB5/src/
$ premake4 gmake
Building configurations...
Running action 'gmake'...
Generating Makefile...
Generating sfDB5.make...
Generating backend.make...
Done.
$ make
==== Building backend ====

...

Linking backend
==== Building sfDB5 ====

...

Linking sfDB5
```

sfDB5 does not run as of yet.

Contributing and/or Sponsoring
------------------------------

See [CONTRIBUTING.md](CONTRIBUTING.md).

---

`sfDB5` is inspired by [Redis](http://redis.io/) and
[Couchbase](http://www.couchbase.com/), and is licensed under the MIT license.
