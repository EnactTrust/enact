/* ifx_firmware_update.h
 *
 * Copyright (C) 2006-2025 wolfSSL Inc.
* Copyright (C) 2024-2025 DesignFirst OU
 *
 * This file is part of wolfTPM.
 *
 * wolfTPM is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * wolfTPM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1335, USA
 */

#ifndef _IFX_FIRMWARE_H_
#define _IFX_FIRMWARE_H_

#ifdef __cplusplus
    extern "C" {
#endif

int IFX_Firmware_Update(const char* manifest_file, const char* firmware_file);
int ifx_fw_extract(char *fw_filename);

#ifdef __cplusplus
    }  /* extern "C" */
#endif

#endif /* _IFX_FIRMWARE_H_ */
