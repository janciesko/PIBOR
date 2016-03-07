# PIBOR
Privatization with In-lined Block-ordering


This example show the use of a technique that accelerates scattered memory updates by re
directing them into thread-local buffers. Each buffer corresponds to a region of the original memory address range of the scattered update. Once a buffer runs full, it is reduced to the original region. This allows to group accesses into blocks for improved locality and to decrease the lock contention in case of multi-threaded executions.

For demonstration, the package includes the RandomAccess benchmark implemented with Privatization, Atomics and PIBOR. The code uses tasks and the Makefile was prepared for the use with the OmpSs programming model.



