sfDB5
=====

A NoSQL Schemaless Relational Key-Structure Store

---

Have a look at the [transcribed talk](https://smarturl.it/sfDB5-talk).

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

Contributing and/or Sponsoring
------------------------------

See [CONTRIBUTING.md](CONTRIBUTING.md).

---

`sfDB5` is inspired by [Redis](http://redis.io/) and
[Couchbase](http://www.couchbase.com/), and is licensed under the MIT license.
