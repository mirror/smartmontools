/*
 * ata_common.h
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

#ifndef ATA_COMMON_H_
#define ATA_COMMON_H_

#define ATA_COMMON_H_CVSID "$Id$\n"

#include "atacmds.h"

const char *infofound(const char *output);

const char *get_form_factor(unsigned short word168);

const char *get_ata_minor_version(const ata_identify_device *drive);

const char *get_ata_major_version(const ata_identify_device *drive);

const char *get_pata_version(unsigned short word222, char (&buf)[32]);

const char *get_sata_version(unsigned short word222, char (&buf)[32]);

const char *get_sata_maxspeed(const ata_identify_device *drive);

const char *get_sata_curspeed(const ata_identify_device *drive);

#endif
