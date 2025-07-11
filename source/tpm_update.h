/* tpm_update.h
 *
 * Copyright (C) 2018-2025 DesignFirst OU
 *
 * This file is part of EnactTrust.
 *
 * EnactTrust is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * EnactTrust is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with EnactTrust.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef _ENACT_TPM_UPDATE_H_
#define _ENACT_TPM_UPDATE_H_

#include "enact.h"
#include "ifx_fw_update.h"

#ifdef __cplusplus
    extern "C" {
#endif

#define DEBUG_NO_TPM_UPDATE 0

int tpm_update(void);

#ifdef __cplusplus
    }
#endif

#endif /* _ENACT_TPM_UPDATE_H_ */
