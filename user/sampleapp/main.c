/**
    Frostbyte kernel and operating system
    Copyright (C) 2023  Amol Dhamale <amoldhamale1105@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "flib.h"

#define WORLDWIDE_MIN_VOTING_AGE 16

enum En_Country
{
    NONE = 0,
    INDIA,
    AUSTRIA,
    EAST_TIMOR,
    BAHRAIN,
    SOLOMON_ISLANDS,
    TONGA,
    OTHER
};

int main(int argc, char** argv)
{
    char name[100] = {0};
    if (argc > 1){
        memcpy(name, argv[1], strlen(argv[1]));
        if (argc > 2)
            memcpy(name+sizeof(name)/2, argv[2], strlen(argv[2]));
        else{
            printf("\nEnter your last name or surname: ");
            scanf("%s", name+sizeof(name)/2);
        }
    }

    int country_code = NONE;
    int attempts = 2;
    uint32_t age, age_limit = WORLDWIDE_MIN_VOTING_AGE;
    if (argc < 2){
        printf("\nEnter your full name (first name + surname): ");
        scanf("%s %s", name, name+sizeof(name)/2);
    }
    printf("\n\nWhich country are you from, %s?\n1. India\n2. Austria\n3. East Timor\n4. Bahrain\n5. Solomon Islands\n6. Tonga\n7. Other\n", name);
    printf("Select your choice (Attempts remaining: %d): ", attempts);
    scanf("%d", &country_code);
    if (country_code < 1 || country_code > 7){
        attempts--;
        printf("\nInvalid selection. Please pick a number between 1-7, 7 if your country is not listed in first 6 choices\n");
        printf("Select your choice. (Attempts remaining: %d): ", attempts);
        scanf("%d", &country_code);
        if (country_code < 1 || country_code > 7){
            attempts--;
            if (attempts == 0){
                printf("Sorry %s, you have no attempts remaining. Aborting\n\n", name);
                exit(1);
            }
        }
    }
    
    printf("\n\nHow old are you, Mx. %s? ", name+sizeof(name)/2);
    scanf("%u", &age);

    if (age < WORLDWIDE_MIN_VOTING_AGE){
        if (age == 0)
            printf("Invalid age. Aborting\n\n");
        else
            printf("\n%s, you're below the worldwide age limit for voting!\n\n", name);
        exit(1);
    }
    
    switch (country_code)
    {
    case INDIA:
        age_limit = 18;
        break;
    case AUSTRIA:
        age_limit = WORLDWIDE_MIN_VOTING_AGE;
        break;
    case EAST_TIMOR:
        age_limit = 17;
        break;
    case BAHRAIN:
        age_limit = 20;
        break;
    case SOLOMON_ISLANDS:
        age_limit = 19;
        break;
    case TONGA:
        age_limit = 21;
        break;
    case OTHER:
    default:
        age_limit = 18;
        break;
    }

    if (age >= age_limit){
        if (country_code != OTHER)
            printf("\nCongratulations Mx. %s, you're eligible to vote in your country. Select a canditate responsibly using your own judgement\n\n", name+sizeof(name)/2);
        else
            printf("\n%s, we do not have the exact data for your country but you're most probably eligible to vote. Please confirm with your local legislation\n\n", name);
    }
    else{
        if (country_code != OTHER)
            printf("\nSorry %s, you're not yet eligible to vote in your country. %d more year%s to go\n\n", name, age_limit-age, age_limit-age > 1 ? "s" : " \b");
        else
            printf("\n%s, we do not have the exact data for your country but you're most probably ineligible to vote. Please confirm with your local legislation\n\n", name);
    }

    return 0;
}