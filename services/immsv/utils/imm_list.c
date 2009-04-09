/*      -*- OpenSAF  -*-
 *
 * (C) Copyright 2008 The OpenSAF Foundation
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. This file and program are licensed
 * under the GNU Lesser General Public License Version 2.1, February 1999.
 * The complete license can be accessed from the following location:
 * http://opensource.org/licenses/lgpl-license.php
 * See the Copying file included with the OpenSAF distribution for full
 * licensing terms.
 *
 * Author(s): Ericsson AB
 *
 */

/*
 * This file contains a command line utility to view IMM objects.
 * Example: immlist ..."
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/un.h>
#include <sys/time.h>
#include <fcntl.h>
#include <ctype.h>
#include <libgen.h>
#include <time.h>
    
#include <saAis.h>
#include <saImmOm.h>

#include "saf_error.h"

static SaVersionT immVersion = {'A', 2, 1};

static void usage(const char *progname)
{
    printf("\nNAME\n");
    printf("\t%s - list IMM objects\n", progname);

    printf("\nSYNOPSIS\n");
    printf("\t%s [options] <object name> [object name]\n", progname);

    printf("\nDESCRIPTION\n");
    printf("\t%s is an IMM OM client used to print attributes of IMM objects.\n", progname);

    printf("\nOPTIONS\n");
    printf("  -h or --help  this help\n");

    printf("\nEXAMPLE\n");
    printf("   immlist safApp=myApp\n");
    printf("   immlist safApp=myApp safApp=myApp2\n");
}

static void print_attr_value(SaImmValueTypeT attrValueType, SaImmAttrValueT *attrValue)
{
    switch (attrValueType)
    {
        case SA_IMM_ATTR_SAINT32T:
            printf("%d (0x%x)", *((SaInt32T*) attrValue), *((SaInt32T*) attrValue));
            break;
        case SA_IMM_ATTR_SAUINT32T:
            printf("%u (0x%x)", *((SaUint32T*) attrValue), *((SaUint32T*) attrValue));
            break;
        case SA_IMM_ATTR_SAINT64T:
            printf("%lld (0x%llx)", *((SaInt64T*) attrValue), *((SaInt64T*) attrValue));
            break;
        case SA_IMM_ATTR_SAUINT64T:
            printf("%llu (0x%llx)", *((SaUint64T*) attrValue), *((SaUint64T*) attrValue));
            break;
        case SA_IMM_ATTR_SATIMET:
        {
            char buf[32];
            const time_t time = *((SaTimeT*) attrValue) / SA_TIME_ONE_SECOND;

            ctime_r(&time, buf);
            buf[strlen(buf) - 1] = '\0'; /* Remove new line */
            printf("%llu (0x%llx, %s)", *((SaTimeT*) attrValue),
                *((SaTimeT*) attrValue), buf);
            break;
        }
        case SA_IMM_ATTR_SAFLOATT:
            printf("%f ", *((SaFloatT*) attrValue));
            break;
        case SA_IMM_ATTR_SADOUBLET:
            printf("%lf ", *((SaDoubleT*) attrValue));
            break;
        case SA_IMM_ATTR_SANAMET:
        {
            SaNameT *myNameT = (SaNameT*) attrValue;
            printf("%s (%u) ", myNameT->value, myNameT->length);
            break;
        }
        case SA_IMM_ATTR_SASTRINGT:
            printf("%s ", *((char**) attrValue));
            break;
        default:
            printf("Unknown");
            break;
    }
}

static char *get_attr_type_name(SaImmValueTypeT attrValueType)
{
    switch (attrValueType)
    {
        case SA_IMM_ATTR_SAINT32T:
            return "SA_INT32_T";
            break;
        case SA_IMM_ATTR_SAUINT32T:
            return "SA_UINT32_T";
            break;
        case SA_IMM_ATTR_SAINT64T:
            return "SA_INT64_T";
            break;
        case SA_IMM_ATTR_SAUINT64T:
            return "SA_UINT64_T";
            break;
        case SA_IMM_ATTR_SATIMET:
            return "SA_TIME_T";
            break;
        case SA_IMM_ATTR_SANAMET:
            return "SA_NAME_T";
            break;
        case SA_IMM_ATTR_SAFLOATT:
            return "SA_FLOAT_T";
            break;
        case SA_IMM_ATTR_SADOUBLET:
            return "SA_DOUBLE_T";
            break;
        case SA_IMM_ATTR_SASTRINGT:
            return "SA_STRING_T";
            break;
        case SA_IMM_ATTR_SAANYT:
            return "SA_ANY_T";
            break;
        default:
            return "Unknown";
            break;
    }
}

int main(int argc, char *argv[])
{
    int c;
    struct option long_options[] =
    {
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    SaAisErrorT error;
    SaImmHandleT immHandle;
    SaNameT objectName;
    SaImmAccessorHandleT accessorHandle;
    SaImmAttrValuesT_2 **attributes;

    while (1)
    {
        c = getopt_long(argc, argv, "h", long_options, NULL);

        if (c == -1) /* have all command-line options have been parsed? */
            break;

        switch (c)
        {
            case 'h':
                usage(basename(argv[0]));
                exit(EXIT_SUCCESS);
                break;
            default:
                fprintf(stderr, "Try '%s --help' for more information\n", argv[0]);
                exit(EXIT_FAILURE);
                break;
        }
    }

    /* Need at least one object to operate on */
    if ((argc - optind) == 0)
    {
        fprintf(stderr, "error - wrong number of arguments\n");
        exit(EXIT_FAILURE);
    }

    error = saImmOmInitialize(&immHandle, NULL, &immVersion);
    if (error != SA_AIS_OK)
    {
        fprintf(stderr, "error - saImmOmInitialize FAILED: %s\n", saf_error(error));
        exit(EXIT_FAILURE);
    }

    error = saImmOmAccessorInitialize(immHandle, &accessorHandle);
    if (SA_AIS_OK != error)
    {
        fprintf(stderr, "error - saImmOmAccessorInitialize FAILED: %s\n",  saf_error(error));
        exit(EXIT_FAILURE);
    }

    /* Remaining arguments should be object names to print attributes for. */
    while (optind < argc)
    {
        int i = 0, j;
        SaImmAttrValuesT_2 *attr;
        strncpy((char *) objectName.value, argv[optind], SA_MAX_NAME_LENGTH);
        objectName.length = strnlen((char *) objectName.value, SA_MAX_NAME_LENGTH);

        error = saImmOmAccessorGet_2(accessorHandle, &objectName, NULL, &attributes);
        if (SA_AIS_OK != error)
        {
            if (error == SA_AIS_ERR_NOT_EXIST)
                fprintf(stderr, "error - object '%s' does not exist\n", objectName.value);
            else
                fprintf(stderr, "error - saImmOmAccessorGet_2 FAILED: %s\n", saf_error(error));

            exit(EXIT_FAILURE);
        }

        printf("%-50s %-12s Value(s)\n", "Name", "Type");
        printf("========================================================================");
        while ((attr = attributes[i++]) != NULL)
        {
            printf("\n%-50s %-12s ", attr->attrName, get_attr_type_name(attr->attrValueType));
            if (attr->attrValuesNumber > 0)
            {
                for (j = 0; j < attr->attrValuesNumber; j++)
                    print_attr_value(attr->attrValueType, attr->attrValues[j]);
            }
            else
                printf("<Empty>");
        }

        printf("\n\n");
        optind++;
    }

    error = saImmOmAccessorFinalize(accessorHandle);
    if (SA_AIS_OK != error)
    {
        fprintf(stderr, "error - saImmOmAccessorFinalize FAILED: %s\n",  saf_error(error));
        exit(EXIT_FAILURE);
    }

    error = saImmOmFinalize(immHandle);
    if (SA_AIS_OK != error)
    {
        fprintf(stderr, "error - saImmOmFinalize FAILED: %s\n",  saf_error(error));
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}

