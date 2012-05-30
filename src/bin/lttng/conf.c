/*
 * Copyright (c)  2011 David Goulet <david.goulet@polymtl.ca>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2 only,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#define _GNU_SOURCE
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <common/error.h>

#include "conf.h"

/*
 *  config_get_file_path
 *
 *  Returns the path with '/CONFIG_FILENAME' added to it;
 *  path will be NULL if an error occurs.
 */
char *config_get_file_path(char *path)
{
	int ret;
	char *file_path;

	ret = asprintf(&file_path, "%s/%s", path, CONFIG_FILENAME);
	if (ret < 0) {
		ERR("Fail allocating config file path");
	}

	return file_path;
}

/*
 *  open_config
 *
 *  Returns an open FILE pointer to the config file;
 *  on error, NULL is returned.
 */
static FILE *open_config(char *path, const char *mode)
{
	FILE *fp = NULL;
	char *file_path;

	file_path = config_get_file_path(path);
	if (file_path == NULL) {
		goto error;
	}

	fp = fopen(file_path, mode);
	if (fp == NULL) {
		goto error;
	}

error:
	if (file_path) {
		free(file_path);
	}
	return fp;
}

/*
 *  create_config_file
 *
 *  Creates the empty config file at the path.
 *  On success, returns 0;
 *  on error, returns -1.
 */
static int create_config_file(char *path)
{
	int ret;
	FILE *fp;

	fp = open_config(path, "w+");
	if (fp == NULL) {
		ERR("Unable to create config file");
		ret = -1;
		goto error;
	}

	ret = fclose(fp);

error:
	return ret;
}

/*
 *  write_config
 *
 *  Append data to the config file in file_path
 *  On success, returns 0;
 *  on error, returns -1.
 */
static int write_config(char *file_path, size_t size, char *data)
{
	FILE *fp;
	size_t len;
	int ret = 0;

	fp = open_config(file_path, "a");
	if (fp == NULL) {
		ret = -1;
		goto end;
	}

	/* Write session name into config file */
	len = fwrite(data, size, 1, fp);
	if (len != 1) {
		ret = -1;
	}
	fclose(fp);
end:
	return ret;
}

/*
 *  config_get_default_path
 *
 *  Returns the HOME directory path. Caller MUST NOT free(3) the return pointer.
 */
char *config_get_default_path(void)
{
	return getenv("HOME");
}

/*
 *  config_destroy
 *
 *  Destroys directory config and file config.
 */
void config_destroy(char *path)
{
	int ret;
	char *config_path;

	config_path = config_get_file_path(path);
	if (config_path == NULL) {
		return;
	}

	if (!config_exists(config_path)) {
		goto end;
	}

	DBG("Removing %s\n", config_path);
	ret = remove(config_path);
	if (ret < 0) {
		perror("remove config file");
	}
end:
	free(config_path);
}

/*
 *  config_destroy_default
 *
 *  Destroys the default config
 */

void config_destroy_default(void)
{
	char *path = config_get_default_path();
	if (path == NULL) {
		return;
	}
	config_destroy(path);
}

/*
 *  config_exists
 *
 *  Returns 1 if config exists, 0 otherwise
 */
int config_exists(const char *path)
{
	int ret;
	struct stat info;

	ret = stat(path, &info);
	if (ret < 0) {
		return 0;
	}
	return S_ISREG(info.st_mode) || S_ISDIR(info.st_mode);
}

/*
 *  config_read_session_name
 *
 *  Returns the session name from the config file.
 *  The caller is responsible for freeing the returned string.
 *  On error, NULL is returned.
 */
char *config_read_session_name(char *path)
{
	int ret;
	FILE *fp;
	char var[NAME_MAX], *session_name;

	session_name = malloc(NAME_MAX);
	if (session_name == NULL) {
		ERR("Out of memory");
		goto error;
	}

	fp = open_config(path, "r");
	if (fp == NULL) {
		ERR("Can't find valid lttng config %s/.lttngrc", path);
		MSG("Did you create a session? (lttng create <my_session>)");
		goto error;
	}

	while (!feof(fp)) {
		if ((ret = fscanf(fp, "%[^'=']=%s\n", var, session_name)) != 2) {
			if (ret == -1) {
				ERR("Missing session=NAME in config file.");
				goto error_close;
			}
			continue;
		}

		if (strcmp(var, "session") == 0) {
			goto found;
		}
	}

error_close:
	fclose(fp);

error:
	return NULL;

found:
	fclose(fp);
	return session_name;

}

/*
 *  config_add_session_name
 *
 *  Write session name option to the config file.
 *  On success, returns 0;
 *  on error, returns -1.
 */
int config_add_session_name(char *path, char *name)
{
	int ret;
	char session_name[NAME_MAX];

	/*
	 * With GNU C <  2.1, snprintf returns -1 if the target buffer is too small;
	 * With GNU C >= 2.1, snprintf returns the required size (excluding closing null)
	 */
	ret = snprintf(session_name, NAME_MAX, "session=%s\n", name);
	if ((ret < 0) || (ret >= NAME_MAX)) {
		ret = -1;
		goto error;
	}
	ret = write_config(path, ret, session_name);
error:
	return ret;
}

/*
 *  config_init
 *
 *  Init configuration directory and file.
 *  On success, returns 0;
 *  on error, returns -1.
 */
int config_init(char *session_name)
{
	int ret;
	char *path;

	path = config_get_default_path();
	if (path == NULL) {
		ret = -1;
		goto error;
	}

	/* Create default config file */
	ret = create_config_file(path);
	if (ret < 0) {
		goto error;
	}

	ret = config_add_session_name(path, session_name);
	if (ret < 0) {
		goto error;
	}

	DBG("Init config session in %s", path);

error:
	return ret;
}
