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
