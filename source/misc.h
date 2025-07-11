/* misc.h
 *
 * Copyright (C) 2018-2021 DesignFirst OU
 * Copyright (C) 2022 EnactTrust LTD
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

#ifndef _ENACT_MISC_H_
#define _ENACT_MISC_H_

#ifdef __cplusplus
    extern "C" {
#endif

char misc_get_single_key(void);
int misc_is_valid_email(const char *email);
int misc_get_user_email(char *email, long email_size);
int misc_is_valid_email_code(const char *input);
int misc_get_email_code(char *buffer, long size);
int misc_uuid_str2bin(const char *str, int str_size, char *bytes, int size);

#ifdef __cplusplus
    }
#endif

#endif /* _ENACT_MISC_H_ */
