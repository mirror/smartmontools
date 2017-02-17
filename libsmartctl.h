/*
 * libsmartctl.h
 *
 * Home page of code is: http://www.smartmontools.org
 *
 * Copyright (C) 2002-10 Bruce Allen
 * <smartmontools-support@lists.sourceforge.net>
 * Copyright (C) 2008-10 Christian Franke
 * <smartmontools-support@lists.sourceforge.net>
 * Copyright (C) 2000 Michael Cornwell <cornwell@acm.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * You should have received a copy of the GNU General Public License
 * (for example COPYING); if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * This code was originally developed as a Senior Thesis by Michael Cornwell
 * at the Concurrent Systems Laboratory (now part of the Storage Systems
 * Research Center), Jack Baskin School of Engineering, University of
 * California, Santa Cruz. http://ssrc.soe.ucsc.edu/
 *
 */

#ifndef LIBSMARTCTL_H_
#define LIBSMARTCTL_H_

#define LIBSMARTCTL_H_CVSID "$Id$\n"

#include <map>

#include "atacmds.h"
#include "ataprint.h"
#include "config.h"
#include "dev_interface.h"
#include "knowndrives.h"
#include "scsicmds.h"
#include "scsiprint.h"
#include "utility.h"

namespace libsmartctl {

std::string errstr(ctlerr_t err);

struct DevInfoResp {
  std::map<std::string, std::string> content;
  ctlerr_t err;
};

struct DevVendorAttrsResp {
  std::vector<std::map<std::string, std::string>> content;
  ctlerr_t err;
};

class Client {
public:
  /**
   * @brief GetSmartCtl provides the interface to retrieve an instance of the
   * singleton class Client.
   *
   * @return reference to the instance of Client.  Will throw a runtime error if
   * initialization routines do not succeed.
   */
  static Client &getClient();

  /**
   *@brief Retrieves drive information
   *
   * @param string of full path device name
   *
   * @return map of information type and value, both strings
   */
  DevInfoResp getDevInfo(std::string const &devname);

  /**
   *@brief Retrieves drive vendor attributes
   *
   * @param string of full path device name
   *
   * @return a vector maps containing string key and values of vendor attribute
   *type name and attribute type values
   */
  DevVendorAttrsResp getDevVendorAttrs(std::string const &devname);

  Client(Client const &l) = delete;
  void operator=(Client const &l) = delete;

private:
  // Client default constructor intializes the smart interface and default
  // database.  User supplied drive databases are not supported at this.
  Client();

  ctlerr_t initDevice(smart_device_auto_ptr &device,
                      std::string const &devname);
};
}
#endif
