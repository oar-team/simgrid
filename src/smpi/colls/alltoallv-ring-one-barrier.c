/* Copyright (c) 2013-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "colls_private.h"
/*****************************************************************************

 * Function: alltoall_ring

 * Return: int

 * Inputs:
    send_buff: send input buffer
    send_count: number of elements to send
    send_type: data type of elements being sent
    recv_buff: receive output buffer
    recv_count: number of elements to received
    recv_type: data type of elements being received
    comm: communicator

 * Descrp: Function works in P - 1 steps. In step i, node j - i -> j -> j + i.

 * Auther: Ahmad Faraj

 ****************************************************************************/
int
smpi_coll_tuned_alltoallv_ring_one_barrier(void *send_buff, int *send_counts, int *send_disps,
                                          MPI_Datatype send_type,
                                          void *recv_buff, int *recv_counts, int *recv_disps,
                                          MPI_Datatype recv_type, MPI_Comm comm)
{
  MPI_Status s;
  MPI_Aint send_chunk, recv_chunk;
  int i, src, dst, rank, num_procs;
  int tag = COLL_TAG_ALLTOALLV;

  char *send_ptr = (char *) send_buff;
  char *recv_ptr = (char *) recv_buff;

  rank = smpi_comm_rank(comm);
  num_procs = smpi_comm_size(comm);
  send_chunk = smpi_datatype_get_extent(send_type);
  recv_chunk = smpi_datatype_get_extent(recv_type);

  smpi_mpi_barrier(comm);
  for (i = 0; i < num_procs; i++) {
    src = (rank - i + num_procs) % num_procs;
    dst = (rank + i) % num_procs;

    smpi_mpi_sendrecv(send_ptr + send_disps[dst] * send_chunk, send_counts[dst], send_type, dst,
                 tag, recv_ptr + recv_disps[src] * recv_chunk, recv_counts[src], recv_type,
                 src, tag, comm, &s);
  }
  return MPI_SUCCESS;
}
