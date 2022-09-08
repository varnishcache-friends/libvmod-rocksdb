/*
 * Copyright (c) 2014-2015, 2022 Federico G. Schwindt <fgsch@lodoss.net>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <rocksdb/c.h>

#include "cache/cache.h"

#include "vsb.h"

#include "vcc_if.h"

struct vmod_rocksdb_rocksdb {
	unsigned	 	 magic;
#define VMOD_ROCKSDB_MAGIC	 	0x19501220
	rocksdb_t		*db;
	rocksdb_options_t	*options;
	rocksdb_readoptions_t	*read_options;
	rocksdb_writeoptions_t	*write_options;
};


static void
vslv(VRT_CTX, enum VSL_tag_e tag, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	if (ctx->vsl)
		VSLbv(ctx->vsl, tag, fmt, ap);
	else
		VSLv(tag, 0, fmt, ap);
	va_end(ap);
}

VCL_VOID
vmod_rocksdb__init(VRT_CTX, struct vmod_rocksdb_rocksdb **vpp,
    const char *vcl_name, VCL_STRING filename,
    VCL_BOOL create_if_missing)
{
	struct vmod_rocksdb_rocksdb *vp;
	rocksdb_t *db;
	rocksdb_options_t *options;
	char *error;

	(void)vcl_name;

	CHECK_OBJ_NOTNULL(ctx, VRT_CTX_MAGIC);
	AN(vpp);
	AZ(*vpp);

	VSL(SLT_Debug, 0, "rocksdb.rocksdb: Using rocksdb %s",
	    ROCKSDB_VERSION);

	options = rocksdb_options_create();
	rocksdb_options_set_create_if_missing(options,
	    create_if_missing);

	error = NULL;
	db = rocksdb_open(options, filename, &error);
	if (error) {
		char errstr[512];

		snprintf(errstr, sizeof(errstr), "rocksdb.rocksdb: %s",
		    error);
		free(error);
		VSL(SLT_Error, 0, "%s", errstr);
		/* Send the error to the CLI too. */
		VSB_printf(ctx->msg, "%s\n", errstr);
		return;
	}

	ALLOC_OBJ(vp, VMOD_ROCKSDB_MAGIC);
	AN(vp);
	*vpp = vp;
	vp->db = db;
	vp->options = options;
	vp->read_options = rocksdb_readoptions_create();
	vp->write_options = rocksdb_writeoptions_create();
}

VCL_VOID
vmod_rocksdb__fini(struct vmod_rocksdb_rocksdb **vpp)
{
	struct vmod_rocksdb_rocksdb *vp;

	AN(*vpp);
	vp = *vpp;
	*vpp = NULL;
	CHECK_OBJ_NOTNULL(vp, VMOD_ROCKSDB_MAGIC);
	rocksdb_writeoptions_destroy(vp->write_options);
	rocksdb_readoptions_destroy(vp->read_options);
	rocksdb_options_destroy(vp->options);
	rocksdb_close(vp->db);
	FREE_OBJ(vp);
}

VCL_STRING
vmod_rocksdb_get(VRT_CTX, struct vmod_rocksdb_rocksdb *vp,
    VCL_STRING key)
{
	char *error, *value;
	const char *p;
	size_t len;

	CHECK_OBJ_NOTNULL(ctx, VRT_CTX_MAGIC);
	CHECK_OBJ_NOTNULL(vp, VMOD_ROCKSDB_MAGIC);

	if (!key || !*key) {
		vslv(ctx, SLT_Error,
		    "rocksdb.get: Invalid or missing key (%s)",
		    key ? key : "NULL");
		return NULL;
	}

	error = NULL;
	value = rocksdb_get(vp->db, vp->read_options, key, strlen(key),
	    &len, &error);
	if (error) {
		/* Make the error available */
		free(error);
		return NULL;
	}

	p = NULL;
	if (value) {
		p = WS_Printf(ctx->ws, "%.*s", (int)len, value);
		free(value);
	}

	return p;
}

VCL_VOID
vmod_rocksdb_put(VRT_CTX, struct vmod_rocksdb_rocksdb *vp,
    VCL_STRING key, VCL_STRING value)
{
	char *error;

	CHECK_OBJ_NOTNULL(ctx, VRT_CTX_MAGIC);
	CHECK_OBJ_NOTNULL(vp, VMOD_ROCKSDB_MAGIC);

	if (!key || !*key) {
		vslv(ctx, SLT_Error,
		    "rocksdb.put: Invalid or missing key (%s)",
		    key ? key : "NULL");
		return;
	}

	if (!value || !*value) {
		vslv(ctx, SLT_Error,
		    "rocksdb.put: Invalid or missing value (%s)",
		    value ? value : "NULL");
		return;
	}

	error = NULL;
	rocksdb_put(vp->db, vp->write_options, key, strlen(key),
	    value, strlen(value), &error);
	if (error) {
		/* Make the error available */
		free(error);
	}
}

VCL_VOID
vmod_rocksdb_delete(VRT_CTX, struct vmod_rocksdb_rocksdb *vp,
    VCL_STRING key)
{
	char *error;

	CHECK_OBJ_NOTNULL(ctx, VRT_CTX_MAGIC);
	CHECK_OBJ_NOTNULL(vp, VMOD_ROCKSDB_MAGIC);

	if (!key || !*key) {
		vslv(ctx, SLT_Error,
		    "rocksdb.delete: Invalid or missing key (%s)",
		    key ? key : "NULL");
		return;
	}

	error = NULL;
	rocksdb_delete(vp->db, vp->write_options, key, strlen(key),
	    &error);
	if (error) {
		/* Make the error available */
		free(error);
	}
}
