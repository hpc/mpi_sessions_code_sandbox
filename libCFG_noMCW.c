#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int MPIX_COMM_CREATE_FROM_GROUP(MPI_Group group, int tag, MPI_Comm *comm) {
    int groupRank, groupSize;
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
    int localSize;
    MPI_Comm_size(localComm, &localSize);
    if (localSize < groupSize) {
        char port[MPI_MAX_PORT_NAME];
        MPI_Open_port(MPI_INFO_NULL, &port[0]);
        while (localSize < groupSize) {
            MPI_Comm_group(localComm, &localGroup);
            int zero = 0, localRank;
            MPI_Group_translate_ranks(group, 1, &zero, localGroup, &localRank);
            MPI_Comm interComm;
            if (MPI_UNDEFINED == localRank) {
                MPI_Comm_connect(port, MPI_INFO_NULL, 0, localComm, &interComm);
                MPI_Comm_free(&localComm);
                MPI_Intercomm_merge(interComm, 1, &localComm);
                MPI_Comm_disconnect(&interComm);
            } else {
                MPI_Comm_accept(port, MPI_INFO_NULL, 0, localComm, &interComm);
                MPI_Comm_free(&localComm);
                MPI_Intercomm_merge(interComm, 0, &localComm);
                MPI_Comm_disconnect(&interComm);
            }
            MPI_Comm_size(localComm, &localSize);
        }
        MPI_Close_port(port);
    }
    *comm = localComm;
    return MPI_SUCCESS;
}

int main(int argc, char **argv)
{
    int i, n,  my_wrank, wsize, *granks = NULL;
    MPI_Group wgroup, newgroup;
    MPI_Comm new_comm;
    MPI_Init(&argc, &argv);
    MPI_Comm_group(MPI_COMM_WORLD, &wgroup);
    MPI_Comm_size(MPI_COMM_WORLD, &wsize);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_wrank);

    granks = (int *)malloc(sizeof(int) * ((wsize + 1)/2));
    for (i = 0, n = 0; i < wsize; i+=2) {
        granks[n] = i;
        n++;
    }

    MPI_Group_incl(wgroup, (wsize + 1)/2, granks, &newgroup);
    if (my_wrank % 2 == 0) {
        fprintf(stderr, "process %d (MPI_COMM_WORLD) calling from group thingy\n", my_wrank);
        MPIX_COMM_CREATE_FROM_GROUP(newgroup, 10, &new_comm);
        fprintf(stderr, "process %d (MPI_COMM_WORLD) now calling barrier \n",
                my_wrank);
        MPI_Barrier(new_comm);
        fprintf(stderr, "process %d (MPI_COMM_WORLD) called barrier \n",
                my_wrank);
    }

    MPI_Finalize();
}

