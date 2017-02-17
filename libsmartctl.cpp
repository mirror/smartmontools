/*
 * libsmartctl.cpp
 *
 * Home page of code is: http://www.smartmontools.org
 *
 * Copyright (C) 2002-11 Bruce Allen
 * Copyright (C) 2008-17 Christian Franke
 * Copyright (C) 2000 Michael Cornwell <cornwell@acm.org>
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

#include <errno.h>
#include <getopt.h>
#include <stdarg.h>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "config.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#if defined(__FreeBSD__)
#include <sys/param.h>
#endif

#include "atacmds.h"
#include "ataprint.h"
#include "dev_interface.h"
#include "int64.h"
#include "knowndrives.h"
#include "libsmartctl.h"
#include "nvmeprint.h"
#include "scsicmds.h"
#include "scsiprint.h"
#include "smartctl.h"
#include "utility.h"

const char *smartctl_cpp_cvsid = "$Id$" CONFIG_H_CVSID SMARTCTL_H_CVSID;

// Will remove after abstracting out lib matrial from ataprint.cpp
// START HERE
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

/*
* @brief Compares failure type to policy.
*
* @param failure type of either OPTIONAL_CMD or MANDATORY_CMD
*
* @return true for pass, false for fail
 */
bool softfailuretest(failure_type type) {
  // If this is an error in an "optional" SMART command
  if (type == OPTIONAL_CMD) {
    if (!failuretest_conservative)
      return true;
    return false;
  }

  // If this is an error in a "mandatory" SMART command
  if (type == MANDATORY_CMD) {
    if (failuretest_permissive--)
      return true;
    return false;
  }

  throw std::logic_error("failuretest: Unknown type");
}

// END HERE

namespace libsmartctl {

Client &Client::getClient() {
  static Client c;
  return c;
}

// Default constructor
Client::Client() {
  check_config();
  // Initialize interface and check registration
  smart_interface::init();
  if (!smi()) {
    throw std::runtime_error("could not register smart-interface");
  }

  // database init has to occur after smart_interface::init();
  if (!init_drive_database(false)) {
    throw std::runtime_error("could not init drive db");
  }
}

ctlerr_t Client::initDevice(smart_device_auto_ptr &device,
                            std::string const &devname) {
  const char *type = 0;
  device = smi()->get_smart_device(devname.c_str(), type);
  if (!device) {
    return GETDEVICERR;
  }

  smart_device::device_info oldinfo = device->get_info();

  // Open with autodetect support, may return 'better' device
  device.replace(device->autodetect_open());

  if (!device->is_open()) {
    // std::cerr << "could not open smart device: " << device->get_info_name()
    //           << device->get_errmsg();
    return DEVICEOPENERR;
  }

  return NOERR;
}

DevInfoResp Client::getDevInfo(std::string const &devname) {
  DevInfoResp resp = {};

  ata_print_options ataopts;
  scsi_print_options scsiopts;
  nvme_print_options nvmeopts;

  ataopts.drive_info = scsiopts.drive_info = nvmeopts.drive_info = true;

  smart_device_auto_ptr device;
  resp.err = initDevice(device, devname);
  if (resp.err != NOERR) {
    return resp;
  }

  if (device->is_ata()) {
    resp.err = get_ata_information(resp.content, device->to_ata(), ataopts);
  }

  return resp;
}

DevVendorAttrsResp Client::getDevVendorAttrs(std::string const &devname) {
  DevVendorAttrsResp resp = {};

  ata_print_options ataopts;
  scsi_print_options scsiopts;
  nvme_print_options nvmeopts;

  ataopts.smart_vendor_attrib = scsiopts.smart_vendor_attrib =
      nvmeopts.smart_vendor_attrib = true;

  smart_device_auto_ptr device;
  resp.err = initDevice(device, devname);
  if (resp.err != NOERR) {
    return resp;
  }

  if (device->is_ata()) {
    resp.err = get_ata_vendor_attr(resp.content, device->to_ata(), ataopts);
  }

  return resp;
}
}
