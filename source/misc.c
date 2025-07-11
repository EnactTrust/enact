/* misc.c
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

#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

/* Get a single character without without waiting for Enter */
char misc_get_single_key(void)
{
    struct termios oldt, newt;
    char ch;

    /* Get current terminal settings and disable canonical mode and echo */
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    ch = getchar();
    /* Restore old terminal settings */
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}

/* Validate if the email is in a correct format, returns 0 upon success */
int misc_is_valid_email(const char *email)
{
    if(email == NULL) {
        return -1;
    }
    // Ensure email contains '@'
    const char *at_pos = strchr(email, '@');
    if (!at_pos || at_pos == email || at_pos == email + strlen(email) - 1) {
        return -2; // Invalid: no '@' or it is at the start/end
    }

    // Ensure there is a '.' after the '@'
    const char *dot_pos = strchr(at_pos, '.');
    if (!dot_pos || dot_pos == at_pos + 1 || dot_pos == email + strlen(email) - 1) {
        return -3; // Invalid: no '.' or it is immediately after '@'
    }

    // Ensure there are no invalid characters
    for (size_t i = 0; i < strlen(email); i++) {
        if (!(isalnum(email[i]) || email[i] == '@' || email[i] == '.' || email[i] == '-' || email[i] == '_')) {
            return -4; // Invalid character found
        }
    }

    return 0;
}

int misc_get_user_email(char *email, long email_size)
{
    while (1) {
        printf("Please enter your email: ");
        if (fgets(email, email_size, stdin)) {
            // Remove newline character if it's present
            email[strcspn(email, "\n")] = '\0';

            // Check if the email is valid
            if (misc_is_valid_email(email) == 0) {
                printf("Valid email: %s\n", email);
                break;  // Exit the loop when a valid email is entered
            } else {
                printf("Invalid email format. Please try again.\n");
            }
        } else {
            // Handle fgets failure (unlikely but possible)
            printf("Error reading input.\n");
            return -1;
        }
    }
    return 0;
}

/* Validate the email code is in a correct format, returns 0 upon success */
int misc_is_valid_email_code(const char *input)
{
    if(input == NULL) {
        return -1;
    }
    /* Expects a data structure like ENACT_EMAIL_CODE */
    for (int i = 0; i < 6; i++) {
        if (!(islower(input[i]) || isdigit(input[i]))) {
            return -2;
        }
    }
    return 0;
}

int misc_get_email_code(char *buffer, long size)
{
    int res;

    while (1) {
        printf("Enter code: ");
        if (fgets(buffer, size, stdin)) {
            size_t len = strlen(buffer);
            // Remove newline if present
            if (buffer[len - 1] == '\n') {
                buffer[len - 1] = '\0';
                len--;
            }

            if (len != 6) {
                printf("Error: You must enter exactly 6 characters.\n");
                buffer[0] = '\0'; // Invalidate input
                continue;
            }

            if (misc_is_valid_email_code(buffer)) {
                printf("Error: Only lowercase letters and digits are allowed.\n");
                buffer[0] = '\0'; // Invalidate input
                continue;
            }

            res = 0; // valid input
            break;
        } else {
            printf("Input error.\n");
            buffer[0] = '\0';
            res = -1;
        }
    }
    return res;
}

/* Convert a nodeid from string to a hex representation */
int misc_uuid_str2bin(const char *str, int str_size, char *bytes, int size)
{
    int i, b;
    char hexstr[3];

    i = b = 0;
    while(i < str_size)
    {
        if(str[i] == '-') {
            i++;
            continue;
        }

        hexstr[0] = str[i++];
        hexstr[1] = str[i++];
        hexstr[2] = '\0';

        bytes[b++] = (char)strtol(hexstr, NULL, 16);
    }

    return b;
}
