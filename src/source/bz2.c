/*
 * Copyright 2014 Red Hat Inc., Durham, North Carolina.
 * All Rights Reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors:
 *     Šimon Lukašík
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_BZ2

#include <bzlib.h>
#include <libxml/tree.h>
#include <stdlib.h>
#include <string.h>

#include "bz2_priv.h"
#include "common/_error.h"

struct bz2_file {
	BZFILE *file;
};

static struct bz2_file *bz2_open(const char *filename)
{
	struct bz2_file *b = NULL;
	FILE* f;
	int bzerror;

	f = fopen (filename, "r" );
	if (f) {
		b = malloc(sizeof(struct bz2_file));
		b->file = BZ2_bzReadOpen(&bzerror, f, 0, 0, NULL, 0);
		if (bzerror != BZ_OK) {
			oscap_seterr(OSCAP_EFAMILY_OSCAP, "Could not build BZ2FILE from %s: %s",
					BZ2_bzerror(b->file, &bzerror));
			BZ2_bzReadClose(&bzerror, b->file);
			free(b);
			b = NULL;
		}
	}
	return b;
}

//xmlInputReadCallback
static int bz2_read(void *bzfile, char *buffer, int len)
{
	int bzerror;
	int size = BZ2_bzRead(&bzerror, ((struct bz2_file *)bzfile)->file, buffer, len);
	if (bzerror == BZ_OK || bzerror == BZ_STREAM_END)
		return size;
	else {
		oscap_seterr(OSCAP_EFAMILY_OSCAP, "Could not read from bZ2FILE: %s",
				BZ2_bzerror(((struct bz2_file *)bzfile)->file, &bzerror));
		return -1;
	}
}

// xmlInputCloseCallback
static int bz2_close(void *bzfile)
{
	int bzerror;
	BZ2_bzReadClose(&bzerror, ((struct bz2_file *)bzfile)->file);
	oscap_free(bzfile);
	return bzerror == BZ_OK ? 0 : -1;
}

xmlDoc *bz2_read_doc(const char *filepath)
{
	struct bz2_file *bzfile = bz2_open(filepath);
	if (bzfile == NULL) {
		return NULL;
	}
	return xmlReadIO(bz2_read, bz2_close, bzfile, "url", NULL, XML_PARSE_PEDANTIC);
}

bool bz2_is_file_bzip(const char *filepath)
{
	int offset = strlen(filepath) - strlen(".xml.bz2");
	if (offset >= 0) {
		return strcasecmp(filepath + offset, ".xml.bz2") == 0;
	}
	return false;
}
#endif
