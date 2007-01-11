/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* camel-pop3-folder.c : class for a pop3 folder */

/* 
 * Authors:
 *   Dan Winship <danw@ximian.com>
 *   Michael Zucchi <notzed@ximian.com>
 *
 * Copyright (C) 2002 Ximian, Inc. (www.ximian.com)
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of version 2 of the GNU Lesser General Public 
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>

#include "camel-pop3-folder.h"
#include "camel-pop3-store.h"
#include "camel-exception.h"
#include "camel-stream-mem.h"
#include "camel-stream-filter.h"
#include "camel-mime-message.h"
#include "camel-operation.h"
#include "camel-data-cache.h"
#include "camel-i18n.h"

#include <libedataserver/md5-utils.h>

#include <stdlib.h>
#include <string.h>

#define d(x) 

#define CF_CLASS(o) (CAMEL_FOLDER_CLASS (CAMEL_OBJECT_GET_CLASS(o)))
static CamelFolderClass *parent_class;

static void pop3_finalize (CamelObject *object);
static void pop3_refresh_info (CamelFolder *folder, CamelException *ex);
static void pop3_sync (CamelFolder *folder, gboolean expunge, CamelException *ex);
static gint pop3_get_message_count (CamelFolder *folder);
static GPtrArray *pop3_get_uids (CamelFolder *folder);
static CamelMimeMessage *pop3_get_message (CamelFolder *folder, const char *uid, gboolean full, CamelException *ex);
static gboolean pop3_set_message_flags (CamelFolder *folder, const char *uid, guint32 flags, guint32 set);
static CamelMimeMessage *pop3_get_top (CamelFolder *folder, const char *uid, CamelException *ex);

static void
camel_pop3_folder_class_init (CamelPOP3FolderClass *camel_pop3_folder_class)
{
	CamelFolderClass *camel_folder_class = CAMEL_FOLDER_CLASS(camel_pop3_folder_class);
	
	parent_class = CAMEL_FOLDER_CLASS(camel_folder_get_type());
	
	/* virtual method overload */
	camel_folder_class->refresh_info = pop3_refresh_info;
	camel_folder_class->sync = pop3_sync;
	
	camel_folder_class->get_message_count = pop3_get_message_count;
	camel_folder_class->get_uids = pop3_get_uids;
	camel_folder_class->free_uids = camel_folder_free_shallow;
	
	camel_folder_class->get_message = pop3_get_message;
	camel_folder_class->set_message_flags = pop3_set_message_flags;
}

CamelType
camel_pop3_folder_get_type (void)
{
	static CamelType camel_pop3_folder_type = CAMEL_INVALID_TYPE;
	
	if (!camel_pop3_folder_type) {
		camel_pop3_folder_type = camel_type_register (CAMEL_FOLDER_TYPE, "CamelPOP3Folder",
							      sizeof (CamelPOP3Folder),
							      sizeof (CamelPOP3FolderClass),
							      (CamelObjectClassInitFunc) camel_pop3_folder_class_init,
							      NULL,
							      NULL,
							      (CamelObjectFinalizeFunc) pop3_finalize);
	}
	
	return camel_pop3_folder_type;
}

static void
pop3_finalize (CamelObject *object)
{
	CamelPOP3Folder *pop3_folder = CAMEL_POP3_FOLDER (object);
	CamelPOP3FolderInfo **fi = (CamelPOP3FolderInfo **)pop3_folder->uids->pdata;
	CamelPOP3Store *pop3_store = (CamelPOP3Store *)((CamelFolder *)pop3_folder)->parent_store;
	int i;

	if (pop3_folder->uids) {
		for (i=0;i<pop3_folder->uids->len;i++,fi++) {
			if (fi[0]->cmd) {
				while (camel_pop3_engine_iterate(pop3_store->engine, fi[0]->cmd) > 0)
					;
				camel_pop3_engine_command_free(pop3_store->engine, fi[0]->cmd);
			}
			
			g_free(fi[0]->uid);
			g_free(fi[0]);
		}
		
		g_ptr_array_free(pop3_folder->uids, TRUE);
		g_hash_table_destroy(pop3_folder->uids_uid);
	}
}

CamelFolder *
camel_pop3_folder_new (CamelStore *parent, CamelException *ex)
{
	CamelFolder *folder;
	CamelPOP3Store *p3store = (CamelPOP3Store*) parent;
	gchar *summary_file;

	d(printf("opening pop3 INBOX folder\n"));
	
	folder = CAMEL_FOLDER (camel_object_new (CAMEL_POP3_FOLDER_TYPE));
	camel_folder_construct (folder, parent, "inbox", "inbox");
	
	summary_file = g_strdup_printf ("%s/summary.mmap", p3store->root);
	folder->summary = camel_folder_summary_new (folder);
	camel_folder_summary_set_build_content (folder->summary, TRUE);
	camel_folder_summary_set_filename (folder->summary, summary_file);

	if (camel_folder_summary_load (folder->summary) == -1) {
		camel_folder_summary_clear (folder->summary);
		camel_folder_summary_touch (folder->summary);
		camel_folder_summary_save (folder->summary);
		camel_folder_summary_load (folder->summary);
	}
	g_free (summary_file);


	/* mt-ok, since we dont have the folder-lock for new() */
	camel_folder_refresh_info (folder, ex);/* mt-ok */
	if (camel_exception_is_set (ex)) {
		camel_object_unref (CAMEL_OBJECT (folder));
		folder = NULL;
	}

	if (!folder->summary) {
		camel_object_unref (CAMEL_OBJECT (folder));
		camel_exception_setv (ex, CAMEL_EXCEPTION_SYSTEM,
				      _("Could not load summary for INBOX"));
		return NULL;
	}
	
	folder->folder_flags |= CAMEL_FOLDER_HAS_SUMMARY_CAPABILITY;

	return folder;
}

/* create a uid from md5 of 'top' output */
static void
cmd_builduid(CamelPOP3Engine *pe, CamelPOP3Stream *stream, void *data)
{
	CamelPOP3FolderInfo *fi = data;
	MD5Context md5;
	unsigned char digest[16];
	struct _camel_header_raw *h;
	CamelMimeParser *mp;

	/* TODO; somehow work out the limit and use that for proper progress reporting
	   We need a pointer to the folder perhaps? */
	camel_operation_progress_count(NULL, fi->id);

	md5_init(&md5);
	mp = camel_mime_parser_new();
	camel_mime_parser_init_with_stream(mp, (CamelStream *)stream);
	switch (camel_mime_parser_step(mp, NULL, NULL)) {
	case CAMEL_MIME_PARSER_STATE_HEADER:
	case CAMEL_MIME_PARSER_STATE_MESSAGE:
	case CAMEL_MIME_PARSER_STATE_MULTIPART:
		h = camel_mime_parser_headers_raw(mp);
		while (h) {
			if (g_ascii_strcasecmp(h->name, "status") != 0
			    && g_ascii_strcasecmp(h->name, "x-status") != 0) {
				md5_update(&md5, (const guchar*) h->name, strlen(h->name));
				md5_update(&md5, (const guchar*) h->value, strlen(h->value));
			}
			h = h->next;
		}
	default:
		break;
	}
	camel_object_unref(mp);
	md5_final(&md5, digest);
	fi->uid = camel_base64_encode_simple((const char*) digest, 16);

	d(printf("building uid for id '%d' = '%s'\n", fi->id, fi->uid));
}

static void
cmd_list(CamelPOP3Engine *pe, CamelPOP3Stream *stream, void *data)
{
	int ret;
	unsigned int len, id, size;
	unsigned char *line;
	CamelFolder *folder = data;
	CamelPOP3Store *pop3_store = CAMEL_POP3_STORE (folder->parent_store);
	CamelPOP3FolderInfo *fi;

	do {
		ret = camel_pop3_stream_line(stream, &line, &len);
		if (ret>=0) {
			if (sscanf((char *) line, "%u %u", &id, &size) == 2) {
				fi = g_malloc0 (sizeof(*fi));
				fi->size = size;
				fi->id = id;
				fi->index = ((CamelPOP3Folder *)folder)->uids->len;
				if ((pop3_store->engine->capa & CAMEL_POP3_CAP_UIDL) == 0)
					fi->cmd = camel_pop3_engine_command_new(pe, CAMEL_POP3_COMMAND_MULTI, cmd_builduid, fi, "TOP %u 0\r\n", id);
				g_ptr_array_add(((CamelPOP3Folder *)folder)->uids, fi);
				g_hash_table_insert(((CamelPOP3Folder *)folder)->uids_id, GINT_TO_POINTER(id), fi);
			}
		}
	} while (ret>0);
}

static void
cmd_uidl(CamelPOP3Engine *pe, CamelPOP3Stream *stream, void *data)
{
	int ret;
	unsigned int len;
	unsigned char *line;
	char uid[1025];
	unsigned int id;
	CamelPOP3FolderInfo *fi;
	CamelPOP3Folder *folder = data;
	
	do {
		ret = camel_pop3_stream_line(stream, &line, &len);
		if (ret>=0) {
			if (strlen((char*) line) > 1024)
				line[1024] = 0;
			if (sscanf((char *) line, "%u %s", &id, uid) == 2) {
				fi = g_hash_table_lookup(folder->uids_id, GINT_TO_POINTER(id));
				if (fi) {
					camel_operation_progress(NULL, (fi->index+1) * 100 / folder->uids->len);
					fi->uid = g_strdup(uid);
					g_hash_table_insert(folder->uids_uid, fi->uid, fi);
				} else {
					g_warning("ID %u (uid: %s) not in previous LIST output", id, uid);
				}
			}
		}
	} while (ret>0);
}

static void 
pop3_refresh_info (CamelFolder *folder, CamelException *ex)
{
	CamelPOP3Store *pop3_store = CAMEL_POP3_STORE (folder->parent_store);
	CamelPOP3Folder *pop3_folder = (CamelPOP3Folder *) folder;
	CamelPOP3Command *pcl, *pcu = NULL;
	int i;

	camel_operation_start (NULL, _("Retrieving POP summary"));

	pop3_folder->uids = g_ptr_array_new ();
	pop3_folder->uids_uid = g_hash_table_new(g_str_hash, g_str_equal);
	/* only used during setup */
	pop3_folder->uids_id = g_hash_table_new(NULL, NULL);

	pcl = camel_pop3_engine_command_new(pop3_store->engine, CAMEL_POP3_COMMAND_MULTI, cmd_list, folder, "LIST\r\n");
	if (pop3_store->engine->capa & CAMEL_POP3_CAP_UIDL) {
		pcu = camel_pop3_engine_command_new(pop3_store->engine, CAMEL_POP3_COMMAND_MULTI, cmd_uidl, folder, "UIDL\r\n");
	}
	while ((i = camel_pop3_engine_iterate(pop3_store->engine, NULL)) > 0)
		;

	if (i == -1) {
		if (errno == EINTR)
			camel_exception_setv(ex, CAMEL_EXCEPTION_USER_CANCEL, _("User canceled"));
		else
			camel_exception_setv (ex, CAMEL_EXCEPTION_SYSTEM,
					      _("Cannot get POP summary: %s"),
					      g_strerror (errno));
	}

	/* TODO: check every id has a uid & commands returned OK too? */
	
	camel_pop3_engine_command_free(pop3_store->engine, pcl);
	

	/* Update the summary.mmap file */
	for (i=0;i<pop3_folder->uids->len;i++) 
	{
		CamelPOP3FolderInfo *fi = pop3_folder->uids->pdata[i];
		CamelMessageInfoBase *mi = NULL;

		mi = (CamelMessageInfoBase*) camel_folder_summary_uid (folder->summary, fi->uid);
		if (!mi)
		{
			CamelMimeMessage *msg = NULL;

			if (pop3_store->engine->capa & CAMEL_POP3_CAP_TOP) 
				msg = pop3_get_top (folder, fi->uid, NULL);
			else
				msg = pop3_get_message (folder, fi->uid, FALSE, NULL);

			if (msg) 
				camel_object_unref (CAMEL_OBJECT (msg));

		} else
			camel_message_info_free (mi);
	}

	if (pop3_store->engine->capa & CAMEL_POP3_CAP_UIDL) {
		camel_pop3_engine_command_free(pop3_store->engine, pcu);
	} else {
		for (i=0;i<pop3_folder->uids->len;i++) {
			CamelPOP3FolderInfo *fi = pop3_folder->uids->pdata[i];

			if (fi->cmd) {
				camel_pop3_engine_command_free(pop3_store->engine, fi->cmd);
				fi->cmd = NULL;
			}
			if (fi->uid)
				g_hash_table_insert(pop3_folder->uids_uid, fi->uid, fi);
		}
	}

	/* dont need this anymore */
	g_hash_table_destroy(pop3_folder->uids_id);
	
	camel_operation_end (NULL);
	return;
}

static void
pop3_sync (CamelFolder *folder, gboolean expunge, CamelException *ex)
{
	CamelPOP3Folder *pop3_folder;
	CamelPOP3Store *pop3_store;
	int i; CamelPOP3FolderInfo *fi;

	pop3_folder = CAMEL_POP3_FOLDER (folder);
	pop3_store = CAMEL_POP3_STORE (folder->parent_store);

	if(pop3_store->delete_after && !expunge)
	{	
		camel_operation_start(NULL, _("Expunging old messages"));
		camel_pop3_delete_old(folder, pop3_store->delete_after,ex);	
	}	

	if (!expunge)
		return;
	
	camel_operation_start(NULL, _("Expunging deleted messages"));
	
	for (i = 0; i < pop3_folder->uids->len; i++) {
		fi = pop3_folder->uids->pdata[i];
		/* busy already?  wait for that to finish first */
		if (fi->cmd) {
			while (camel_pop3_engine_iterate(pop3_store->engine, fi->cmd) > 0)
				;
			camel_pop3_engine_command_free(pop3_store->engine, fi->cmd);
			fi->cmd = NULL;
		}

		if (fi->flags & CAMEL_MESSAGE_DELETED) {
			fi->cmd = camel_pop3_engine_command_new(pop3_store->engine, 0, NULL, NULL, "DELE %u\r\n", fi->id);

			/* also remove from cache */
			if (pop3_store->cache && fi->uid)
				camel_data_cache_remove(pop3_store->cache, "cache", fi->uid, NULL);
		}
	}

	for (i = 0; i < pop3_folder->uids->len; i++) {
		fi = pop3_folder->uids->pdata[i];
		/* wait for delete commands to finish */
		if (fi->cmd) {
			while (camel_pop3_engine_iterate(pop3_store->engine, fi->cmd) > 0)
				;
			camel_pop3_engine_command_free(pop3_store->engine, fi->cmd);
			fi->cmd = NULL;
		}
		camel_operation_progress(NULL, (i+1) * 100 / pop3_folder->uids->len);
	}

	camel_operation_end(NULL);

	camel_pop3_store_expunge (pop3_store, ex);
}

int
camel_pop3_delete_old(CamelFolder *folder, int days_to_delete,	CamelException *ex)
{
	CamelPOP3Folder *pop3_folder;
	CamelPOP3FolderInfo *fi;
	int i;
	CamelPOP3Store *pop3_store;
	time_t temp;

	pop3_folder = CAMEL_POP3_FOLDER (folder);
	pop3_store = CAMEL_POP3_STORE (CAMEL_FOLDER(pop3_folder)->parent_store);	
	temp = time(&temp);

	for (i = 0; i < pop3_folder->uids->len; i++) 
	{
		CamelMimeMessage *message = NULL;

		fi = pop3_folder->uids->pdata[i];
		message = pop3_get_message (folder, fi->uid, TRUE, ex);	
		time_t message_time = message->date + message->date_offset;

		if (message) 
		{
		    double time_diff = difftime(temp,message_time);
		    int day_lag = time_diff/(60*60*24);
		    if (day_lag > days_to_delete)
		    {
			if (fi->cmd) 
			{
			    while (camel_pop3_engine_iterate(pop3_store->engine, fi->cmd) > 0)
				    ;
			    camel_pop3_engine_command_free(pop3_store->engine, fi->cmd);
			    fi->cmd = NULL;
		    	}

		    	fi->cmd = camel_pop3_engine_command_new(pop3_store->engine, 0, NULL, NULL, "DELE %u\r\n", fi->id);
			/* also remove from cache */
		    	if (pop3_store->cache && fi->uid)
			    camel_data_cache_remove(pop3_store->cache, "cache", fi->uid, NULL);
		    }

		    camel_object_unref (CAMEL_OBJECT (message));
		}

	}

	for (i = 0; i < pop3_folder->uids->len; i++) 
	{
		fi = pop3_folder->uids->pdata[i];
		/* wait for delete commands to finish */
		if (fi->cmd) {
			while (camel_pop3_engine_iterate(pop3_store->engine, fi->cmd) > 0)
				;
			camel_pop3_engine_command_free(pop3_store->engine, fi->cmd);
			fi->cmd = NULL;
		}
		camel_operation_progress(NULL, (i+1) * 100 / pop3_folder->uids->len);
	}

	camel_operation_end(NULL);

	camel_pop3_store_expunge (pop3_store, ex);

	return 0;
	
}

static void
cmd_tocache(CamelPOP3Engine *pe, CamelPOP3Stream *stream, void *data)
{
	CamelPOP3FolderInfo *fi = data;
	char buffer[2048];
	int w = 0, n;

	/* What if it fails? */
	/* We write an '*' to the start of the stream to say its not complete yet */
	/* This should probably be part of the cache code */

	if ((n = camel_stream_write(fi->stream, "*", 1)) == -1)
		goto done;

	while ((n = camel_stream_read((CamelStream *)stream, buffer, sizeof(buffer))) > 0) 
	{
		n = camel_stream_write(fi->stream, buffer, n);
		if (n == -1)
			break;

		w += n;
		if (w > fi->size)
			w = fi->size;
		if (fi->size != 0)
			camel_operation_progress(NULL, (w * 100) / fi->size);
	}

	/* it all worked, output a '#' to say we're a-ok */
	if (n != -1) {
		camel_stream_reset(fi->stream);
		n = camel_stream_write(fi->stream, "#", 1);
	}
done:
	if (n == -1) {
		fi->err = errno;
		g_warning("POP3 retrieval failed: %s", strerror(errno));
	} else {
		fi->err = 0;
	}
	
	camel_object_unref((CamelObject *)fi->stream);
	fi->stream = NULL;
}


static void
cmd_tocache_partial (CamelPOP3Engine *pe, CamelPOP3Stream *stream, void *data)
{
	CamelPOP3FolderInfo *fi = data;
	char buffer[2048];
	int w = 0, n;
	char boundary[1024];
	char search[9] = "boundary=";
	int bpos = 0;
	gboolean continue_next = FALSE;
	gboolean have_boundary = FALSE, will_have_boundary = FALSE;

	/* We write an '*' to the start of the stream to say its not complete yet */
	if ((n = camel_stream_write(fi->stream, "*", 1)) == -1)
		goto done;

	while ((n = camel_stream_read((CamelStream *)stream, buffer, sizeof(buffer))) > 0) 
	{
		char *p = NULL;

		n = camel_stream_write(fi->stream, buffer, n);
		if (n == -1)
			break;

		if (!have_boundary)
		{
		    if (!continue_next)
			    p = strchr (buffer, search[0]);

		    if (p || continue_next)
		    {
			    gint pos = continue_next ? 0 : (gint) (p-buffer);
			    gboolean detected_end = FALSE;
			    gboolean first_dq = FALSE;

			    while (!detected_end && p)
			    {
				if (bpos < sizeof (boundary) && pos < sizeof (buffer))
				{

					if (bpos == 8 && buffer[pos] == '=')
						will_have_boundary = TRUE;
					else if (bpos == 8) 
					{
						will_have_boundary = FALSE;
						continue_next = FALSE;
						first_dq = FALSE;
						p = strchr (buffer + pos, search[0]);
						bpos = 0;
						if (p) 
							pos = (gint) (p - buffer);
						else
							detected_end = TRUE;
						continue;
					}

					if (buffer[pos] != '\n')
					{
					    if (buffer[pos] == '\"')
					    {
						    if (first_dq) {
							    if (will_have_boundary) {
								have_boundary = TRUE;
							    	detected_end = TRUE;
							    	continue_next = FALSE;
							    }
						    }
						    first_dq = TRUE;
					    }

					    boundary[bpos] = buffer[pos];
					} else {
						if (will_have_boundary) {
							have_boundary = TRUE;
	 					    	detected_end = TRUE;
						    	continue_next = FALSE;
						} else {

						    will_have_boundary = FALSE;
						    continue_next = FALSE;
						    first_dq = FALSE;						  
						    p = strchr (buffer + pos, search[0]);
						    bpos = 0;
						    if (p) 
							    pos = (gint) (p - buffer);
						    else 
							    detected_end = TRUE;
						    continue;						
						}
					}

					bpos++;
					pos++;

				} else {
				    detected_end = TRUE;
				    if (!(bpos < sizeof (boundary)))
					    continue_next = TRUE;
				    else
					    continue_next = FALSE;
				}
			    }
		    }
		}

		w += n;
		if (w > fi->size)
			w = fi->size;
		if (fi->size != 0)
			camel_operation_progress(NULL, (w * 100) / fi->size);
	}

	/* it all worked, output a '#' to say we're a-ok */
	if (n != -1) {
		camel_stream_reset(fi->stream);
		n = camel_stream_write(fi->stream, "#", 1);
	}
done:
	if (n == -1) {
		fi->err = errno;
		g_warning("POP3 retrieval failed: %s", strerror(errno));
	} else {
		fi->err = 0;
	}
	
	camel_object_unref((CamelObject *)fi->stream);
	fi->stream = NULL;

	if (have_boundary)
		printf ("Boundary=%s\n", boundary);
}


static CamelMimeMessage *
pop3_get_message (CamelFolder *folder, const char *uid, gboolean full, CamelException *ex)
{
	CamelMimeMessage *message = NULL;
	CamelPOP3Store *pop3_store = CAMEL_POP3_STORE (folder->parent_store);
	CamelPOP3Folder *pop3_folder = (CamelPOP3Folder *)folder;
	CamelPOP3Command *pcr;
	CamelPOP3FolderInfo *fi;
	char buffer[1]; int i;
	CamelStream *stream = NULL;
	CamelFolderSummary *summary = folder->summary;
	CamelMessageInfoBase *mi;

	/* TNY TODO: Implement partial message retrieval if full==TRUE */

	fi = g_hash_table_lookup(pop3_folder->uids_uid, uid);
	if (fi == NULL) {
		camel_exception_setv (ex, CAMEL_EXCEPTION_FOLDER_INVALID_UID,
				      _("No message with UID %s"), uid);
		return NULL;
	}

	/* Sigh, most of the crap in this function is so that the cancel button
	   returns the proper exception code.  Sigh. */

	camel_operation_start_transient(NULL, _("Retrieving POP message %d"), fi->id);

	/* If we have an oustanding retrieve message running, wait for that to complete
	   & then retrieve from cache, otherwise, start a new one, and similar */

	if (fi->cmd != NULL) {
		while ((i = camel_pop3_engine_iterate(pop3_store->engine, fi->cmd)) > 0)
			;

		if (i == -1)
			fi->err = errno;

		/* getting error code? */
		/*g_assert (fi->cmd->state == CAMEL_POP3_COMMAND_DATA);*/
		camel_pop3_engine_command_free(pop3_store->engine, fi->cmd);
		fi->cmd = NULL;

		if (fi->err != 0) {
			if (fi->err == EINTR)
				camel_exception_setv(ex, CAMEL_EXCEPTION_USER_CANCEL, _("User canceled"));
			else
				camel_exception_setv (ex, CAMEL_EXCEPTION_SYSTEM,
						      _("Cannot get message %s: %s"),
						      uid, g_strerror (fi->err));
			goto fail;
		}
	}
	
	/* check to see if we have safely written flag set */
	if (pop3_store->cache == NULL
	    || (stream = camel_data_cache_get(pop3_store->cache, "cache", fi->uid, NULL)) == NULL
	    || camel_stream_read(stream, buffer, 1) != 1
	    || buffer[0] != '#') {

		/* Initiate retrieval, if disk backing fails, use a memory backing */
		if (pop3_store->cache == NULL
		    || (stream = camel_data_cache_add(pop3_store->cache, "cache", fi->uid, NULL)) == NULL)
			stream = camel_stream_mem_new();

		/* ref it, the cache storage routine unref's when done */
		camel_object_ref((CamelObject *)stream);
		fi->stream = stream;
		fi->err = EIO;
		pcr = camel_pop3_engine_command_new(pop3_store->engine, CAMEL_POP3_COMMAND_MULTI, cmd_tocache /*_partial*/, fi, "RETR %u\r\n", fi->id);

		while ((i = camel_pop3_engine_iterate(pop3_store->engine, pcr)) > 0)
			;
		if (i == -1)
			fi->err = errno;

		/* getting error code? */
		/*g_assert (pcr->state == CAMEL_POP3_COMMAND_DATA);*/
		camel_pop3_engine_command_free(pop3_store->engine, pcr);
		camel_stream_reset(stream);

		/* Check to see we have safely written flag set */
		if (fi->err != 0) {
			if (fi->err == EINTR)
				camel_exception_setv(ex, CAMEL_EXCEPTION_USER_CANCEL, _("User canceled"));
			else
				camel_exception_setv (ex, CAMEL_EXCEPTION_SYSTEM,
						      _("Cannot get message %s: %s"),
						      uid, g_strerror (fi->err));
			goto done;
		}

		if (camel_stream_read(stream, buffer, 1) != 1 || buffer[0] != '#') {
			camel_exception_setv(ex, CAMEL_EXCEPTION_SYSTEM,
					     _("Cannot get message %s: %s"), uid, _("Unknown reason"));
			goto done;
		}
	}

	message = camel_mime_message_new ();
	if (camel_data_wrapper_construct_from_stream((CamelDataWrapper *)message, stream) == -1) {
		if (errno == EINTR)
			camel_exception_setv(ex, CAMEL_EXCEPTION_USER_CANCEL, _("User canceled"));
		else
			camel_exception_setv (ex, CAMEL_EXCEPTION_SYSTEM,
					      _("Cannot get message %s: %s"),
					      uid, g_strerror (errno));
		camel_object_unref((CamelObject *)message);
		message = NULL;
	}

	mi = (CamelMessageInfoBase *) camel_folder_summary_uid (summary, uid);
	if (mi) camel_message_info_free (mi);

	mi = (CamelMessageInfoBase *) camel_folder_summary_info_new_from_message (summary, message);

	mi->flags |= CAMEL_MESSAGE_INFO_UID_NEEDS_FREE;
	mi->uid = g_strdup (fi->uid);

	camel_folder_summary_add (summary, (CamelMessageInfo *)mi);

done:
	camel_object_unref((CamelObject *)stream);
fail:
	camel_operation_end(NULL);

	return message;
}



static CamelMimeMessage *
pop3_get_top (CamelFolder *folder, const char *uid, CamelException *ex)
{
	CamelMimeMessage *message = NULL;
	CamelPOP3Store *pop3_store = CAMEL_POP3_STORE (folder->parent_store);
	CamelPOP3Folder *pop3_folder = (CamelPOP3Folder *)folder;
	CamelPOP3Command *pcr;
	CamelPOP3FolderInfo *fi;
	char buffer[1]; int i; 
	CamelStream *stream = NULL, *old;
	CamelFolderSummary *summary = folder->summary;
	CamelMessageInfoBase *mi;

	fi = g_hash_table_lookup(pop3_folder->uids_uid, uid);

	if (fi == NULL) {
		camel_exception_setv (ex, CAMEL_EXCEPTION_FOLDER_INVALID_UID,
				      _("No message with UID %s"), uid);
		return NULL;
	}

	old = fi->stream;

	/* Sigh, most of the crap in this function is so that the cancel button
	   returns the proper exception code.  Sigh. */

	camel_operation_start_transient(NULL, _("Retrieving POP message %d"), fi->id);

	/* If we have an oustanding retrieve message running, wait for that to complete
	   & then retrieve from cache, otherwise, start a new one, and similar */

	if (fi->cmd != NULL) {
		while ((i = camel_pop3_engine_iterate(pop3_store->engine, fi->cmd)) > 0)
			;

		if (i == -1)
			fi->err = errno;

		/* getting error code? */
		/*g_assert (fi->cmd->state == CAMEL_POP3_COMMAND_DATA);*/
		camel_pop3_engine_command_free(pop3_store->engine, fi->cmd);
		fi->cmd = NULL;

		if (fi->err != 0) {
			if (fi->err == EINTR)
				camel_exception_setv(ex, CAMEL_EXCEPTION_USER_CANCEL, _("User canceled"));
			else
				camel_exception_setv (ex, CAMEL_EXCEPTION_SYSTEM,
						      _("Cannot get message %s: %s"),
						      uid, g_strerror (fi->err));
			goto fail;
		}
	}
	
	/* check to see if we have safely written flag set */
	if (pop3_store->cache == NULL
	    || (stream = camel_data_cache_get(pop3_store->cache, "cache", fi->uid, NULL)) == NULL
	    || camel_stream_read(stream, buffer, 1) != 1
	    || buffer[0] != '#') {
			

		stream = camel_stream_mem_new();

		/* the cmd_tocache thing unrefs it, therefore this is to keep it
		   compatible with existing code */
		camel_object_ref (CAMEL_OBJECT (stream));

		fi->stream = stream;
		fi->err = EIO;

		/* TOP %s 1 only returns the headers of a message and the first
		   line. Which is fine and to make sure broken POP servers also
		   return something (in case TOP %s 0 would otherwise be 
		   misinterpreted by the POP server) */

		pcr = camel_pop3_engine_command_new(pop3_store->engine, CAMEL_POP3_COMMAND_MULTI, 
			cmd_tocache, fi, "TOP %u 1\r\n", fi->id);

		while ((i = camel_pop3_engine_iterate(pop3_store->engine, pcr)) > 0)
			;
		if (i == -1)
			fi->err = errno;

		camel_pop3_engine_command_free(pop3_store->engine, pcr);
		camel_stream_reset(stream);

		fi->stream = old;

		/* Check to see we have safely written flag set */
		if (fi->err != 0) {
			if (fi->err == EINTR)
				camel_exception_setv(ex, CAMEL_EXCEPTION_USER_CANCEL, _("User canceled"));
			else
				camel_exception_setv (ex, CAMEL_EXCEPTION_SYSTEM,
						      _("Cannot get message %s: %s"),
						      uid, g_strerror (fi->err));
			goto done;
		}

		if (camel_stream_read(stream, buffer, 1) != 1 || buffer[0] != '#') {
			camel_exception_setv(ex, CAMEL_EXCEPTION_SYSTEM,
					     _("Cannot get message %s: %s"), uid, _("Unknown reason"));
			goto done;
		}
	}

	message = camel_mime_message_new ();
	if (camel_data_wrapper_construct_from_stream((CamelDataWrapper *)message, stream) == -1) {
		if (errno == EINTR)
			camel_exception_setv(ex, CAMEL_EXCEPTION_USER_CANCEL, _("User canceled"));
		else
			camel_exception_setv (ex, CAMEL_EXCEPTION_SYSTEM,
					      _("Cannot get message %s: %s"),
					      uid, g_strerror (errno));
		camel_object_unref((CamelObject *)message);
		message = NULL;
	}

	mi = (CamelMessageInfoBase *) camel_folder_summary_uid (summary, uid);
	if (mi) camel_message_info_free (mi);

	mi = (CamelMessageInfoBase *) camel_folder_summary_info_new_from_message (summary, message);

	mi->flags |= CAMEL_MESSAGE_INFO_UID_NEEDS_FREE;
	mi->uid = g_strdup (fi->uid);

	camel_folder_summary_add (summary, (CamelMessageInfo *)mi);

done:
	camel_object_unref((CamelObject *)stream);
fail:
	camel_operation_end(NULL);

	return message;
}

static gboolean
pop3_set_message_flags (CamelFolder *folder, const char *uid, guint32 flags, guint32 set)
{
	CamelPOP3Folder *pop3_folder = CAMEL_POP3_FOLDER (folder);
	CamelPOP3FolderInfo *fi;
	gboolean res = FALSE;

	fi = g_hash_table_lookup(pop3_folder->uids_uid, uid);
	if (fi) {
		guint32 new = (fi->flags & ~flags) | (set & flags);

		if (fi->flags != new) {
			fi->flags = new;
			res = TRUE;
		}
	}

	return res;
}

static gint
pop3_get_message_count (CamelFolder *folder)
{
	CamelPOP3Folder *pop3_folder = CAMEL_POP3_FOLDER (folder);
	
	return pop3_folder->uids->len;
}

static GPtrArray *
pop3_get_uids (CamelFolder *folder)
{
	CamelPOP3Folder *pop3_folder = CAMEL_POP3_FOLDER (folder);
	GPtrArray *uids = g_ptr_array_new();
	CamelPOP3FolderInfo **fi = (CamelPOP3FolderInfo **)pop3_folder->uids->pdata;
	int i;

	for (i=0;i<pop3_folder->uids->len;i++,fi++) {
		if (fi[0]->uid)
			g_ptr_array_add(uids, fi[0]->uid);
	}
	
	return uids;
}
