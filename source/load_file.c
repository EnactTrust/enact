/* load_file.c
 *
 * Copyright (C) 2006-2025 wolfSSL Inc.
 * Copyright (C) 2025 DesignFirst OU
 *
 * Based on the loadFile function from wolfTPM.
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

#ifdef HAVE_CONFIG_H
    #include <config.h>
#endif

/* use ANSI stdio for support of format strings, must be set before
 * including stdio.h
 */
#if defined(__MINGW32__) || defined(__MINGW64__)
    #define __USE_MINGW_ANSI_STDIO 1
#endif
#ifdef _MSC_VER
    /* hide fopen warnings on Windows Visual Studio */
    #define _CRT_SECURE_NO_WARNINGS
#endif

#include <wolftpm/tpm2_wrap.h>
#include "load_file.h"

#include <stdio.h>

#define XFILE FILE*

int loadFile(const char* fname, byte** buf, size_t* bufLen)
{
    int ret = 0;
    ssize_t fileSz, readLen;
    XFILE fp;

    if (fname == NULL || buf == NULL || bufLen == NULL)
        return BAD_FUNC_ARG;

    /* open file (read-only binary) */
    fp = XFOPEN(fname, "rb");
    if (fp == XBADFILE) {
        fprintf(stderr, "Error loading %s\n", fname);
        return BUFFER_E;
    }

    XFSEEK(fp, 0, XSEEK_END);
    fileSz = XFTELL(fp);
    XREWIND(fp);
    if (fileSz > 0) {
        if (*buf == NULL) {
        #if !defined(WOLFTPM2_NO_HEAP)
            *buf = (byte*)XMALLOC(fileSz, NULL, DYNAMIC_TYPE_TMP_BUFFER);
            if (*buf == NULL)
                ret = MEMORY_E;
        #endif
        }
        else if (*buf != NULL && fileSz > (ssize_t)*bufLen) {
            ret = BUFFER_E;
        }
        *bufLen = (size_t)fileSz;
        if (ret == 0) {
            readLen = XFREAD(*buf, 1, *bufLen, fp);
            ret = (readLen == (ssize_t)*bufLen) ? 0 : -1;
        }
    }
    else {
        ret = BUFFER_E;
    }
    XFCLOSE(fp);
    return ret;
}


static signed char hexCharToByte(signed char ch)
{
    signed char ret = (signed char)ch;
    if (ret >= '0' && ret <= '9')
        ret -= '0';
    else if (ret >= 'A' && ret <= 'F')
        ret -= 'A' - 10;
    else if (ret >= 'a' && ret <= 'f')
        ret -= 'a' - 10;
    else
        ret = -1; /* error case - return code must be signed */
    return ret;
}
int hexToByte(const char *hex, unsigned char *output, unsigned long sz)
{
    int outSz = 0;
    word32 i;
    for (i = 0; i < sz; i+=2) {
        signed char ch1, ch2;
        ch1 = hexCharToByte(hex[i]);
        ch2 = hexCharToByte(hex[i+1]);
        if ((ch1 < 0) || (ch2 < 0)) {
            return -1;
        }
        output[outSz++] = (unsigned char)((ch1 << 4) + ch2);
    }
    return outSz;
}

void printHexString(const unsigned char* bin, unsigned long sz,
    unsigned long maxLine)
{
    unsigned long i;
    printf("\t");
    for (i = 0; i < sz; i++) {
        printf("%02x", bin[i]);
        if (((i+1) % maxLine) == 0 && i+1 != sz)
            printf("\n\t");
    }
    printf("\n");
}
