/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */
/* log.c
 *
 * Log implementation for specific environment
 */



// Logging API
#include "rambus_log.h"            // the API to implement


#ifdef LOG_DUMP_ENABLED
void
Log_HexDump(
        const char * szPrefix_p,
        const unsigned int PrintOffset,
        const uint8_t * Buffer_p,
        const unsigned int ByteCount)
{
    unsigned int i;
    char Format[] = "%s %08d:";

    for(i = 0; i < ByteCount; i += 16)
    {
        unsigned int j, Limit;

        // if we do not have enough data for a full line
        if (i + 16 > ByteCount)
            Limit = ByteCount - i;
        else
            Limit = 16;

        Log_FormattedMessage(Format, szPrefix_p, PrintOffset + i);

        for (j = 0; j < Limit; j++)
            Log_FormattedMessage(" %02X", Buffer_p[i+j]);

        Log_FormattedMessage("\n");
    } // for
}
#endif

/* end of file log.c */
