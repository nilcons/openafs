/*
 * Copyright (c) 2008, 2009, 2010, 2011 Kernel Drivers, LLC.
 * Copyright (c) 2009, 2010, 2011 Your File System, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice,
 *   this list of conditions and the following disclaimer in the
 *   documentation
 *   and/or other materials provided with the distribution.
 * - Neither the names of Kernel Drivers, LLC and Your File System, Inc.
 *   nor the names of their contributors may be used to endorse or promote
 *   products derived from this software without specific prior written
 *   permission from Kernel Drivers, LLC and Your File System, Inc.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

//
// File: AFSClose.cpp
//

#include "AFSCommon.h"

//
// Function: AFSClose
//
// Description:
//
//      This function is the IRP_MJ_CLOSE dispatch handler
//
// Return:
//
//       A status is returned for the handling of this request
//

NTSTATUS
AFSClose( IN PDEVICE_OBJECT DeviceObject,
          IN PIRP Irp)
{

    NTSTATUS ntStatus = STATUS_SUCCESS;

    __try
    {

        if( DeviceObject == AFSDeviceObject)
        {

            AFSCompleteRequest( Irp,
                                ntStatus);

            try_return( ntStatus);
        }

        ntStatus = AFSCommonClose( DeviceObject,
                                   Irp);

try_exit:

        NOTHING;
    }
    __except( AFSExceptionFilter( GetExceptionCode(), GetExceptionInformation()) )
    {

        AFSDbgLogMsg( 0,
                      0,
                      "EXCEPTION - AFSClose\n");
    }

    return ntStatus;
}

NTSTATUS
AFSCommonClose( IN PDEVICE_OBJECT DeviceObject,
                IN PIRP Irp)
{

    NTSTATUS ntStatus = STATUS_SUCCESS;
    ULONG ulRequestType = 0;
    IO_STACK_LOCATION *pIrpSp = IoGetCurrentIrpStackLocation( Irp);
    AFSDeviceExt *pDeviceExt = NULL;
    AFSDeviceExt *pControlDeviceExt = (AFSDeviceExt *)AFSDeviceObject->DeviceExtension;

    __Enter
    {

        pDeviceExt = (AFSDeviceExt *)DeviceObject->DeviceExtension;

        pIrpSp = IoGetCurrentIrpStackLocation( Irp);

        if( pIrpSp->FileObject->FsContext == NULL)
        {

            AFSCompleteRequest( Irp, ntStatus);

            try_return( ntStatus);
        }

        //
        // Check the state of the library
        //

        ntStatus = AFSCheckLibraryState( Irp);

        if( !NT_SUCCESS( ntStatus) ||
            ntStatus == STATUS_PENDING)
        {

            if( ntStatus != STATUS_PENDING)
            {
                AFSCompleteRequest( Irp, ntStatus);
            }

            try_return( ntStatus);
        }

        IoSkipCurrentIrpStackLocation( Irp);

        ntStatus = IoCallDriver( pControlDeviceExt->Specific.Control.LibraryDeviceObject,
                                 Irp);

        //
        // Indicate the library is done with the request
        //

        AFSClearLibraryRequest();

try_exit:

        NOTHING;
    }

    return ntStatus;
}
