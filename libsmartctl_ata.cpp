/*
 * libsmartctl_ata.cpp
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

#include <iostream>
#include <map>

#include "config.h"

#include "ata_common.h"
#include "atacmds.h"
#include "ataidentify.h"
#include "ataprint.h"
#include "int64.h"
#include "knowndrives.h"
#include "libsmartctl_ata.h"
#include "smartctl_errs.h"
#include "utility.h"

const char *libsmartctl_ata_cpp_cvsid =
    "$Id$" CONFIG_H_CVSID LIBSMARTCTL_ATA_H_CVSID;

static void get_drive_info(std::map<std::string, std::string> &results,
                           const ata_identify_device *drive,
                           const ata_size_info &sizes, int rpm,
                           const drive_settings *dbentry) {

  // format drive information (with byte swapping as needed)
  char model[40 + 1], serial[20 + 1], firmware[8 + 1];
  ata_format_id_string(model, drive->model, sizeof(model) - 1);
  ata_format_id_string(serial, drive->serial_no, sizeof(serial) - 1);
  ata_format_id_string(firmware, drive->fw_rev, sizeof(firmware) - 1);

  // Print model family if known
  if (dbentry && *dbentry->modelfamily) {
    results["model_family"] = dbentry->modelfamily;
  }

  results["device_model"] = infofound(model);

  if (!dont_print_serial_number) {
    results["serial_number"] = infofound(serial);

    unsigned oui = 0;
    uint64_t unique_id = 0;
    int naa = ata_get_wwn(drive, oui, unique_id);
    if (naa >= 0) {
      results["LU_WWN_device_id"] =
          strprintf("%x %06x %09" PRIx64, naa, oui, unique_id);
    }
  }

  // Additional Product Identifier (OEM Id) string in words 170-173
  // (e08130r1, added in ACS-2 Revision 1, December 17, 2008)
  if (0x2020 <= drive->words088_255[170 - 88] &&
      drive->words088_255[170 - 88] <= 0x7e7e) {
    char add[8 + 1];
    ata_format_id_string(
        add, (const unsigned char *)(drive->words088_255 + 170 - 88),
        sizeof(add) - 1);

    if (add[0]) {
      results["additional_product_id"] = add;
    }
  }

  results["firmware_version"] = infofound(firmware);

  if (sizes.capacity) {
    // Print capacity
    char num[64], cap[32];
    results["user_capacity"] =
        strprintf("%s bytes [%s]",
                  format_with_thousands_sep(num, sizeof(num), sizes.capacity),
                  format_capacity(cap, sizeof(cap), sizes.capacity));

    // Print sector sizes.
    if (sizes.phy_sector_size == sizes.log_sector_size) {
      results["sector_sizes"] =
          strprintf("%u bytes logical/physical", sizes.log_sector_size);

    } else {
      results["sector_sizes"] =
          strprintf("%u bytes logical, %u bytes physical",
                    sizes.log_sector_size, sizes.phy_sector_size);

      if (sizes.log_sector_offset) {
        results["sector_sizes"] =
            results["sector_sizes"] +
            strprintf(" (offset %u bytes)", sizes.log_sector_offset);
      }
    }
  }

  // Print nominal media rotation rate if reported
  if (rpm) {
    if (rpm == 1) {
      results["rotation_rate"] = "solid state device";
    } else if (rpm > 1) {
      results["rotation_rate"] = strprintf("%d rpm", rpm);
    } else {
      results["rotation_rate"] = strprintf("Unknown (0x%04x)", -rpm);
    }
  }

  // Print form factor if reported
  unsigned short word168 = drive->words088_255[168 - 88];
  if (word168) {
    const char *form_factor = get_form_factor(word168);
    if (form_factor) {
      results["form_factor"] = form_factor;

    } else {
      results["form_factor"] = strprintf("Unknown (0x%04x)\n", word168);
    }
  }

  // See if drive is recognized
  results["in_smartctl_db"] = (!dbentry ? "0" : "1");

  // Print ATA version
  std::string ataver;
  if ((drive->major_rev_num != 0x0000 && drive->major_rev_num != 0xffff) ||
      (drive->minor_rev_num != 0x0000 && drive->minor_rev_num != 0xffff)) {
    const char *majorver = get_ata_major_version(drive);
    const char *minorver = get_ata_minor_version(drive);

    if (majorver && minorver && str_starts_with(minorver, majorver)) {
      // Major and minor strings match, print minor string only
      ataver = minorver;
    } else {
      if (majorver)
        ataver = majorver;
      else
        ataver = strprintf("Unknown(0x%04x)", drive->major_rev_num);

      if (minorver)
        ataver += strprintf(", %s", minorver);
      else if (drive->minor_rev_num != 0x0000 && drive->minor_rev_num != 0xffff)
        ataver += strprintf(" (unknown minor revision code: 0x%04x)",
                            drive->minor_rev_num);
      else
        ataver += " (minor revision not indicated)";
    }
  }
  results["ata_version"] = infofound(ataver.c_str());

  // Print Transport specific version
  // cppcheck-suppress variableScope
  char buf[32] = "";
  unsigned short word222 = drive->words088_255[222 - 88];
  if (word222 != 0x0000 && word222 != 0xffff)
    switch (word222 >> 12) {
    case 0x0: // PATA
      results["transport_type"] =
          strprintf(" Parallel, %s\n", get_pata_version(word222, buf));
      break;
    case 0x1: // SATA
    {
      const char *sataver = get_sata_version(word222, buf);
      const char *maxspeed = get_sata_maxspeed(drive);
      const char *curspeed = get_sata_curspeed(drive);

      results["sata_version"] =
          strprintf("%s%s%s%s%s%s\n", sataver, (maxspeed ? ", " : ""),
                    (maxspeed ? maxspeed : ""), (curspeed ? " (current: " : ""),
                    (curspeed ? curspeed : ""), (curspeed ? ")" : ""));

    } break;

    case 0xe: // PCIe (ACS-4)
      results["transport_type"] =
          strprintf("PCIe (0x%03x)\n", word222 & 0x0fff);
      break;

    default:
      results["transport_type"] = strprintf("Unknown (0x%04x)\n", word222);
      break;
    }

  char timedatetz[DATEANDEPOCHLEN];
  dateandtimezone(timedatetz);
  results["local_time"] = timedatetz;

  // Print warning message, if there is one
  if (dbentry && *dbentry->warningmsg) {
    results["warnings"] = dbentry->warningmsg;
  }
}

static void
get_smart_attr_w_thres(std::vector<std::map<std::string, std::string>> &results,
                       const ata_smart_values *data,
                       const ata_smart_thresholds_pvt *thresholds,
                       const ata_vendor_attr_defs &defs, int rpm,
                       int onlyfailed, unsigned char format) {
  bool hexid = !!(format & ata_print_options::FMT_HEX_ID);
  bool hexval = !!(format & ata_print_options::FMT_HEX_VAL);

  // step through all vendor attributes
  for (int i = 0; i < NUMBER_ATA_SMART_ATTRIBUTES; i++) {
    // each attribute can be thought of as a row
    std::map<std::string, std::string> a_row;

    const ata_smart_attribute &attr = data->vendor_attributes[i];

    // Check attribute and threshold
    unsigned char threshold = 0;
    ata_attr_state state = ata_get_attr_state(
        attr, i, thresholds->thres_entries, defs, &threshold);
    if (state == ATTRSTATE_NON_EXISTING)
      continue;

    // These break out of the loop if we are only printing certain entries...
    if (onlyfailed == 1 && !(ATTRIBUTE_FLAGS_PREFAILURE(attr.flags) &&
                             state == ATTRSTATE_FAILED_NOW))
      continue;

    if (onlyfailed == 2 && state < ATTRSTATE_FAILED_PAST)
      continue;

    if (state > ATTRSTATE_NO_NORMVAL)
      a_row["value"] = (!hexval ? strprintf("%.3d", attr.current)
                                : strprintf("0x%02x", attr.current));
    else
      a_row["value"] = (!hexval ? "---" : "----");
    if (!(defs[attr.id].flags & ATTRFLAG_NO_WORSTVAL))
      a_row["worst"] = (!hexval ? strprintf("%.3d", attr.worst)
                                : strprintf("0x%02x", attr.worst));
    else
      a_row["worst"] = (!hexval ? "---" : "----");
    if (state > ATTRSTATE_NO_THRESHOLD)
      a_row["threshold"] = (!hexval ? strprintf("%.3d", threshold)
                                    : strprintf("0x%02x", threshold));
    else
      a_row["threshold"] = (!hexval ? "---" : "----");

    // Print line for each valid attribute
    a_row["id"] =
        (!hexid ? strprintf("%3d", attr.id) : strprintf("0x%02x", attr.id));
    a_row["attr_name"] = ata_get_smart_attr_name(attr.id, defs, rpm);
    a_row["raw_value"] = ata_format_attr_raw_value(attr, defs);
    a_row["flag"] = strprintf("0x%04x", attr.flags);
    a_row["flag_type"] =
        (ATTRIBUTE_FLAGS_PREFAILURE(attr.flags) ? "Pre-fail" : "Old_age");
    a_row["flag_updated"] =
        (ATTRIBUTE_FLAGS_ONLINE(attr.flags) ? "Always" : "Offline");
    a_row["when_failed"] =
        (state == ATTRSTATE_FAILED_NOW
             ? "FAILING_NOW"
             : state == ATTRSTATE_FAILED_PAST ? "In_the_past" : "    -");

    results.push_back(a_row);
  }
}

ctlerr_t
get_ata_vendor_attr(std::vector<std::map<std::string, std::string>> &results,
                    ata_device *device, const ata_print_options &options) {

  ata_identify_device drive;
  memset(&drive, 0, sizeof(drive));
  unsigned char raw_drive[sizeof(drive)];
  memset(&raw_drive, 0, sizeof(raw_drive));

  device->clear_err();

  if (!ataDoesSmartWork(device)) {
    return FAILEDSMARTCMD;
  }

  ata_vendor_attr_defs attribute_defs = options.attribute_defs;
  firmwarebug_defs firmwarebugs = options.firmwarebugs;
  const drive_settings *dbentry = 0;
  if (!options.ignore_presets)
    dbentry = lookup_drive_apply_presets(&drive, attribute_defs, firmwarebugs);
  // Get capacity, sector sizes and rotation rate
  ata_size_info sizes;
  ata_get_size_info(&drive, sizes);
  int rpm = ata_get_rotation_rate(&drive);

  // Read SMART values and thresholds if necessary
  ata_smart_values smartval;
  memset(&smartval, 0, sizeof(smartval));
  ata_smart_thresholds_pvt smartthres;
  memset(&smartthres, 0, sizeof(smartthres));

  // Print vendor-specific attributes
  if (ataReadSmartValues(device, &smartval) == 0 &&
      ataReadSmartThresholds(device, &smartthres) == 0) {
    get_smart_attr_w_thres(results, &smartval, &smartthres, attribute_defs, rpm,
                           (printing_is_switchable ? 2 : 0),
                           options.output_format);
  }

  return NOERR;
}

bool cant_id(ata_device *device) {
  ata_identify_device drive;
  memset(&drive, 0, sizeof(drive));

  if ((smartcommandhandler(device, IDENTIFY, 0, (char *)&drive))) {
    if (smartcommandhandler(device, PIDENTIFY, 0, (char *)&drive)) {
      return true;
    }
  }

  return false;
}

ctlerr_t get_ata_information(std::map<std::string, std::string> &results,
                             ata_device *device,
                             const ata_print_options &options) {

  // Start by getting Drive ID information.  We need this, to know if SMART is
  // supported.
  ata_identify_device drive;
  memset(&drive, 0, sizeof(drive));
  unsigned char raw_drive[sizeof(drive)];
  memset(&raw_drive, 0, sizeof(raw_drive));

  device->clear_err();
  int retid =
      ata_read_identity(device, &drive, options.fix_swapped_id, raw_drive);

  if (retid < 0 || !nonempty(&drive, sizeof(drive))) {
    return FAILEDDEVICEIDREAD;
  }

  // Use preset vendor attribute options unless user has requested otherwise.
  ata_vendor_attr_defs attribute_defs = options.attribute_defs;
  firmwarebug_defs firmwarebugs = options.firmwarebugs;
  const drive_settings *dbentry = 0;
  if (!options.ignore_presets)
    dbentry = lookup_drive_apply_presets(&drive, attribute_defs, firmwarebugs);

  // Get capacity, sector sizes and rotation rate
  ata_size_info sizes;
  ata_get_size_info(&drive, sizes);
  int rpm = ata_get_rotation_rate(&drive);

  get_drive_info(results, &drive, sizes, rpm, dbentry);

  // Check and print SMART support and state
  int smart_supported = -1, smart_enabled = -1;

  // Packet device ?
  if (retid > 0) {
    results["smart_supported"] = "no";
    results["smart_enabled"] = "no";
    results["packet_device_type"] = packetdevicetype(retid - 1);

  } else {
    // Disk device: SMART supported and enabled ?
    smart_supported = ataSmartSupport(&drive);
    smart_enabled = ataIsSmartEnabled(&drive);

    if (smart_supported && smart_enabled < 0) {
      if (ataDoesSmartWork(device)) {
        smart_supported = smart_enabled = 1;
      }

    } else if (smart_supported < 0 && (smart_enabled > 0 || dbentry)) {
      // Assume supported if enabled or in drive database
      smart_supported = 1;
    }

    if (smart_supported < 0) {
      results["smart_supported"] = "unknown - Try to enable with smartctl";

    } else if (!smart_supported) {
      results["smart_supported"] = "no";

    } else {
      if (options.drive_info) {
        results["smart_supported"] = "yes";
      }

      if (smart_enabled >= 0) {
        if (device->ata_identify_is_cached()) {
          // Check if it does work if the status is OS cached
          smart_enabled = ataDoesSmartWork(device);
        }

        results["smart_enabled"] = (smart_enabled ? "yes" : "no");
      }
    }
  }

  return NOERR;
}
