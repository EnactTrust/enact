/* tpm_update.c
 *
 * Copyright (C) 2018-2025 DesignFirst OU
 *
 * This file is part of EnactTrust.
 *
 * EnactTrust for Linux is a free software;
 * you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation;
 * either version 2 of the License, or (at your option) any later version.
 *
 * EnactTrust for Linux is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with EnactTrust. If not, see <https://www.gnu.org/licenses/>.
 */


#include <stdio.h>
#include <string.h>
/* wolfTPM TPM 2.0 library - GPLv2 */
#include <wolftpm/options.h>
/* EnactTrust */
#include "tpm_update.h"
#include "ifx_fw_update.h"

#ifdef DEBUG_VERBOSE
static int verbose = 1;
#else
static int verbose = 0;
#endif

int tpm_update(void)
{
    int ret;

    (void)verbose;

    printf("TPM update: Extracting...\n");
    ret = ifx_fw_extract(ENACT_TPM_UPDATE_FILE);
    if(ret) {
        printf("error %X (decimal = %d)\n", ret, ret);
    }
#if (DEBUG_NO_TPM_UPDATE > 0u)
    printf("Debugging without performing the firmware update\n");
#else
    else {
        ret = IFX_Firmware_Update(ENACT_IFX_MANIFEST_FILE, ENACT_IFX_DATA_FILE);
    }
#endif

    return ret;
}
