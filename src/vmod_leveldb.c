/*
 * Copyright (c) 2014, Federico G. Schwindt <fgsch@lodoss.net>
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

#include <leveldb/c.h>

#include "vrt.h"
#include "cache/cache.h"

#include "vcc_if.h"

struct vmod_leveldb {
	unsigned	 	 magic;
#define VMOD_LEVELDB_MAGIC	 	0x19501220
	leveldb_t		*db;
	leveldb_options_t	*opt;
	leveldb_readoptions_t	*rdopt;
	leveldb_writeoptions_t	*wropt;
};


static void
vmod_free(void *priv)
{
	struct vmod_leveldb *v;

	CAST_OBJ_NOTNULL(v, priv, VMOD_LEVELDB_MAGIC);

	if (v->db)
		leveldb_close(v->db);
	leveldb_options_destroy(v->opt);
	leveldb_readoptions_destroy(v->rdopt);
	leveldb_writeoptions_destroy(v->wropt);

	FREE_OBJ(v);
}

static struct vmod_leveldb *
vmod_getv(struct vmod_priv *priv)
{
	struct vmod_leveldb *v;

	AN(priv);

	if (priv->priv) {
		CAST_OBJ_NOTNULL(v, priv->priv, VMOD_LEVELDB_MAGIC);
		return (v);
	}

	ALLOC_OBJ(v, VMOD_LEVELDB_MAGIC);
	AN(v);

	v->opt = leveldb_options_create();
	v->rdopt = leveldb_readoptions_create();
	v->wropt = leveldb_writeoptions_create();

	priv->free = vmod_free;
	priv->priv = v;

	return (v);
}

VCL_VOID __match_proto__(td_leveldb_create_if_missing)
vmod_create_if_missing(const struct vrt_ctx *ctx, struct vmod_priv *priv,
    VCL_BOOL enable)
{
	struct vmod_leveldb *v;

	(void)ctx;

	v = vmod_getv(priv);
	leveldb_options_set_create_if_missing(v->opt, enable);
}

VCL_VOID __match_proto__(td_leveldb_open)
vmod_open(const struct vrt_ctx *ctx, struct vmod_priv *priv, VCL_STRING name)
{
	struct vmod_leveldb *v;
	char *error = NULL;

	CHECK_OBJ_NOTNULL(ctx, VRT_CTX_MAGIC);

	v = vmod_getv(priv);
	if (v->db)
		return;

	v->db = leveldb_open(v->opt, name, &error);
	if (error) {
		/* Make the error available */
		leveldb_free(error);
	}
}

VCL_STRING __match_proto__(td_leveldb_get)
vmod_get(const struct vrt_ctx *ctx, struct vmod_priv *priv, VCL_STRING key)
{
	struct vmod_leveldb *v;
	char *error = NULL;
	char *p, *value;
	size_t len;

	CHECK_OBJ_NOTNULL(ctx, VRT_CTX_MAGIC);

	v = vmod_getv(priv);
	if (!v->db) {
		/* Make the error available */
		return (NULL);
	}

	value = leveldb_get(v->db, v->rdopt, key, strlen(key), &len, &error);
	if (error) {
		p = WS_Copy(ctx->ws, error, -1);
		leveldb_free(error);
	} else if (value) {
		p = WS_Alloc(ctx->ws, len + 1);
		if (p) {
			memcpy(p, value, len);
			p[len] = '\0';
		}
		leveldb_free(value);
	} else
		p = NULL;

	return (p);
}

VCL_VOID __match_proto__(td_leveldb_put)
vmod_put(const struct vrt_ctx *ctx, struct vmod_priv *priv, VCL_STRING key,
    VCL_STRING value)
{
	struct vmod_leveldb *v;
	char *error = NULL;

	CHECK_OBJ_NOTNULL(ctx, VRT_CTX_MAGIC);

	v = vmod_getv(priv);
	if (!v->db) {
		/* Make the error available */
		return;
	}

	leveldb_put(v->db, v->wropt, key, strlen(key), value, strlen(value),
	    &error);
	if (error) {
		/* Make the error available */
		leveldb_free(error);
	}
}

VCL_VOID __match_proto__(td_leveldb_delete)
vmod_delete(const struct vrt_ctx *ctx, struct vmod_priv *priv, VCL_STRING key)
{
	struct vmod_leveldb *v;
	char *error = NULL;

	CHECK_OBJ_NOTNULL(ctx, VRT_CTX_MAGIC);

	v = vmod_getv(priv);
	if (!v->db) {
		/* Make the error available */
		return;
	}

	leveldb_delete(v->db, v->wropt, key, strlen(key), &error);
	if (error) {
		/* Make the error available */
		leveldb_free(error);
	}
}

VCL_VOID __match_proto__(td_leveldb_close)
vmod_close(const struct vrt_ctx *ctx, struct vmod_priv *priv)
{
	CHECK_OBJ_NOTNULL(ctx, VRT_CTX_MAGIC);
	AN(priv);

	if (priv->priv) {
		vmod_free(priv->priv);
		priv->priv = NULL;
		priv->free = NULL;
	}
}
