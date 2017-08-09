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
  UNSUPPORTEDDEVICETYPE,
  CLIENTINITIALIZTIONFAILURE,
};

namespace libsmartctl {
/**
 * @brief Returns std::string version of ctlerr_t
 *
 * @param err ctlerr_t to stringify
 *
 * @return std::string represntation of err code
 */
const std::string errStr(ctlerr_t err);
} // namespace libsmartctl

#endif
