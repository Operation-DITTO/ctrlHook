# Operation-DITTO ctrlHook

## About

This is the source code for the OP-DITTO ctrlHook plugin. It works by hooking the `sceCtrl` syscall and patching `sceCtrlReadBufferPositive()` to read from the serial controller and dump its contents into `SceCtrlData.Rsrv[]`.

---

## Explanation

By doing this, we are able to target compatibility with DualShock 3 supported commercial titles and any PSP homebrewed titles that have support for the PS Vita right analog stick. It also allows for easier porting of camera game patches originally targeted for the PS Vita.

---

## Building

All libraries are included, assuming the PSPSDK is properly installed, the ctrlHook `.prx` will build without issue by running `make` in the project root directory.

---

## Extra Credits

* uOFW Team - SystemControl libraries and `ctrl.prx` reversing. See [LICENSE_uOFW](LICENSE_uOFW) for licensing information!
* sajattack - Pointing towards the SystemControl libraries and header.
* Felix_Dev - Misc. Plugin assistance and an interesting conversation about `sceCtrlReadBufferNegative()`!