/* Copyright (C) 2014 Geneviève Bastien <gbastien@versatic.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */

/* This test generates a single event and exits.
 */

#include <unistd.h>

#define TRACEPOINT_DEFINE
#define TRACEPOINT_CREATE_PROBES
#include "ust_tests_gtd.h"

int main(int argc, char *argv[])
{
	int i;
	char text[10] = "test";

	for (i = 0; i < 2; i++) {
		tracepoint(ust_tests_gtd, tptest, i%2, (i+1)%2);
		tracepoint(ust_tests_gtd, tptest_bis, i%2);
	}

	tracepoint(ust_tests_gtd, tptest_struct, 0, text, strlen(text));

	/*
	 * Add a normal tracepoint after, just to be sure the previous dynamic
	 * fields didn't wrongly offset the next event.
	 */
	tracepoint(ust_tests_gtd, tptest_bis, 1);

	return 0;
}
