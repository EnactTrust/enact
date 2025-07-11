/* enact.c
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
 * along with EnactTrust. If not, see <https://www.gnu.org/licenses/>.
 */

#include "enact.h"
#include "enact_api.h"
#include "tpm.h"
#include "tpm_update.h"
#include "misc.h"
#include "wolftpm/tpm2_wrap.h"

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <curl/curl.h>

int enact(ENACT_TPM *tpm);

static ENACT_API_CALL api_type;
static ENACT_API_TOKEN api_token;
static ENACT_EMAIL_CODE email_code;
static ENACT_EMAIL user_email;
static ENACT_BACKEND_RESPONSE response;
static char update_file[MAX_ENACT_TPM_UPDATE_SIZE];
static char license_file[MAX_ENACT_TPM_LICENSE_SIZE];
static volatile size_t update_file_size = 0;
static volatile size_t license_file_size = 0;
static CURL *curl = NULL;
static int trial = 0;
static int verbose = 0;

/* EnactTrust is a painless TPM update service provided by
 * DesignFirst OU, est. 2018 in Tallinn, Estonia (EU member)
 *
 * Contact us at info@enacttrust.com with any questions
 */
void usage(void) {
    printf("Usage:\n");
    printf("  ./enact\tRun the application with default settings (no account required)\n\n");
    printf("  ./enact menu  Launch interactive menu mode to perform custom actions\n\n");

    printf("Description:\n");
    printf("  Run without arguments to receive an update for your TPM 2.0 (Trusted Platform Module).\n");
    printf("  No user account is needed for default operation.\n\n");

    printf("Supported TPM Models:\n");
    printf("  - Nuvoton  NPCT7xx (requires approval, send us an email)\n");
    printf("  - Infineon SLB9673\n");
    printf("  - Infineon SLB9672\n");
    printf("  - Infineon SLB9670 (BETA, send us an email)\n\n");

    printf("For more information, contact us at: support@enacttrust.com\n\n");
}

void print_api_endpoints(void)
{
    printf("EnactTrust endpoints in use:\n");
    printf("\t%s\n", URL_BACKEND_USER_TRIAL);
    printf("\t%s\n", URL_BACKEND_USER_REGISTER);
    printf("\t%s\n", URL_BACKEND_USER_VERIFY);
    printf("\t%s\n", URL_BACKEND_TPM_UPDATE_ONCE);
    printf("\t%s\n", URL_BACKEND_TPM_UPDATE_COMPLETE);
    printf("\t%s\n", URL_BACKEND_TPM_GET_LICENSE);
}

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    int real_size = size * nmemb;
    static size_t total_size = 0;
    long http_code = 0;

    if(api_type == ENACT_API_CALL_UPDATE_ONCE) {
        if((update_file_size + real_size) < MAX_ENACT_TPM_UPDATE_SIZE) {
            memcpy(&update_file[update_file_size], ptr, real_size);
            update_file_size += real_size;
        }
        else {
            printf("Update file is larger than maximum allowed size\n");
            return update_file_size; // err instead of real_size for success
        }
    }
    else if(api_type == ENACT_API_CALL_GET_LICENSE) {
        if((license_file_size + real_size) < MAX_ENACT_TPM_LICENSE_SIZE) {
            memcpy(&license_file[license_file_size], ptr, real_size);
            license_file_size += real_size;
        }
        else {
            printf("License file is larger than maximum allowed size\n");
            return license_file_size; // err instead of real_size for success
        }
    }
    else {
        memcpy(response.buffer, ptr, real_size);
        response.size = real_size;
        response.buffer[real_size] = 0; // Null-terminate
        response.complete = 1;
    }

    if(verbose) {
        total_size += size*nmemb;
        printf("total_size=%zu\n", total_size);
        printf("resp:\t%s\n", response.buffer);
        /* curl is a static variable */
        if(curl) {
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
            printf("http_code=%ld\n", http_code);
        }
    }

    return real_size;
}

int enact_start_trial(CURL *curl)
{
    int ret = ENACT_ERROR;
    CURLcode res;
    long http_code;

    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, URL_USER_TRIAL);
        curl_easy_setopt(curl, CURLOPT_POST, 1L); // Enable POST
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, ""); // No data sent
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

        api_type = ENACT_API_CALL_TRIAL;
        res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
        }
        else {
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
            if(http_code == 200) {
                ret = ENACT_SUCCESS;
            }
            else if(http_code == 301) {
                printf("Requires a registered user. Please contact us at support@enacttrust.com\n");
            }
            else {
                printf("HTTP Error about access to non-registered users: %ld\n", http_code);
            }
        }
    }

    curl_easy_reset(curl);
    return ret;
}

int enact_perform_update_once(CURL *curl, ENACT_TPM *tpm)
{
    int ret = ENACT_ERROR;
    CURLcode res;
    curl_mime *form = NULL;
    curl_mimepart *field = NULL;
    long http_code;

    if(curl) {
        form = curl_mime_init(curl);

        field = curl_mime_addpart(form);
        curl_mime_name(field, ENACT_API_USER_ARG_API_TOKEN);
        curl_mime_data(field, (const char *)api_token.buffer, api_token.size);

        field = curl_mime_addpart(form);
        curl_mime_name(field, ENACT_API_TPM_ARG_FW_VERSION);
        curl_mime_filedata(field, ENACT_TPM_FWVER_FILE);

        curl_easy_setopt(curl, CURLOPT_URL, URL_TPM_UPDATE_ONCE);
        curl_easy_setopt(curl, CURLOPT_MIMEPOST, form);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

        api_type = ENACT_API_CALL_UPDATE_ONCE;
        res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
        }
        else {
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
            if(http_code == 200) {
                ret = ENACT_SUCCESS;
            }
            else if(http_code == 401) {
                printf("Invalid API token. Please contact us at support@enacttrust.com\n");
            }
            else {
                printf("HTTP Error during request to retrieve the TPM update: %ld\n", http_code);
            }
        }
    }

    curl_easy_reset(curl);
    curl_mime_free(form);
    return ret;
}

int enact_get_tpm_license(CURL *curl)
{
    int ret = ENACT_ERROR;
    CURLcode res;
    curl_mime *form = NULL;
    curl_mimepart *field = NULL;

    if(curl) {
        form = curl_mime_init(curl);

        field = curl_mime_addpart(form);
        curl_mime_name(field, ENACT_API_USER_ARG_API_TOKEN);
        curl_mime_data(field, (const char *)api_token.buffer, api_token.size);

        field = curl_mime_addpart(form);
        curl_mime_name(field, ENACT_API_TPM_ARG_FW_VERSION);
        curl_mime_filedata(field, ENACT_TPM_FWVER_FILE);

        curl_easy_setopt(curl, CURLOPT_URL, URL_TPM_GET_LICENSE);
        curl_easy_setopt(curl, CURLOPT_MIMEPOST, form);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

        api_type = ENACT_API_CALL_GET_LICENSE;
        res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
        }
        else {
            ret = ENACT_SUCCESS;
        }
    }

    curl_easy_reset(curl);
    curl_mime_free(form);
    return ret;
}

int enact_register_newuser(CURL *curl)
{
    int ret = ENACT_ERROR;
    CURLcode res;
    curl_mime *form = NULL;
    curl_mimepart *field = NULL;
    long http_code = 0;

    if(curl) {
        form = curl_mime_init(curl);

        field = curl_mime_addpart(form);
        curl_mime_name(field, ENACT_API_USER_ARG_EMAIL);
        curl_mime_data(field, (const char *)user_email.buffer, user_email.size);

        if(api_token.size == ENACT_API_TOKEN_SIZE) {
            field = curl_mime_addpart(form);
            curl_mime_name(field, ENACT_API_USER_ARG_API_TOKEN);
            curl_mime_data(field, (const char *)api_token.buffer, api_token.size);
        }

        curl_easy_setopt(curl, CURLOPT_URL, URL_USER_REGISTER);
        curl_easy_setopt(curl, CURLOPT_POST, 1L); // Enable POST
        curl_easy_setopt(curl, CURLOPT_MIMEPOST, form);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

        api_type = ENACT_API_CALL_REGISTER;
        res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
        }
        else {
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
            if(http_code == 200) {
                ret = ENACT_SUCCESS;
            }
            else if(http_code == 400) {
                printf("Wrong verification code\n");
            }
            else if(http_code == 409) {
                printf("User is already verified\n");
                ret = ENACT_ABORT;
            }
            else {
                printf("HTTP Error about registering a new users: %ld\n", http_code);
            }
        }
    }

    curl_easy_reset(curl);
    curl_mime_free(form);
    return ret;
}

int enact_perform_email_verify(CURL *curl)
{
    int ret = ENACT_ERROR;
    CURLcode res;
    curl_mime *form = NULL;
    curl_mimepart *field = NULL;

    if(curl) {
        form = curl_mime_init(curl);

        field = curl_mime_addpart(form);
        curl_mime_name(field, ENACT_API_USER_ARG_EMAIL_TOKEN);
        curl_mime_data(field, email_code.buffer, email_code.size);

        curl_easy_setopt(curl, CURLOPT_URL, URL_USER_VERIFY);
        curl_easy_setopt(curl, CURLOPT_POST, 1L); // Enable POST
        curl_easy_setopt(curl, CURLOPT_MIMEPOST, form);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

        api_type = ENACT_API_CALL_VERIFY;
        res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
        }
        else {
            ret = ENACT_SUCCESS;
        }
    }

    curl_easy_reset(curl);
    curl_mime_free(form);
    return ret;
}

int enact_new_trial_user(void)
{
    int ret = ENACT_ERROR;
    FILE *fp;
    size_t written;

    ret = enact_start_trial(curl);
    if(ret == ENACT_SUCCESS) {
        if(response.size == ENACT_API_TOKEN_SIZE) {
            memcpy(api_token.buffer, response.buffer, ENACT_API_TOKEN_SIZE);
            api_token.size = ENACT_API_TOKEN_SIZE;
            response.complete = 0; // clear ready flag
            /* Store the user's API token locally for reuse in next runs */
            fp = fopen(ENACT_API_TOKEN_FILE, "wt");
            if(fp == NULL) {
                printf("Error creating a file to store your access token required for using the service\n");
                ret = ENACT_ERROR_FILE;
            }
            else {
                written = fwrite((const char *)api_token.buffer, api_token.size, 1, fp);
                if(written != 1) {
                    perror("Failed to write complete buffer");
                    ret = ENACT_ERROR_FILE;
                }
                else {
                    if(verbose) {
                        printf("Your EnactTrust credentials are saved to the `api.token` file\n");
                    }
                    ret = ENACT_SUCCESS;
                }
                fclose(fp);
            }
        }
        else {
            printf("API token is incomplete.\n");
            ret = ENACT_ERROR;
        }
    }
    else {
        printf("Unsuccessful\n");
        ret = ENACT_ERROR;
    }
    return ret;
}

int enact_new_registered_user(void)
{
    int ret = ENACT_ERROR;

    printf("Do you want an EnactTrust account? (Y/N) ");
    char input = misc_get_single_key();
    input = toupper(input);
    if(input == 'N' || input != 'Y') {
        printf("N\n");
        return ENACT_ERROR;
    }

    printf("Y\n");
    ret = misc_get_user_email(user_email.buffer, sizeof(user_email.buffer));
    if(ret) {
        printf("Failed to capture email.\n");
        return ENACT_ERROR;
    }
    user_email.size = strlen(user_email.buffer);

    ret = enact_register_newuser(curl);
    if(ret == ENACT_SUCCESS) {
        printf("Verification code sent to email %s. ", user_email.buffer);
    }
    else if(ret == ENACT_ABORT) {
        return ret;
    }
    else {
        printf("Failed to register a new account\n");
        return ENACT_ERROR;
    }

    ret = misc_get_email_code(email_code.buffer, sizeof(email_code.buffer));
    if(ret) {
        printf("Failed to capture a verification code\n");
        return ENACT_ERROR;
    }
    email_code.size = ENACT_EMAIL_CODE_SIZE;

    ret = enact_perform_email_verify(curl);
    if(ret == ENACT_SUCCESS) {
        printf("\n=== Account is registered ===\n\n");
        printf("Your EnactTrust API token is sent to %s\n", user_email.buffer);
    }
    else {
        printf("Failed to verify\n");
    }
    return ret;
}

int enact(ENACT_TPM *tpm)
{
    int ret = ENACT_ERROR;
    static int tpm_is_updated_flag = 0;
    FILE *fp;
    size_t written, bytesread;

    printf("Starting EnactTrust...    ");
    fp = fopen(ENACT_API_TOKEN_FILE, "rt");
    if(fp == NULL) {
        ret = enact_new_trial_user();
    }
    else {
        bytesread = fread(api_token.buffer, ENACT_API_TOKEN_SIZE, 1, fp);
        if(bytesread == 1) {
            api_token.size = ENACT_API_TOKEN_SIZE;
            trial = 0; // API token found
            ret = ENACT_SUCCESS;
        }
        else {
            ret = enact_new_trial_user();
        }
    }

    if(ret == ENACT_SUCCESS) {
        printf("Success\n");
    }
    else {
        return ret;
    }

    /* store the TPM fw version as TPMS_ATTEST firmwareVersion */
    fp = fopen(ENACT_TPM_FWVER_FILE, "wt");
    if(fp == NULL) {
        printf("Error creating a file to store the TPM firmware version\n");
        return ENACT_ERROR_FILE;
    }
    written = fwrite((const char *)&tpm->tpm_info.fwversion.value, sizeof(tpm->tpm_info.fwversion.value), 1, fp);
    if(written != 1) {
       perror("Failed to write complete buffer");
       fclose(fp);
       return ENACT_ERROR_FILE;
    }
    fclose(fp);

    printf("Downloading TPM update package... ");
    ret = enact_perform_update_once(curl, tpm);
    if(ret == ENACT_SUCCESS) {
        printf("Success\n");
        if(verbose) printf("Size of TPM update file in bytes: %zu\n", update_file_size);
        /* Prepare file to store the TPM Update blob */
        fp = fopen(ENACT_TPM_UPDATE_FILE, "wt");
        if(fp == NULL) {
            printf("Error creating a file to store the TPM update\n");
            return ENACT_ERROR_FILE;
        }
        /* (RichOS) Store the whole file at once */
        written = fwrite(update_file, update_file_size, 1, fp);
        if(written != 1) {
            perror("Failed to write complete buffer");
            fclose(fp);
            return ENACT_ERROR_FILE;
        }
        fclose(fp);
    }
    else {
        printf("Unsuccessful\n");
        if(verbose) printf("Error while receiving TPM update file\n");
        goto exit;
    }

    printf("Downloading TPM update license... ");
    ret = enact_get_tpm_license(curl);
    if(ret == ENACT_SUCCESS) {
        printf("Success\n");
        if(verbose) printf("Size of TPM license file in bytes: %zu\n", update_file_size);
        /* Prepare file to store the TPM license text */
        fp = fopen(ENACT_TPM_LICENSE_FILE, "wt");
        if(fp == NULL) {
            printf("Error creating a file to store the TPM update license\n");
            return ENACT_ERROR_FILE;
        }
        /* (RichOS) Store the whole file at once */
        written = fwrite(license_file, license_file_size, 1, fp);
        if(written != 1) {
            perror("Failed to write complete buffer");
            fclose(fp);
            return ENACT_ERROR_FILE;
        }
        fclose(fp);
    }

    printf("\t *** This TPM is manufactured by %s ***\n", tpm->tpm_info.caps.mfgStr);
    printf("\t *** Please read the vendor license  ***\n");
    printf("\t *** available as %s     ***\n", ENACT_TPM_LICENSE_FILE);

    printf("TPM update start\n");
    ret = tpm_update();
    if(ret == ENACT_SUCCESS) {
        tpm_is_updated_flag++;
        printf("\n=== Success - TPM is updated ===\n\n");
    }
    else {
        printf("\nError 0x%X = %d. Please contact us at support@enacttrust.com\n", ret, ret);
        goto exit;
    }

    if(trial) {
        ret = enact_new_registered_user();
        if(ret != ENACT_SUCCESS) {
            printf("Write to support@enacttrust.com for help with an account.\n");
        }
    }
    else {
        printf("Please tell us what we can make better. Contact us at info@enacttrust.com\n");
    }

exit:
    if(tpm_is_updated_flag) {
        printf("\n=== Important - Reboot the system to finish the update (power off-on) ===\n\n");
    }
    return ret;
}

int main(int argc, char *argv[])
{
    int ret = ENACT_ERROR;
    ENACT_TPM tpm;

    printf("\n=== EnactTrust TPM Updater === (version %s) \n\n", ENACT_VERSION_STRING);
    if(verbose) {
        print_api_endpoints();
    }
    /* Parse arguments */
    if(argc >= 2) {
        if(XSTRNCMP(argv[1], "menu", 7) == 0) {
            printf("Interactive mode is in BETA. Contact us at info@enacttrust.com for BETA access.\n");
            return NOT_IMPLEMENTED;
        }
        else {
            usage();
            return BAD_ARG;
        }
    }
    else {
        trial = 1;
        if(verbose) {
            printf("Performing a one-time update.\n");
        }
    }

#if (DEBUG_NO_TPM > 0u)
    printf("Debugging without TPM in use\n");
#else
    ret = tpm_init(&tpm);
    if(ret) {
        printf("Error while accessing the Trusted Platform Module\n");
        return ENACT_ERROR_TPM;
    }
    else {
        ret = tpm_get_info(&tpm);
        if(ret) {
            printf("Error while accessing the Trusted Platform Module\n");
            return ENACT_ERROR_TPM;
        }
        else {
            if(tpm.tpm_info.fwversion.value == 0) {
                printf("Unable to retrieve fw version. Try rebooting the system (power off-on).\n");
                return ENACT_ERROR_TPM;
            }
            if(tpm.tpm_info.caps.mfg != TPM_MFG_INFINEON) {
                printf("Your TPM manufacturer %s does not offer open-source update process\n", tpm.tpm_info.caps.mfgStr);
                if(tpm.tpm_info.caps.mfg == TPM_MFG_NUVOTON || tpm.tpm_info.caps.mfg == TPM_MFG_STM) {
                    printf("\nbut we can help - Contact us at support@enacttrust.com.\n");
                }
                return NOT_PUBLIC;
            }
        }
    }
#endif
    /* Initialize libcurl (done only once) */
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if(!curl) {
        printf("Unable to use libcurl\n");
    }

    /* The outcome of enact is reported as the end result value */
    ret = enact(&tpm);

    /* Make sure we do a clean exit */
    if(curl) {
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
#ifndef DEBUG_NO_TPM
    tpm_deinit(tpm);
#endif
    printf("Goodbye\n");
    return ret;
}
