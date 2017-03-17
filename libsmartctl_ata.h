/*
 * libsmartctl_ata.h
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

#ifndef LIBSMARTCTL_ATA_H_
#define LIBSMARTCTL_ATA_H_

#define LIBSMARTCTL_ATA_H_CVSID "$Id$\n"

#include <iostream>
#include <map>

#include "ataprint.h"
#include "smartctl_errs.h"

void get_ata(std::map<std::string, std::string> &results, ata_device *device,
             const ata_print_options &options);

bool cant_id(ata_device *device);

ctlerr_t get_ata_information(std::map<std::string, std::string> &results,
                             ata_device *device,
                             const ata_print_options &options);

ctlerr_t
get_ata_vendor_attr(std::vector<std::map<std::string, std::string>> &results,
                    ata_device *device, const ata_print_options &options);

#endif
