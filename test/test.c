/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * HOWOT test:
 * 1. cd build/android
 * 2. ndk-build
 * 3. adb root && adb remount
 * 4. adb push libs/armeabi/libamemcheck.so /system/lib/
 *    adb push libs/armeabi/libamemcheck_malloc_debug_leak.so /system/lib/
 *    adb push obj/local/armeabi/amemcheck_test /system/bin/  (with debug symbols)
 * 5. adb shell setprop libamc.debug.malloc 10  (same as libc.debug.malloc)
 * 6. adb shell amemcheck_test
 * 7. adb logcat:
 * D/AMemcheck( 1335): [AMemcheck][malloc_leak_check]: initialize amemcheck malloc_debug component.
 * D/AMemcheck( 1335): [AMemcheck][libc]: Messi: using libamc.debug.malloc 10 (chk)
 * D/AMemcheck( 1335): [AMemcheck][libc]: +++ ALLOCATION 0xc0c1b0 SIZE 40 BYTES MULTIPLY FREED!
 * D/AMemcheck( 1335): [AMemcheck][libc]: +++ ALLOCATION 0xc0c1b0 SIZE 40 ALLOCATED HERE:
 * D/AMemcheck( 1335): [AMemcheck][libc]: *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** ***
 * D/AMemcheck( 1335): [AMemcheck][libc]:                          #00  pc 0000349a        /system/lib/libamemcheck_malloc_debug_leak.so (chk_malloc+0x11)
 * D/AMemcheck( 1335): [AMemcheck][libc]:                          #01  pc 00002318        /system/lib/libamemcheck.so (amemcheck_malloc+0xb)
 * D/AMemcheck( 1335): [AMemcheck][libc]:                          #02  pc 00000d36        /system/bin/amemcheck_test
 * D/AMemcheck( 1335): [AMemcheck][libc]:                          #03  pc 0000e402        /system/lib/libc.so (__libc_init+0x31)
 * D/AMemcheck( 1335): [AMemcheck][libc]: +++ ALLOCATION 0xc0c1b0 SIZE 40 FIRST FREED HERE:
 * D/AMemcheck( 1335): [AMemcheck][libc]: *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** ***
 * D/AMemcheck( 1335): [AMemcheck][libc]:                          #00  pc 000035f8        /system/lib/libamemcheck_malloc_debug_leak.so (chk_free+0x13f)
 * D/AMemcheck( 1335): [AMemcheck][libc]:                          #01  pc 0000232c        /system/lib/libamemcheck.so (amemcheck_free+0xb)
 * D/AMemcheck( 1335): [AMemcheck][libc]:                          #02  pc 00000d3c        /system/bin/amemcheck_test
 * D/AMemcheck( 1335): [AMemcheck][libc]:                          #03  pc 0000e402        /system/lib/libc.so (__libc_init+0x31)
 * D/AMemcheck( 1335): [AMemcheck][libc]: +++ ALLOCATION 0xc0c1b0 SIZE 40 NOW BEING FREED HERE:
 * D/AMemcheck( 1335): [AMemcheck][libc]: *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** ***
 * D/AMemcheck( 1335): [AMemcheck][libc]:                          #00  pc 000034d6        /system/lib/libamemcheck_malloc_debug_leak.so (chk_free+0x1d)
 * D/AMemcheck( 1335): [AMemcheck][libc]:                          #01  pc 0000232c        /system/lib/libamemcheck.so (amemcheck_free+0xb)
 * D/AMemcheck( 1335): [AMemcheck][libc]:                          #02  pc 00000d42        /system/bin/amemcheck_test
 * D/AMemcheck( 1335): [AMemcheck][libc]:                          #03  pc 0000e402        /system/lib/libc.so (__libc_init+0x31)
 * D/AMemcheck( 1335): [AMemcheck][libc]: +++ ALLOCATION 0xc0c330 IS CORRUPTED OR NOT ALLOCATED VIA TRACKER!
 * D/AMemcheck( 1335): [AMemcheck][libc]: *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** ***
 * D/AMemcheck( 1335): [AMemcheck][libc]:                          #00  pc 000034d6        /system/lib/libamemcheck_malloc_debug_leak.so (chk_free+0x1d)
 * D/AMemcheck( 1335): [AMemcheck][libc]:                          #01  pc 0000232c        /system/lib/libamemcheck.so (amemcheck_free+0xb)
 * D/AMemcheck( 1335): [AMemcheck][libc]:                          #02  pc 00000d52        /system/bin/amemcheck_test
 * D/AMemcheck( 1335): [AMemcheck][libc]:                          #03  pc 0000e402        /system/lib/libc.so (__libc_init+0x31)
 * D/AMemcheck( 1335): [AMemcheck][malloc_leak_check]: finalize amemcheck malloc_debug component.
 * D/AMemcheck( 1335): [AMemcheck][libc]: +++
 * D/AMemcheck( 1335): [AMemcheck][libc]: +++ /system/bin/amemcheck_test ReportMemoryLeaks: +++
 * D/AMemcheck( 1335): [AMemcheck][libc]: +++
 * D/AMemcheck( 1335): [AMemcheck][libc]: +++ /system/bin/amemcheck_test leaked block of size 80 at 0xc0c2b8 (leak 1 of 1)
 * D/AMemcheck( 1335): [AMemcheck][libc]: *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** ***
 * D/AMemcheck( 1335): [AMemcheck][libc]:                          #00  pc 0000349a        /system/lib/libamemcheck_malloc_debug_leak.so (chk_malloc+0x11)
 * D/AMemcheck( 1335): [AMemcheck][libc]:                          #01  pc 00002318        /system/lib/libamemcheck.so (amemcheck_malloc+0xb)
 * D/AMemcheck( 1335): [AMemcheck][libc]:                          #02  pc 00000d48        /system/bin/amemcheck_test
 * D/AMemcheck( 1335): [AMemcheck][libc]:                          #03  pc 0000e402        /system/lib/libc.so (__libc_init+0x31)
 * D/AMemcheck( 1335): [AMemcheck][libc]: +++ REAR GUARD MISMATCH [0, 32)
 * D/AMemcheck( 1335): [AMemcheck][libc]: +++ ALLOCATION 0xc0c0c0 SIZE 20 HAS A CORRUPTED REAR GUARD
 * D/AMemcheck( 1335): [AMemcheck][libc]: +++ ALLOCATION 0xc0c0c0 SIZE 20 ALLOCATED HERE:
 * D/AMemcheck( 1335): [AMemcheck][libc]: *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** ***
 * D/AMemcheck( 1335): [AMemcheck][libc]:                          #00  pc 0000349a        /system/lib/libamemcheck_malloc_debug_leak.so (chk_malloc+0x11)
 * D/AMemcheck( 1335): [AMemcheck][libc]:                          #01  pc 00002318        /system/lib/libamemcheck.so (amemcheck_malloc+0xb)
 * D/AMemcheck( 1335): [AMemcheck][libc]:                          #02  pc 00000d20        /system/bin/amemcheck_test
 * D/AMemcheck( 1335): [AMemcheck][libc]:                          #03  pc 0000e402        /system/lib/libc.so (__libc_init+0x31)
 * D/AMemcheck( 1335): [AMemcheck][libc]: +++ ALLOCATION 0xc0c0c0 SIZE 20 FREED HERE:
 * D/AMemcheck( 1335): [AMemcheck][libc]: *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** ***
 * D/AMemcheck( 1335): [AMemcheck][libc]:                          #00  pc 000035f8        /system/lib/libamemcheck_malloc_debug_leak.so (chk_free+0x13f)
 * D/AMemcheck( 1335): [AMemcheck][libc]:                          #01  pc 0000232c        /system/lib/libamemcheck.so (amemcheck_free+0xb)
 * D/AMemcheck( 1335): [AMemcheck][libc]:                          #02  pc 00000d30        /system/bin/amemcheck_test
 * D/AMemcheck( 1335): [AMemcheck][libc]:                          #03  pc 0000e402        /system/lib/libc.so (__libc_init+0x31)
 */

#include "amemcheck.h"

#include <stdlib.h>

int main(int argc, char **argv) {
	int i;
	int *corrupted_mem = NULL;
	int *double_free_mem = NULL;
	int *no_free_mem = NULL;
	int *other_mem = NULL;

	//init
	amemcheck_init();

	//1. corrupted mem
	corrupted_mem = (int *)amemcheck_malloc(20);
	for (i = 0; i <= 20; i++) {
		corrupted_mem[i] = i;
	}
	amemcheck_free(corrupted_mem);

	//2. double free
	double_free_mem = (int *)amemcheck_malloc(40);
	amemcheck_free(double_free_mem);
	amemcheck_free(double_free_mem);

	//3. no free
	no_free_mem = (int *)amemcheck_malloc(80);

	//4. not allocated via our amemcheck tracker
	other_mem = (int *)malloc(100);
	amemcheck_free(other_mem);

	//fini
	amemcheck_fini();

    return 0;
}
