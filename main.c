//
// OP-DITTO ctrlHook (c) 2021 by Operation Ditto Team
// 
// OP-DITTO ctrlHook is licensed under a
// Creative Commons Attribution-ShareAlike 4.0 International License.
// 
// You should have received a copy of the license along with this
// work. If not, see <http://creativecommons.org/licenses/by-sa/4.0/>.
//
// OP-DITTO ctrlHook is loosely based upon the syscall hook example 
// written by fLaSh, it can be found here:
// https://wololo.net/talk/viewtopic.php?f=5&t=11689
//
#include <pspctrl.h>
#include <pspdisplay.h>
#include <pspdisplay_kernel.h>
#include <pspkernel.h>
#include <pspmodulemgr.h>
#include <psputility_sysparam.h>
#include "systemctrl.h"

#define PSP_KERNEL_MODULE_NAME          "sceKernelLibrary"

#define COMMANDS_RIGHT_ANALOG           0xB

PSP_MODULE_INFO("op_ditto", 0x1000, 0, 1);
PSP_MAIN_THREAD_ATTR(0);

void    pspUARTInit(int baud);                                 
int     pspUARTRead(void);                                  
void    pspUARTWrite(int ch);                

void *hooked_readbuffer_func;

unsigned char xAxis = 0;
unsigned char yAxis = 0;

static void waitForKernel()
{
    /* Wait for the kernel to boot */
    while (sceKernelFindModuleByName(PSP_KERNEL_MODULE_NAME) == NULL)
    {        
        sceKernelDelayThread(10000); /* wait 10 milliseconds */
    }
}

s32 sceCtrlReadBufferPositive_patch(SceCtrlData *data, u8 nBufs)
{
    int k1 = pspSdkSetK1(0);

    // create a function ptr to sceCtrlReadBufferPositive() to fill the buttons normally
    s32 (*hooked_readbuffer_func)(SceCtrlData*, u8) = (s32 (*)(SceCtrlData*, u8))sctrlHENFindFunction("sceController_Service", "sceCtrl", 0x1F803938);
    hooked_readbuffer_func(data, nBufs);

    data->Rsrv[0] = xAxis;
    data->Rsrv[1] = yAxis;

    pspSdkSetK1(k1);
    return 0;
}

int main_thread(SceSize args, void *argp)
{ 
    // dont do anything until we can confirm all modules are loaded
    waitForKernel();    

    // enable analog sampling in the kernel
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);   

    // grab readbufferpositive from the NID
    hooked_readbuffer_func = (void *)sctrlHENFindFunction("sceController_Service", "sceCtrl", 0x1F803938); 

    // patch it
    sctrlHENPatchSyscall(hooked_readbuffer_func, sceCtrlReadBufferPositive_patch);
 
    return 0;
}

int SIO_thread(SceSize args, void *argp)
{ 
    // dont do anything until we can confirm all modules are loaded
    waitForKernel();    

    // Initialize the psp-uart-library
    pspUARTInit(115200);

    while (1) {
        pspUARTWrite(COMMANDS_RIGHT_ANALOG);

        // small delay to wait for response
        sceKernelDelayThread(100);

        int valueX = pspUARTRead();
        int valueY = pspUARTRead();

        if (valueX != -1 && valueY != -1) {
            xAxis = valueX;
            yAxis = valueY;
        }
        // maybe should clear the buffer on else condition
        // If we didnt get the response to command expected the contents of the buffer could be compromised
        // Possibly reset stick to center position?

        // May not be needed as we have the delay anove. Doesnt seem to introduce noticeable lag however
        sceKernelDelayThread(9900);
    }
 
    return 0;
}

/* Create a user thread */
int module_start(SceSize args, void *argp)
{ 
    SceUID thid;

    thid = sceKernelCreateThread("thread", main_thread, 0x18, 0x500, 0, NULL);
    if (thid >= 0) {
        sceKernelStartThread(thid, args, argp);
    }
    
    SceUID ioThid;

    ioThid = sceKernelCreateThread("IOthread", SIO_thread, 0x18, 0x500, 0, NULL);
    if (ioThid >= 0) {
        sceKernelStartThread(ioThid, args, argp);
    }
    return 0;
}

/* terminate the entire module, so we can return to XMB without freezing */
int module_stop(SceSize args, void *argp)
{
    SceUID thid;   
    pspSdkReferThreadStatusByName("thread", &thid, NULL);
    
    sceKernelTerminateThread(thid);
    
    SceUID ioThid;   
    pspSdkReferThreadStatusByName("IOthread", &ioThid, NULL);
    
    sceKernelTerminateThread(ioThid);
    return 0;
}