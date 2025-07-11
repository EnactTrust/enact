/* tpm.c
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
#include "enact.h"
#include "tpm.h"
#include "endianess.h"

#ifdef DEBUG_VERBOSE
static int verbose = 1;
#else
static int verbose = 0;
#endif

void tpm_printError(int verbose, int ret)
{
    if(verbose) printf("TPM error 0x%x: %s\n", ret, TPM2_GetRCString(ret));
}

int tpm_init(ENACT_TPM *tpm)
{
    int ret = ENACT_ERROR;

    if(tpm == NULL) {
        return BAD_ARG;
    }

    if(wolfTPM2_Init(&tpm->dev, NULL, NULL) != TPM_RC_SUCCESS) {
        printf("Unable to access the TPM. Check your permissions.\n");
    }
    else {
        ret = ENACT_SUCCESS;
    }

    return ret;
}


int tpm_deinit(ENACT_TPM *tpm)
{
    int ret = 0;

    if(tpm == NULL) {
        return BAD_ARG;
    }

    ret = wolfTPM2_Cleanup(&tpm->dev);
    if(ret == TPM_RC_SUCCESS) {
        ret = ENACT_SUCCESS;
        if(verbose) printf("TPM device released\n");
    }
    else {
        ret = ENACT_ERROR;
        if(verbose) printf("Failure while releasing the TPM device\n");
    }

    return ret;
}

int tpm_get_ekcert(ENACT_TPM *tpm, const char *filename)
{
    int ret = ENACT_ERROR;
    NV_Read_In nvReadCmd;
    NV_Read_Out nvReadResp;
    NV_ReadPublic_In nvReadPubCmd;
    NV_ReadPublic_Out nvReadPubResp;
    word32 chunkSize, dataSize, readSize = 0;
    size_t fileSize;
    XFILE fp = NULL;

    /* nvReadPubCmd is just nvindex that we set below */
    XMEMSET(&nvReadCmd, 0, sizeof(nvReadCmd));
    XMEMSET(&nvReadResp, 0, sizeof(nvReadResp));
    nvReadPubCmd.nvIndex = TPM2_NV_ECC_EK_CERT;
    XMEMSET(&nvReadPubResp, 0, sizeof(nvReadPubResp));

    chunkSize = 512; /* Safe NV read step */
    if(verbose) printf("NVRead step is %d\n", chunkSize);

    ret = TPM2_NV_ReadPublic(&nvReadPubCmd, &nvReadPubResp);
    if(ret != TPM_RC_SUCCESS) {
        return ret;
    }
    printf("Found EKCert (%d bytes).\n", nvReadPubResp.nvPublic.nvPublic.dataSize);
    dataSize = nvReadPubResp.nvPublic.nvPublic.dataSize;

    /* Prepare file to store the EK Certificate */
    fp = XFOPEN(ENACT_EKCERT_FILENAME, "wt");
    if(fp == XBADFILE) {
        printf("Error creating a file to store the EK certificate\n");
        return ret;
    }

    /* Set Auth */
    wolfTPM2_SetAuthPassword(&tpm->dev, 0, NULL);
    wolfTPM2_UnsetAuth(&tpm->dev, 1);
    wolfTPM2_UnsetAuth(&tpm->dev, 2);

    /* Prepare NV Read command */
    nvReadCmd.authHandle = nvReadPubCmd.nvIndex;
    nvReadCmd.nvIndex = nvReadPubCmd.nvIndex;
    /* Read in steps from the TPM's NVRAM */
    while(dataSize != readSize) {
        nvReadCmd.offset = readSize;
        if(dataSize - readSize < chunkSize) {
            nvReadCmd.size = dataSize - readSize;
        }
        else {
            nvReadCmd.size = chunkSize;
        }

        ret = TPM2_NV_Read(&nvReadCmd, &nvReadResp);
        if(ret == TPM_RC_SUCCESS) {
            if(verbose) printf("NVRead %d bytes\n", nvReadResp.data.size);
            readSize += nvReadResp.data.size;
            fileSize = XFWRITE(nvReadResp.data.buffer, 1, nvReadResp.data.size, fp);
            if(fileSize != nvReadResp.data.size) {
                if(verbose) printf("Error while storing the EK Certificate\n");
                break;
            }
        }
        else {
            tpm_printError(verbose, ret);
            break;
        }
    }
    XFCLOSE(fp);

    if(dataSize == readSize) {
        ret = ENACT_SUCCESS;
        if(verbose) printf("Read EKCert of %d size\n", readSize);
    }
    return ret;
}

int tpm_get_property(ENACT_TPM *tpm, UINT32 tag, UINT32 *value)
{
    int ret = ENACT_SUCCESS;
    GetCapability_In cmdGetCap;
    GetCapability_Out respGetCap;

    cmdGetCap.capability = TPM_CAP_TPM_PROPERTIES;
    cmdGetCap.property = tag;
    cmdGetCap.propertyCount = 1; /* ask for one property */
    ret = TPM2_GetCapability(&cmdGetCap, &respGetCap);
    if(ret == TPM_RC_SUCCESS) {
        ret = ENACT_SUCCESS;
        printf("PT NV Read MAX is %d\n",
               respGetCap.capabilityData.data.tpmProperties.tpmProperty[0].value);
        *value = respGetCap.capabilityData.data.tpmProperties.tpmProperty[0].value;
    }
    else {
        printf("Failed to read TPM property\n");
    }

    return ret;
}

int tpm_get_info(ENACT_TPM *tpm)
{
    int ret;
    ENACT_TPM_INFO *tpm_info = &tpm->tpm_info;

    tpm_info->fwversion.value = 0;

    ret = wolfTPM2_GetCapabilities(&tpm->dev, &tpm_info->caps);
    if (ret == TPM_RC_SUCCESS) {
        tpm_info->fwversion.part.major = htobe16(tpm_info->caps.fwVerMajor);
        tpm_info->fwversion.part.minor = htobe16(tpm_info->caps.fwVerMinor);
        tpm_info->fwversion.part.vendor = htobe32(tpm_info->caps.fwVerVendor);
    }

    if(verbose) {
        printf("TPM vendor %s (%d), Model %s, FW %u.%u\n",
            tpm_info->caps.mfgStr, tpm_info->caps.mfg, tpm_info->caps.vendorStr,
            tpm_info->caps.fwVerMajor, tpm_info->caps.fwVerMinor);
        printf("\nFW version as hex: %016lX\n", tpm_info->fwversion.value);
    }
    return ret;
}
