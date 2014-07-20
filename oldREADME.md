sfDB5
=====

`sfDB5` aims to do many things similar to what `sfDB2` tried to
achieve, in somewhat similar ways:

 - Process requests extremely quickly by allowing only simple
   requests
 - Process requests even faster by interlacing requests as they
   come in and are processed
 - Use various techniques to allow the kernel to swap pages that
   are not being used out and quickly load them back in
 - Ensure that the maximum performance is pulled out of a single
   machine by scaling efficiently across threads, cores, memory,
   and also swap so that, even on a multi-GigE network pipe the
   network is the limiting factor

... and then it does some things slightly differently:

 - Use a **hash** tree using the **BLAKE2** hash to provide high
   speed lookups with virtually no collisions
 - Make read requests much much faster than write requests using
   **file level specialized allocation trees**

... and finally introduces some new concepts (compared to sfDB2):

 - Provide a new kind of NoSQL database: a key-structure store
 - Accelerate filesystem accesses by allowing the use of multiple
   files to store data (ie - multiple disks)
 - Use parity or a hash to ensure data is stored correctly

`sfDB5` is inspired by Redis and Couchbase, and is licensed under
the MIT license.
