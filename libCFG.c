#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int MPIX_COMM_CREATE_FROM_GROUP(MPI_Group group, int tag, MPI_Comm *comm) {
    int groupRank, groupSize;
    MPI_Group_rank(group, &groupRank);
    MPI_Group_size(group, &groupSize);
    if (MPI_UNDEFINED == groupRank || 0==groupSize) {
        *comm = MPI_COMM_NULL;
        return MPI_SUCCESS;
    } else if (1 == groupSize) {
        MPI_Comm_dup(MPI_COMM_SELF, comm);
        return MPI_SUCCESS;
    }
    MPI_Group MGW, localGroup;
    MPI_Comm_group(MPI_COMM_WORLD, &MGW);
    MPI_Group_intersection(group, MGW, &localGroup);
    MPI_Group_free(&MGW);
    MPI_Comm localComm;
    MPI_Comm_create_group(MPI_COMM_WORLD, localGroup, tag, &localComm);
    MPI_Group_free(&localGroup);
    int localSize;
    MPI_Comm_size(localComm, &localSize);
    if (localSize < groupSize) {
        char serviceName[100];
        snprintf(serviceName, 100, "COMM_CREATE_FROM_GROUP with tag %d", tag);
        char port[MPI_MAX_PORT_NAME];
        if (0 == groupRank) {
          MPI_Open_port(MPI_INFO_NULL, &port[0]);
          MPI_Publish_name(serviceName, MPI_INFO_NULL, port);
        } else {
          MPI_Lookup_name(serviceName, MPI_INFO_NULL, port);
        }
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
    MPI_Comm_split(localComm, /*color:=*/0, /*key:=*/groupRank, comm);
    MPI_Comm_free(&localComm);
    return MPI_SUCCESS;
}

int main(int argc, char **argv)
{
    int i, my_wrank, my_grank, wsize, *granks = NULL;
    MPI_Group wgroup, newgroup;
    MPI_Comm new_comm;
    MPI_Init(&argc, &argv);
    MPI_Comm_group(MPI_COMM_WORLD, &wgroup);
    MPI_Comm_size(MPI_COMM_WORLD, &wsize);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_wrank);

    granks = (int *)malloc(sizeof(int) * ((wsize + 1)/2));
    for (i = 0; i < wsize; i+=2) {
        granks[i] = i;
    }

    MPI_Group_incl(wgroup, (wsize + 1)/2, granks, &newgroup);
    MPI_Group_rank(newgroup,&my_grank);
    if (my_grank != MPI_PROC_NULL) {
        fprintf(stderr, "process %d (MPI_COMM_WORLD) calling from group thingy\n", my_wrank);
        MPIX_COMM_CREATE_FROM_GROUP(newgroup, 10, &new_comm) {
        fprintf(stderr, "process %d (MPI_COMM_WORLD) now calling barrier \n",
                my_wrank);
        MPI_Barrier(new_comm);
        fprintf(stderr, "process %d (MPI_COMM_WORLD) called barrier \n",
                my_wrank);
    }
    MPI_Finalize();
}
