#ifndef SMARTCTL_ERRS_H_
#define SMARTCTL_ERRS_H_

#define SMARTCTL_ERRS_H_CVSID "$Id$\n"

#include <iostream>

// Failure types used by libsmartctl specific functions
enum ctlerr_t {
  NOERR = 1,
  POWERMODEBELOWOPTION,
  FAILEDDEVICEIDREAD,
  FAILEDSMARTCMD,
  GETDEVICERR,
  // invalid blk device specificied
  DEVICEOPENERR,
};

namespace libsmartctl {
std::string errstr(ctlerr_t err);
}

#endif
