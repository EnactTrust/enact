/* enact_api.h
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

#ifndef _ENACT_API_H_
#define _ENACT_API_H_

#ifdef __cplusplus
    extern "C" {
#endif

/* EnactTrust API */
#define ENACT_API_USER_TRIAL    "user/trial"
#define ENACT_API_USER_REGISTER "user/register"
#define ENACT_API_USER_VERIFY   "user/verify"
#define ENACT_API_USER_LOGIN    "user/login"
#define ENACT_API_TPM_LIST      "tpm/list"
/* Enterprise only */
#define ENACT_API_TPM_ONBOARD   "tpm/onboard"
#define ENACT_API_TPM_UPDATE    "tpm/update"
/* Update API */
#define ENACT_API_TPM_UPDATE_ONCE       "tpm/update-once"
#define ENACT_API_TPM_UPDATE_COMPLETE   "tpm/update-complete"
/* TPM vendor license */
#define ENACT_API_TPM_GET_LICENSE       "tpm/get-license"

/* User related arguments */
#define ENACT_API_USER_ARG_EMAIL        "email"
#define ENACT_API_USER_ARG_EMAIL_TOKEN  "token" /* email verification code */
#define ENACT_API_USER_ARG_API_TOKEN    "api_token"
/* TPM related arguments */
#define ENACT_API_TPM_ARG_NODEID        "node_id"
#define ENACT_API_TPM_ARG_FW_VERSION    "fw_version"
/* Update-complete */
#define ENACT_API_TPM_ARG_STATUS        "status"

/* Endpoints */
#define URL_LOCALHOST   "http://localhost:8000/"
/* Set server */
#ifdef ON_PREMISE
#define ENACT_BACKEND   "https://intranet.siemens.com:8095/"
#else /* EnactTrust A3S */
#define ENACT_BACKEND   "https://api.enacttrust.com/"
#endif
/* Complete endpoint URL */
#define URL_BACKEND_USER_TRIAL          ENACT_BACKEND ENACT_API_USER_TRIAL
#define URL_BACKEND_USER_REGISTER       ENACT_BACKEND ENACT_API_USER_REGISTER
#define URL_BACKEND_USER_VERIFY         ENACT_BACKEND ENACT_API_USER_VERIFY
#define URL_BACKEND_USER_LOGIN          ENACT_BACKEND ENACT_API_USER_LOGIN
#define URL_BACKEND_TPM_LIST            ENACT_BACKEND ENACT_API_TPM_LIST
#define URL_BACKEND_TPM_UPDATE_ONCE     ENACT_BACKEND ENACT_API_TPM_UPDATE_ONCE
#define URL_BACKEND_TPM_UPDATE_COMPLETE ENACT_BACKEND ENACT_API_TPM_UPDATE_COMPLETE
#define URL_BACKEND_TPM_GET_LICENSE     ENACT_BACKEND ENACT_API_TPM_GET_LICENSE
/* Only for local development */
#define URL_LOCALHOST_USER_TRIAL          URL_LOCALHOST ENACT_API_USER_TRIAL
#define URL_LOCALHOST_TPM_UPDATE_ONCE     URL_LOCALHOST ENACT_API_TPM_UPDATE_ONCE
#define URL_LOCALHOST_TPM_UPDATE_COMPLETE URL_LOCALHOST ENACT_API_USER_VERIFY

/* Endpoints in use - production or development (localhost) */
#define URL_USER_TRIAL          URL_BACKEND_USER_TRIAL
#define URL_USER_REGISTER       URL_BACKEND_USER_REGISTER
#define URL_USER_VERIFY         URL_BACKEND_USER_VERIFY
#define URL_TPM_UPDATE_ONCE     URL_BACKEND_TPM_UPDATE_ONCE
#define URL_TPM_UPDATE_COMPLETE URL_BACKEND_TPM_UPDATE_COMPLETE
#define URL_TPM_GET_LICENSE     URL_BACKEND_TPM_GET_LICENSE

#ifdef __cplusplus
    }
#endif

#endif /* _ENACT_API_H_ */
