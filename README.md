## ==========================================================
# smartmontools - S.M.A.R.T. utility toolset for Darwin/Mac OSX, FreeBSD, Linux, NetBSD, OpenBSD, Solaris, and Windows.
## ==========================================================

## libsmartctl Project Fork

The smartmontools toolset, although comprehensive, did not provide an interface
to programatically retrieving SMART information from devices.  This project
exposes smartmontools ability to retrieve device information and vendor
attributes as a static library.  At this time, only ATA devices are supported.

The `libsmartctl::Client` works much like the `smartctl` CLI tool.  You can pass
empty device type information, or explicitly pass it a hardware controller.
The client itself is a singleton, which on initialization checks the config,
initializes the SMART interface, and initialized the drive database.  Current
trunk is at Release 6.6 of upstream.

Example:
```c++
#include <smartmontools/libsmartctl.h>
#include <smartmontools/smartctl_errs.h>

int main() {
  // Initialize the client...
  libsmartctl::Client c;

  // Collect autodetected info like so...
  auto resp = c.getDevInfo(devname, "");
  if (resp.err != NOERR) {
    // Handle the error.
    std::cerr << "There was an error retrieving drive information: "
              << libsmartctl::errStr(resp.err) << "\n";
    return 1;
  }

  // The SMART information is in the "content" field of the response structure.
  for (const auto& entry : resp.content) {
    // The map key is the name of the datapoint, the map value is the value
    // of the datapoint.
    std::cout << entry.first << ": " << entry.second << "\n"
  }

  // Sometimes you may need to pass in an explict mass storage controller.
  // For example, lets try a MegaRAID controller (see smartctl man page for
  // different controller configurations)...
  for (size_t i = 0; i <= 127; i++) {
    std::string fullType = "megaraid," + std::to_string(i);

    // First we must we must try to ID the device with this type...
    auto cantId = c.cantIdDev(devname, fullType);
    if (cantId.err != NOERR) {
      std::cerr << "Error while trying to identify device: "
                << libsmartctl::errStr(cantId.err) << "\n";
      continue;
    }

    // If device is not identifiable, the type is invalid, so skip.
    // NOTE: The interface gaurantees that if true, the device is not
    // identifiable.  If the returns False, it does not gaurantee that the
    // device is identifiable.
    if (cantId.content) {
      continue;
    }

    // Now let's try to get the SMART information..
    resp = c.getDevInfo(devname, fullType);
    if (resp.err != NOERR) {
      std::cerr << "There was an error retrieving drive information with "
                      "hardware driver: "
                   << libsmartctl::errStr(resp.err) << "\n";
      return 1;
    }

    // Now you can do whatever you want with the information...
  ...
  }
```

To build:
```bash
$ git clone git@github.com:allanliu/smartmontools.git
$ ...
$ cd smartmontools
$ ./autogen.sh
$ ...
$ ./configure --prefix=/your/working/dir
$ ...
$ make
$ ...
$ make install
$ ...
```


## Contents of original README

$Id$

== HOME ==
The home for smartmontools is located at:

    http://www.smartmontools.org/

Please see this web site for updates, documentation, and for submitting
patches and bug reports.

You will find a mailing list for support and other questions at:

    https://listi.jpberlin.de/mailman/listinfo/smartmontools-support


== COPYING ==
Copyright (C) 2002-9 Bruce Allen
Copyright (C) 2004-15 Christian Franke

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2, or (at your option) any later
version.

You should have received a copy of the GNU General Public License (for
example COPYING).  If not, see <http://www.gnu.org/licenses/>.


== CREDITS ==
See AUTHORS file.


== OVERVIEW ==
smartmontools contains utilities that control and monitor storage
devices using the Self-Monitoring, Analysis and Reporting Technology
(SMART) system build into ATA/SATA and SCSI/SAS hard drives and
solid-state drives.  This is used to check the reliability of the
drive and to predict drive failures.


== CONTENTS ==
The suite contains two utilities:

smartctl is a command line utility designed to perform S.M.A.R.T. tasks
	 such as disk self-checks, and to report the S.M.A.R.T. status of
	 the disk.

smartd   is a daemon that periodically monitors S.M.A.R.T. status and
         reports errors and changes in S.M.A.R.T. attributes to syslog.


== OBTAINING SMARTMONTOOLS ==

Source tarballs
---------------

http://sourceforge.net/projects/smartmontools/files/

SVN
---

svn co http://svn.code.sf.net/p/smartmontools/code/trunk/smartmontools smartmontools

This will create a subdirectory called smartmontools containing the code.

To instead get the 5.38 release:

svn co http://svn.code.sf.net/p/smartmontools/code/tags/RELEASE_5_38/sm5 smartmontools

You can see what the different tags are by looking at
http://sourceforge.net/p/smartmontools/code/HEAD/tree/tags/

== BUILDING/INSTALLING SMARTMONTOOLS ==

Refer to the "INSTALL" file for detailed installation instructions.

== GETTING STARTED ==

To examine SMART data from a disk, try:
  smartctl -a /dev/sda
See the manual page 'man smartctl' for more information.

To start automatic monitoring of your disks with the smartd daemon,
try:
  smartd -d
to start the daemon in foreground (debug) mode, or
  smartd
to start the daemon in background mode.  This will log messages to
SYSLOG.  If you would like to get email warning messages, please set
up the configuration file smartd.conf with the '-m' mail warning
Directive.  See the manual page 'man smartd' for more information.
