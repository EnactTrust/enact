/* enact.h
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

#ifndef _ENACT_H_
#define _ENACT_H_

#include "enact_api.h"
#include "wolftpm/tpm2_types.h"
#include <wolftpm/tpm2_wrap.h>

#ifdef __cplusplus
    extern "C" {
#endif

#define ENACT_VERSION_STRING "1.0.1"
#define ENACT_VERSION_HEX 0x00100010

#define DEBUG_NO_TPM 0

/* Return codes */
#define ENACT_SUCCESS        0
#define ENACT_ERROR         -1
#define ENACT_ERROR_START   -2
#define ENACT_ERROR_FILE    -3
#define ENACT_ERROR_TPM     -4
#define ENACT_ABORT         -64
#define NOT_PUBLIC          -126
#define NOT_IMPLEMENTED     -127
#define BAD_ARG             -128

#define UUID_V4_SIZE    36
#define UUID_V4_CHARS   32
#define UUID_V4_BYTES   16

#define ENACT_API_TOKEN_FILE    "api.token\0"
#define ENACT_TPM_FWVER_FILE    "TPMversion.bin\0"
#define ENACT_TPM_UPDATE_FILE   "TPMupdate.bin\0"
#define ENACT_TPM_LICENSE_FILE  "TPMlicense.txt\0"
#define ENACT_IFX_MANIFEST_FILE "IFXmanifest.bin\0"
#define ENACT_IFX_DATA_FILE     "IFXdata.bin\0"
#define ENACT_NODEID_TEMPFILE   "node.id\0"
#define ENACT_EKCERT_FILENAME   "ek.cert\0"

#define ENACT_TPM_QUOTE_PCR   23
#define ENACT_NONCE_SIZE      16
#define ENACT_API_TOKEN_SIZE  32
#define ENACT_EMAIL_CODE_SIZE 6

#define MAX_ENACT_FW_VERSION 8
#define MAX_ENACT_EMAIL_SIZE 64
#define MAX_ENACT_TOKEN_SIZE 32
#define MAX_PEM_SIZE         512
#define MAX_FILE_COUNT       20
#define MAX_FILE_NAME        100
#define MAX_ENACT_RESPONSE_SIZE 2048
#define MAX_ENACT_TPM_UPDATE_SIZE 1024*1024 // 1 MB
#define MAX_ENACT_TPM_LICENSE_SIZE 10*1024   //10 kB

typedef union ENACT_TPM_FW_VERSION {
    struct {
        uint16_t major;
        uint16_t minor;
        uint32_t vendor;
    } part;
    uint64_t value;
} ENACT_TPM_FW_VERSION;

typedef struct ENACT_TPM_INFO {
    WOLFTPM2_CAPS caps;
    ENACT_TPM_FW_VERSION fwversion;
} ENACT_TPM_INFO;

typedef struct ENACT_TPM {
    ENACT_TPM_INFO tpm_info;
    WOLFTPM2_DEV dev;
    WOLFTPM2_KEY ek;
    WOLFTPM2_KEY primary; /* Storage Key */
    WOLFTPM2_KEY ak; /* Attestation Key */
    WOLFTPM2_SESSION sessionA; /* Param Enc */
#ifdef ENACT_TPM_GPIO_ENABLE
    ENACT_TPM_GPIO gpio;
#endif
} ENACT_TPM;

typedef struct ENACT_EVIDENCE {
    char nodeid[UUID_V4_BYTES];
    char nonce[ENACT_NONCE_SIZE];
    TPM2B_ATTEST raw;
    TPMS_ATTEST data;
    TPMT_SIGNATURE signature;
} ENACT_EVIDENCE;

typedef struct ENACT_BACKEND_RESPONSE {
    char buffer[MAX_ENACT_RESPONSE_SIZE];
    size_t size;
    int complete;
} ENACT_BACKEND_RESPONSE;

typedef struct ENACT_API_TOKEN {
    char buffer[MAX_ENACT_TOKEN_SIZE];
    size_t size;
} ENACT_API_TOKEN;

typedef struct ENACT_EMAIL_CODE {
    char buffer[MAX_ENACT_TOKEN_SIZE];
    size_t size;
} ENACT_EMAIL_CODE;

typedef struct ENACT_EMAIL {
    char buffer[MAX_ENACT_EMAIL_SIZE];
    size_t size;
} ENACT_EMAIL;

typedef enum ENACT_API_CALL {
    ENACT_API_CALL_UNSET = 0,
    ENACT_API_CALL_TRIAL,
    ENACT_API_CALL_UPDATE_ONCE,
    ENACT_API_CALL_GET_LICENSE,
    ENACT_API_CALL_REGISTER,
    ENACT_API_CALL_VERIFY,
    ENACT_API_CALL_MAX
} ENACT_API_CALL;

#ifdef __cplusplus
    }
#endif

#endif /* _ENACT_H_ */
