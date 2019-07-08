#ifndef _TRANSPONDER_DRIVER_H_INCLUDED_
  #define _TRANSPONDER_DRIVER_H_INCLUDED_

void lch_set_admin_state(uint32_t lch, char const* const val);

void component_set_target_power(char const* const name, char const* const val);

void component_set_frequency(char const* const name, char const* const val);

void component_set_operational_mode(char const* const name, char const* const val);

void intHardwareCommunication();

void stopHardwareCommunication();

#endif