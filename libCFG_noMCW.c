#include <mpi.h>

int MPIX_COMM_CREATE_FROM_GROUP(MPI_Group group, int tag, MPI_Comm *comm) {
    int groupRank, groupSize;
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
	MPI_Group localGroup;
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
