#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mpi.h>

int my_wrank;

int MPIX_COMM_CREATE_FROM_GROUP(MPI_Group group, const char *tag, MPI_Comm *comm) {
    int groupRank, groupSize;
    int zero = 0, localRank;
    int localSize;
    char port[MPI_MAX_PORT_NAME];
    MPI_Group localGroup;
    MPI_Group_rank(group, &groupRank);
    MPI_Group_size(group, &groupSize);
    if (MPI_UNDEFINED == groupRank || 0==groupSize) {
        *comm = MPI_COMM_NULL;
        return MPI_SUCCESS;
    }
    MPI_Comm localComm;
    MPI_Comm_dup(MPI_COMM_SELF, &localComm);
    if (1 == groupSize) {
        *comm = localComm;
        return MPI_SUCCESS;
    }
    MPI_Comm_size(localComm, &localSize);
    fprintf(stderr, "rank %d non-trivial use-case: target group size %d, localGroup size %d\n", my_wrank, groupSize, localSize);
    if (localSize < groupSize) {
        MPI_Comm_group(localComm, &localGroup);
        MPI_Group_translate_ranks(group, 1, &zero, localGroup, &localRank);
        if (MPI_UNDEFINED == localRank) {
            fprintf(stderr, "rank %d sleeping for %d seconds\n", my_wrank, 1 + my_wrank);
            sleep(1+my_wrank); /* temporary till we do better error code return values */
            fprintf(stderr, "rank %d looking up port using name %s\n", my_wrank, tag);
            MPI_Lookup_name(tag, MPI_INFO_NULL, port);
            fprintf(stderr, "rank %d looked up port %s using name %s\n", my_wrank, port, tag);
        } else {
            fprintf(stderr, "rank %d opening port\n", my_wrank);
            MPI_Open_port(MPI_INFO_NULL, &port[0]);
            fprintf(stderr, "rank %d opened port %s\n", my_wrank, port);
            MPI_Publish_name(tag, MPI_INFO_NULL, port);
            fprintf(stderr, "rank %d published port %s using name %s\n", my_wrank, port, tag);
        }
        while (localSize < groupSize) {
            MPI_Comm_group(localComm, &localGroup);
            MPI_Group_translate_ranks(group, 1, &zero, localGroup, &localRank);
            MPI_Comm interComm;
            if (MPI_UNDEFINED == localRank) {
                fprintf(stderr, "rank %d connecting to port %s (localSize %d)\n", my_wrank, port, localSize);
                MPI_Comm_connect(port, MPI_INFO_NULL, 0, localComm, &interComm);
                fprintf(stderr, "rank %d connected using port %s (localSize %d)\n", my_wrank, port, localSize);
                MPI_Comm_free(&localComm);
                fprintf(stderr, "rank %d merging intercomm (high group)\n", my_wrank);
                MPI_Intercomm_merge(interComm, 1, &localComm);
                fprintf(stderr, "rank %d merged intercomm (high group)\n", my_wrank);
            } else {
                fprintf(stderr, "rank %d accepting on port %s (localSize %d)\n", my_wrank, port, localSize);
                MPI_Comm_accept(port, MPI_INFO_NULL, 0, localComm, &interComm);
                fprintf(stderr, "rank %d accepted using port %s (localSize %d)\n", my_wrank, port, localSize);
                MPI_Comm_free(&localComm);
                fprintf(stderr, "rank %d merging intercomm (low group)\n", my_wrank);
                MPI_Intercomm_merge(interComm, 0, &localComm);
                fprintf(stderr, "rank %d merged intercomm (low group)\n", my_wrank);
            }
            MPI_Comm_disconnect(&interComm);
            MPI_Comm_size(localComm, &localSize);
            fprintf(stderr, "rank %d disconnected intercomm (localSize %d)\n", my_wrank, localSize);
        }
        fprintf(stderr, "rank %d closing port\n", my_wrank);
        MPI_Close_port(port);
        fprintf(stderr, "rank %d closed port\n", my_wrank);
    }
    *comm = localComm;
    return MPI_SUCCESS;
}

int main(int argc, char **argv)
{
    int i, n,  wsize, *granks = NULL;
    MPI_Group wgroup, newgroup;
    MPI_Comm new_comm;
    MPI_Init(&argc, &argv);
    MPI_Comm_group(MPI_COMM_WORLD, &wgroup);
    MPI_Comm_size(MPI_COMM_WORLD, &wsize);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_wrank);

    granks = (int *)malloc(sizeof(int) * ((wsize + 1)/2));
    for (n = 0; n < ((wsize + 1)/2); ++n)
        granks[n] = n*2;

    if (my_wrank % 2 == 0) {
        MPI_Group_incl(wgroup, (wsize + 1)/2, granks, &newgroup);
        fprintf(stderr, "process %d (MPI_COMM_WORLD) calling from group thingy\n", my_wrank);
        MPIX_COMM_CREATE_FROM_GROUP(newgroup, "foobar10", &new_comm);
        fprintf(stderr, "process %d (MPI_COMM_WORLD) now calling barrier on new_comm\n", my_wrank);
        MPI_Barrier(new_comm);
        fprintf(stderr, "process %d (MPI_COMM_WORLD) done barrier on new_comm\n", my_wrank);
    } else {
        sleep(20);
    }

    fprintf(stderr, "process %d (MPI_COMM_WORLD) now calling barrier on MPI_COMM_WORLD\n", my_wrank);
    MPI_Barrier(MPI_COMM_WORLD);
    fprintf(stderr, "process %d (MPI_COMM_WORLD) done barrier on MPI_COMM_WORLD\n", my_wrank);

    MPI_Finalize();
}

