/**
  ******************************************************************************
  * @file    hid.h
  * @author  Melchor Varela - EA4FRB
  * @version V1.0
  * @date    27-Aug-2019
  * @brief   SARK110 DLL - HID Functions
  ******************************************************************************
  * @copy
  *
  *  This file is a part of the "SARK110 Antenna Vector Impedance Analyzer" software
  *
  *  "SARK110 Antenna Vector Impedance Analyzer software" is free software: you can redistribute it
  *  and/or modify it under the terms of the GNU General Public License as
  *  published by the Free Software Foundation, either version 3 of the License,
  *  or (at your option) any later version.
  *
  *  "SARK110 Antenna Vector Impedance Analyzer" software is distributed in the hope that it will be
  *  useful,  but WITHOUT ANY WARRANTY; without even the implied warranty of
  *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  *  GNU General Public License for more details.
  *
  *  You should have received a copy of the GNU General Public License
  *  along with "SARK110 Antenna Vector Impedance Analyzer" software.  If not,
  *  see <http://www.gnu.org/licenses/>.
  *
  * <h2><center>&copy; COPYRIGHT 2011-2019 Melchor Varela - EA4FRB </center></h2>
  *  Melchor Varela, Madrid, Spain.
  *  melchor.varela@gmail.com
  */
/** @addtogroup SARK110
  * @{
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __HID_H__
#define __HID_H__

int rawhid_open(int max, int vid, int pid, int usage_page, int usage);
int rawhid_recv(int num, void *buf, int len, int timeout);
int rawhid_send(int num, void *buf, int len, int timeout);
void rawhid_close(int num);

#endif	 /* __HID_H__ */

/**
  * @}
  */

/************* (C) COPYRIGHT 2011-2019 Melchor Varela - EA4FRB *****END OF FILE****/