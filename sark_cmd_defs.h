/**
  ******************************************************************************
  * @file    sark_cmd_defs.h
  * @author  Melchor Varela - EA4FRB
  * @version V1.0
  * @date    27-Aug-2019
  * @brief   Remote commands
  ******************************************************************************
  * @copy
  *
  *  This file is a part of the "SARK110 Antenna Vector Impedance Analyzer" software
  *
  *  "SARK110 Antenna Vector Impedance Analyzer" software is free software: you can redistribute it
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
  *  along with "SARK110 Antenna Vector Impedance Analyzer" firmware.  If not,
  *  see <http://www.gnu.org/licenses/>.
  *
  * <h2><center>&copy; COPYRIGHT 2011-2019 Melchor Varela - EA4FRB </center></h2>
  *  Melchor Varela, Madrid, Spain.
  *  melchor.varela@gmail.com
  */

/** @addtogroup SARK
  * @{
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SARK_CMD_DEFS_H__
#define __SARK_CMD_DEFS_H__

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define CMD_SARK_VERSION		1	/* Returns version of the protocol */
#define CMD_SARK_MEAS_RX		2	/* Measures R and X */
#define CMD_SARK_MEAS_VECTOR	3	/* Measures raw vector data */
#define CMD_SARK_SIGNAL_GEN		4	/* Signal generator */
#define CMD_SARK_MEAS_RF		5	/* Measures RF */
#define CMD_SARK_MEAS_VEC_THRU	6	/* Measures raw vector thru data */
#define CMD_BATT_STAT			7	/* Battery charger status */
#define CMD_DISK_INFO			8	/* Get disk information */
#define CMD_DISK_VOLUME			9	/* Get disk volume name */
#define CMD_SET_SETTING			10	/* Sets device setting */
#define CMD_GET_SETTING			11	/* Gets device setting */
#define CMD_SARK_MEAS_RX_EFF	12	/* Measures R and X; efficient (x4 samples) */
#define CMD_BUZZER				20	/* Sounds buzzer */
#define CMD_GET_KEY				21	/* Get key */
#define CMD_DEV_RST				50	/* Reset */
#define CMD_GPIO				60	/* GPIO command */

/* Position 0 in responses */
#define ANS_SARK_OK				'O'
#define ANS_SARK_ERR			'E'

/* Position 5 in CMD_SARK_MEAS_RX */
#define PAR_SARK_CAL			1	/* OSL calibrated val */
#define PAR_SARK_UNCAL			2	/* Raw val */

/* Commands CMD_SET_SETUP and CMD_GET_SETUP */
#define SETTING_SAMPLING		0
#define SETTING_FILTER			1
#define SETTING_RUN             2

/* GPIO Command */
#define GPIO_MODE				0
#define GPIO_WRITE				1
#define GPIO_READ				2

#define GPIO_MODE_IPU			0
#define GPIO_MODE_IPD			1
#define GPIO_MODE_IN_FLOAT		2
#define GPIO_MODE_OUT_PP		3

#define SARKCMD_TX_SIZE			18	/* transmit command size */
#define SARKCMD_RX_SIZE			18	/* receive command size  */

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#endif	 /* __SARK_CMD_DEFS_H__ */

/**
  * @}
  */

/************* (C) COPYRIGHT 2011-2019 Melchor Varela - EA4FRB *****END OF FILE****/

