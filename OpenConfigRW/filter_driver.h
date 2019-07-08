#ifndef _FILTER_DRIVER_H_INCLUDED_
  #define _FILTER_DRIVER_H_INCLUDED_


void intFilterCommunication();

void stopFilterCommunication();

void wss_m_set(uint32_t connection_id, char const* const  m_value);

void wss_n_set(uint32_t connection_id, char const* const  n_value);

void wss_inport_set(uint32_t connection_id, char const* const  inport);

void wss_outport_set(uint32_t connection_id, char const* const  outport);

#endif