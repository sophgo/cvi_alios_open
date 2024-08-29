/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 * Copyright (c) 2015 Runtime Inc
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <assert.h>
#include <ble_os.h>
#include <stdbool.h>
#include <string.h>

#include <ff.h>
#include <vfs.h>

#include "settings/settings.h"
#include "settings/settings_filef.h"
#include "settings_priv.h"

#define BT_DBG_ENABLED 0
#include "common/log.h"


struct settings_file_read_arg_t {
	char *val;
	uint16_t val_size;
};

// Structure to hold configuration key-value pairs
struct ConfigEntry {
	char name[32];
	char value[32];
};

static struct ConfigEntry configs[CONFIG_SETTINGS_FS_MAX_LINES];
static int num_configs;

int settings_backend_init(void);
void settings_mount_fs_backend(struct settings_file *cf);

static int settings_fatfs_load(struct settings_store *cs, const struct settings_load_arg *arg);
static int settings_fatfs_save(struct settings_store *cs, const char *name, const char *value,
				size_t val_len);

static const struct settings_store_itf settings_file_itf = {
	.csi_load = settings_fatfs_load,
	.csi_save = settings_fatfs_save,
};

/*
 * Register a file to be a source of configuration.
 */
int settings_file_src(struct settings_file *cf)
{
	if (!cf->cf_name) {
		return -EINVAL;
	}
	cf->cf_store.cs_itf = &settings_file_itf;
	settings_src_register(&cf->cf_store);

	return 0;
}

/*
 * Register a file to be a destination of configuration.
 */
int settings_file_dst(struct settings_file *cf)
{
	if (!cf->cf_name) {
		return -EINVAL;
	}
	cf->cf_store.cs_itf = &settings_file_itf;
	settings_dst_register(&cf->cf_store);

	return 0;
}

// static void settings_print_configs_info(void)
// {
// 	for (int i = 0; i < num_configs; i++)
// 		printf("Configs: Name: %s, Value: %s.\n", configs[i].name, configs[i].value);
// }

// Function to load all configurations from the file into memory
static int settings_load_all_configs(const char *path_name)
{
	FIL file;

	if (f_open(&file, path_name, FA_READ) == FR_OK) {
		char line[100];

		while (f_gets(line, 100, &file) != NULL) {
			sscanf(line, "%[^=]=%[^\n]", configs[num_configs].name, configs[num_configs].value);
			//     printf("===init: Name: %s, Value: %s. num config=%d.\n", configs[num_configs].name, configs[num_configs].value, num_configs);
			num_configs++;

			if (num_configs >= CONFIG_SETTINGS_FS_MAX_LINES)
				break; // Reached maximum number of configurations

		}

		f_close(&file);

		return 0;
	} else {
		printf("Error opening file\n");
		return 1;
	}
}

// Function to save all configurations from memory back to the file
static int settings_save_all_configs(const char *path_name)
{
	FIL file;

	if (f_open(&file, path_name, FA_WRITE | FA_CREATE_ALWAYS) == FR_OK) {
		for (int i = 0; i < num_configs; i++)
			f_printf(&file, "%s=%s\n", configs[i].name, configs[i].value);

		f_close(&file);
		return 0;
	} else {
		printf("Error opening file\n");
		return 1;
	}
}

static ssize_t _settings_file_read_cb(void *cb_arg, void *data, size_t len)
{
	struct settings_file_read_arg_t *read_arg = cb_arg;
	size_t read_len = 0;

	BT_DBG("cb_arg %p, read_arg->val %p, read_arg->val_size %d, data %p, len %d", cb_arg, read_arg ? read_arg->val : 0,
		read_arg ? read_arg->val_size:0, data, len);
	if (data && read_arg && read_arg->val && read_arg->val_size && len >= read_arg->val_size) {
		read_len = len > read_arg->val_size ? read_arg->val_size : len;
		memcpy(data, read_arg->val, read_len);
		return read_len;
	}
	return 1;
}

static int settings_fatfs_load(struct settings_store *cs, const struct settings_load_arg *arg)
{
#if 0
	struct settings_file_read_arg_t read_arg = {0};

	for (int i = 0; i < num_configs; i++) {
		printf("Load: Name: %s, Value: %s, val_size: %ld\n", configs[i].name, configs[i].value, strlen(configs[i].value));

		strcpy(read_arg.val, configs[i].value);
		read_arg.val_size = strlen(configs[i].value);
		settings_call_set_handler(configs[i].name, strlen(configs[i].value), _settings_file_read_cb, &read_arg, arg);
	}

	return 0;
#else
	FIL file;
	char line[100];
	struct settings_file_read_arg_t read_arg = {0};
	struct settings_file *cf = (struct settings_file *)cs;

	if (f_open(&file, cf->cf_name, FA_READ) == FR_OK) {
		while (f_gets(line, 100, &file) != NULL) {
			char curr_name[50];
			char curr_value[50];
			if (sscanf(line, "%[^=]=%[^\n]", curr_name, curr_value) == 2) {
				// printf("Load: Name: %s, Value: %s, val_size: %ld\n", curr_name, curr_value, strlen(curr_value));
				read_arg.val = curr_value;
				read_arg.val_size = strlen(curr_value);
				settings_call_set_handler(curr_name, strlen(curr_value), _settings_file_read_cb, &read_arg, arg);
			}
		}
		f_close(&file);
		return 0;
	} else {
		printf("Error opening file for reading\n");
		return 1;
	}
#endif
}

static int settings_save_config(struct settings_store *cs,
				const char *name,
				const char *new_value,
				size_t value_len)
{
	int name_found = 0;
	struct settings_file *cf = (struct settings_file *)cs;

	if (num_configs >= CONFIG_SETTINGS_FS_MAX_LINES) {
		printf("Err: Config line exceeds maximum value.\n");
		return 1;
	}

	// printf("Save: Name: %s, Value: %s, val_size: %ld\n", name, new_value, value_len);

	for (int i = 0; i < num_configs; i++) {
		if (strcmp(configs[i].name, name) == 0) {
			snprintf(configs[i].value, value_len + 1, "%s", new_value);
			name_found = 1;
			break;
		}
	}

	if (!name_found && num_configs < CONFIG_SETTINGS_FS_MAX_LINES) {
		snprintf(configs[num_configs].name, sizeof(configs[num_configs].name), "%s", name);
		snprintf(configs[num_configs].value, value_len + 1, "%s", new_value);
		num_configs++;
	}

	//update config file
	return settings_save_all_configs(cf->cf_name);
}

static int settings_fatfs_save(struct settings_store *cs, const char *name, const char *value,
				size_t val_len)
{
	return settings_save_config(cs, name, value, val_len);
}

int settings_backend_init(void)
{
	FIL file;
	FF_DIR dp;
	int rc;
	static struct settings_file config_init_settings_file = {
		.cf_name = CONFIG_SETTINGS_FS_FILE,
		.cf_maxlines = CONFIG_SETTINGS_FS_MAX_LINES
	};

	rc = settings_file_src(&config_init_settings_file);
	if (rc) {
		printf("settings_file_src: err.\n");
		return 1;
	}

	rc = settings_file_dst(&config_init_settings_file);
	if (rc) {
		printf("settings_file_dst: err.\n");
		return 1;
	}

	/*
	 * Must be called after root FS has been initialized.
	 */
	rc = f_opendir(&dp, CONFIG_SETTINGS_FS_DIR);
	if (rc) {
		printf("%s isn't exist, create it.\n", CONFIG_SETTINGS_FS_DIR);
		rc = f_mkdir(CONFIG_SETTINGS_FS_DIR);
		if (rc != 0) {
			printf("fail to mkdir %s.\n", CONFIG_SETTINGS_FS_DIR);
			return rc;
		}
	}

	rc = f_open(&file, CONFIG_SETTINGS_FS_FILE, FA_OPEN_EXISTING);
	if (rc != FR_OK) {
		rc = f_open(&file, CONFIG_SETTINGS_FS_FILE, FA_CREATE_NEW);
		if (rc != 0) {
			printf("fail to create %s.\n", CONFIG_SETTINGS_FS_FILE);
			return rc;
		}
	} else {
		rc = settings_load_all_configs(config_init_settings_file.cf_name);
		if (rc)
			return rc;
	}

	f_close(&file);

	/*
	 * The following lines mask the file exist error.
	 */
	if (rc == -EEXIST) {
		rc = 0;
	}

	return rc;
}
