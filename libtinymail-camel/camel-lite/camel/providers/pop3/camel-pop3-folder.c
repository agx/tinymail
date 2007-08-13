/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* camel-pop3-folder.c : class for a pop3 folder */

/* 
 * Authors:
 *   Dan Winship <danw@ximian.com>
 *   Michael Zucchi <notzed@ximian.com>
 *   Philip Van Hoof <pvanhoof@gnome.org>
 *
 * This is CamelPop3Store for camel-lite that implements CamelDiscoFolder and
 * has support for CamelFolderSummary. Its implementation is significantly 
 * different from Camel's upstream version (being used by Evolution): this
 * version supports offline and online modes.
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


#include <glib/gstdio.h>

#include <errno.h>
#include <stdlib.h>

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <string.h>

#include <glib/gi18n-lib.h>

#include <libedataserver/md5-utils.h>

#include "camel-data-cache.h"
#include "camel-exception.h"
#include "camel-mime-message.h"
#include "camel-operation.h"

#include "camel-pop3-folder.h"
#include "camel-pop3-store.h"
#include "camel-stream-filter.h"
#include "camel-pop3-stream.h"
#include "camel-stream-mem.h"
#include "camel-string-utils.h"

#include "camel-disco-diary.h"

#include "camel-stream-buffer.h"

#define d(x) 

#define CF_CLASS(o) (CAMEL_FOLDER_CLASS (CAMEL_OBJECT_GET_CLASS(o)))
static CamelFolderClass *parent_class;
static CamelDiscoFolderClass *disco_folder_class = NULL;

static void pop3_finalize (CamelObject *object);
static void pop3_refresh_info (CamelFolder *folder, CamelException *ex);
static void pop3_sync (CamelFolder *folder, gboolean expunge, CamelException *ex);
static GPtrArray *pop3_get_uids (CamelFolder *folder);
static CamelMimeMessage *pop3_get_message (CamelFolder *folder, const char *uid, CamelFolderReceiveType type, gint param, CamelException *ex);
static gboolean pop3_set_message_flags (CamelFolder *folder, const char *uid, guint32 flags, guint32 set);
static CamelMimeMessage *pop3_get_top (CamelFolder *folder, const char *uid, CamelException *ex);
static int pop3_get_local_size (CamelFolder *folder);

static void pop3_delete_attachments (CamelFolder *folder, const char *uid);

char *strcasestr(const char *haystack, const char *needle);

static void 
destroy_lists (CamelPOP3Folder *pop3_folder)
{

	if (pop3_folder->uids != NULL) 
	{
		CamelPOP3FolderInfo **fi = (CamelPOP3FolderInfo **)pop3_folder->uids->pdata;
		CamelPOP3Store *pop3_store = (CamelPOP3Store *)((CamelFolder *)pop3_folder)->parent_store;
		int i;

		for (i=0;i<pop3_folder->uids->len;i++,fi++) {
			if (fi[0]->cmd) {

				g_static_rec_mutex_lock (pop3_store->eng_lock);

				if (pop3_store->engine == NULL) {
					g_ptr_array_free(pop3_folder->uids, TRUE);
					g_hash_table_destroy(pop3_folder->uids_uid);
					g_free(fi[0]->uid);
					g_free(fi[0]);
					g_static_rec_mutex_unlock (pop3_store->eng_lock);
					return;
				}

				while (camel_pop3_engine_iterate(pop3_store->engine, fi[0]->cmd) > 0)
					;
				camel_pop3_engine_command_free(pop3_store->engine, fi[0]->cmd);

				g_static_rec_mutex_unlock (pop3_store->eng_lock);
			}
			
			g_free(fi[0]->uid);
			g_free(fi[0]);
		}
		
		g_ptr_array_free(pop3_folder->uids, TRUE);
		g_hash_table_destroy(pop3_folder->uids_uid);
	}
}

static void
pop3_finalize (CamelObject *object)
{
	CamelPOP3Folder *pop3_folder = CAMEL_POP3_FOLDER (object);

	destroy_lists (pop3_folder);

}

static void
camel_pop3_summary_set_extra_flags (CamelFolder *folder, CamelMessageInfoBase *mi)
{
	CamelPOP3Store *pop3_store = (CamelPOP3Store *)folder->parent_store;
	camel_data_cache_set_flags (pop3_store->cache, "cache", mi);
}

CamelFolder *
camel_pop3_folder_new (CamelStore *parent, CamelException *ex)
{
	CamelFolder *folder;
	CamelPOP3Store *p3store = (CamelPOP3Store*) parent;
	gchar *summary_file;
	CamelPOP3Folder *pop3_folder;

	d(printf("opening pop3 INBOX folder\n"));
	
	folder = CAMEL_FOLDER (camel_object_new (CAMEL_POP3_FOLDER_TYPE));
	pop3_folder = CAMEL_POP3_FOLDER (folder);
	pop3_folder->uids = NULL;

	camel_folder_construct (folder, parent, "inbox", "inbox");

	summary_file = g_strdup_printf ("%s/summary.mmap", p3store->storage_path);
	folder->summary = camel_folder_summary_new (folder);
	folder->summary->set_extra_flags_func = camel_pop3_summary_set_extra_flags;
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
	/* camel_folder_refresh_info (folder, ex); 
	if (camel_exception_is_set (ex)) {
		camel_object_unref (CAMEL_OBJECT (folder));
		folder = NULL;
	}*/

	if (!folder->summary) {
		camel_object_unref (CAMEL_OBJECT (folder));
		camel_exception_setv (ex, CAMEL_EXCEPTION_SYSTEM,
				      _("Could not load summary for INBOX"));
		return NULL;
	}
	
	folder->folder_flags |= CAMEL_FOLDER_HAS_SUMMARY_CAPABILITY;

	return folder;
}

static int 
pop3_get_local_size (CamelFolder *folder)
{
	CamelPOP3Store *p3store = CAMEL_POP3_STORE (folder->parent_store);
	int msize = 0;
	camel_du (p3store->storage_path , &msize);
	return msize;
}

/* create a uid from md5 of 'top' output */
static int
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

	return 1;
}

static int
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
				if ((pop3_store->engine && pop3_store->engine->capa & CAMEL_POP3_CAP_UIDL) == 0)
					fi->cmd = camel_pop3_engine_command_new(pe, CAMEL_POP3_COMMAND_MULTI, cmd_builduid, fi, "TOP %u 0\r\n", id);
				g_ptr_array_add(((CamelPOP3Folder *)folder)->uids, fi);
				g_hash_table_insert(((CamelPOP3Folder *)folder)->uids_id, GINT_TO_POINTER(id), fi);
			}
		}
	} while (ret>0);

	return 1;
}

static int
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
					camel_operation_progress(NULL, (fi->index+1) , folder->uids->len);
					fi->uid = g_strdup(uid);
					g_hash_table_insert(folder->uids_uid, fi->uid, fi);
				} else {
					g_warning("ID %u (uid: %s) not in previous LIST output", id, uid);
				}
			}
		}
	} while (ret>0);

	return 1;
}

static void 
pop3_refresh_info (CamelFolder *folder, CamelException *ex)
{
	CamelPOP3Store *pop3_store = CAMEL_POP3_STORE (folder->parent_store);
	CamelPOP3Folder *pop3_folder = (CamelPOP3Folder *) folder;
	CamelPOP3Command *pcl, *pcu = NULL;
	int i, hcnt = 0;
	CamelException dex = CAMEL_EXCEPTION_INITIALISER;

	if (camel_disco_store_status (CAMEL_DISCO_STORE (pop3_store)) == CAMEL_DISCO_STORE_OFFLINE)
		return;

	g_static_rec_mutex_lock (pop3_store->eng_lock);
	pop3_store->is_refreshing = TRUE;
	if (pop3_store->engine == NULL)
	{
		camel_service_connect (CAMEL_SERVICE (pop3_store), ex);
		if (camel_exception_is_set (ex)) {
			pop3_store->is_refreshing = FALSE;
			g_static_rec_mutex_unlock (pop3_store->eng_lock);
			return;
		}
	}
	g_static_rec_mutex_unlock (pop3_store->eng_lock);

	destroy_lists (pop3_folder);

	pop3_folder->uids = g_ptr_array_new ();
	pop3_folder->uids_uid = g_hash_table_new(g_str_hash, g_str_equal);
	/* only used during setup */
	pop3_folder->uids_id = g_hash_table_new(NULL, NULL);

	camel_operation_start (NULL, _("Fetching summary information for new messages in folder"));


	g_static_rec_mutex_lock (pop3_store->eng_lock);

	if (pop3_store->engine == NULL) {
		pop3_store->is_refreshing = FALSE;
		g_static_rec_mutex_unlock (pop3_store->eng_lock);
		goto mfail;
	}

	pcl = camel_pop3_engine_command_new(pop3_store->engine, CAMEL_POP3_COMMAND_MULTI, cmd_list, folder, "LIST\r\n");
	if (pop3_store->engine->capa & CAMEL_POP3_CAP_UIDL)
		pcu = camel_pop3_engine_command_new(pop3_store->engine, CAMEL_POP3_COMMAND_MULTI, cmd_uidl, folder, "UIDL\r\n");
	while ((i = camel_pop3_engine_iterate(pop3_store->engine, NULL)) > 0)
		;

	g_static_rec_mutex_unlock (pop3_store->eng_lock);

	if (i == -1) 
	{
		if (errno == EINTR)
			camel_exception_setv(ex, CAMEL_EXCEPTION_USER_CANCEL, _("User canceled"));
		else
			camel_exception_setv (ex, CAMEL_EXCEPTION_SYSTEM,
					      _("Cannot get POP summary: %s"),
					      g_strerror (errno));
	}

	/* TODO: check every id has a uid & commands returned OK too? */

	g_static_rec_mutex_lock (pop3_store->eng_lock);

	if (pop3_store->engine == NULL) {
		pop3_store->is_refreshing = FALSE;
		g_static_rec_mutex_unlock (pop3_store->eng_lock);
		goto mfail;
	}

	camel_pop3_engine_command_free(pop3_store->engine, pcl);

	g_static_rec_mutex_unlock (pop3_store->eng_lock);

	/* Update the summary.mmap file */

	camel_folder_summary_prepare_hash (folder->summary);

	camel_pop3_logbook_open (pop3_store->book);

	for (i=0;i<pop3_folder->uids->len;i++) 
	{
		CamelPOP3FolderInfo *fi = pop3_folder->uids->pdata[i];
		CamelMessageInfoBase *mi = NULL;
		CamelFolderChangeInfo *changes;

		mi = (CamelMessageInfoBase*) camel_folder_summary_uid (folder->summary, fi->uid);

		if (!mi && !camel_pop3_logbook_is_registered (pop3_store->book, fi->uid))
		{
			CamelMimeMessage *msg = NULL;

			g_static_rec_mutex_lock (pop3_store->eng_lock);

			if (pop3_store->engine == NULL) {
				g_static_rec_mutex_unlock (pop3_store->eng_lock);
				continue;
			}

			if (pop3_store->engine && pop3_store->engine->capa & CAMEL_POP3_CAP_TOP) 
				msg = pop3_get_top (folder, fi->uid, NULL);
			else if (pop3_store->engine)
				msg = pop3_get_message (folder, fi->uid, CAMEL_FOLDER_RECEIVE_PARTIAL, -1, NULL);

			g_static_rec_mutex_unlock (pop3_store->eng_lock);

			if (msg) 
			{
				mi = (CamelMessageInfoBase*) camel_folder_summary_uid (folder->summary, fi->uid);
				if (mi) {
				    mi->size = (fi->size);
				   /* TNY TODO: This is a hack! But else we need to parse 
				    * BODYSTRUCTURE (and I'm lazy). It needs fixing though. */
				    if (mi->size > 102400)
					mi->flags |= CAMEL_MESSAGE_ATTACHMENTS;
				    /* ... it does */
				    camel_message_info_free (mi);
				}

				camel_object_unref (CAMEL_OBJECT (msg));

				hcnt++;
				if (hcnt > 1000)
				{
					/* Periodically save the summary (this reduces 
					   memory usage too) */
					camel_folder_summary_save (folder->summary);
					hcnt = 0;
				}

				changes = camel_folder_change_info_new ();
				camel_folder_change_info_add_uid (changes, fi->uid);
				if (camel_folder_change_info_changed (changes))
					camel_object_trigger_event (CAMEL_OBJECT (folder), "folder_changed", changes);
				camel_folder_change_info_free (changes);

				camel_pop3_logbook_register (pop3_store->book, fi->uid);

			} if (!pop3_store->engine)
				break;

		} else if (mi)
			camel_message_info_free (mi);

		camel_operation_progress (NULL, i , pop3_folder->uids->len);

	}

	camel_pop3_logbook_close (pop3_store->book);

	camel_folder_summary_save (folder->summary);

	camel_folder_summary_kill_hash (folder->summary);

	g_static_rec_mutex_lock (pop3_store->eng_lock);

	if (pop3_store->engine == NULL) {
		pop3_store->is_refreshing = FALSE;
		g_static_rec_mutex_unlock (pop3_store->eng_lock);
		goto mfail;
	}

	if (pop3_store->engine)
	{
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
	}

	g_static_rec_mutex_unlock (pop3_store->eng_lock);


mfail:

	pop3_store->is_refreshing = FALSE;

	/* dont need this anymore */
	g_hash_table_destroy(pop3_folder->uids_id);

	camel_operation_end (NULL);

	camel_service_disconnect (CAMEL_SERVICE (pop3_store), TRUE, &dex);

	return;
}


static void
pop3_sync (CamelFolder *folder, gboolean expunge, CamelException *ex)
{
	CamelPOP3Folder *pop3_folder;
	CamelPOP3Store *pop3_store;
	int i, max; CamelPOP3FolderInfo *fi;
	CamelMessageInfoBase *info;
	CamelException dex = CAMEL_EXCEPTION_INITIALISER;
	GList *deleted = NULL;
	CamelFolderChangeInfo *changes = NULL;

	pop3_folder = CAMEL_POP3_FOLDER (folder);
	pop3_store = CAMEL_POP3_STORE (folder->parent_store);

	if (!pop3_store->delete_after && !expunge)
		return;

	if (camel_disco_store_status (CAMEL_DISCO_STORE (pop3_store)) == CAMEL_DISCO_STORE_OFFLINE)
		return;

	g_static_rec_mutex_lock (pop3_store->eng_lock);
	if (pop3_store->engine == NULL)
	{
		camel_service_connect (CAMEL_SERVICE (pop3_store), ex);
		if (camel_exception_is_set (ex)) {
			g_static_rec_mutex_unlock (pop3_store->eng_lock);
			return;
		}
	}

	if(pop3_store->delete_after && !expunge)
	{
		camel_operation_start(NULL, _("Expunging old messages"));
		camel_pop3_delete_old(folder, pop3_store->delete_after,ex);
		camel_operation_end (NULL);
	}

	if (!expunge) {
		camel_service_disconnect (CAMEL_SERVICE (pop3_store), TRUE, &dex);
		g_static_rec_mutex_unlock (pop3_store->eng_lock);
		return;
	}

	g_static_rec_mutex_unlock (pop3_store->eng_lock);

	camel_operation_start(NULL, _("Expunging deleted messages"));

	max = camel_folder_summary_count (folder->summary);
	for (i = 0; i < max; i++) 
	{
		gchar *expunged_path = NULL;

		info = (CamelMessageInfoBase*) camel_folder_summary_index (folder->summary, i);

		if (!info)
			continue;

		expunged_path = g_strdup_printf ("%s/%s.expunged", pop3_store->storage_path, info->uid);

		if ((info->flags & CAMEL_MESSAGE_DELETED)&& g_file_test (expunged_path, G_FILE_TEST_EXISTS)) 
		{
			struct _CamelPOP3Command *cmd;

			g_static_rec_mutex_lock (pop3_store->eng_lock);

			if (pop3_store->engine == NULL) {
				g_static_rec_mutex_unlock (pop3_store->eng_lock);
				return;
			}

			g_unlink (expunged_path);
			cmd = camel_pop3_engine_command_new(pop3_store->engine, 0, NULL, NULL, "DELE %u\r\n", info->uid);
			while (camel_pop3_engine_iterate(pop3_store->engine, cmd) > 0);
			if (pop3_store->cache && info->uid)
				camel_data_cache_remove(pop3_store->cache, "cache", info->uid, NULL);
			camel_pop3_engine_command_free(pop3_store->engine, cmd);

			g_static_rec_mutex_unlock (pop3_store->eng_lock);

		}

		if (info->flags & CAMEL_MESSAGE_DELETED) {
			camel_message_info_ref (info);
			deleted = g_list_prepend (deleted, info);
		} else {
			gboolean found = FALSE;

			for (i=0; i < pop3_folder->uids->len; i++)
			{
				CamelPOP3FolderInfo *fi = pop3_folder->uids->pdata[i];
				if (!strcmp (fi->uid, info->uid)) {
					found = TRUE;
					break;
				}
			}

			if (!found) {
				/* Removed remotely (only if not cached locally) */
				CamelObject *o = (CamelObject *) camel_data_cache_get (pop3_store->cache, "cache", fi->uid, NULL);

				if (!o) {
					camel_message_info_ref (info);
					deleted = g_list_prepend (deleted, info);
				} else 
					camel_object_unref (o);
			}
		}

		camel_message_info_free((CamelMessageInfo *)info);

		g_free (expunged_path);
	}

	while (deleted)
	{
		CamelMessageInfo *info = deleted->data;

		if (!changes)
			changes = camel_folder_change_info_new ();
		camel_folder_change_info_remove_uid (changes, info->uid);
		camel_folder_summary_remove (folder->summary, info);
		camel_message_info_free(info);
		deleted = g_list_next (deleted);
	}


	if (changes)
	{
		camel_object_trigger_event (CAMEL_OBJECT (folder), "folder_changed", changes);
		camel_folder_change_info_free (changes);
	}

	camel_operation_end(NULL);

	camel_folder_summary_save (folder->summary);

	camel_service_disconnect (CAMEL_SERVICE (pop3_store), TRUE, &dex);

	return;
}


int
camel_pop3_delete_old(CamelFolder *folder, int days_to_delete, CamelException *ex)
{
	CamelPOP3Folder *pop3_folder;
	CamelPOP3FolderInfo *fi;
	int i;
	CamelPOP3Store *pop3_store;
	time_t temp;

	pop3_folder = CAMEL_POP3_FOLDER (folder);
	pop3_store = CAMEL_POP3_STORE (CAMEL_FOLDER(pop3_folder)->parent_store);
	temp = time(&temp);

	if (camel_disco_store_status (CAMEL_DISCO_STORE (pop3_store)) == CAMEL_DISCO_STORE_OFFLINE)
		return;

	g_static_rec_mutex_lock (pop3_store->eng_lock);
	if (pop3_store->engine == NULL)
	{
		camel_service_connect (CAMEL_SERVICE (pop3_store), ex);
		if (camel_exception_is_set (ex)) {
			g_static_rec_mutex_unlock (pop3_store->eng_lock);
			return;
		}
	}

	for (i = 0; i < pop3_folder->uids->len; i++) 
	{
		CamelMimeMessage *message = NULL;

		fi = pop3_folder->uids->pdata[i];

		if (pop3_store->cache && fi->uid && !camel_data_cache_is_partial(pop3_store->cache, "cache", fi->uid))
			message = pop3_get_message (folder, fi->uid, CAMEL_FOLDER_RECEIVE_FULL, -1, ex);
		else if (fi->uid)
			message = pop3_get_message (folder, fi->uid, CAMEL_FOLDER_RECEIVE_PARTIAL, -1, ex);

		if (message) 
		{
		    time_t message_time = message->date + message->date_offset;
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
		camel_operation_progress(NULL, (i+1) , pop3_folder->uids->len);
	}

	camel_operation_end(NULL);

	/* camel_pop3_store_expunge (pop3_store, ex); */

	g_static_rec_mutex_unlock (pop3_store->eng_lock);

	return 0;

}

static int
cmd_tocache(CamelPOP3Engine *pe, CamelPOP3Stream *stream, void *data)
{
	CamelPOP3Store *tstore = (CamelPOP3Store *) pe->store;
	CamelPOP3FolderInfo *fi = data;
	char buffer[2048];
	int w = 0, n;
	int retval = 1;

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
			camel_operation_progress(NULL, w , fi->size);
	}

	/* it all worked, output a '#' to say we're a-ok */
	if (n != -1) {
		camel_stream_reset(fi->stream);
		n = camel_stream_write(fi->stream, "#", 1);

		camel_data_cache_set_partial (tstore->cache, "cache", fi->uid, FALSE);
	}
done:
	if (n == -1) {
		fi->err = errno;
		retval = 0;
		g_warning("POP3 retrieval failed: %s", strerror(errno));
	} else {
		fi->err = 0;
	}
	
	camel_object_unref((CamelObject *)fi->stream);
	fi->stream = NULL;

	return retval;
}


static int
cmd_tocache_partial (CamelPOP3Engine *pe, CamelPOP3Stream *stream, void *data)
{
	CamelPOP3Store *tstore = (CamelPOP3Store *) pe->store;
	CamelPOP3FolderInfo *fi = data;
	unsigned char *buffer;
	int w = 0, n;
	gchar *boundary = NULL;
	gboolean occurred = FALSE, theend = FALSE;
	unsigned int len;
	int retval = 1;

	/* We write an '*' to the start of the stream to say its not complete yet */
	if ((n = camel_stream_write(fi->stream, "*", 1)) == -1)
		goto done;


	while (!theend && camel_pop3_stream_line (stream, &buffer, &len) > 0)
	{
		if (!buffer)
			continue;

		if (boundary == NULL)
		{
			   CamelContentType *ct = NULL;
			   const char *bound=NULL;
			   char *pstr = (char*)strcasestr ((const char *) buffer, "Content-Type:");

			   if (pstr) 
			   {
				pstr = strchr (pstr, ':'); 
				if (pstr) { pstr++;
				ct = camel_content_type_decode(pstr); } 
			   }

			   if (ct) 
			   { 
				bound = camel_content_type_param(ct, "boundary");
				if (bound && strlen (bound) > 0) 
					boundary = g_strdup (bound);
			   }
		} else if (strstr ((const char*) buffer, (const char*) boundary))
		{
			if (occurred)
			{
				CamelException myex = CAMEL_EXCEPTION_INITIALISER;
				camel_service_disconnect (CAMEL_SERVICE (tstore), FALSE, &myex);
				camel_service_connect (CAMEL_SERVICE (tstore), &myex);
				pe->partial_happening = TRUE;
				theend = TRUE;
			} 

			occurred = TRUE;
		}

		if (!theend)
		{
		    n = camel_stream_write(fi->stream, (const char*) buffer, len);
		    if (n == -1 || camel_stream_write(fi->stream, "\n", 1) == -1)
			break;
		    w += n+1;
		} else if (boundary != NULL)
		{
		    gchar *nb = g_strdup_printf ("\n--%s\n", boundary);
		    n = camel_stream_write(fi->stream, nb, strlen (nb));
		    g_free (nb);
		}

		if (w > fi->size)
			w = fi->size;
		if (fi->size != 0)
			camel_operation_progress(NULL, w , fi->size);
	}

	/* it all worked, output a '#' to say we're a-ok */
	if (n != -1 || theend) {
		camel_stream_reset(fi->stream);
		n = camel_stream_write(fi->stream, "#", 1);
		if (theend)
			camel_data_cache_set_partial (tstore->cache, "cache", fi->uid, TRUE);
		else
 			camel_data_cache_set_partial (tstore->cache, "cache", fi->uid, FALSE);

	}
done:
	if (n == -1 && !theend) {
		fi->err = errno;
		retval = 0;
		g_warning("POP3 retrieval failed: %s", strerror(errno));
	} else {
		fi->err = 0;
	}
	
	camel_object_unref((CamelObject *)fi->stream);
	fi->stream = NULL;

	if (boundary)
		g_free (boundary);

	return retval;
}


static void
pop3_delete_attachments (CamelFolder *folder, const char *uid)
{
	CamelPOP3Store *pop3_store = CAMEL_POP3_STORE (folder->parent_store);
	camel_data_cache_delete_attachments (pop3_store->cache, "cache", uid);
	return;
}

static CamelMimeMessage *
pop3_get_message (CamelFolder *folder, const char *uid, CamelFolderReceiveType type, gint param, CamelException *ex)
{
	CamelMimeMessage *message = NULL;
	CamelPOP3Store *pop3_store = CAMEL_POP3_STORE (folder->parent_store);
	CamelPOP3Folder *pop3_folder = (CamelPOP3Folder *)folder;
	CamelPOP3Command *pcr=NULL;
	CamelPOP3FolderInfo *fi;
	char buffer[1]; int i;
	CamelStream *stream = NULL;
	CamelFolderSummary *summary = folder->summary;
	CamelMessageInfoBase *mi; gboolean im_certain=FALSE;
	CamelException dex = CAMEL_EXCEPTION_INITIALISER;

	stream = camel_data_cache_get(pop3_store->cache, "cache", uid, NULL);
	if (stream)
	{
		message = camel_mime_message_new ();
		if (camel_data_wrapper_construct_from_stream((CamelDataWrapper *)message, stream) == -1) {
			if (errno == EINTR)
				camel_exception_setv(ex, CAMEL_EXCEPTION_USER_CANCEL, "User canceled");
			else
				camel_exception_setv (ex, CAMEL_EXCEPTION_SYSTEM,
						      "Cannot get message %s: %s",
						      uid, g_strerror (errno));
			if (message)
				camel_object_unref (message);
			message = NULL;
		}

		camel_object_unref (CAMEL_OBJECT (stream));

		return message;
	}

	if (pop3_folder->uids_uid)
		fi = g_hash_table_lookup(pop3_folder->uids_uid, uid);
	else 
		fi = NULL;

	if (fi == NULL) {
		camel_exception_setv (ex, CAMEL_EXCEPTION_FOLDER_INVALID_UID,
				      _("No message with UID %s"), uid);
		return NULL;
	}

	/* Sigh, most of the crap in this function is so that the cancel button
	   returns the proper exception code.  Sigh. */

	if (camel_disco_store_status (CAMEL_DISCO_STORE (pop3_store)) == CAMEL_DISCO_STORE_OFFLINE)
	{
		camel_exception_setv (ex, CAMEL_EXCEPTION_FOLDER_INVALID_UID,
				      _("No message with UID %s and not online"), uid);
		return NULL;
	}

	g_static_rec_mutex_lock (pop3_store->eng_lock);
	if (pop3_store->engine == NULL)
	{
		camel_service_connect (CAMEL_SERVICE (pop3_store), ex);
		if (camel_exception_is_set (ex)) {
			g_static_rec_mutex_unlock (pop3_store->eng_lock);
			return NULL;
		}
	}
	g_static_rec_mutex_unlock (pop3_store->eng_lock);

	camel_operation_start (NULL, _("Retrieving message"));

	/* If we have an oustanding retrieve message running, wait for that to complete
	   & then retrieve from cache, otherwise, start a new one, and similar */

	if (fi->cmd != NULL) {

		g_static_rec_mutex_lock (pop3_store->eng_lock);

		if (pop3_store->engine == NULL) {
			g_static_rec_mutex_unlock (pop3_store->eng_lock);
			goto fail;
		}

		while ((i = camel_pop3_engine_iterate(pop3_store->engine, fi->cmd)) > 0)
			;

		g_static_rec_mutex_unlock (pop3_store->eng_lock);

		if (i == -1)
			fi->err = errno;

		/* getting error code? */
		/* g_assert (fi->cmd->state == CAMEL_POP3_COMMAND_DATA); */

		g_static_rec_mutex_lock (pop3_store->eng_lock);
		if (pop3_store->engine == NULL) {
			g_static_rec_mutex_unlock (pop3_store->eng_lock);
			goto fail;
		}
		camel_pop3_engine_command_free(pop3_store->engine, fi->cmd);
		g_static_rec_mutex_unlock (pop3_store->eng_lock);

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
	
	if (pop3_store->cache != NULL)
	{
		CamelException tex = CAMEL_EXCEPTION_INITIALISER;

		if ((type & CAMEL_FOLDER_RECEIVE_FULL) && camel_data_cache_is_partial (pop3_store->cache, "cache", fi->uid))
		{

			camel_data_cache_remove (pop3_store->cache, "cache", fi->uid, &tex);
			im_certain = TRUE;

		} else if ((type & CAMEL_FOLDER_RECEIVE_PARTIAL) 
			&& !camel_data_cache_is_partial (pop3_store->cache, "cache", fi->uid))
		{
			camel_data_cache_remove (pop3_store->cache, "cache", fi->uid, &tex);
			im_certain = TRUE;
		}
	}

	/* check to see if we have safely written flag set */
	if (im_certain || pop3_store->cache == NULL
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


		g_static_rec_mutex_lock (pop3_store->eng_lock);

		if (pop3_store->engine == NULL) {
			g_static_rec_mutex_unlock (pop3_store->eng_lock);
			goto done;
		}

		pop3_store->engine->type = type;
		pop3_store->engine->param = param;

		if (type & CAMEL_FOLDER_RECEIVE_FULL || type & CAMEL_FOLDER_RECEIVE_ANY_OR_FULL) {

			pcr = camel_pop3_engine_command_new(pop3_store->engine, CAMEL_POP3_COMMAND_MULTI, 
				cmd_tocache, fi, "RETR %u\r\n", fi->id);
		}
		else if (type & CAMEL_FOLDER_RECEIVE_PARTIAL || type & CAMEL_FOLDER_RECEIVE_ANY_OR_PARTIAL) {
			pcr = camel_pop3_engine_command_new(pop3_store->engine, CAMEL_POP3_COMMAND_MULTI, 
				cmd_tocache_partial, fi, "RETR %u\r\n", fi->id);
		}

		while ((i = camel_pop3_engine_iterate(pop3_store->engine, pcr)) > 0)
			;
		if (i == -1)
			fi->err = errno;

		/* getting error code? */
		/*g_assert (pcr->state == CAMEL_POP3_COMMAND_DATA);*/

		camel_pop3_engine_command_free(pop3_store->engine, pcr);

		g_static_rec_mutex_unlock (pop3_store->eng_lock);

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
	} else {
		if (type & CAMEL_FOLDER_RECEIVE_FULL && pop3_store->immediate_delete_after)
		{
			struct _CamelPOP3Command *cmd = NULL;

			g_static_rec_mutex_lock (pop3_store->eng_lock);

			if (pop3_store->engine == NULL) {
				g_static_rec_mutex_unlock (pop3_store->eng_lock);
				goto done;
			}

			cmd = camel_pop3_engine_command_new(pop3_store->engine, 0, NULL, NULL, "DELE %u\r\n", uid);
			while (camel_pop3_engine_iterate(pop3_store->engine, cmd) > 0);
			camel_pop3_engine_command_free(pop3_store->engine, cmd);

			g_static_rec_mutex_unlock (pop3_store->eng_lock);
		}
	}

	mi = (CamelMessageInfoBase *) camel_folder_summary_uid (summary, uid);
	if (!mi && !camel_pop3_logbook_is_registered (pop3_store->book, uid)) {
		mi = (CamelMessageInfoBase *) camel_folder_summary_info_new_from_message (summary, message);
		if (mi->uid)
			g_free (mi->uid);
		mi->uid = g_strdup (uid);
		camel_folder_summary_add (summary, (CamelMessageInfo *)mi);
	} if (mi)
		camel_message_info_free (mi);

done:
	camel_object_unref((CamelObject *)stream);
fail:
	camel_operation_end(NULL);

	g_static_rec_mutex_lock (pop3_store->eng_lock);
	if (pop3_store->is_refreshing == FALSE)
		camel_service_disconnect (CAMEL_SERVICE (pop3_store), TRUE, &dex);
	g_static_rec_mutex_unlock (pop3_store->eng_lock);

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
	CamelPOP3Engine *mengine = NULL;

	if (camel_disco_store_status (CAMEL_DISCO_STORE (pop3_store)) == CAMEL_DISCO_STORE_OFFLINE)
	{
		camel_exception_setv (ex, CAMEL_EXCEPTION_FOLDER_INVALID_UID,
				      _("No message with UID %s and not online"), uid);
		return NULL;
	}

	g_static_rec_mutex_lock (pop3_store->eng_lock);

	if (pop3_store->engine == NULL) {
		camel_service_connect (CAMEL_SERVICE (pop3_store), ex);
		if (camel_exception_is_set (ex)) {
			g_static_rec_mutex_unlock (pop3_store->eng_lock);
			return NULL;
		}
	}

	fi = g_hash_table_lookup(pop3_folder->uids_uid, uid);

	if (fi == NULL) {
		camel_exception_setv (ex, CAMEL_EXCEPTION_FOLDER_INVALID_UID,
				      _("No message with UID %s"), uid);

		g_static_rec_mutex_unlock (pop3_store->eng_lock);

		return NULL;
	}

	g_static_rec_mutex_unlock (pop3_store->eng_lock);

	old = fi->stream;

	/* Sigh, most of the crap in this function is so that the cancel button
	   returns the proper exception code.  Sigh. */

	camel_operation_start_transient(NULL, _("Retrieving POP message's TOP %d"), fi->id);

	/* If we have an oustanding retrieve message running, wait for that to complete
	   & then retrieve from cache, otherwise, start a new one, and similar */

	if (fi->cmd != NULL) {

		g_static_rec_mutex_lock (pop3_store->eng_lock);

		if (pop3_store->engine == NULL) {
			g_static_rec_mutex_unlock (pop3_store->eng_lock);
			goto fail;
		}

		mengine = pop3_store->engine;
		camel_object_ref (mengine);

		g_static_rec_mutex_unlock (pop3_store->eng_lock);

		while ((i = camel_pop3_engine_iterate(mengine, fi->cmd)) > 0)
			;

		if (i == -1)
			fi->err = errno;

		/* getting error code? */
		/* g_assert (fi->cmd->state == CAMEL_POP3_COMMAND_DATA); */

		g_static_rec_mutex_lock (pop3_store->eng_lock);

		camel_object_unref (mengine);

		if (pop3_store->engine == NULL) {
			g_static_rec_mutex_unlock (pop3_store->eng_lock);
			goto fail;
		}

		camel_pop3_engine_command_free(pop3_store->engine, fi->cmd);
		g_static_rec_mutex_unlock (pop3_store->eng_lock);

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


		g_static_rec_mutex_lock (pop3_store->eng_lock);

		if (pop3_store->engine == NULL) {
			g_static_rec_mutex_unlock (pop3_store->eng_lock);
			goto done;
		}

		pcr = camel_pop3_engine_command_new(pop3_store->engine, CAMEL_POP3_COMMAND_MULTI, 
			cmd_tocache, fi, "TOP %u 0\r\n", fi->id);
		while ((i = camel_pop3_engine_iterate(pop3_store->engine, pcr)) > 0)
			;
		if (i == -1)
			fi->err = errno;
		camel_pop3_engine_command_free(pop3_store->engine, pcr);

		g_static_rec_mutex_unlock (pop3_store->eng_lock);


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
	if (!mi && !camel_pop3_logbook_is_registered (pop3_store->book, uid)) {
		mi = (CamelMessageInfoBase *) camel_folder_summary_info_new_from_message (summary, message);
		if (mi->uid)
			g_free (mi->uid);
		mi->uid = g_strdup(uid);
		camel_folder_summary_add (summary, (CamelMessageInfo *)mi);
	} else if (mi)
		camel_message_info_free (mi);

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
	CamelMessageInfo *info;

	fi = g_hash_table_lookup(pop3_folder->uids_uid, uid);
	if (fi) {
		guint32 new = (fi->flags & ~flags) | (set & flags);

		if (fi->flags != new) {
			fi->flags = new;
			res = TRUE;
		}
	}

	info = camel_folder_summary_uid(folder->summary, uid);
	if (info == NULL)
		return FALSE;

	res = camel_message_info_set_flags(info, flags, set);
	camel_message_info_free(info);

	return res;
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


static void
pop3_sync_offline (CamelFolder *folder, CamelException *ex)
{
	camel_folder_summary_save (folder->summary);
}

static void
pop3_sync_online (CamelFolder *folder, CamelException *ex)
{
	camel_exception_set (ex, CAMEL_EXCEPTION_SYSTEM, _("Not supported"));

	pop3_sync_offline (folder, ex);

	return;
}

static void
pop3_expunge_uids_online (CamelFolder *folder, GPtrArray *uids, CamelException *ex)
{
	camel_exception_set (ex, CAMEL_EXCEPTION_SYSTEM, _("Not supported"));

	return;
}


static int
uid_compar (const void *va, const void *vb)
{
	const char **sa = (const char **)va, **sb = (const char **)vb;
	unsigned long a, b;

	a = strtoul (*sa, NULL, 10);
	b = strtoul (*sb, NULL, 10);
	if (a < b)
		return -1;
	else if (a == b)
		return 0;
	else
		return 1;
}

static void
pop3_expunge_uids_offline (CamelFolder *folder, GPtrArray *uids, CamelException *ex)
{
	CamelFolderChangeInfo *changes;
	int i;
	
	qsort (uids->pdata, uids->len, sizeof (void *), uid_compar);
	
	changes = camel_folder_change_info_new ();
	
	for (i = 0; i < uids->len; i++) {
		camel_folder_summary_remove_uid (folder->summary, uids->pdata[i]);
		camel_folder_change_info_remove_uid (changes, uids->pdata[i]);
		/* We intentionally don't remove it from the cache because
		 * the cached data may be useful in replaying a COPY later. */
	}
	camel_folder_summary_save (folder->summary);

	camel_disco_diary_log (CAMEL_DISCO_STORE (folder->parent_store)->diary,
			       CAMEL_DISCO_DIARY_FOLDER_EXPUNGE, folder, uids);

	camel_object_trigger_event (CAMEL_OBJECT (folder), "folder_changed", changes);
	camel_folder_change_info_free (changes);

	return;
}

static void
pop3_expunge_uids_resyncing (CamelFolder *folder, GPtrArray *uids, CamelException *ex)
{
	camel_exception_set (ex, CAMEL_EXCEPTION_SYSTEM, _("Not supported"));

	return;
}


static void
pop3_append_offline (CamelFolder *folder, CamelMimeMessage *message,
		     const CamelMessageInfo *info, char **appended_uid,
		     CamelException *ex)
{
	camel_exception_set (ex, CAMEL_EXCEPTION_SYSTEM, _("Not supported"));

	return;
}

static void
pop3_transfer_offline (CamelFolder *source, GPtrArray *uids,
		       CamelFolder *dest, GPtrArray **transferred_uids,
		       gboolean delete_originals, CamelException *ex)
{
	camel_exception_set (ex, CAMEL_EXCEPTION_SYSTEM, _("Not supported"));
	return;
}

static void
pop3_cache_message (CamelDiscoFolder *disco_folder, const char *uid,
		    CamelException *ex)
{
	CamelMimeMessage *msg = pop3_get_message (CAMEL_FOLDER (disco_folder), uid,
		 CAMEL_FOLDER_RECEIVE_FULL, -1, ex);

	if (msg) 
		camel_object_unref (CAMEL_OBJECT (msg));

}

static void
camel_pop3_folder_class_init (CamelPOP3FolderClass *camel_pop3_folder_class)
{
	CamelFolderClass *camel_folder_class = CAMEL_FOLDER_CLASS(camel_pop3_folder_class);
	CamelDiscoFolderClass *camel_disco_folder_class = CAMEL_DISCO_FOLDER_CLASS (camel_pop3_folder_class);

	disco_folder_class = CAMEL_DISCO_FOLDER_CLASS (camel_type_get_global_classfuncs (camel_disco_folder_get_type ()));

	parent_class = CAMEL_FOLDER_CLASS(camel_folder_get_type());
	
	/* virtual method overload */
	camel_folder_class->get_local_size = pop3_get_local_size;
	camel_folder_class->refresh_info = pop3_refresh_info;
	camel_folder_class->sync = pop3_sync;
	
	camel_folder_class->get_uids = pop3_get_uids;
	camel_folder_class->free_uids = camel_folder_free_shallow;
	camel_folder_class->get_message = pop3_get_message;
	camel_folder_class->set_message_flags = pop3_set_message_flags;
	camel_folder_class->delete_attachments = pop3_delete_attachments;

	camel_disco_folder_class->refresh_info_online = pop3_refresh_info;
	camel_disco_folder_class->sync_online = pop3_sync_online;
	camel_disco_folder_class->sync_offline = pop3_sync_offline;

	camel_disco_folder_class->sync_resyncing = pop3_sync_offline;

	camel_disco_folder_class->expunge_uids_online = pop3_expunge_uids_online;
	camel_disco_folder_class->expunge_uids_offline = pop3_expunge_uids_offline;
	camel_disco_folder_class->expunge_uids_resyncing = pop3_expunge_uids_resyncing;

	camel_disco_folder_class->append_online = pop3_append_offline;
	camel_disco_folder_class->append_offline = pop3_append_offline;
	camel_disco_folder_class->append_resyncing = pop3_append_offline;

	camel_disco_folder_class->transfer_online = pop3_transfer_offline;
	camel_disco_folder_class->transfer_offline = pop3_transfer_offline;
	camel_disco_folder_class->transfer_resyncing = pop3_transfer_offline;

	camel_disco_folder_class->cache_message = pop3_cache_message;
}

CamelType
camel_pop3_folder_get_type (void)
{
	static CamelType camel_pop3_folder_type = CAMEL_INVALID_TYPE;
	
	if (!camel_pop3_folder_type) {
		camel_pop3_folder_type = camel_type_register (CAMEL_DISCO_FOLDER_TYPE, "CamelPOP3Folder",
							      sizeof (CamelPOP3Folder),
							      sizeof (CamelPOP3FolderClass),
							      (CamelObjectClassInitFunc) camel_pop3_folder_class_init,
							      NULL,
							      NULL,
							      (CamelObjectFinalizeFunc) pop3_finalize);
	}
	
	return camel_pop3_folder_type;
}
