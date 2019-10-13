//
//  sysctlhandler.c
//  sysctlhandler
//
//  Created by will on 2019/10/13.
//  Copyright © 2019 will. All rights reserved.
//

#include <mach/mach_types.h>
#include <sys/sysctl.h>
#include <libkern/OSAtomic.h>

kern_return_t sysctlhandler_start(kmod_info_t * ki, void *d);
kern_return_t sysctlhandler_stop(kmod_info_t *ki, void *d);


char g_szBuffer[0x1000] = {0};
static UInt32 g_oid_registered = 0;

int sysctl_handler(
    struct sysctl_oid *oid,
    void *arg1,
    int arg2,
    struct sysctl_req *req)
{
    int status = 0;
    printf("[kern:will]:[pre:%s]\n", g_szBuffer);
    status = sysctl_handle_string(oid, arg1, arg2, req);
    
    if (!status && req->newptr) {
        printf("[kern:will]:[aft:%s]\n", g_szBuffer);
    }
    
    return status;
}

SYSCTL_OID(
           _kern,
           OID_AUTO,
           will,
           CTLTYPE_STRING | CTLFLAG_RW | CTLFLAG_KERN,
           g_szBuffer,
           sizeof(g_szBuffer),
           sysctl_handler,
           "A",
           ""
           );


/*
 promote:~ test$ sysctl -w kern.will="helloworld"
 kern.will:
 sysctl: kern.will=helloworld: Operation not permitted
 promote:~ test$ sudo sysctl -w kern.will="helloworld"
 Password:
 kern.will:  -> helloworld
 promote:~ test$ sysctl kern.will
 kern.will: helloworld

 */


kern_return_t sysctl_register(boolean_t flag)
{
    kern_return_t status = KERN_FAILURE;
    
    if (flag)
    {
        //register will's handler
        sysctl_register_oid(&sysctl__kern_will);
        OSCompareAndSwap(0, 1, &g_oid_registered);
    }
    else
    {
        //unregister
        if (OSCompareAndSwap(1, 0, &g_oid_registered)) {
            sysctl_unregister_oid(&sysctl__kern_will);
        }
    }
    return status;
}


kern_return_t sysctlhandler_start(kmod_info_t * ki, void *d)
{
    sysctl_register(TRUE);
    printf("[will]start");
    return KERN_SUCCESS;
}

kern_return_t sysctlhandler_stop(kmod_info_t *ki, void *d)
{
    sysctl_register(FALSE);
    printf("[will]exit");
    return KERN_SUCCESS;
}
