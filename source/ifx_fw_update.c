/* ifx_fw_update.c
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

/* This tool will perform a firmware update on Infineon SLB9672 or SLB9673 */

#include <wolftpm/tpm2_wrap.h>
#include "load_file.h"
#include "ifx_fw_update.h"

/******************************************************************************/
/* --- BEGIN TPM2.0 Firmware Update tool  -- */
/******************************************************************************/

typedef struct {
    byte*  manifest_buf;
    byte*  firmware_buf;
    size_t manifest_bufSz;
    size_t firmware_bufSz;
} fw_info_t;

static int TPM2_IFX_FwData_Cb(uint8_t* data, uint32_t data_req_sz,
    uint32_t offset, void* cb_ctx)
{
    fw_info_t* fwinfo = (fw_info_t*)cb_ctx;
    if (offset > fwinfo->firmware_bufSz) {
        return BUFFER_E;
    }
    if (offset + data_req_sz > (uint32_t)fwinfo->firmware_bufSz) {
        data_req_sz = (uint32_t)fwinfo->firmware_bufSz - offset;
    }
    if (data_req_sz > 0) {
        XMEMCPY(data, &fwinfo->firmware_buf[offset], data_req_sz);
    }
    return data_req_sz;
}

static const char* TPM2_IFX_GetOpModeStr(int opMode)
{
    const char* opModeStr = "Unknown";
    switch (opMode) {
        case 0x00:
            opModeStr = "Normal TPM operational mode";
            break;
        case 0x01:
            opModeStr = "TPM firmware update mode (abandon possible)";
            break;
        case 0x02:
            opModeStr = "TPM firmware update mode (abandon not possible)";
            break;
        case 0x03:
            opModeStr = "After successful update, but before finalize";
            break;
        case 0x04:
            opModeStr = "After finalize or abandon, reboot required";
            break;
        default:
            break;
    }
    return opModeStr;
}

static void TPM2_IFX_PrintInfo(WOLFTPM2_CAPS* caps)
{
    printf("Mfg %s (%d), Vendor %s, Fw %u.%u (0x%x)\n",
        caps->mfgStr, caps->mfg, caps->vendorStr, caps->fwVerMajor,
        caps->fwVerMinor, caps->fwVerVendor);
    printf("Operational mode: %s (0x%x)\n",
        TPM2_IFX_GetOpModeStr(caps->opMode), caps->opMode);
    printf("KeyGroupId 0x%x, FwCounter %d (%d same)\n",
        caps->keyGroupId, caps->fwCounter, caps->fwCounterSame);
}

int IFX_Firmware_Update(const char* manifest_file, const char* firmware_file)
{
    int rc;
    WOLFTPM2_DEV dev;
    WOLFTPM2_CAPS caps;
    fw_info_t fwinfo;
    int abandon = 0, recovery = 0;

    XMEMSET(&fwinfo, 0, sizeof(fwinfo));

    printf("Infineon Firmware Update Tool\n");
    rc = wolfTPM2_GetCapabilities(&dev, &caps);
    if (rc != TPM_RC_SUCCESS) {
        goto exit;
    }

    TPM2_IFX_PrintInfo(&caps);
    if (caps.keyGroupId == 0) {
        printf("Error getting key group id from TPM!\n");
    }
    if (caps.opMode == 0x02 || (caps.opMode & 0x80)) {
        /* if opmode == 2 or 0x8x then we need to use recovery mode */
        recovery = 1;
    }

    if (abandon) {
        printf("Firmware Update Abandon:\n");
        rc = wolfTPM2_FirmwareUpgradeCancel(&dev);
        if (rc != 0) {
            printf("Abandon failed 0x%x: %s\n", rc, TPM2_GetRCString(rc));
        }
        else {
            printf("Success: Please reset or power cycle TPM\n");
        }
        return rc;
    }

    if (manifest_file == NULL || firmware_file == NULL) {
        printf("Manifest file or firmware file arguments missing!\n");
        goto exit;
    }

    /* loadFile is used to load the manifest file and data file */
    rc = loadFile(manifest_file,
        &fwinfo.manifest_buf, &fwinfo.manifest_bufSz);
    if (rc == 0) {
        rc = loadFile(firmware_file,
            &fwinfo.firmware_buf, &fwinfo.firmware_bufSz);
    }
    if (rc == 0) {
        if (recovery) {
            printf("Firmware Update (recovery mode):\n");
            rc = wolfTPM2_FirmwareUpgradeRecover(&dev,
                fwinfo.manifest_buf, (uint32_t)fwinfo.manifest_bufSz,
                TPM2_IFX_FwData_Cb, &fwinfo);
        }
        else {
            printf("Firmware Update (normal mode):\n");
            rc = wolfTPM2_FirmwareUpgrade(&dev,
                fwinfo.manifest_buf, (uint32_t)fwinfo.manifest_bufSz,
                TPM2_IFX_FwData_Cb, &fwinfo);
        }
    }
    if (rc == 0) {
        printf("Firmware Update Complete\n");
    }

exit:

    if (rc != 0) {
        printf("Infineon firmware update failed 0x%x: %s\n",
            rc, TPM2_GetRCString(rc));
    }

    XFREE(fwinfo.firmware_buf, NULL, DYNAMIC_TYPE_TMP_BUFFER);
    XFREE(fwinfo.manifest_buf, NULL, DYNAMIC_TYPE_TMP_BUFFER);

    return rc;
}

/******************************************************************************/
/* --- END TPM2.0 Firmware Update tool  -- */
/******************************************************************************/
