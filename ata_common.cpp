/*
 * ata_common.cpp
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

#include "ata_common.h"
#include "atacmds.h"

const char *ata_common_cpp_cvsid = "$Id$" ATA_COMMON_H_CVSID;

static int find_msb(unsigned short word) {
  for (int bit = 15; bit >= 0; bit--)
    if (word & (1 << bit))
      return bit;
  return -1;
}

const char *infofound(const char *output) {
  return (*output ? output : "[No Information Found]");
}

const char *get_form_factor(unsigned short word168) {
  // Table A.32 of T13/2161-D (ACS-3) Revision 4p, September 19, 2013
  // Table 236 of T13/BSR INCITS 529 (ACS-4) Revision 04, August 25, 2014
  switch (word168) {
  case 0x1:
    return "5.25 inches";
  case 0x2:
    return "3.5 inches";
  case 0x3:
    return "2.5 inches";
  case 0x4:
    return "1.8 inches";
  case 0x5:
    return "< 1.8 inches";
  case 0x6:
    return "mSATA"; // ACS-4
  case 0x7:
    return "M.2"; // ACS-4
  case 0x8:
    return "MicroSSD"; // ACS-4
  case 0x9:
    return "CFast"; // ACS-4
  default:
    return 0;
  }
}

const char *get_ata_major_version(const ata_identify_device *drive) {
  switch (find_msb(drive->major_rev_num)) {
  case 10:
    return "ACS-3";
  case 9:
    return "ACS-2";
  case 8:
    return "ATA8-ACS";
  case 7:
    return "ATA/ATAPI-7";
  case 6:
    return "ATA/ATAPI-6";
  case 5:
    return "ATA/ATAPI-5";
  case 4:
    return "ATA/ATAPI-4";
  case 3:
    return "ATA-3";
  case 2:
    return "ATA-2";
  case 1:
    return "ATA-1";
  default:
    return 0;
  }
}

const char *get_ata_minor_version(const ata_identify_device *drive) {
  // Table 10 of X3T13/2008D (ATA-3) Revision 7b, January 27, 1997
  // Table 28 of T13/1410D (ATA/ATAPI-6) Revision 3b, February 26, 2002
  // Table 31 of T13/1699-D (ATA8-ACS) Revision 6a, September 6, 2008
  // Table 46 of T13/BSR INCITS 529 (ACS-4) Revision 08, April 28, 2015
  switch (drive->minor_rev_num) {
  case 0x0001:
    return "ATA-1 X3T9.2/781D prior to revision 4";
  case 0x0002:
    return "ATA-1 published, ANSI X3.221-1994";
  case 0x0003:
    return "ATA-1 X3T9.2/781D revision 4";
  case 0x0004:
    return "ATA-2 published, ANSI X3.279-1996";
  case 0x0005:
    return "ATA-2 X3T10/948D prior to revision 2k";
  case 0x0006:
    return "ATA-3 X3T10/2008D revision 1";
  case 0x0007:
    return "ATA-2 X3T10/948D revision 2k";
  case 0x0008:
    return "ATA-3 X3T10/2008D revision 0";
  case 0x0009:
    return "ATA-2 X3T10/948D revision 3";
  case 0x000a:
    return "ATA-3 published, ANSI X3.298-1997";
  case 0x000b:
    return "ATA-3 X3T10/2008D revision 6"; // 1st ATA-3 revision with SMART
  case 0x000c:
    return "ATA-3 X3T13/2008D revision 7 and 7a";
  case 0x000d:
    return "ATA/ATAPI-4 X3T13/1153D revision 6";
  case 0x000e:
    return "ATA/ATAPI-4 T13/1153D revision 13";
  case 0x000f:
    return "ATA/ATAPI-4 X3T13/1153D revision 7";
  case 0x0010:
    return "ATA/ATAPI-4 T13/1153D revision 18";
  case 0x0011:
    return "ATA/ATAPI-4 T13/1153D revision 15";
  case 0x0012:
    return "ATA/ATAPI-4 published, ANSI NCITS 317-1998";
  case 0x0013:
    return "ATA/ATAPI-5 T13/1321D revision 3";
  case 0x0014:
    return "ATA/ATAPI-4 T13/1153D revision 14";
  case 0x0015:
    return "ATA/ATAPI-5 T13/1321D revision 1";
  case 0x0016:
    return "ATA/ATAPI-5 published, ANSI NCITS 340-2000";
  case 0x0017:
    return "ATA/ATAPI-4 T13/1153D revision 17";
  case 0x0018:
    return "ATA/ATAPI-6 T13/1410D revision 0";
  case 0x0019:
    return "ATA/ATAPI-6 T13/1410D revision 3a";
  case 0x001a:
    return "ATA/ATAPI-7 T13/1532D revision 1";
  case 0x001b:
    return "ATA/ATAPI-6 T13/1410D revision 2";
  case 0x001c:
    return "ATA/ATAPI-6 T13/1410D revision 1";
  case 0x001d:
    return "ATA/ATAPI-7 published, ANSI INCITS 397-2005";
  case 0x001e:
    return "ATA/ATAPI-7 T13/1532D revision 0";
  case 0x001f:
    return "ACS-3 T13/2161-D revision 3b";

  case 0x0021:
    return "ATA/ATAPI-7 T13/1532D revision 4a";
  case 0x0022:
    return "ATA/ATAPI-6 published, ANSI INCITS 361-2002";

  case 0x0027:
    return "ATA8-ACS T13/1699-D revision 3c";
  case 0x0028:
    return "ATA8-ACS T13/1699-D revision 6";
  case 0x0029:
    return "ATA8-ACS T13/1699-D revision 4";

  case 0x0031:
    return "ACS-2 T13/2015-D revision 2";

  case 0x0033:
    return "ATA8-ACS T13/1699-D revision 3e";

  case 0x0039:
    return "ATA8-ACS T13/1699-D revision 4c";

  case 0x0042:
    return "ATA8-ACS T13/1699-D revision 3f";

  case 0x0052:
    return "ATA8-ACS T13/1699-D revision 3b";

  case 0x005e:
    return "ACS-4 T13/BSR INCITS 529 revision 5";

  case 0x006d:
    return "ACS-3 T13/2161-D revision 5";

  case 0x0082:
    return "ACS-2 published, ANSI INCITS 482-2012";

  case 0x0107:
    return "ATA8-ACS T13/1699-D revision 2d";

  case 0x010a:
    return "ACS-3 published, ANSI INCITS 522-2014";

  case 0x0110:
    return "ACS-2 T13/2015-D revision 3";

  case 0x011b:
    return "ACS-3 T13/2161-D revision 4";

  default:
    return 0;
  }
}

const char *get_pata_version(unsigned short word222, char (&buf)[32]) {
  switch (word222 & 0x0fff) {
  default:
    snprintf(buf, sizeof(buf), "Unknown (0x%03x)", word222 & 0x0fff);
    return buf;
  case 0x001:
  case 0x003:
    return "ATA8-APT";
  case 0x002:
    return "ATA/ATAPI-7";
  }
}

const char *get_sata_version(unsigned short word222, char (&buf)[32]) {
  switch (find_msb(word222 & 0x0fff)) {
  default:
    snprintf(buf, sizeof(buf), "SATA >3.2 (0x%03x)", word222 & 0x0fff);
    return buf;
  case 7:
    return "SATA 3.2";
  case 6:
    return "SATA 3.1";
  case 5:
    return "SATA 3.0";
  case 4:
    return "SATA 2.6";
  case 3:
    return "SATA 2.5";
  case 2:
    return "SATA II Ext";
  case 1:
    return "SATA 1.0a";
  case 0:
    return "ATA8-AST";
  case -1:
    return "Unknown";
  }
}

inline const char *get_sata_speed(int level) {
  if (level <= 0)
    return 0;
  switch (level) {
  default:
    return ">6.0 Gb/s (7)";
  case 6:
    return ">6.0 Gb/s (6)";
  case 5:
    return ">6.0 Gb/s (5)";
  case 4:
    return ">6.0 Gb/s (4)";
  case 3:
    return "6.0 Gb/s";
  case 2:
    return "3.0 Gb/s";
  case 1:
    return "1.5 Gb/s";
  }
}

const char *get_sata_maxspeed(const ata_identify_device *drive) {
  unsigned short word076 = drive->words047_079[76 - 47];
  if (word076 & 0x0001)
    return 0;
  return get_sata_speed(find_msb(word076 & 0x00fe));
}

const char *get_sata_curspeed(const ata_identify_device *drive) {
  unsigned short word077 = drive->words047_079[77 - 47];
  if (word077 & 0x0001)
    return 0;
  return get_sata_speed((word077 >> 1) & 0x7);
}
