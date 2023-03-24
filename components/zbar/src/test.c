#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "zbar.h"
#include <aos/cli.h>
#include <aos/kernel.h>

#define IMAGE_WIDTH (114)
#define IMAGE_HEIGHT (80)
zbar_image_scanner_t *scanner = NULL;

static void get_data (const char *name, int *width, int *height, void **raw);

int zbar_test(int32_t argc, char **argv)
{

    /* create a reader */
    scanner = zbar_image_scanner_create();

    /* configure the reader */
    zbar_image_scanner_set_config(scanner, 0, ZBAR_CFG_ENABLE, 1);

    /* obtain image data */
    int width = 0, height = 0;
    void *raw = NULL;
    get_data("/mnt/sd/barcode.yuv", &width, &height, &raw);

    /* wrap image data */
    zbar_image_t *image = zbar_image_create();
    zbar_image_set_format(image, *(int*)"Y800");
    zbar_image_set_size(image, width, height);
    zbar_image_set_data(image, raw, width * height, zbar_image_free_data);

    /* scan the image for barcodes */
    int n = zbar_scan_image(scanner, image);

    /* extract results */
    const zbar_symbol_t *symbol = zbar_image_first_symbol(image);
    for(; symbol; symbol = zbar_symbol_next(symbol)) {
        /* do something useful with results */
        zbar_symbol_type_t typ = zbar_symbol_get_type(symbol);
        const char *data = zbar_symbol_get_data(symbol);
        printf("decoded %s symbol \"%s\"\n",
               zbar_get_symbol_name(typ), data);
    }

    /* clean up */
    zbar_image_destroy(image);
    zbar_image_scanner_destroy(scanner);

    return 0;
}

static void get_data (const char *name,
                      int *width, int *height,
                      void **raw)
{
    printf("%s %d: file name %s\n", __FUNCTION__, __LINE__, name);
    FILE *file = fopen(name, "rb");
    int ret = 0;
    if(!file)
        exit(2);

    /* allocate image */
    *width = IMAGE_WIDTH;
    *height = IMAGE_HEIGHT;
    *raw = malloc(*width * *height);

    ret = fread(*raw, 1, (*width) * (*height), file);

    printf("%s %d: read size: %d, total size: %d\n",__FUNCTION__, __LINE__, ret, (*width) * (*height));

    return;
}

ALIOS_CLI_CMD_REGISTER(zbar_test, zbar_test, zbar_test);