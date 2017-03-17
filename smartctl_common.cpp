/*
 * smartctl_common.cpp
 *
 * Home page of code is: http://www.smartmontools.org
 *
 * Copyright (C) 2002-11 Bruce Allen
 * Copyright (C) 2008-17 Christian Franke
 * Copyright (C) 1999-2000 Michael Cornwell <cornwell@acm.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * You should have received a copy of the GNU General Public License
 * (for example COPYING); If not, see <http://www.gnu.org/licenses/>.
 *
 * This code was originally developed as a Senior Thesis by Michael Cornwell
 * at the Concurrent Systems Laboratory (now part of the Storage Systems
 * Research Center), Jack Baskin School of Engineering, University of
 * California, Santa Cruz. http://ssrc.soe.ucsc.edu/
 *
 */
#include <stdarg.h>
#include <stdio.h>

#include "config.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#if defined(__FreeBSD__)
#include <sys/param.h>
#endif

#include "atacmds.h"
#include "smartctl.h"

// Printing function (controlled by global printing_is_off)
// [From GLIBC Manual: Since the prototype doesn't specify types for
// optional arguments, in a call to a variadic function the default
// argument promotions are performed on the optional argument
// values. This means the objects of type char or short int (whether
// signed or not) are promoted to either int or unsigned int, as
// appropriate.]
void pout(const char *fmt, ...) {
  va_list ap;

  // initialize variable argument list
  va_start(ap, fmt);
  if (printing_is_off) {
    va_end(ap);
    return;
  }

  // print out
  vprintf(fmt, ap);
  va_end(ap);
  fflush(stdout);
  return;
}

// Globals to control printing
bool printing_is_switchable = false;
bool printing_is_off = false;

// Values for  --long only options, see parse_options()
// enum { opt_identify = 1000, opt_scan, opt_scan_open, opt_set, opt_smart };

// Checksum error mode
enum checksum_err_mode_t {
  CHECKSUM_ERR_WARN,
  CHECKSUM_ERR_EXIT,
  CHECKSUM_ERR_IGNORE
};

static checksum_err_mode_t checksum_err_mode = CHECKSUM_ERR_WARN;

// Used to warn users about invalid checksums. Called from atacmds.cpp.
// Action to be taken may be altered by the user.
void checksumwarning(const char *string) {
  // user has asked us to ignore checksum errors
  if (checksum_err_mode == CHECKSUM_ERR_IGNORE)
    return;

  pout("Warning! %s error: invalid SMART checksum.\n", string);

  // user has asked us to fail on checksum errors
  if (checksum_err_mode == CHECKSUM_ERR_EXIT)
    EXIT(FAILSMART);
}

// Globals to set failuretest() policy
bool failuretest_conservative = false;
unsigned char failuretest_permissive = 0;

// Need to stay here until we refactor ataprint.cpp to break common components
// needed for the libsmartctl and smartctl.
// Compares failure type to policy in effect, and either exits or
// simply returns to the calling routine.
// Used in ataprint.cpp and scsiprint.cpp.
void failuretest(failure_type type, int returnvalue) {
  // If this is an error in an "optional" SMART command
  if (type == OPTIONAL_CMD) {
    if (!failuretest_conservative)
      return;
    pout("An optional SMART command failed: exiting. Remove '-T conservative' "
         "option to continue.\n");
    EXIT(returnvalue);
  }

  // If this is an error in a "mandatory" SMART command
  if (type == MANDATORY_CMD) {
    if (failuretest_permissive--)
      return;
    pout("A mandatory SMART command failed: exiting. To continue, add one or "
         "more '-T permissive' options.\n");
    EXIT(returnvalue);
  }

  throw std::logic_error("failuretest: Unknown type");
}
