/*
 * Copyright 2000, International Business Machines Corporation and others.
 * All Rights Reserved.
 *
 * This software has been released under the terms of the IBM Public
 * License.  For details, see the LICENSE file in the top-level source
 * directory or online at http://www.openafs.org/dl/license10.html
 */

#ifndef OPENAFS_WINNT_AFSD_CM_CONN_H
#define OPENAFS_WINNT_AFSD_CM_CONN_H 1

#define	CM_CONN_DEFAULTRDRTIMEOUT	45
#ifndef CM_CONN_CONNDEADTIME
#define CM_CONN_CONNDEADTIME		 0
#endif
#ifndef CM_CONN_HARDDEADTIME
#define CM_CONN_HARDDEADTIME             0
#endif
#ifndef CM_CONN_IDLEDEADTIME
#define CM_CONN_IDLEDEADTIME             0
#endif
#ifndef CM_CONN_IDLEDEADTIME_REP
#define CM_CONN_IDLEDEADTIME_REP         0
#endif
#ifndef CM_CONN_NATPINGINTERVAL
#define CM_CONN_NATPINGINTERVAL          0
#endif

#define CM_CONN_IFS_HARDDEADTIME       120
#define CM_CONN_IFS_CONNDEADTIME        60
#define CM_CONN_IFS_IDLEDEADTIME      1200
#define CM_CONN_IFS_IDLEDEADTIME_REP   180      /* must be larger than file server hard dead timeout = 120 */

extern unsigned short ConnDeadtimeout;
extern unsigned short HardDeadtimeout;
extern DWORD          RDRtimeout;
extern afs_uint32     rx_pmtu_discovery;

typedef struct cm_conn {
	struct cm_conn *nextp;		/* locked by cm_connLock */
	struct cm_server *serverp;	/* locked by cm_serverLock */
        struct rx_connection *rxconnp;	/* locked by mx */
        struct cm_user *userp;		/* locked by mx; a held reference */
        osi_mutex_t mx;			/* mutex for some of these fields */
        afs_int32 refCount;		/* Interlocked */
	int ucgen;			/* ucellp's generation number */
        afs_uint32 flags;		/* locked by mx */
	int cryptlevel;			/* encrytion status */
} cm_conn_t;

#define CM_CONN_FLAG_FORCE_NEW          1
#define CM_CONN_FLAG_REPLICATION        2

/*
 * structure used for tracking RPC progress
 * and for passing path info from the smb layer
 * to the cache manager functions.
 */
typedef struct cm_req {
    DWORD startTime;		/* GetTickCount() when this struct was initialized */
    int rpcError;		/* RPC error code */
    int volumeError;		/* volume error code */
    int accessError;		/* access error code */
    struct cm_server * errorServp;  /* server that reported a token/idle error other than expired */
    int tokenError;
    int idleError;
    int vnovolError;
    afs_uint32 flags;
    clientchar_t * tidPathp;
    clientchar_t * relPathp;
} cm_req_t;

/* flags in cm_req structure */
#define	CM_REQ_NORETRY		0x01
#define CM_REQ_NEW_CONN_FORCED  0x02
#define CM_REQ_SOURCE_SMB       0x04
#define CM_REQ_VOLUME_UPDATED   0x08
#define CM_REQ_WOW64            0x10
#define CM_REQ_SOURCE_REDIR     0x20
#define CM_REQ_OFFLINE_VOL_CHK  0x40

/*
 * Vice2 error codes
 * 3/20/85
 * Note:  all of the errors listed here are currently generated by the volume
 * package.  Other vice error codes, should they be needed, could be included
 * here also.
 */

#define VREADONLY	EROFS	/* Attempt to write a read-only volume */

/* Special error codes, which may require special handling (other than just
   passing them through directly to the end user) are listed below */

#define VICE_SPECIAL_ERRORS	101	/* Lowest special error code */

#define VSALVAGE	101	/* Volume needs salvage */
#define VNOVNODE	102	/* Bad vnode number quoted */
#define VNOVOL		103	/* Volume not attached, doesn't exist,
				   not created or not online */
#define VVOLEXISTS	104	/* Volume already exists */
#define VNOSERVICE	105	/* Volume is not in service (i.e. it's
				   is out of funds, is obsolete, or somesuch) */
#define VOFFLINE	106	/* Volume is off line, for the reason
				   given in the offline message */
#define VONLINE		107	/* Volume is already on line */
#define VDISKFULL	108 	/* ENOSPC - Partition is "full",
				   i.e. roughly within n% of full */
#define VOVERQUOTA	109	/* EDQUOT - Volume max quota exceeded */
#define VBUSY		110	/* Volume temporarily unavailable; try again.
				   The volume should be available again shortly;
				   if it isn't something is wrong.
				   Not normally to be propagated to the
				   application level */
#define VMOVED		111	/* Volume has moved to another server;
				   do a VGetVolumeInfo to THIS server to find
				   out where */
#define VIO             112     /* Vnode temporarily unaccessible, but not known
                                 * to be permanently bad. */
#define VRESTRICTED     120     /* Volume is restricted from using one or more
                                 * of the given residencies; do a
                                 * vos examine to find out the current
                                 * restrictions. */

#define VRESTARTING	-100    /* server is restarting, otherwise similar to
				   VBUSY above.  This is negative so that old
				   cache managers treat it as "server is down"*/

#include "cm_server.h"
#ifndef AFS_PTHREAD_ENV
#define AFS_PTHREAD_ENV 1
#endif
#include "rx.h"

/*
 * connp->serverp can be accessed without holding a lock because that
 * never changes once the connection is created.
 */
#define SERVERHAS64BIT(connp) (!((connp)->serverp->flags & CM_SERVERFLAG_NO64BIT))
#define SET_SERVERHASNO64BIT(connp) (cm_SetServerNo64Bit((connp)->serverp, TRUE))

#define SERVERHASINLINEBULK(connp) (!((connp)->serverp->flags & CM_SERVERFLAG_NOINLINEBULK))
#define SET_SERVERHASNOINLINEBULK(connp) (cm_SetServerNoInlineBulk((connp)->serverp, TRUE))

extern void cm_InitConn(void);

extern void cm_InitReq(cm_req_t *reqp);

extern int cm_Analyze(cm_conn_t *connp, struct cm_user *up, struct cm_req *reqp,
                      struct cm_fid *fidp, struct cm_cell *cellp,
                      afs_uint32 storeOp,
                      struct AFSFetchStatus *statusp,
                      struct AFSVolSync *volInfop,
                      cm_serverRef_t ** vlServerspp,
                      struct cm_callbackRequest *cbrp, long code);

extern long cm_ConnByMServers(struct cm_serverRef *, afs_uint32, struct cm_user *,
	cm_req_t *, cm_conn_t **);

extern long cm_ConnByServer(struct cm_server *, struct cm_user *, afs_uint32, cm_conn_t **);

extern long cm_ConnFromFID(struct cm_fid *, struct cm_user *, struct cm_req *,
	cm_conn_t **);

extern long cm_ConnFromVolume(struct cm_volume *volp, unsigned long volid,
                              struct cm_user *userp, cm_req_t *reqp,
                              cm_conn_t **connpp);

extern void cm_PutConn(cm_conn_t *connp);

extern void cm_GCConnections(cm_server_t *serverp);

extern struct rx_connection * cm_GetRxConn(cm_conn_t *connp);

extern void cm_ForceNewConnections(cm_server_t *serverp);

extern long cm_ServerAvailable(struct cm_fid *fidp, struct cm_user *userp);

extern long cm_GetServerList(struct cm_fid *fidp, struct cm_user *userp,
                             struct cm_req *reqp, afs_uint32 *replicated,
                             cm_serverRef_t ***serversppp);

extern long cm_GetVolServerList(struct cm_volume *volp, afs_uint32 volid,
                                struct cm_user *userp,
                                struct cm_req *reqp, afs_uint32 *replicated,
                                cm_serverRef_t ***serversppp);

#endif /*  OPENAFS_WINNT_AFSD_CM_CONN_H */
