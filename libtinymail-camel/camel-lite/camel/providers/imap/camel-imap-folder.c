/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* camel-imap-folder.c: class for an imap folder
 * 
 * This is the mmap version of camel-imap-folder.c which has a memory
 * consumption reduced imap_update_summary implementation that will
 * periodically instruct the mmap CamelFolderSummary instance to sync
 * headers to disk.
 *
 * Authors:
 *   Dan Winship <danw@ximian.com>
 *   Jeffrey Stedfast <fejj@ximian.com> 
 *   Philip Van Hoof <pvanhoof@gnome.org>
 *
 * Copyright (C) 2000, 2001 Ximian, Inc.
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

/* BODY always returns "textual data", which means a series of characters 
   no containing NUL, CR, or LF, of length <= 1000.*/

#define MAX_LINE_LEN 1024 

#include <config.h> 

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>

#include <string.h>

/*#include "libedataserver/e-path.h"*/
#include "libedataserver/e-time-utils.h"
#include "libedataserver/e-data-server-util.h"

#include "camel-imap-folder.h"
#include "camel-imap-command.h"
#include "camel-imap-message-cache.h"
#include "camel-imap-private.h"
#include "camel-imap-search.h"
#include "camel-imap-store.h"
#include "camel-imap-summary.h"
#include "camel-imap-utils.h"
#include "camel-imap-wrapper.h"
#include "camel-data-wrapper.h"
#include "camel-disco-diary.h"
#include "camel-exception.h"
#include "camel-mime-filter-crlf.h"
#include "camel-mime-filter-from.h"
#include "camel-mime-message.h"
#include "camel-mime-utils.h"
#include "camel-multipart.h"
#include "camel-multipart-signed.h"
#include "camel-multipart-encrypted.h"
#include "camel-operation.h"
#include "camel-session.h"
#include "camel-stream-buffer.h"
#include "camel-stream-filter.h"
#include "camel-stream-mem.h"
#include "camel-stream.h"
#include "camel-private.h"
#include "camel-string-utils.h"
#include "camel-file-utils.h"
#include "camel-debug.h"
#include "camel-i18n.h"
#include "camel/camel-store-summary.h"
#include <camel/camel-tcp-stream.h>

#define d(x) 

/* set to -1 for infinite size (suggested max command-line length is
 * 1000 octets (see rfc2683), so we should keep the uid-set length to
 * something under that so that our command-lines don't exceed 1000
 * octets) */
#define UID_SET_LIMIT  (768)


#define CF_CLASS(o) (CAMEL_FOLDER_CLASS (CAMEL_OBJECT_GET_CLASS(o)))
static CamelDiscoFolderClass *disco_folder_class = NULL;

static void imap_finalize (CamelObject *object);
static int imap_getv(CamelObject *object, CamelException *ex, CamelArgGetV *args);

static gboolean imap_rescan_condstore (CamelFolder *folder, int exists, const char *highestmodseq, CamelException *ex);
static gboolean imap_rescan (CamelFolder *folder, int exists, CamelException *ex);
static void imap_refresh_info (CamelFolder *folder, CamelException *ex);
static void imap_sync_online (CamelFolder *folder, CamelException *ex);
static void imap_sync_offline (CamelFolder *folder, CamelException *ex);
static void imap_expunge_uids_online (CamelFolder *folder, GPtrArray *uids, CamelException *ex);
static void imap_expunge_uids_offline (CamelFolder *folder, GPtrArray *uids, CamelException *ex);
static void imap_expunge_uids_resyncing (CamelFolder *folder, GPtrArray *uids, CamelException *ex);
static void imap_cache_message (CamelDiscoFolder *disco_folder, const char *uid, CamelException *ex);
static void imap_rename (CamelFolder *folder, const char *new);
static void imap_set_push_email (CamelFolder *folder, gboolean setting);

/* message manipulation */
static CamelMimeMessage *imap_get_message (CamelFolder *folder, const gchar *uid,
					   CamelFolderReceiveType type, gint param, CamelException *ex);
static void imap_append_online (CamelFolder *folder, CamelMimeMessage *message,
				const CamelMessageInfo *info, char **appended_uid,
				CamelException *ex);
static void imap_append_offline (CamelFolder *folder, CamelMimeMessage *message,
				 const CamelMessageInfo *info, char **appended_uid,
				 CamelException *ex);
static void imap_append_resyncing (CamelFolder *folder, CamelMimeMessage *message,
				   const CamelMessageInfo *info, char **appended_uid,
				   CamelException *ex);

static void imap_transfer_online (CamelFolder *source, GPtrArray *uids,
				  CamelFolder *dest, GPtrArray **transferred_uids,
				  gboolean delete_originals,
				  CamelException *ex);
static void imap_transfer_offline (CamelFolder *source, GPtrArray *uids,
				   CamelFolder *dest, GPtrArray **transferred_uids,
				   gboolean delete_originals,
				   CamelException *ex);
static void imap_transfer_resyncing (CamelFolder *source, GPtrArray *uids,
				     CamelFolder *dest, GPtrArray **transferred_uids,
				     gboolean delete_originals,
				     CamelException *ex);

/* searching */
static GPtrArray *imap_search_by_expression (CamelFolder *folder, const char *expression, CamelException *ex);
static GPtrArray *imap_search_by_uids	    (CamelFolder *folder, const char *expression, GPtrArray *uids, CamelException *ex);
static void       imap_search_free          (CamelFolder *folder, GPtrArray *uids);

static void imap_thaw (CamelFolder *folder);

static CamelObjectClass *parent_class;

static GData *parse_fetch_response (CamelImapFolder *imap_folder, char *msg_att);
static void camel_imap_folder_changed_for_idle (CamelFolder *folder, int exists,
			   GArray *expunged, CamelException *ex);

GPtrArray* _camel_imap_store_get_recent_messages (CamelImapStore *imap_store, const char *folder_name, int *messages, int *unseen, gboolean withthem);

#ifdef G_OS_WIN32
/* The strtok() in Microsoft's C library is MT-safe (but still uses
 * only one buffer pointer per thread, but for the use of strtok_r()
 * here that's enough).
 */
#define strtok_r(s,sep,lasts) (*(lasts)=strtok((s),(sep)))
#endif



static void
camel_imap_folder_class_init (CamelImapFolderClass *camel_imap_folder_class)
{
	CamelFolderClass *camel_folder_class = CAMEL_FOLDER_CLASS (camel_imap_folder_class);
	CamelDiscoFolderClass *camel_disco_folder_class = CAMEL_DISCO_FOLDER_CLASS (camel_imap_folder_class);

	disco_folder_class = CAMEL_DISCO_FOLDER_CLASS (camel_type_get_global_classfuncs (camel_disco_folder_get_type ()));

	/* virtual method overload */
	((CamelObjectClass *)camel_imap_folder_class)->getv = imap_getv;

	camel_folder_class->set_push_email = imap_set_push_email;
	camel_folder_class->get_message = imap_get_message;
	camel_folder_class->rename = imap_rename;
	camel_folder_class->search_by_expression = imap_search_by_expression;
	camel_folder_class->search_by_uids = imap_search_by_uids;
	camel_folder_class->search_free = imap_search_free;
	camel_folder_class->thaw = imap_thaw;

	camel_disco_folder_class->refresh_info_online = imap_refresh_info;
	camel_disco_folder_class->sync_online = imap_sync_online;
	camel_disco_folder_class->sync_offline = imap_sync_offline;
	/* We don't sync flags at resync time: the online code will
	 * deal with it eventually.
	 */
	camel_disco_folder_class->sync_resyncing = imap_sync_offline;
	camel_disco_folder_class->expunge_uids_online = imap_expunge_uids_online;
	camel_disco_folder_class->expunge_uids_offline = imap_expunge_uids_offline;
	camel_disco_folder_class->expunge_uids_resyncing = imap_expunge_uids_resyncing;
	camel_disco_folder_class->append_online = imap_append_online;
	camel_disco_folder_class->append_offline = imap_append_offline;
	camel_disco_folder_class->append_resyncing = imap_append_resyncing;
	camel_disco_folder_class->transfer_online = imap_transfer_online;
	camel_disco_folder_class->transfer_offline = imap_transfer_offline;
	camel_disco_folder_class->transfer_resyncing = imap_transfer_resyncing;
	camel_disco_folder_class->cache_message = imap_cache_message;
}


static void
camel_imap_folder_init (gpointer object, gpointer klass)
{
	CamelImapFolder *imap_folder = CAMEL_IMAP_FOLDER (object);
	CamelFolder *folder = CAMEL_FOLDER (object);

	/* ((CamelObject *)folder)-> flags |= CAMEL_OBJECT_REF_DEBUG; */

	imap_folder->idle_lock = g_new0 (GStaticRecMutex, 1);
	g_static_rec_mutex_init (imap_folder->idle_lock);
	imap_folder->stopping = FALSE;
	imap_folder->in_idle = FALSE;

	imap_folder->do_push_email = TRUE;
	folder->permanent_flags = CAMEL_MESSAGE_ANSWERED | CAMEL_MESSAGE_DELETED |
		CAMEL_MESSAGE_DRAFT | CAMEL_MESSAGE_FLAGGED | CAMEL_MESSAGE_SEEN;
	
	folder->folder_flags |= (CAMEL_FOLDER_HAS_SUMMARY_CAPABILITY |
				 CAMEL_FOLDER_HAS_SEARCH_CAPABILITY);
	
	imap_folder->priv = g_malloc0(sizeof(*imap_folder->priv));
#ifdef ENABLE_THREADS
	g_static_mutex_init(&imap_folder->priv->search_lock);
	g_static_rec_mutex_init(&imap_folder->priv->cache_lock);
#endif

	imap_folder->need_rescan = TRUE;
}

CamelType
camel_imap_folder_get_type (void)
{
	static CamelType camel_imap_folder_type = CAMEL_INVALID_TYPE;
	
	if (camel_imap_folder_type == CAMEL_INVALID_TYPE) {
		parent_class = camel_disco_folder_get_type();
		camel_imap_folder_type =
			camel_type_register (parent_class, "CamelImapFolder",
					     sizeof (CamelImapFolder),
					     sizeof (CamelImapFolderClass),
					     (CamelObjectClassInitFunc) camel_imap_folder_class_init,
					     NULL,
					     (CamelObjectInitFunc) camel_imap_folder_init,
					     (CamelObjectFinalizeFunc) imap_finalize);
	}
	
	return camel_imap_folder_type;
}

CamelFolder *
camel_imap_folder_new (CamelStore *parent, const char *folder_name,
		       const char *folder_dir, CamelException *ex)
{
	CamelImapStore *imap_store = CAMEL_IMAP_STORE (parent);
	CamelFolder *folder;
	CamelImapFolder *imap_folder;
	const char *short_name;
	char *summary_file, *state_file;

	if (e_util_mkdir_hier (folder_dir, S_IRWXU) != 0) {
		camel_exception_setv (ex, CAMEL_EXCEPTION_SYSTEM,
				      _("Could not create directory %s: %s"),
				      folder_dir, g_strerror (errno));
		return NULL;
	}

	folder = CAMEL_FOLDER (camel_object_new (camel_imap_folder_get_type ()));
	imap_folder = CAMEL_IMAP_FOLDER (folder);

	imap_folder->folder_dir = g_strdup (folder_dir);

	short_name = strrchr (folder_name, '/');
	if (short_name)
		short_name++;
	else
		short_name = folder_name;
	camel_folder_construct (folder, parent, folder_name, short_name);

	summary_file = g_strdup_printf ("%s/summary.mmap", folder_dir);
	folder->summary = camel_imap_summary_new (folder, summary_file);
	g_free (summary_file);
	if (!folder->summary) {
		camel_object_unref (CAMEL_OBJECT (folder));
		camel_exception_setv (ex, CAMEL_EXCEPTION_SYSTEM,
				      _("Could not load summary for %s"),
				      folder_name);
		return NULL;
	}

	/* set/load persistent state */
	state_file = g_strdup_printf ("%s/cmeta", folder_dir);
	camel_object_set(folder, NULL, CAMEL_OBJECT_STATE_FILE, state_file, NULL);
	g_free(state_file);
	camel_object_state_read(folder);

	imap_folder->cache = camel_imap_message_cache_new (folder_dir, folder->summary, ex);

	if (!imap_folder->cache) {
		camel_object_unref (CAMEL_OBJECT (folder));
		return NULL;
	}

	if (!g_ascii_strcasecmp (folder_name, "INBOX")) {
		if ((imap_store->parameters & IMAP_PARAM_FILTER_INBOX))
			folder->folder_flags |= CAMEL_FOLDER_FILTER_RECENT;
		if ((imap_store->parameters & IMAP_PARAM_FILTER_JUNK))
			folder->folder_flags |= CAMEL_FOLDER_FILTER_JUNK;
	} else {
		if ((imap_store->parameters & (IMAP_PARAM_FILTER_JUNK|IMAP_PARAM_FILTER_JUNK_INBOX)) == (IMAP_PARAM_FILTER_JUNK))
			folder->folder_flags |= CAMEL_FOLDER_FILTER_JUNK;
	}

	if (imap_store->capabilities & IMAP_CAPABILITY_IDLE)
		folder->folder_flags |= CAMEL_FOLDER_HAS_PUSHEMAIL_CAPABILITY;

	imap_folder->search = camel_imap_search_new(folder_dir);

	return folder;
}


static void 
put_highestmodseq (CamelImapFolder *imap_folder, const char *highestmodseq)
{
	char *filename = g_strdup_printf ("%s/highestmodseq", imap_folder->folder_dir);
	FILE *file;
	
	file = fopen (filename, "w");
	g_free (filename);

	if (file != NULL)
	{
		fprintf (file, "%s", highestmodseq);
		fclose (file);
	}
}

static char* 
get_highestmodseq (CamelImapFolder *imap_folder)
{
	char *filename = g_strdup_printf ("%s/highestmodseq", imap_folder->folder_dir);
	char *retval = NULL;
	FILE *file;

	file = fopen (filename, "r");
	g_free (filename);

	if (file != NULL)
	{
		retval = g_malloc0 (25); /* a 64bit number must fit in it */
		fscanf (file, "%s", retval);
		fclose (file);
	}

	return retval;
}

/* Called with the store's connect_lock locked */


/*
The assumption we have to make is that any folder can have been offline 
before it got here to become selected. 

Which is nasty because that means that our IDLE code might not have kept it
synchronized. We might not have been notified by EXISTS, FETCH and EXPUNGE
unsolicited events (you can't assume that).

We also can't assume the availability of HIGHESTMODSEQ (or CONDSTORE). Not
even if the IMAP service announced the condstore capability. That's because
condstore is to be enabled per mailbox!

UIDNEXT might be incorrect, I have seen IMAP servers that simply get it wrong.

Although it has been reported that Exchange's IMAP server sometimes gives to high
values for EXISTS, it's quite safe to assume that the EXISTS is going to be 
correct on most servers. Just don't crash on sequences that don't exist remote.

More documentation inline
*/

void
camel_imap_folder_selected (CamelFolder *folder, CamelImapResponse *response,
			    CamelException *ex, gboolean idle)
{
	CamelImapFolder *imap_folder = CAMEL_IMAP_FOLDER (folder);
	CamelImapSummary *imap_summary = CAMEL_IMAP_SUMMARY (folder->summary);
	unsigned long exists = 0, validity = 0, val, uid;
	CamelMessageInfo *info;
	guint32 perm_flags = 0;
	GData *fetch_data;
	int i, count, uidnext = -1;
	char *resp, *phighestmodseq = NULL, *highestmodseq = NULL;
	CamelImapStore *store = CAMEL_IMAP_STORE (folder->parent_store);
	gboolean removals = FALSE, condstore = FALSE, needtoput=FALSE, suc=FALSE;

	count = camel_folder_summary_count (folder->summary);

	/* With CONDSTORE this is the typical output.
	 * C: A142 SELECT INBOX (CONDSTORE)
	 * S: * 172 EXISTS
	 * S: * 1 RECENT
	 * S: * OK [UNSEEN 12] Message 12 is first unseen
	 * S: * OK [UIDVALIDITY 3857529045] UIDs valid
	 * S: * OK [UIDNEXT 4392] Predicted next UID
	 * S: * FLAGS (\Answered \Flagged \Deleted \Seen \Draft)
	 * S: * OK [PERMANENTFLAGS (\Deleted \Seen \*)] Limited
	 * S: * OK [HIGHESTMODSEQ 715194045007]
	 * S: A142 OK [READ-WRITE] SELECT completed, CONDSTORE is now enabled */

	for (i = 0; i < response->untagged->len; i++) 
	{
		resp = response->untagged->pdata[i] + 2;

		if (!g_ascii_strncasecmp (resp, "FLAGS ", 6) && !perm_flags) {
			resp += 6;
			folder->permanent_flags = imap_parse_flag_list (&resp);
		} else if (!g_ascii_strncasecmp (resp, "OK [PERMANENTFLAGS ", 19)) {
			resp += 19;
			
			/* workaround for broken IMAP servers that send "* OK [PERMANENTFLAGS ()] Permanent flags"
			 * even tho they do allow storing flags. *Sigh* So many fucking broken IMAP servers out there. */
			if ((perm_flags = imap_parse_flag_list (&resp)) != 0)
				folder->permanent_flags = perm_flags;
		} else if (!g_ascii_strncasecmp (resp, "OK [UIDNEXT ", 12)) {
			char *marker = strchr (resp, ']');
			if (marker) *marker='\0';
			uidnext = strtoul (resp + 12, NULL, 10);
		} else if (!g_ascii_strncasecmp (resp, "OK [HIGHESTMODSEQ ", 18)) 
		{
		
			/* So we have a HIGHESTMODSEQ, we are going to store this
			 * one assuming that our code that will follow is correct
			 * and that after that upcoming code, the local folder 
			 * state will be in sync with that remote HIGHESTMODSEQ 
			 * value. */
			
			char *marker;
			unsigned int len;
			resp += 18;

			marker = strchr (resp, ']');

			if (marker) 
			{
				condstore = TRUE;
				len = (unsigned int) (marker - resp);
				
				highestmodseq = g_strndup (resp, len);
				phighestmodseq = get_highestmodseq (imap_folder);

				if (phighestmodseq !=NULL && !strcmp (phighestmodseq, highestmodseq)) 
				{
					g_free (phighestmodseq);
					phighestmodseq = NULL;
				} else 
					needtoput = TRUE;
			} else {
				phighestmodseq = NULL;
				highestmodseq = NULL;
			}

		} else if (!g_ascii_strncasecmp (resp, "OK [UIDVALIDITY ", 16)) {
			validity = strtoul (resp + 16, NULL, 10);
		} else if (isdigit ((unsigned char)*resp)) {
			unsigned long num = strtoul (resp, &resp, 10);
			
			if (!g_ascii_strncasecmp (resp, " EXISTS", 7)) {
				exists = num;
				/* Remove from the response so nothing
				 * else tries to interpret it. */
				g_free (response->untagged->pdata[i]);
				g_ptr_array_remove_index (response->untagged, i--);
			}
		}
	}

	if (camel_strstrcase (response->status, "OK [READ-ONLY]"))
	{
		folder->folder_flags |= CAMEL_FOLDER_IS_READONLY;
		imap_folder->read_only = TRUE;
	}

	if (camel_disco_store_status (CAMEL_DISCO_STORE (folder->parent_store)) == CAMEL_DISCO_STORE_RESYNCING) 
	{
		if (phighestmodseq != NULL)
			g_free (phighestmodseq);
		if (highestmodseq != NULL)
			g_free (highestmodseq);
		
		if (validity != imap_summary->validity) {
			camel_exception_setv (ex, CAMEL_EXCEPTION_FOLDER_SUMMARY_INVALID,
					      _("Folder was destroyed and recreated on server."));
			return;
		}

		/* FIXME: find missing UIDs ? */
		return;
	}

	/* If there's no match on VALIDITY, we are dealing with a different 
	 * folder. For example the folder got removed and re-created. We'll
	 * simply clear everyting and do things from scratch. */
	
	if (!imap_summary->validity)
		imap_summary->validity = validity;
	else if (validity != imap_summary->validity) {
		imap_summary->validity = validity;
		camel_folder_summary_clear (folder->summary);
		CAMEL_IMAP_FOLDER_REC_LOCK (imap_folder, cache_lock);
		camel_imap_message_cache_clear (imap_folder->cache);
		CAMEL_IMAP_FOLDER_REC_UNLOCK (imap_folder, cache_lock);
		imap_folder->need_rescan = FALSE;
		camel_imap_folder_changed (folder, exists, NULL, ex);
		
		if (phighestmodseq != NULL)
			g_free (phighestmodseq);
		if (highestmodseq != NULL)
			g_free (highestmodseq);
		
		return;
	}

	/* If we already had stuff locally, get the last one's UID . Store it in
	 * what we will start calling VAL */
	
	if (count > 0)
	{
		info = camel_folder_summary_index (folder->summary, count - 1);
		val = strtoul (camel_message_info_uid (info), NULL, 10);
		camel_message_info_free(info);
	} else 
		val = -1;

	/* If we are going the CONDSTORE route and if (uidnext-1) is not the 
	 * same as the last uid in our summary, it's very likely that expunges
	 * happened. This CONDSTORE code does not yet support expunges. 
	 * Therefore we will simply use the old code. */

	if (phighestmodseq != NULL && (val != uidnext-1))
	{
		g_free (phighestmodseq); 
		phighestmodseq = NULL;
		removals = TRUE;
	}

	/* If our local count isn't the same as the EXISTS, then our CONDSTORE 
	 * implementation can't be used. Period. */
	
	if (exists != count)
	{
		if (phighestmodseq != NULL)
			g_free (phighestmodseq); 
		phighestmodseq = NULL;
		removals = TRUE;
	}

	if (removals)
		imap_folder->need_rescan = TRUE;
	else if (condstore && (store->capabilities & IMAP_CAPABILITY_CONDSTORE))
		imap_folder->need_rescan = FALSE;

	/* We still aren't certain. For example if at the end of the mailbox 
	 * both an add and an expunge happened, then all of above figured out
	 * nothing meaningful. So we will compare the last local uid with the
	 * remote uid at the same sequence number (that's count, as the count
	 * is the index + 1). This is where we need the VAL thingy of above. */
	
	if (!imap_folder->need_rescan) 
	{
		int mval = 0;

		/* If the UID of the highest message we know about has changed, 
		 * then that indicates that messages have been both added and 
		 * removed, so we have to rescan to find the removed ones */

		response = camel_imap_command (store, NULL, ex, "FETCH %d UID", count);
		if (!response) {
			if (phighestmodseq != NULL)
				g_free (phighestmodseq);
			if (highestmodseq != NULL)
				g_free (highestmodseq);
			return; 
		}
		uid = 0;
		for (i = 0; i < response->untagged->len; i++) 
		{
			resp = response->untagged->pdata[i];
			mval = strtoul (resp + 2, &resp, 10);
			if (mval == 0)
				continue;
			if (!g_ascii_strcasecmp (resp, " EXISTS")) 
			{
				/* Another one?? */
				exists = mval;
				continue;
			}
			if (uid != 0 || mval != count || g_ascii_strncasecmp (resp, " FETCH (", 8) != 0)
				continue;
			
			fetch_data = parse_fetch_response (imap_folder, resp + 7);
			uid = strtoul (g_datalist_get_data (&fetch_data, "UID"), NULL, 10);
			g_datalist_clear (&fetch_data);
		}
		camel_imap_response_free_without_processing (store, response);
		if (uid == 0 || uid != val)
			imap_folder->need_rescan = TRUE;
	}

	/* Okay, it survived ALL checks and CONDSTORE is available too. Lucky we 
	 * are, aren't we? So we use CONDSTORE. Note, however, that if condstore 
	 * still finds removals (sequences don't match) or if anything goes wrong
	 * during the CONDSTORE code, that it'll set need_rescan TRUE and that 
	 * as a result the lines below this if{} block will still happen. This is
	 * indeed on purpose and as a fall-back situation (our detection got it
	 * wrong) */
	
	if (phighestmodseq)
	{
		if (!imap_folder->need_rescan)
			suc = imap_rescan_condstore (folder, exists, phighestmodseq, ex);
		g_free (phighestmodseq);
		phighestmodseq = NULL;
	}

	if (imap_folder->need_rescan)
		suc = imap_rescan (folder, exists, ex);
	else if (exists > count)
		camel_imap_folder_changed (folder, exists, NULL, ex);

	if (highestmodseq != NULL & suc && needtoput)
		put_highestmodseq (imap_folder, (const char *) highestmodseq);
	
	if (highestmodseq != NULL)
		g_free (highestmodseq);

	if (idle)
		camel_imap_folder_start_idle (folder);
}

static void 
imap_finalize (CamelObject *object)
{
	CamelImapFolder *imap_folder = CAMEL_IMAP_FOLDER (object);
	CamelImapStore *store = CAMEL_IMAP_STORE (CAMEL_FOLDER(imap_folder)->parent_store);

	imap_folder->do_push_email = FALSE;

	imap_folder->stopping = TRUE;

	if (imap_folder->idle_signal > 0) 
		g_source_remove (imap_folder->idle_signal);

	if (!imap_folder->in_idle || imap_folder->idle_lock != NULL)
	{
		g_static_rec_mutex_free (imap_folder->idle_lock);
		imap_folder->idle_lock = NULL;
	}

	if (store->current_folder == (CamelFolder*) object)
		store->current_folder = NULL;

	if (imap_folder->search)
		camel_object_unref (CAMEL_OBJECT (imap_folder->search));
	if (imap_folder->cache)
		camel_object_unref (CAMEL_OBJECT (imap_folder->cache));

	if (imap_folder->folder_dir)
		g_free (imap_folder->folder_dir);

#ifdef ENABLE_THREADS
	g_static_mutex_free(&imap_folder->priv->search_lock);
	g_static_rec_mutex_free(&imap_folder->priv->cache_lock);
#endif

	g_free(imap_folder->priv);
}

static int
imap_getv(CamelObject *object, CamelException *ex, CamelArgGetV *args)
{
	CamelFolder *folder = (CamelFolder *)object;
	int i, count=0;
	guint32 tag;

	for (i=0;i<args->argc;i++) {
		CamelArgGet *arg = &args->argv[i];

		tag = arg->tag;

		switch (tag & CAMEL_ARG_TAG) {
			/* CamelObject args */
		case CAMEL_OBJECT_ARG_DESCRIPTION:
			if (folder->description == NULL) {
				CamelURL *uri = ((CamelService *)folder->parent_store)->url;

				/* what if the full name doesn't incclude /'s?  does it matter? */
				folder->description = g_strdup_printf("%s@%s:%s", uri->user, uri->host, folder->full_name);
			}
			*arg->ca_str = folder->description;
			break;
		default:
			count++;
			continue;
		}

		arg->tag = (tag & CAMEL_ARG_TYPE) | CAMEL_ARG_IGNORE;
	}

	if (count)
		return ((CamelObjectClass *)parent_class)->getv(object, ex, args);

	return 0;
}

static void
imap_rename (CamelFolder *folder, const char *new)
{
	CamelImapFolder *imap_folder = (CamelImapFolder *)folder;
	CamelImapStore *imap_store = (CamelImapStore *)folder->parent_store;
	char *folder_dir, *summary_path, *state_file;
	char *folders;

	folders = g_strconcat (imap_store->storage_path, "/folders", NULL);
	folder_dir = imap_path_to_physical (folders, new);
	g_free (folders);
	summary_path = g_strdup_printf("%s/summary", folder_dir);

	CAMEL_IMAP_FOLDER_REC_LOCK (folder, cache_lock);
	camel_imap_message_cache_set_path(imap_folder->cache, folder_dir);
	CAMEL_IMAP_FOLDER_REC_UNLOCK (folder, cache_lock);

	camel_folder_summary_set_filename(folder->summary, summary_path);

	state_file = g_strdup_printf ("%s/cmeta", folder_dir);
	camel_object_set(folder, NULL, CAMEL_OBJECT_STATE_FILE, state_file, NULL);
	g_free(state_file);

	g_free(summary_path);
	g_free(folder_dir);

	((CamelFolderClass *)disco_folder_class)->rename(folder, new);
}

static void
imap_refresh_info (CamelFolder *folder, CamelException *ex)
{
	CamelImapStore *imap_store = CAMEL_IMAP_STORE (folder->parent_store);
	CamelImapFolder *imap_folder = CAMEL_IMAP_FOLDER (folder);
	CamelImapResponse *response;
	CamelStoreInfo *si;

	if (camel_disco_store_status (CAMEL_DISCO_STORE (imap_store)) == CAMEL_DISCO_STORE_OFFLINE)
		return;

	if (camel_folder_is_frozen (folder)) {
		imap_folder->need_refresh = TRUE;
		return;
	}

	/* If the folder isn't selected, select it (which will force
	 * a rescan if one is needed).
	 * Also, if this is the INBOX, some servers (cryus) wont tell
	 * us with a NOOP of new messages, so force a reselect which
	 * should do it.  */
	CAMEL_SERVICE_REC_LOCK (imap_store, connect_lock);

	if (!camel_disco_store_check_online ((CamelDiscoStore*)imap_store, ex))
		goto done;

	if (imap_store->current_folder != folder
	    || g_ascii_strcasecmp(folder->full_name, "INBOX") == 0) {
		response = camel_imap_command (imap_store, folder, ex, NULL);
		if (response) {
			camel_imap_folder_selected (folder, response, ex, TRUE);
			camel_imap_response_free (imap_store, response);
		}
	} else if (imap_folder->need_rescan) {
		/* Otherwise, if we need a rescan, do it, and if not, just do
		 * a NOOP to give the server a chance to tell us about new
		 * messages. */
		imap_rescan (folder, camel_folder_summary_count (folder->summary), ex);
	} else {
#if 0
		/* on some servers need to CHECKpoint INBOX to recieve new messages?? */
		/* rfc2060 suggests this, but havent seen a server that requires it */
		if (g_ascii_strcasecmp(folder->full_name, "INBOX") == 0) {
			response = camel_imap_command (imap_store, folder, ex, "CHECK");
			camel_imap_response_free (imap_store, response);
			camel_imap_folder_start_idle (folder);
		}
#endif
		response = camel_imap_command (imap_store, folder, ex, "NOOP");
		camel_imap_response_free (imap_store, response);
		camel_imap_folder_start_idle (folder);
	}

	si = camel_store_summary_path((CamelStoreSummary *)((CamelImapStore *)folder->parent_store)->summary, folder->full_name);
	if (si) {
		guint32 unread, total;

		camel_object_get(folder, NULL, CAMEL_FOLDER_TOTAL, &total, CAMEL_FOLDER_UNREAD, &unread, NULL);
		if (si->total != total
		    || si->unread != unread){
			si->total = total;
			si->unread = unread;
			camel_store_summary_touch((CamelStoreSummary *)((CamelImapStore *)folder->parent_store)->summary);
		}
		camel_store_summary_info_free((CamelStoreSummary *)((CamelImapStore *)folder->parent_store)->summary, si);
	}
done:
	CAMEL_SERVICE_REC_UNLOCK (imap_store, connect_lock);

	/* TNY TOCHECK: I think all situations are already saving the summary?!
	 * camel_folder_summary_save(folder->summary); */

	camel_store_summary_save((CamelStoreSummary *)((CamelImapStore *)folder->parent_store)->summary);
}

static void
flags_to_label(CamelFolder *folder, CamelImapMessageInfo *mi)
{
	/* We snoop the server flag setting, and map it to
	   the label 'user tag'.  We also clean it up at this
	   point, there can only be 1 label set at a time */
	if (folder->permanent_flags & CAMEL_IMAP_MESSAGE_LABEL_MASK) {
		const char *label = NULL;
		guint32 mask = 0;

		if (mi->info.flags & CAMEL_IMAP_MESSAGE_LABEL1) {
			mask = CAMEL_IMAP_MESSAGE_LABEL1;
			label = "important";
		} else if (mi->info.flags & CAMEL_IMAP_MESSAGE_LABEL2) {
			mask = CAMEL_IMAP_MESSAGE_LABEL2;
			label = "work";
		} else if (mi->info.flags & CAMEL_IMAP_MESSAGE_LABEL3) {
			mask = CAMEL_IMAP_MESSAGE_LABEL3;
			label = "personal";
		} else if (mi->info.flags & CAMEL_IMAP_MESSAGE_LABEL4) {
			mask = CAMEL_IMAP_MESSAGE_LABEL4;
			label = "todo";
		} else if (mi->info.flags & CAMEL_IMAP_MESSAGE_LABEL5) {
			mask = CAMEL_IMAP_MESSAGE_LABEL5;
			label = "later";
		}

		mi->info.flags = (mi->info.flags & ~CAMEL_IMAP_MESSAGE_LABEL_MASK) | mask;

#ifdef NON_TINYMAIL_FEATURES
		camel_tag_set(&mi->info.user_tags, "label", label);
#endif
	}
}

static gboolean 
imap_rescan_condstore (CamelFolder *folder, int exists, const char *highestmodseq, CamelException *ex)
{

	/* So this is what we'll get. All the changes since highestmodseq.
	 * C: s100 UID FETCH 1:* (FLAGS) (CHANGEDSINCE 12345)
	 * S: * 1 FETCH (UID 4 MODSEQ (65402) FLAGS (\Seen))
	 * S: * 2 FETCH (UID 6 MODSEQ (75403) FLAGS (\Deleted))
	 * S: * 4 FETCH (UID 8 MODSEQ (29738) FLAGS ($NoJunk $AutoJunk $MDNSent))
	 * S: s100 OK FETCH completed */

	CamelImapFolder *imap_folder = CAMEL_IMAP_FOLDER (folder);
	CamelImapStore *store = CAMEL_IMAP_STORE (folder->parent_store);
	struct {
		char *uid;
		guint32 flags;
	} *new;
	char *resp;
	CamelImapResponseType type;
	int i, summary_len, summary_got;
	CamelMessageInfo *info;
	CamelImapMessageInfo *iinfo;
	gboolean ok, retval = TRUE;
	CamelFolderChangeInfo *changes = NULL;

	imap_folder->need_rescan = FALSE;

	summary_len = camel_folder_summary_count (folder->summary);
	if (summary_len == 0) {
		if (exists)
			camel_imap_folder_changed (folder, exists, NULL, ex);
		return;
	}

	camel_operation_start (NULL, _("Scanning for changed messages in %s"), folder->name);
	ok = camel_imap_command_start (store, folder, ex,
				       "UID FETCH 1:* (FLAGS) (CHANGEDSINCE %s)",
				       highestmodseq);
	if (!ok) {

		/* This probably means that the server doesn't understand 
		 * CHANGEDSINCE, though it advertised support for CONDSTORE.
		 * But we are the client, we need to forgive! So we'll simply
		 * set need_rescan to TRUE and let the fallback stuff in the
		 * 'selected' method deal with it by launching imap_rescan. */

		imap_folder->need_rescan = TRUE;

		/* TNY TODO: turn-off the CONDSTORE capability? */

		camel_operation_end (NULL);
		return FALSE;
	}

	/* Highfive! everything is still up and running. It's still possible 
	 * that things go wrong. But there's no need to stop then: we'll need
	 * to read-away what the server is sending anyway. So why not perform 
	 * some work each time both the UID and the SEQUENCE match with what
	 * we have locally?
	 * 
	 * If not, we simply set need_rescan TRUE and keep on going. This does,
	 * however, mean that we actually didn't really win traffic by using
	 * condstore. We even lost some. But being truly in sync is far more
	 * important. */

	summary_got = 0;
	while ((type = camel_imap_command_response (store, &resp, ex)) == CAMEL_IMAP_RESPONSE_UNTAGGED) 
	{
		GData *data;
		char *uid;
		guint32 flags, seq;

		data = parse_fetch_response (imap_folder, resp);
		g_free (resp);

		if (!data)
			continue;

		uid = g_datalist_get_data (&data, "UID");
		flags = GPOINTER_TO_UINT (g_datalist_get_data (&data, "FLAGS"));
		seq = GPOINTER_TO_UINT (g_datalist_get_data (&data, "SEQUENCE"));

		/* So basically: if the UID is not found locally, we have flags
		 * for a new message. That's cool, but not an error as the 
		 * camel_imap_folder_changed will launch imap_update_summary 
		 * who will deal with that.
		 * 
		 * See, we wont find it if it's larger than the count of the
		 * local summary list. We don't even have to check that, right? 
		 * 
		 * We also check for negative sequences. No reason for that, 
		 * it's just wrong (actually impossible since it's a unsigned 
		 * int, but it's just for security-clarity here. Let it be, it 
		 * doesn't hurt either) */
		
		if (!uid || seq < 0 || seq-1 > summary_len) {
			imap_folder->need_rescan = TRUE;
			retval = FALSE;
			g_datalist_clear (&data);
			continue;
		}

		info = camel_folder_summary_index (folder->summary, seq-1);
		iinfo = (CamelImapMessageInfo *) info;
		
		
		/* However, if we do find the message-info at that index then we 
		 * we should also check whether the UID matches. If not, well ..
		 * that's not good eh. That means that we shouldn't have been 
		 * using this CONDSTORE code in the first place! Bad luck, we 
		 * just set need_rescan TRUE and let the selected metod launch
		 * the conventional imap_rescan later-on. Since we are receiving 
		 * stuff anyway, and we need to read that away nonetheless, we
		 * just continue doing our stuff. */
		
		if (info) 
		{
			/* printf ("%s vs %s on %d\n", info->uid, uid, seq-1); */
			if (!strcmp (info->uid, uid))
			{
			  if (flags != iinfo->server_flags) 
			  {
				camel_folder_summary_touch (folder->summary);
				guint32 server_set, server_cleared;
				server_set = flags & ~iinfo->server_flags;
				server_cleared = iinfo->server_flags & ~flags;
				iinfo->info.flags = (iinfo->info.flags | server_set) & ~server_cleared;
				iinfo->server_flags = flags;
				if (changes == NULL)
					changes = camel_folder_change_info_new();
				camel_folder_change_info_change_uid(changes, uid);
				/* flags_to_label(folder, (CamelImapMessageInfo *)info); */
			  }
			  camel_message_info_free (info);
			} else {
				imap_folder->need_rescan = TRUE;
				retval = FALSE;
			}
		} else 
			retval = FALSE;

		camel_operation_progress (NULL, ++summary_got , summary_len);
		g_datalist_clear (&data);
	}

	camel_operation_end (NULL);

	if (type == CAMEL_IMAP_RESPONSE_ERROR)
		return FALSE;

	/* Free the final tagged response */
	if (resp)
		g_free (resp);

	if (changes) 
	{
		camel_object_trigger_event(CAMEL_OBJECT (folder), "folder_changed", changes);
		camel_folder_change_info_free(changes);
	}

	/* The thing with te if is that the selected folder will do imap_rescan 
	 * anyway, that function will call the camel_imap_folder_changed too.
	 * There's no need to let this happen twice, so we filter it. 
	 * 
	 * However. If we were lucky and condstore succeeded without problems,
	 * then we must check for new messages of course. We can't have removals
	 * which is why it's set to NULL. If we had removals, then need_rescan
	 * has been set to TRUE. Right? Right! */
	
	if (!imap_folder->need_rescan)
		camel_imap_folder_changed (folder, exists, NULL, ex);
	
	return retval;
}

/* Called with the store's connect_lock locked */
static gboolean
imap_rescan (CamelFolder *folder, int exists, CamelException *ex)
{
	gboolean retval = TRUE;
	
	/* Welcome to the most hairy code of Camel. On the left you
	 * have the bathroom where you can puke. On the right you have 
	 * fresh water and soap to clean it up. In front of you, you have ..
	 * 
	 * The code */
	
	CamelImapFolder *imap_folder = CAMEL_IMAP_FOLDER (folder);
	CamelImapStore *store = CAMEL_IMAP_STORE (folder->parent_store);
	struct {
		char *uid;
		guint32 flags;
	} *new;
	char *resp;
	CamelImapResponseType type;
	int i, seq, summary_len, summary_got;
	CamelMessageInfo *info;
	CamelImapMessageInfo *iinfo;
	GArray *removed;
	gboolean ok;
	CamelFolderChangeInfo *changes = NULL;

	imap_folder->need_rescan = FALSE;
	
	summary_len = camel_folder_summary_count (folder->summary);
	
	if (summary_len == 0) 
	{
		if (exists)
			camel_imap_folder_changed (folder, exists, NULL, ex);
		return TRUE;
	}
	
	/* Check UIDs and flags of all messages we already know of. */
	camel_operation_start (NULL, _("Scanning for changed messages in %s"), folder->name);
	info = camel_folder_summary_index (folder->summary, summary_len - 1);
	ok = camel_imap_command_start (store, folder, ex,
				       "UID FETCH 1:%s (FLAGS)",
				       camel_message_info_uid (info));
	camel_message_info_free(info);
	if (!ok) {
		camel_operation_end (NULL);
		return FALSE;
	}
	
	/* Soo ... we allocate a matrix of the exact size of the local summary 
	 * after we launched a command that asks for the flags of all messages
	 * until the last uid we have locally. Let's iterate them ... */
	
	new = g_malloc0 (summary_len * sizeof (*new));
	summary_got = 0;
	resp = NULL;
	
	while ((type = camel_imap_command_response (store, &resp, ex)) 
	       	== CAMEL_IMAP_RESPONSE_UNTAGGED) 
	{
		GData *data;
		char *uid;
		guint32 flags;
		
		data = parse_fetch_response (imap_folder, resp);
		g_free (resp);
		if (!data)
			continue;
		
		seq = GPOINTER_TO_INT (g_datalist_get_data (&data, "SEQUENCE"));
		uid = g_datalist_get_data (&data, "UID");
		flags = GPOINTER_TO_UINT (g_datalist_get_data (&data, "FLAGS"));
		
		if (!uid || !seq || seq > summary_len || seq < 0) 
		{
			retval = FALSE;
			g_datalist_clear (&data);
			continue;
		}
		
		/* See, we are just storing them in that matrix. Nothing
		 * difficult here. */
		
		camel_operation_progress (NULL, ++summary_got , summary_len);
		new[seq - 1].uid = g_strdup (uid);
		new[seq - 1].flags = flags;
		g_datalist_clear (&data);
	}
	
	camel_operation_end (NULL);
	
	if (type == CAMEL_IMAP_RESPONSE_ERROR) 
	{
		for (i = 0; i < summary_len && new[i].uid; i++)
			g_free (new[i].uid);
		g_free (new);
		return FALSE;
	}
	
	/* Free the final tagged response */
	if (resp)
		g_free (resp);
	
	/* If we find a UID in the summary that doesn't correspond to
	 * the UID in the folder, then either: (a) it's a real UID,
	 * but the message was deleted on the server, or (b) it's a
	 * fake UID, and needs to be removed from the summary in order
	 * to sync up with the server. So either way, we remove it
	 * from the summary. */
	
	removed = g_array_new (FALSE, FALSE, sizeof (int));
	for (i = 0; i < summary_len && new[i].uid; i++) 
	{
		info = camel_folder_summary_index (folder->summary, i);
		iinfo = (CamelImapMessageInfo *)info;

		if (!info)
			continue;

		/* This is where it gets hairy. It tries to mimic what an 
		 * EXPUNGE would cause. That's because we will pass the removed
		 * sequences to the same handler for it (check below and in
		 * the code in imap-command and the IDLE stuff that handles
		 * EXPUNGE responses). 
		 * 
		 * Now. the consequences are that if at the very beginning of
		 * the remote mailbox a message got expunged, all local messages
		 * will be marked for removal caused by this. That's a terrible
		 * idea, but it is what it is .... */

		/* TODO: recalculating rather than marking a lot perfectly fine
		 * material for removal */

		if (strcmp (camel_message_info_uid (info), new[i].uid) != 0) {
			camel_message_info_free(info);
			seq = i + 1;
			g_array_append_val (removed, seq);
			i--;
			summary_len--;
			continue;
		}
		
		
		/* In case both the uid and the sequence are in match remote and
		 * locally, we can securely update the flags locally. */

		if (new[i].flags != iinfo->server_flags) 
		{
			guint32 server_set, server_cleared;
			
			server_set = new[i].flags & ~iinfo->server_flags;
			server_cleared = iinfo->server_flags & ~new[i].flags;

			iinfo->info.flags = (iinfo->info.flags | server_set) & ~server_cleared;
			iinfo->server_flags = new[i].flags;

			if (changes == NULL)
				changes = camel_folder_change_info_new();
			camel_folder_change_info_change_uid(changes, new[i].uid);
			/* flags_to_label(folder, (CamelImapMessageInfo *)info); */
		}

		camel_message_info_free(info);
		g_free (new[i].uid);
	}

	if (changes) 
	{
		camel_object_trigger_event(CAMEL_OBJECT (folder), "folder_changed", changes);
		camel_folder_change_info_free(changes);
	}
	
	seq = i + 1;
	
	/* Free remaining memory. */
	while (i < summary_len && new[i].uid)
		g_free (new[i++].uid);
	g_free (new);

	/* Remove any leftover cached summary messages. (Yes, we repeatedly add
	 * the same number to the removed array. See RFC2060 7.4.1) */

	for (i = seq; i <= summary_len; i++)
		g_array_append_val (removed, seq);

	/* And finally update the summary. */
	camel_imap_folder_changed (folder, exists, removed, ex);
	g_array_free (removed, TRUE);

	return retval;
}

/* the max number of chars that an unsigned 32-bit int can be is 10 chars plus 1 for a possible : */
#define UID_SET_FULL(setlen, maxlen) (maxlen > 0 ? setlen + 11 >= maxlen : FALSE)

/* Find all messages in @folder with flags matching @flags and @mask.
 * If no messages match, returns %NULL. Otherwise, returns an array of
 * CamelMessageInfo and sets *@set to a message set corresponding the
 * UIDs of the matched messages (up to @UID_SET_LIMIT bytes). The
 * caller must free the infos, the array, and the set string. */
static GPtrArray *
get_matching (CamelFolder *folder, guint32 flags, guint32 mask, char **set)
{
	GPtrArray *matches;
	CamelImapMessageInfo *info;
	int i, max, range;
	GString *gset;
	
	matches = g_ptr_array_new ();
	gset = g_string_new ("");
	max = camel_folder_summary_count (folder->summary);
	range = -1;
	for (i = 0; i < max && !UID_SET_FULL (gset->len, UID_SET_LIMIT); i++) {
		info = (CamelImapMessageInfo *)camel_folder_summary_index (folder->summary, i);
		if (!info)
			continue;
		if ((info->info.flags & mask) != flags) {
			camel_message_info_free((CamelMessageInfo *)info);
			if (range != -1) {
				if (range != i - 1) {
					info = matches->pdata[matches->len - 1];
					g_string_append_printf (gset, ":%s", camel_message_info_uid (info));
				}
				range = -1;
			}
			continue;
		}
		
		g_ptr_array_add (matches, info);
		if (range != -1)
			continue;
		range = i;
		if (gset->len)
			g_string_append_c (gset, ',');
		g_string_append_printf (gset, "%s", camel_message_info_uid (info));
	}
	
	if (range != -1 && range != max - 1) {
		info = matches->pdata[matches->len - 1];
		g_string_append_printf (gset, ":%s", camel_message_info_uid (info));
	}
	
	if (matches->len) {
		*set = gset->str;
		g_string_free (gset, FALSE);
		return matches;
	} else {
		*set = NULL;
		g_string_free (gset, TRUE);
		g_ptr_array_free (matches, TRUE);
		return NULL;
	}
}

static void
imap_sync_offline (CamelFolder *folder, CamelException *ex)
{
	camel_folder_summary_save (folder->summary);
	camel_store_summary_save((CamelStoreSummary *)((CamelImapStore *)folder->parent_store)->summary);
}

static void
imap_sync_online (CamelFolder *folder, CamelException *ex)
{
	CamelImapStore *store = CAMEL_IMAP_STORE (folder->parent_store);
	CamelImapResponse *response = NULL;
	CamelImapMessageInfo *info;
	CamelException local_ex;
	GPtrArray *matches;
	char *set, *flaglist;
	gboolean unset;
	int i, j, max;
	
	if (folder->permanent_flags == 0) {
		imap_sync_offline (folder, ex);
		return;
	}
	
	camel_exception_init (&local_ex);
	CAMEL_SERVICE_REC_LOCK (store, connect_lock);
	
	/* Find a message with changed flags, find all of the other
	 * messages like it, sync them as a group, mark them as
	 * updated, and continue.*/
	max = camel_folder_summary_count (folder->summary);
	for (i = 0; i < max; i++) {
		if (!(info = (CamelImapMessageInfo *)camel_folder_summary_index (folder->summary, i)))
			continue;
		
		if (!(info->info.flags & CAMEL_MESSAGE_FOLDER_FLAGGED)) {
			camel_message_info_free((CamelMessageInfo *)info);
			continue;
		}

		/* Note: Cyrus is broken and will not accept an
		   empty-set of flags so... if this is true then we
		   want to unset the previously set flags.*/
		unset = !(info->info.flags & folder->permanent_flags);
		
		/* Note: get_matching() uses UID_SET_LIMIT to limit
		   the size of the uid-set string. We don't have to
		   loop here to flush all the matching uids because
		   they will be scooped up later by our parent loop (I
		   think?). -- Jeff */
		matches = get_matching (folder, info->info.flags & (folder->permanent_flags | CAMEL_MESSAGE_FOLDER_FLAGGED),
					folder->permanent_flags | CAMEL_MESSAGE_FOLDER_FLAGGED, &set);
		camel_message_info_free(info);
		if (matches == NULL)
			continue;
		
		/* Make sure we're connected before issuing commands */
		if (!camel_disco_store_check_online ((CamelDiscoStore*)store, ex)) {
			g_free(set);
			break;
		}

		/* FIXME: since we don't know the previously set flags,
		   if unset is TRUE then just unset all the flags? */
		flaglist = imap_create_flag_list (unset ? folder->permanent_flags : info->info.flags & folder->permanent_flags);
		
		/* Note: to `unset' flags, use -FLAGS.SILENT (<flag list>) */
		response = camel_imap_command (store, folder, &local_ex,
					       "UID STORE %s %sFLAGS.SILENT %s",
					       set, unset ? "-" : "", flaglist);
		g_free (set);
		g_free (flaglist);
		
		if (response)
			camel_imap_response_free (store, response);
		
		if (!camel_exception_is_set (&local_ex)) {
			for (j = 0; j < matches->len; j++) {
				info = matches->pdata[j];
				info->info.flags &= ~CAMEL_MESSAGE_FOLDER_FLAGGED;
				((CamelImapMessageInfo *) info)->server_flags =	info->info.flags & CAMEL_IMAP_SERVER_FLAGS;
			}
			camel_folder_summary_touch (folder->summary);
		}
		
		for (j = 0; j < matches->len; j++) {
			info = matches->pdata[j];
			camel_message_info_free(&info->info);
		}
		g_ptr_array_free (matches, TRUE);
		
		/* We unlock here so that other threads can have a chance to grab the connect_lock */
		CAMEL_SERVICE_REC_UNLOCK (store, connect_lock);
		
		/* check for an exception */
		if (camel_exception_is_set (&local_ex)) {
			camel_exception_xfer (ex, &local_ex);
			return;
		}
		
		/* Re-lock the connect_lock */
		CAMEL_SERVICE_REC_LOCK (store, connect_lock);
	}
	
	/* Save the summary */
	imap_sync_offline (folder, ex);
	
	CAMEL_SERVICE_REC_UNLOCK (store, connect_lock);
	/* camel_imap_folder_start_idle (folder); */
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
imap_expunge_uids_offline (CamelFolder *folder, GPtrArray *uids, CamelException *ex)
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
}

static void
imap_expunge_uids_online (CamelFolder *folder, GPtrArray *uids, CamelException *ex)
{
	CamelImapStore *store = CAMEL_IMAP_STORE (folder->parent_store);
	CamelImapResponse *response;
	int uid = 0;
	char *set;
	
	CAMEL_SERVICE_REC_LOCK (store, connect_lock);

	if ((store->capabilities & IMAP_CAPABILITY_UIDPLUS) == 0) {
		((CamelFolderClass *)CAMEL_OBJECT_GET_CLASS(folder))->sync(folder, 0, ex);
		if (camel_exception_is_set(ex)) {
			CAMEL_SERVICE_REC_UNLOCK (store, connect_lock);
			return;
		}
	}
	
	qsort (uids->pdata, uids->len, sizeof (void *), uid_compar);
	
	while (uid < uids->len) {
		set = imap_uid_array_to_set (folder->summary, uids, uid, UID_SET_LIMIT, &uid);
		response = camel_imap_command (store, folder, ex,
					       "UID STORE %s +FLAGS.SILENT (\\Deleted)",
					       set);
		if (response)
			camel_imap_response_free (store, response);
		if (camel_exception_is_set (ex)) {
			CAMEL_SERVICE_REC_UNLOCK (store, connect_lock);
			g_free (set);
			return;
		}
		
		if (store->capabilities & IMAP_CAPABILITY_UIDPLUS) {
			response = camel_imap_command (store, folder, ex,
						       "UID EXPUNGE %s", set);
		} else
			response = camel_imap_command (store, folder, ex, "EXPUNGE");
		
		if (response)
			camel_imap_response_free (store, response);
	}
	
	CAMEL_SERVICE_REC_UNLOCK (store, connect_lock);

	camel_imap_folder_start_idle (folder);

}

static void
imap_expunge_uids_resyncing (CamelFolder *folder, GPtrArray *uids, CamelException *ex)
{
	CamelImapFolder *imap_folder = CAMEL_IMAP_FOLDER (folder);
	CamelImapStore *store = CAMEL_IMAP_STORE (folder->parent_store);
	GPtrArray *keep_uids, *mark_uids;
	CamelImapResponse *response;
	char *result;

	if (imap_folder->read_only)
		return;

	if (store->capabilities & IMAP_CAPABILITY_UIDPLUS) {
		imap_expunge_uids_online (folder, uids, ex);
		return;
	}
	
	/* If we don't have UID EXPUNGE we need to avoid expunging any
	 * of the wrong messages. So we search for deleted messages,
	 * and any that aren't in our to-expunge list get temporarily
	 * marked un-deleted. */
	
	CAMEL_SERVICE_REC_LOCK (store, connect_lock);

	((CamelFolderClass *)CAMEL_OBJECT_GET_CLASS(folder))->sync(folder, 0, ex);
	if (camel_exception_is_set(ex)) {
		CAMEL_SERVICE_REC_UNLOCK (store, connect_lock);
		return;
	}

	response = camel_imap_command (store, folder, ex, "UID SEARCH DELETED");
	if (!response) {
		CAMEL_SERVICE_REC_UNLOCK (store, connect_lock);
		return;
	}
	result = camel_imap_response_extract (store, response, "SEARCH", ex);
	if (!result) {
		CAMEL_SERVICE_REC_UNLOCK (store, connect_lock);
		return;
	}
	
	if (result[8] == ' ') {
		char *uid, *lasts = NULL;
		unsigned long euid, kuid;
		int ei, ki;
		
		keep_uids = g_ptr_array_new ();
		mark_uids = g_ptr_array_new ();
		
		/* Parse SEARCH response */
		for (uid = strtok_r (result + 9, " ", &lasts); uid; uid = strtok_r (NULL, " ", &lasts))
			g_ptr_array_add (keep_uids, uid);
		qsort (keep_uids->pdata, keep_uids->len,
		       sizeof (void *), uid_compar);
		
		/* Fill in "mark_uids", empty out "keep_uids" as needed */
		for (ei = ki = 0; ei < uids->len; ei++) {
			euid = strtoul (uids->pdata[ei], NULL, 10);
			
			for (kuid = 0; ki < keep_uids->len; ki++) {
				kuid = strtoul (keep_uids->pdata[ki], NULL, 10);
				
				if (kuid >= euid)
					break;
			}
			
			if (euid == kuid)
				g_ptr_array_remove_index (keep_uids, ki);
			else
				g_ptr_array_add (mark_uids, uids->pdata[ei]);
		}
	} else {
		/* Empty SEARCH result, meaning nothing is marked deleted
		 * on server. */
		
		keep_uids = NULL;
		mark_uids = uids;
	}
	
	/* Unmark messages to be kept */
	
	if (keep_uids) {
		char *uidset;
		int uid = 0;
		
		while (uid < keep_uids->len) {
			uidset = imap_uid_array_to_set (folder->summary, keep_uids, uid, UID_SET_LIMIT, &uid);
			
			response = camel_imap_command (store, folder, ex,
						       "UID STORE %s -FLAGS.SILENT (\\Deleted)",
						       uidset);
			
			g_free (uidset);
			
			if (!response) {
				g_ptr_array_free (keep_uids, TRUE);
				g_ptr_array_free (mark_uids, TRUE);
				CAMEL_SERVICE_REC_UNLOCK (store, connect_lock);
				return;
			}
			camel_imap_response_free (store, response);

		}
	}
	
	/* Mark any messages that still need to be marked */
	if (mark_uids) {
		char *uidset;
		int uid = 0;
		
		while (uid < mark_uids->len) {
			uidset = imap_uid_array_to_set (folder->summary, mark_uids, uid, UID_SET_LIMIT, &uid);
			
			response = camel_imap_command (store, folder, ex,
						       "UID STORE %s +FLAGS.SILENT (\\Deleted)",
						       uidset);
			
			g_free (uidset);
			
			if (!response) {
				g_ptr_array_free (keep_uids, TRUE);
				g_ptr_array_free (mark_uids, TRUE);
				CAMEL_SERVICE_REC_UNLOCK (store, connect_lock);
				return;
			}
		}

		if (mark_uids != uids)
			g_ptr_array_free (mark_uids, TRUE);
	}
	
	/* Do the actual expunging */
	response = camel_imap_command (store, folder, ex, "EXPUNGE");
	if (response)
		camel_imap_response_free (store, response);
	
	/* And fix the remaining messages if we mangled them */
	if (keep_uids) {
		char *uidset;
		int uid = 0;
		
		while (uid < keep_uids->len) {
			uidset = imap_uid_array_to_set (folder->summary, keep_uids, uid, UID_SET_LIMIT, &uid);
			
			/* Don't pass ex if it's already been set */
			response = camel_imap_command (store, folder,
						       camel_exception_is_set (ex) ? NULL : ex,
						       "UID STORE %s +FLAGS.SILENT (\\Deleted)",
						       uidset);
			
			g_free (uidset);
			if (response)
				camel_imap_response_free (store, response);
		}
		
		g_ptr_array_free (keep_uids, TRUE);
	}

	/* now we can free this, now that we're done with keep_uids */
	g_free (result);
	
	CAMEL_SERVICE_REC_UNLOCK (store, connect_lock);

	camel_imap_folder_start_idle (folder);

}

static gchar *
get_temp_uid (void)
{
	gchar *res;

	static int counter = 0;
	G_LOCK_DEFINE_STATIC (lock);

	G_LOCK (lock);
	res = g_strdup_printf ("tempuid-%lx-%d", 
			       (unsigned long) time (NULL),
			       counter++);
	G_UNLOCK (lock);

	return res;
}

static void
imap_append_offline (CamelFolder *folder, CamelMimeMessage *message,
		     const CamelMessageInfo *info, char **appended_uid,
		     CamelException *ex)
{
	CamelImapStore *imap_store = CAMEL_IMAP_STORE (folder->parent_store);
	CamelImapMessageCache *cache = CAMEL_IMAP_FOLDER (folder)->cache;
	CamelFolderChangeInfo *changes;
	char *uid;

	uid = get_temp_uid ();

	camel_imap_summary_add_offline (folder->summary, uid, message, info);
	CAMEL_IMAP_FOLDER_REC_LOCK (folder, cache_lock);
	camel_imap_message_cache_insert_wrapper (cache, uid, "",
						 CAMEL_DATA_WRAPPER (message), ex);
	CAMEL_IMAP_FOLDER_REC_UNLOCK (folder, cache_lock);

	changes = camel_folder_change_info_new ();
	camel_folder_change_info_add_uid (changes, uid);
	camel_object_trigger_event (CAMEL_OBJECT (folder), "folder_changed",
				    changes);
	camel_folder_change_info_free (changes);

	camel_disco_diary_log (CAMEL_DISCO_STORE (imap_store)->diary,
			       CAMEL_DISCO_DIARY_FOLDER_APPEND, folder, uid);
	if (appended_uid)
		*appended_uid = uid;
	else
		g_free (uid);
}

static CamelImapResponse *
do_append (CamelFolder *folder, CamelMimeMessage *message,
	   const CamelMessageInfo *info, char **uid,
	   CamelException *ex)
{
	CamelImapStore *store = CAMEL_IMAP_STORE (folder->parent_store);
	CamelImapResponse *response, *response2;
	CamelStream *memstream;
	CamelMimeFilter *crlf_filter;
	CamelStreamFilter *streamfilter;
	GByteArray *ba;
	char *flagstr, *end;
	guint32 flags = 0;
	
	/* encode any 8bit parts so we avoid sending embedded nul-chars and such  */
	camel_mime_message_encode_8bit_parts (message);
	
	/* FIXME: We could avoid this if we knew how big the message was. */
	memstream = camel_stream_mem_new ();
	ba = g_byte_array_new ();
	camel_stream_mem_set_byte_array (CAMEL_STREAM_MEM (memstream), ba);
	
	streamfilter = camel_stream_filter_new_with_stream (memstream);
	crlf_filter = camel_mime_filter_crlf_new (CAMEL_MIME_FILTER_CRLF_ENCODE,
						  CAMEL_MIME_FILTER_CRLF_MODE_CRLF_ONLY);
	camel_stream_filter_add (streamfilter, crlf_filter);
	camel_data_wrapper_write_to_stream (CAMEL_DATA_WRAPPER (message),
					    CAMEL_STREAM (streamfilter));
	camel_object_unref (CAMEL_OBJECT (streamfilter));
	camel_object_unref (CAMEL_OBJECT (crlf_filter));
	camel_object_unref (CAMEL_OBJECT (memstream));

	/* Some servers dont let us append with custom flags.  If the command fails for
	   whatever reason, assume this is the case and save the state and try again */
retry:
	if (info) {
		flags = camel_message_info_flags(info);
		if (!store->nocustomappend)
			flags |= imap_label_to_flags((CamelMessageInfo *)info);
	}

	flags &= folder->permanent_flags;
	if (flags)
		flagstr = imap_create_flag_list (flags);
	else
		flagstr = NULL;
	
	response = camel_imap_command (store, NULL, ex, "APPEND %F%s%s {%d}",
				       folder->full_name, flagstr ? " " : "",
				       flagstr ? flagstr : "", ba->len);
	g_free (flagstr);
	
	if (!response) {
		if (camel_exception_get_id(ex) == CAMEL_EXCEPTION_SERVICE_INVALID && !store->nocustomappend) {
			camel_exception_clear(ex);
			store->nocustomappend = 1;
			goto retry;
		}
		g_byte_array_free (ba, TRUE);
		return NULL;
	}

	if (*response->status != '+') {
		camel_imap_response_free (store, response);
		g_byte_array_free (ba, TRUE);
		return NULL;
	}
	
	/* send the rest of our data - the mime message */
	response2 = camel_imap_command_continuation (store, (const char *) ba->data, ba->len, ex);
	g_byte_array_free (ba, TRUE);

	/* free it only after message is sent. This may cause more FETCHes. */
	camel_imap_response_free (store, response);
	if (!response2)
		return response2;
	
	if (store->capabilities & IMAP_CAPABILITY_UIDPLUS) {
		*uid = camel_strstrcase (response2->status, "[APPENDUID ");
		if (*uid)
			*uid = strchr (*uid + 11, ' ');
		if (*uid) {
			*uid = g_strndup (*uid + 1, strcspn (*uid + 1, "]"));
			/* Make sure it's a number */
			if (strtoul (*uid, &end, 10) == 0 || *end) {
				g_free (*uid);
				*uid = NULL;
			}
		}
	} else
		*uid = NULL;
	
	return response2;
}

static void
imap_append_online (CamelFolder *folder, CamelMimeMessage *message,
		    const CamelMessageInfo *info, char **appended_uid,
		    CamelException *ex)
{
	CamelImapStore *store = CAMEL_IMAP_STORE (folder->parent_store);
	CamelImapResponse *response;
	char *uid;
	int count;

	count = camel_folder_summary_count (folder->summary);
	response = do_append (folder, message, info, &uid, ex);
	if (!response)
		return;
	
	if (uid) {
		/* Cache first, since freeing response may trigger a
		 * summary update that will want this information.
		 */
		CAMEL_IMAP_FOLDER_REC_LOCK (folder, cache_lock);
		camel_imap_message_cache_insert_wrapper (
			CAMEL_IMAP_FOLDER (folder)->cache, uid,
			"", CAMEL_DATA_WRAPPER (message), ex);
		CAMEL_IMAP_FOLDER_REC_UNLOCK (folder, cache_lock);
		if (appended_uid)
			*appended_uid = uid;
		else
			g_free (uid);
	} else if (appended_uid)
		*appended_uid = NULL;
	
	camel_imap_response_free (store, response);
	
	/* Make sure a "folder_changed" is emitted. */
	CAMEL_SERVICE_REC_LOCK (store, connect_lock);
	if (store->current_folder != folder ||
	    camel_folder_summary_count (folder->summary) == count)
		imap_refresh_info (folder, ex);
	CAMEL_SERVICE_REC_UNLOCK (store, connect_lock);

	camel_imap_folder_start_idle (folder);

}

static void
imap_append_resyncing (CamelFolder *folder, CamelMimeMessage *message,
		       const CamelMessageInfo *info, char **appended_uid,
		       CamelException *ex)
{
	CamelImapStore *store = CAMEL_IMAP_STORE (folder->parent_store);
	CamelImapResponse *response;
	char *uid;
	
	response = do_append (folder, message, info, &uid, ex);
	if (!response)
		return;
	
	if (uid) {
		CamelImapFolder *imap_folder = CAMEL_IMAP_FOLDER (folder);
		const char *olduid = camel_message_info_uid (info);
		
		CAMEL_IMAP_FOLDER_REC_LOCK (imap_folder, cache_lock);
		camel_imap_message_cache_copy (imap_folder->cache, olduid,
					       imap_folder->cache, uid, ex);
		CAMEL_IMAP_FOLDER_REC_UNLOCK (imap_folder, cache_lock);

		if (appended_uid)
			*appended_uid = uid;
		else
			g_free (uid);
	} else if (appended_uid)
		*appended_uid = NULL;
	
	camel_imap_response_free (store, response);
}


static void
imap_transfer_offline (CamelFolder *source, GPtrArray *uids,
		       CamelFolder *dest, GPtrArray **transferred_uids,
		       gboolean delete_originals, CamelException *ex)
{
	CamelImapStore *store = CAMEL_IMAP_STORE (source->parent_store);
	CamelImapMessageCache *sc = CAMEL_IMAP_FOLDER (source)->cache;
	CamelImapMessageCache *dc = CAMEL_IMAP_FOLDER (dest)->cache;
	CamelFolderChangeInfo *changes;
	CamelMimeMessage *message;
	CamelMessageInfo *mi;
	char *uid, *destuid;
	int i;

	/* We grab the store's command lock first, and then grab the
	 * source and destination cache_locks. This way we can't
	 * deadlock in the case where we're simultaneously also trying
	 * to copy messages in the other direction from another thread.
	 */
	CAMEL_SERVICE_REC_LOCK (store, connect_lock);
	CAMEL_IMAP_FOLDER_REC_LOCK (source, cache_lock);
	CAMEL_IMAP_FOLDER_REC_LOCK (dest, cache_lock);
	CAMEL_SERVICE_REC_UNLOCK (store, connect_lock);

	if (transferred_uids) {
		*transferred_uids = g_ptr_array_new ();
		g_ptr_array_set_size (*transferred_uids, uids->len);
	}

	changes = camel_folder_change_info_new ();

	for (i = 0; i < uids->len; i++) {
		uid = uids->pdata[i];

		destuid = get_temp_uid ();

		mi = camel_folder_summary_uid (source->summary, uid);
		g_return_if_fail (mi != NULL);

		/* TNY TODO: detect whether or not to persist partial message retrieval */
		message = camel_folder_get_message (source, uid, CAMEL_FOLDER_RECEIVE_FULL, -1, NULL);

		if (message) {
			camel_imap_summary_add_offline (dest->summary, destuid, message, mi);
			camel_object_unref (CAMEL_OBJECT (message));
		} else
			camel_imap_summary_add_offline_uncached (dest->summary, destuid, mi);

		camel_imap_message_cache_copy (sc, uid, dc, destuid, ex);
		camel_message_info_free(mi);

		camel_folder_change_info_add_uid (changes, destuid);
		if (transferred_uids)
			(*transferred_uids)->pdata[i] = destuid;
		else
			g_free (destuid);

		if (delete_originals)
			camel_folder_delete_message (source, uid);
	}

	CAMEL_IMAP_FOLDER_REC_UNLOCK (dest, cache_lock);
	CAMEL_IMAP_FOLDER_REC_UNLOCK (source, cache_lock);

	camel_object_trigger_event (CAMEL_OBJECT (dest), "folder_changed", changes);
	camel_folder_change_info_free (changes);

	camel_disco_diary_log (CAMEL_DISCO_STORE (store)->diary,
			       CAMEL_DISCO_DIARY_FOLDER_TRANSFER,
			       source, dest, uids, delete_originals);
}

static void
handle_copyuid (CamelImapResponse *response, CamelFolder *source,
		CamelFolder *destination)
{
	CamelImapMessageCache *scache = CAMEL_IMAP_FOLDER (source)->cache;
	CamelImapMessageCache *dcache = CAMEL_IMAP_FOLDER (destination)->cache;
	char *validity, *srcset, *destset;
	GPtrArray *src, *dest;
	int i;

	validity = camel_strstrcase (response->status, "[COPYUID ");
	if (!validity)
		return;
	validity += 9;
	if (strtoul (validity, NULL, 10) !=
	    CAMEL_IMAP_SUMMARY (destination->summary)->validity)
		return;

	srcset = strchr (validity, ' ');
	if (!srcset++)
		goto lose;
	destset = strchr (srcset, ' ');
	if (!destset++)
		goto lose;

	src = imap_uid_set_to_array (source->summary, srcset);
	dest = imap_uid_set_to_array (destination->summary, destset);

	if (src && dest && src->len == dest->len) {
		/* We don't have to worry about deadlocking on the
		 * cache locks here, because we've got the store's
		 * command lock too, so no one else could be here.
		 */
		CAMEL_IMAP_FOLDER_REC_LOCK (source, cache_lock);
		CAMEL_IMAP_FOLDER_REC_LOCK (destination, cache_lock);
		for (i = 0; i < src->len; i++) {
			camel_imap_message_cache_copy (scache, src->pdata[i],
						       dcache, dest->pdata[i],
						       NULL);
		}
		CAMEL_IMAP_FOLDER_REC_UNLOCK (source, cache_lock);
		CAMEL_IMAP_FOLDER_REC_UNLOCK (destination, cache_lock);

		imap_uid_array_free (src);
		imap_uid_array_free (dest);
		return;
	}

	imap_uid_array_free (src);
	imap_uid_array_free (dest);
 lose:
	g_warning ("Bad COPYUID response from server");
}

static void
do_copy (CamelFolder *source, GPtrArray *uids,
	 CamelFolder *destination, int delete_originals, CamelException *ex)
{
	CamelImapStore *store = CAMEL_IMAP_STORE (source->parent_store);
	CamelImapResponse *response;
	char *uidset;
	int uid = 0, last=0, i;
	
	while (uid < uids->len && !camel_exception_is_set (ex)) {
		uidset = imap_uid_array_to_set (source->summary, uids, uid, UID_SET_LIMIT, &uid);

		if ((store->capabilities & IMAP_CAPABILITY_XGWMOVE) && delete_originals) {
			response = camel_imap_command (store, source, ex, "UID XGWMOVE %s %F", uidset, destination->full_name);
			/* TODO: EXPUNGE returns??? */
			camel_imap_response_free (store, response);
		} else {
			response = camel_imap_command (store, source, ex, "UID COPY %s %F", uidset, destination->full_name);
			if (response && (store->capabilities & IMAP_CAPABILITY_UIDPLUS))
				handle_copyuid (response, source, destination);
			camel_imap_response_free (store, response);

			if (!camel_exception_is_set(ex) && delete_originals) {
				for (i=last;i<uid;i++)
					camel_folder_delete_message(source, uids->pdata[i]);
				last = uid;
			}
		}
		g_free (uidset);
	}
}

static void
imap_transfer_online (CamelFolder *source, GPtrArray *uids,
		      CamelFolder *dest, GPtrArray **transferred_uids,
		      gboolean delete_originals, CamelException *ex)
{
	CamelImapStore *store = CAMEL_IMAP_STORE (source->parent_store);
	int count;

	/* Sync message flags if needed. */
	imap_sync_online (source, ex);
	if (camel_exception_is_set (ex))
		return;

	count = camel_folder_summary_count (dest->summary);
	
	qsort (uids->pdata, uids->len, sizeof (void *), uid_compar);
	
	/* Now copy the messages */
	do_copy(source, uids, dest, delete_originals, ex);
	if (camel_exception_is_set (ex))
		return;

	/* Make the destination notice its new messages */

	/*if (store->current_folder != dest ||
	    camel_folder_summary_count (dest->summary) == count)
		camel_folder_refresh_info (dest, ex); */

	/* FIXME */
	if (transferred_uids)
		*transferred_uids = NULL;

	camel_imap_folder_start_idle (source);

}

static void
imap_transfer_resyncing (CamelFolder *source, GPtrArray *uids,
			 CamelFolder *dest, GPtrArray **transferred_uids,
			 gboolean delete_originals, CamelException *ex)
{
	CamelDiscoDiary *diary = CAMEL_DISCO_STORE (source->parent_store)->diary;
	GPtrArray *realuids;
	int first, i;
	const char *uid;
	CamelMimeMessage *message;
	CamelMessageInfo *info;
	
	qsort (uids->pdata, uids->len, sizeof (void *), uid_compar);
	
	/* This is trickier than append_resyncing, because some of
	 * the messages we are copying may have been copied or
	 * appended into @source while we were offline, in which case
	 * if we don't have UIDPLUS, we won't know their real UIDs,
	 * so we'll have to append them rather than copying.
	 */

	realuids = g_ptr_array_new ();

	i = 0;
	while (i < uids->len) {
		/* Skip past real UIDs */
		for (first = i; i < uids->len; i++) {
			uid = uids->pdata[i];

			if (!isdigit ((unsigned char)*uid)) {
				uid = camel_disco_diary_uidmap_lookup (diary, uid);
				if (!uid)
					break;
			}
			g_ptr_array_add (realuids, (char *)uid);
		}

		/* If we saw any real UIDs, do a COPY */
		if (i != first) {
			do_copy (source, realuids, dest, delete_originals, ex);
			g_ptr_array_set_size (realuids, 0);
			if (i == uids->len || camel_exception_is_set (ex))
				break;
		}

		/* Deal with fake UIDs */
		while (i < uids->len &&
		       !isdigit (*(unsigned char *)(uids->pdata[i])) &&
		       !camel_exception_is_set (ex)) {
			uid = uids->pdata[i];

			/* TNY TODO: detect whether or not to persist partial message retrieval */
			message = camel_folder_get_message (source, uid, CAMEL_FOLDER_RECEIVE_FULL, -1, NULL);
			if (!message) {
				i++;
				/* Message must have been expunged */
				continue;
			}
			info = camel_folder_get_message_info (source, uid);
			g_return_if_fail (info != NULL);

			imap_append_online (dest, message, info, NULL, ex);
			camel_folder_free_message_info (source, info);
			camel_object_unref (CAMEL_OBJECT (message));
			if (delete_originals)
				camel_folder_delete_message (source, uid);
			i++;
		}
	}

	g_ptr_array_free (realuids, FALSE);

	/* FIXME */
	if (transferred_uids)
		*transferred_uids = NULL;
}

static GPtrArray *
imap_search_by_expression (CamelFolder *folder, const char *expression, CamelException *ex)
{
	CamelImapFolder *imap_folder = CAMEL_IMAP_FOLDER (folder);
	GPtrArray *matches;

	/* we could get around this by creating a new search object each time,
	   but i doubt its worth it since any long operation would lock the
	   command channel too */
	CAMEL_IMAP_FOLDER_LOCK(folder, search_lock);

	camel_folder_search_set_folder (imap_folder->search, folder);
	matches = camel_folder_search_search(imap_folder->search, expression, NULL, ex);

	CAMEL_IMAP_FOLDER_UNLOCK(folder, search_lock);

	return matches;
}

static GPtrArray *
imap_search_by_uids(CamelFolder *folder, const char *expression, GPtrArray *uids, CamelException *ex)
{
	CamelImapFolder *imap_folder = CAMEL_IMAP_FOLDER(folder);
	GPtrArray *matches;

	if (uids->len == 0)
		return g_ptr_array_new();

	CAMEL_IMAP_FOLDER_LOCK(folder, search_lock);

	camel_folder_search_set_folder(imap_folder->search, folder);
	matches = camel_folder_search_search(imap_folder->search, expression, uids, ex);

	CAMEL_IMAP_FOLDER_UNLOCK(folder, search_lock);

	return matches;
}

static void
imap_search_free (CamelFolder *folder, GPtrArray *uids)
{
	CamelImapFolder *imap_folder = CAMEL_IMAP_FOLDER (folder);

	g_return_if_fail (imap_folder->search);

	CAMEL_IMAP_FOLDER_LOCK(folder, search_lock);

	camel_folder_search_free_result (imap_folder->search, uids);

	CAMEL_IMAP_FOLDER_UNLOCK(folder, search_lock);
}

static CamelMimeMessage *get_message (CamelImapFolder *imap_folder,
				      const char *uid,
				      CamelMessageContentInfo *ci,
				      CamelFolderReceiveType type, gint param, CamelException *ex);

struct _part_spec_stack {
	struct _part_spec_stack *parent;
	int part;
};

static void
part_spec_push (struct _part_spec_stack **stack, int part)
{
	struct _part_spec_stack *node;
	
	node = g_new (struct _part_spec_stack, 1);
	node->parent = *stack;
	node->part = part;
	
	*stack = node;
}

static int
part_spec_pop (struct _part_spec_stack **stack)
{
	struct _part_spec_stack *node;
	int part;
	
	g_return_val_if_fail (*stack != NULL, 0);
	
	node = *stack;
	*stack = node->parent;
	
	part = node->part;
	g_free (node);
	
	return part;
}

static char *
content_info_get_part_spec (CamelMessageContentInfo *ci)
{
	struct _part_spec_stack *stack = NULL;
	CamelMessageContentInfo *node;
	char *part_spec, *buf;
	size_t len = 1;
	int part;
	
	node = ci;
	while (node->parent) {
		CamelMessageContentInfo *child;
		
		/* FIXME: is this only supposed to apply if 'node' is a multipart? */
		if (node->parent->parent && camel_content_type_is (node->parent->type, "message", "*")) {
			node = node->parent;
			continue;
		}
		
		child = node->parent->childs;
		for (part = 1; child; part++) {
			if (child == node)
				break;
			
			child = child->next;
		}
		
		part_spec_push (&stack, part);
		
		len += 2;
		while ((part = part / 10))
			len++;
		
		node = node->parent;
	}
	
	buf = part_spec = g_malloc (len);
	part_spec[0] = '\0';
	
	while (stack) {
		part = part_spec_pop (&stack);
		buf += sprintf (buf, "%d%s", part, stack ? "." : "");
	}
	
	return part_spec;
}

/* Fetch the contents of the MIME part indicated by @ci, which is part
 * of message @uid in @folder.
 */
static CamelDataWrapper *
get_content (CamelImapFolder *imap_folder, const char *uid,
	     CamelMimePart *part, CamelMessageContentInfo *ci,
	     int frommsg,
	     CamelException *ex)
{
	CamelDataWrapper *content = NULL;
	CamelStream *stream;
	char *part_spec;
	
	part_spec = content_info_get_part_spec (ci);

	d(printf("get content '%s' '%s' (frommsg = %d)\n", part_spec, camel_content_type_format(ci->type), frommsg));

	/* There are three cases: multipart/signed, multipart, message/rfc822, and "other" */
	if (camel_content_type_is (ci->type, "multipart", "signed")) {
		CamelMultipartSigned *body_mp;
		char *spec;
		int ret;
		
		/* Note: because we get the content parts uninterpreted anyway, we could potentially
		   just use the normalmultipart code, except that multipart/signed wont let you yet! */
		
		body_mp = camel_multipart_signed_new ();
		/* need to set this so it grabs the boundary and other info about the signed type */
		/* we assume that part->content_type is more accurate/full than ci->type */
		camel_data_wrapper_set_mime_type_field (CAMEL_DATA_WRAPPER (body_mp), CAMEL_DATA_WRAPPER (part)->mime_type);
		
		spec = g_alloca(strlen(part_spec) + 6);
		if (frommsg)
			sprintf(spec, part_spec[0] ? "%s.TEXT" : "TEXT", part_spec);
		else
			strcpy(spec, part_spec);
		g_free(part_spec);
		
		/* TNY TODO: partial message retrieval exception */
		stream = camel_imap_folder_fetch_data (imap_folder, uid, spec, FALSE, CAMEL_FOLDER_RECEIVE_FULL, -1, ex);
		if (stream) {
			ret = camel_data_wrapper_construct_from_stream (CAMEL_DATA_WRAPPER (body_mp), stream);
			camel_object_unref (CAMEL_OBJECT (stream));
			if (ret == -1) {
				camel_object_unref ((CamelObject *) body_mp);
				return NULL;
			}
		}
		
		return (CamelDataWrapper *) body_mp;
	} else if (camel_content_type_is (ci->type, "multipart", "*")) {
		CamelMultipart *body_mp;
		char *child_spec;
		int speclen, num, isdigest;
		
		if (camel_content_type_is (ci->type, "multipart", "encrypted"))
			body_mp = (CamelMultipart *) camel_multipart_encrypted_new ();
		else
			body_mp = camel_multipart_new ();
		
		/* need to set this so it grabs the boundary and other info about the multipart */
		/* we assume that part->content_type is more accurate/full than ci->type */
		camel_data_wrapper_set_mime_type_field (CAMEL_DATA_WRAPPER (body_mp), CAMEL_DATA_WRAPPER (part)->mime_type);
		isdigest = camel_content_type_is(((CamelDataWrapper *)part)->mime_type, "multipart", "digest");
		
		speclen = strlen (part_spec);
		child_spec = g_malloc (speclen + 17); /* dot + 10 + dot + MIME + nul */
		memcpy (child_spec, part_spec, speclen);
		if (speclen > 0)
			child_spec[speclen++] = '.';
		g_free (part_spec);
		
		ci = ci->childs;
		num = 1;
		while (ci) {
			sprintf (child_spec + speclen, "%d.MIME", num++);
			/* TNY TODO: partial message retrieval exception */
			stream = camel_imap_folder_fetch_data (imap_folder, uid, child_spec, TRUE, CAMEL_FOLDER_RECEIVE_FULL, -1, ex);
			if (stream) {
				int ret;
				
				part = camel_mime_part_new ();
				ret = camel_data_wrapper_construct_from_stream (CAMEL_DATA_WRAPPER (part), stream);
				camel_object_unref (CAMEL_OBJECT (stream));
				if (ret == -1) {
					camel_object_unref (CAMEL_OBJECT (part));
					camel_object_unref (CAMEL_OBJECT (body_mp));
					g_free (child_spec);
					return NULL;
				}
				
				content = get_content (imap_folder, uid, part, ci, FALSE, ex);
			}
			
			if (!stream || !content) {
				camel_object_unref (CAMEL_OBJECT (body_mp));
				g_free (child_spec);
				return NULL;
			}

			if (camel_debug("imap:folder")) {
				char *ct = camel_content_type_format(camel_mime_part_get_content_type((CamelMimePart *)part));
				char *ct2 = camel_content_type_format(ci->type);

				printf("Setting part content type to '%s' contentinfo type is '%s'\n", ct, ct2);
				g_free(ct);
				g_free(ct2);
			}

			/* if we had no content-type header on a multipart/digest sub-part, then we need to
			   treat it as message/rfc822 instead */
			if (isdigest && camel_medium_get_header((CamelMedium *)part, "content-type") == NULL) {
				CamelContentType *ct = camel_content_type_new("message", "rfc822");

				camel_data_wrapper_set_mime_type_field(content, ct);
				camel_content_type_unref(ct);
			} else {
				camel_data_wrapper_set_mime_type_field(content, camel_mime_part_get_content_type(part));
			}

			camel_medium_set_content_object (CAMEL_MEDIUM (part), content);
			camel_object_unref(content);

			camel_multipart_add_part (body_mp, part);
			camel_object_unref(part);
			
			ci = ci->next;
		}
		
		g_free (child_spec);
		
		return (CamelDataWrapper *) body_mp;
	} else if (camel_content_type_is (ci->type, "message", "rfc822")) {
		/* TNY TODO: partial message retrieval exception */
		content = (CamelDataWrapper *) get_message (imap_folder, uid, ci->childs, CAMEL_FOLDER_RECEIVE_FULL, -1, ex);
		g_free (part_spec);
		return content;
	} else {
		CamelTransferEncoding enc;
		char *spec;

		/* NB: we need this differently to multipart/signed case above on purpose */
		spec = g_alloca(strlen(part_spec) + 6);
		if (frommsg)
			sprintf(spec, part_spec[0] ? "%s.1" : "1", part_spec);
		else
			strcpy(spec, part_spec[0]?part_spec:"1");

		enc = ci->encoding?camel_transfer_encoding_from_string(ci->encoding):CAMEL_TRANSFER_ENCODING_DEFAULT;
		content = camel_imap_wrapper_new (imap_folder, ci->type, enc, uid, spec, part);
		g_free (part_spec);
		return content;
	}
}

static CamelMimeMessage *
get_message (CamelImapFolder *imap_folder, const char *uid,
	     CamelMessageContentInfo *ci,
	     CamelFolderReceiveType type, gint param, CamelException *ex)
{
	CamelImapStore *store = CAMEL_IMAP_STORE (CAMEL_FOLDER (imap_folder)->parent_store);
	CamelDataWrapper *content;
	CamelMimeMessage *msg;
	CamelStream *stream;
	char *section_text, *part_spec;
	int ret;

	part_spec = content_info_get_part_spec(ci);
	d(printf("get message '%s'\n", part_spec));
	section_text = g_strdup_printf ("%s%s%s", part_spec, *part_spec ? "." : "",
					store->server_level >= IMAP_LEVEL_IMAP4REV1 ? "HEADER" : "0");

	/* TNY: partial message retrieval */
	stream = camel_imap_folder_fetch_data (imap_folder, uid, section_text, FALSE, type, param, ex);
	g_free (section_text);
	g_free(part_spec);
	if (!stream)
		return NULL;

	msg = camel_mime_message_new ();
	ret = camel_data_wrapper_construct_from_stream (CAMEL_DATA_WRAPPER (msg), stream);
	camel_object_unref (CAMEL_OBJECT (stream));
	if (ret == -1) {
		camel_object_unref (CAMEL_OBJECT (msg));
		return NULL;
	}
	
	content = get_content (imap_folder, uid, CAMEL_MIME_PART (msg), ci, TRUE, ex);
	if (!content) {
		camel_object_unref (CAMEL_OBJECT (msg));
		return NULL;
	}

	if (camel_debug("imap:folder")) {
		char *ct = camel_content_type_format(camel_mime_part_get_content_type((CamelMimePart *)msg));
		char *ct2 = camel_content_type_format(ci->type);

		printf("Setting message content type to '%s' contentinfo type is '%s'\n", ct, ct2);
		g_free(ct);
		g_free(ct2);
	}

	camel_data_wrapper_set_mime_type_field(content, camel_mime_part_get_content_type((CamelMimePart *)msg));
	camel_medium_set_content_object (CAMEL_MEDIUM (msg), content);
	camel_object_unref (CAMEL_OBJECT (content));
	
	return msg;
}

#define IMAP_SMALL_BODY_SIZE 5120

static CamelMimeMessage *
get_message_simple (CamelImapFolder *imap_folder, const char *uid,
		    CamelStream *stream, CamelFolderReceiveType type, gint param, CamelException *ex)
{
	CamelMimeMessage *msg;
	int ret;
	
	if (!stream) {
		stream = camel_imap_folder_fetch_data (imap_folder, uid, "",
						       FALSE, type, param, ex);
		if (!stream)
			return NULL;
	}

	msg = camel_mime_message_new ();
	ret = camel_data_wrapper_construct_from_stream (CAMEL_DATA_WRAPPER (msg),
							stream);
	camel_object_unref (CAMEL_OBJECT (stream));
	if (ret == -1) {
		camel_exception_setv (ex, CAMEL_EXCEPTION_SERVICE_UNAVAILABLE,
				      _("Unable to retrieve message: %s"),
				      g_strerror (errno));
		camel_object_unref (CAMEL_OBJECT (msg));
		return NULL;
	}

	return msg;
}

static gboolean
content_info_incomplete (CamelMessageContentInfo *ci)
{
	if (!ci->type)
		return TRUE;
	
	if (camel_content_type_is (ci->type, "multipart", "*")
	    || camel_content_type_is (ci->type, "message", "rfc822")) {
		if (!ci->childs)
			return TRUE;
		for (ci = ci->childs;ci;ci=ci->next)
			if (content_info_incomplete(ci))
				return TRUE;
	}

	return FALSE;
}

static CamelMimeMessage *
imap_get_message (CamelFolder *folder, const char *uid, CamelFolderReceiveType type, gint param, CamelException *ex)
{
	CamelImapFolder *imap_folder = CAMEL_IMAP_FOLDER (folder);
	CamelImapStore *store = CAMEL_IMAP_STORE (folder->parent_store);
	CamelImapMessageInfo *mi;
	CamelMimeMessage *msg = NULL;
	CamelStream *stream = NULL;
	int retry;

	mi = (CamelImapMessageInfo *)camel_folder_summary_uid (folder->summary, uid);
	if (mi == NULL) {
		camel_exception_setv(ex, CAMEL_EXCEPTION_FOLDER_INVALID_UID,
				     _("Cannot get message: %s\n  %s"), uid, _("No such message"));
		return NULL;
	}

	/* If its cached in full, just get it as is, this is only a shortcut,
	   since we get stuff from the cache anyway.  It affects a busted connection though. */
	if ( (stream = camel_imap_folder_fetch_data (imap_folder, uid, "", TRUE, type, param, NULL))
	     && (msg = get_message_simple(imap_folder, uid, stream, type, param, ex)))
		goto done;


	if (camel_disco_store_status (CAMEL_DISCO_STORE (folder->parent_store)) == CAMEL_DISCO_STORE_OFFLINE)
		goto done;

	/* All this mess is so we silently retry a fetch if we fail with
	   service_unavailable, without an (equivalent) mess of gotos */
	retry = 0;
	do {
		retry++;
		camel_exception_clear(ex);

		/* If the message is small or only 1 part, or server doesn't do 4v1 (properly) fetch it in one piece. */
		if (store->server_level < IMAP_LEVEL_IMAP4REV1
		    || store->braindamaged
		    || mi->info.size < IMAP_SMALL_BODY_SIZE
		    || (!content_info_incomplete(mi->info.content) && !mi->info.content->childs)) {
			msg = get_message_simple (imap_folder, uid, NULL, type, param, ex);
		} else {
			if (content_info_incomplete (mi->info.content)) {
				/* For larger messages, fetch the structure and build a message
				 * with offline parts. (We check mi->content->type rather than
				 * mi->content because camel_folder_summary_info_new always creates
				 * an empty content struct.)
				 */
				CamelImapResponse *response;
				GData *fetch_data = NULL;
				char *body, *found_uid;
				int i;
				
				CAMEL_SERVICE_REC_LOCK(store, connect_lock);
				if (!camel_disco_store_check_online ((CamelDiscoStore*)store, ex)) {
					CAMEL_SERVICE_REC_UNLOCK(store, connect_lock);
					camel_exception_set (ex, CAMEL_EXCEPTION_SERVICE_UNAVAILABLE,
							     _("This message is not currently available"));
					goto fail;
				}
				
				response = camel_imap_command (store, folder, ex, "UID FETCH %s BODY", uid);
				CAMEL_SERVICE_REC_UNLOCK(store, connect_lock);

				if (response) {
					for (i = 0, body = NULL; i < response->untagged->len; i++) {
						fetch_data = parse_fetch_response (imap_folder, response->untagged->pdata[i]);
						if (fetch_data) {
							found_uid = g_datalist_get_data (&fetch_data, "UID");
							body = g_datalist_get_data (&fetch_data, "BODY");
							if (found_uid && body && !strcmp (found_uid, uid))
								break;
							g_datalist_clear (&fetch_data);
							fetch_data = NULL;
							body = NULL;
						}
					}
					
					if (body) {
						/* NB: small race here, setting the info.content */
						imap_parse_body ((const char **) &body, folder, mi->info.content);
						camel_folder_summary_touch (folder->summary);
					}

					if (fetch_data)
						g_datalist_clear (&fetch_data);
					
					camel_imap_response_free (store, response);
				} else {
					/* camel_exception_clear(ex); */
					goto fail;
				}
			}

			if (camel_debug_start("imap:folder")) {
				printf("Folder get message '%s' folder info ->\n", uid);
				camel_message_info_dump((CamelMessageInfo *)mi);
				camel_debug_end();
			}
			
			/* FETCH returned OK, but we didn't parse a BODY
			 * response. Courier will return invalid BODY
			 * responses for invalidly MIMEd messages, so
			 * fall back to fetching the entire thing and
			 * let the mailer's "bad MIME" code handle it.
			 */
			if (content_info_incomplete (mi->info.content))
				msg = get_message_simple (imap_folder, uid, NULL, type, param, ex);
			else
				msg = get_message (imap_folder, uid, mi->info.content, type, param, ex);
		}
	} while (msg == NULL
		 && retry < 2
		 && camel_exception_get_id(ex) == CAMEL_EXCEPTION_SERVICE_UNAVAILABLE);

done:
	camel_message_info_free(&mi->info);
	return msg;

fail:
	camel_message_info_free(&mi->info);
	if (msg && CAMEL_IS_OBJECT (msg))
		camel_object_unref (CAMEL_OBJECT (msg));
	
	return NULL;
}

static void
imap_cache_message (CamelDiscoFolder *disco_folder, const char *uid,
		    CamelException *ex)
{
	CamelImapFolder *imap_folder = CAMEL_IMAP_FOLDER (disco_folder);
	CamelStream *stream;

	/* TNY TODO: partial message retrieval exception */
	stream = camel_imap_folder_fetch_data (imap_folder, uid, "", FALSE, CAMEL_FOLDER_RECEIVE_FULL, -1, ex);
	if (stream)
		camel_object_unref (CAMEL_OBJECT (stream));
}

/* We pretend that a FLAGS or RFC822.SIZE response is always exactly
 * 20 bytes long, and a BODY[HEADERS] response is always 2000 bytes
 * long. Since we know how many of each kind of response we're
 * expecting, we can find the total (pretend) amount of server traffic
 * to expect and then count off the responses as we read them to update
 * the progress bar.
 */
#define IMAP_PRETEND_SIZEOF_FLAGS	  20
#define IMAP_PRETEND_SIZEOF_SIZE	  20
#define IMAP_PRETEND_SIZEOF_HEADERS	2000

static char *tm_months[] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

static gboolean
decode_time (const unsigned char **in, int *hour, int *min, int *sec)
{
	register const unsigned char *inptr;
	int *val, colons = 0;
	
	*hour = *min = *sec = 0;
	
	val = hour;
	for (inptr = *in; *inptr && !isspace ((int) *inptr); inptr++) {
		if (*inptr == ':') {
			colons++;
			switch (colons) {
			case 1:
				val = min;
				break;
			case 2:
				val = sec;
				break;
			default:
				return FALSE;
			}
		} else if (!isdigit ((int) *inptr))
			return FALSE;
		else
			*val = (*val * 10) + (*inptr - '0');
	}
	
	*in = inptr;
	
	return TRUE;
}

time_t
decode_internaldate (const unsigned char *in)
{
	const unsigned char *inptr = in;
	int hour, min, sec, n;
	unsigned char *buf;
	struct tm tm;
	time_t date;
	
	memset ((void *) &tm, 0, sizeof (struct tm));
	
	tm.tm_mday = strtoul ((char *) inptr, (char **) &buf, 10);
	if (buf == inptr || *buf != '-')
		return (time_t) -1;
	
	inptr = buf + 1;
	if (inptr[3] != '-')
		return (time_t) -1;
	
	for (n = 0; n < 12; n++) {
		if (!g_ascii_strncasecmp ((gchar *) inptr, tm_months[n], 3))
			break;
	}
	
	if (n >= 12)
		return (time_t) -1;
	
	tm.tm_mon = n;
	
	inptr += 4;
	
	n = strtoul ((char *) inptr, (char **) &buf, 10);
	if (buf == inptr || *buf != ' ')
		return (time_t) -1;
	
	tm.tm_year = n - 1900;
	
	inptr = buf + 1;
	if (!decode_time (&inptr, &hour, &min, &sec))
		return (time_t) -1;
	
	tm.tm_hour = hour;
	tm.tm_min = min;
	tm.tm_sec = sec;
	
	n = strtol ((char *) inptr, NULL, 10);
	
	date = e_mktime_utc (&tm);
	
	/* date is now GMT of the time we want, but not offset by the timezone ... */
	
	/* this should convert the time to the GMT equiv time */
	date -= ((n / 100) * 60 * 60) + (n % 100) * 60;
	
	return date;
}


static CamelImapMessageInfo*
message_from_data (CamelFolder *folder, GData *data)
{
	CamelMimeMessage *msg;
	CamelStream *stream;
	CamelImapMessageInfo *mi;
	const char *idate;
	gint size = 0;

	stream = g_datalist_get_data (&data, "BODY_PART_STREAM");
	if (!stream)
		return NULL;

	msg = camel_mime_message_new ();
	if (camel_data_wrapper_construct_from_stream (CAMEL_DATA_WRAPPER (msg), stream) == -1) {
		camel_object_unref (CAMEL_OBJECT (msg));
		return NULL;
	}

	mi = (CamelImapMessageInfo *)camel_folder_summary_info_new_from_message (folder->summary, msg);
	camel_object_unref (CAMEL_OBJECT (msg));

	size = GPOINTER_TO_INT (g_datalist_get_data (&data, "RFC822.SIZE"));
	if (size)
		mi->info.size = size;

	/* TNY TODO: This is a hack! But else we need to parse 
	 * BODYSTRUCTURE (and I'm lazy). It needs fixing though. */
	if (size > 102400)
		((CamelMessageInfoBase *)mi)->flags |= CAMEL_MESSAGE_ATTACHMENTS;
	/* ... it does */

	if ((idate = g_datalist_get_data (&data, "INTERNALDATE")))
		mi->info.date_received = decode_internaldate ((const unsigned char *) idate);
	
	if (mi->info.date_received == -1)
		mi->info.date_received = mi->info.date_sent;

	return mi;
}

static void
add_message_from_data (CamelFolder *folder, GPtrArray *messages,
		       int first, GData *data)
{
	CamelMimeMessage *msg;
	CamelStream *stream;
	CamelImapMessageInfo *mi;
	const char *idate;
	int seq;
	
	seq = GPOINTER_TO_INT (g_datalist_get_data (&data, "SEQUENCE"));
	if (seq < first)
		return;
	stream = g_datalist_get_data (&data, "BODY_PART_STREAM");
	if (!stream)
		return;
	
	if (seq - first >= messages->len)
		g_ptr_array_set_size (messages, seq - first + 1);
	
	msg = camel_mime_message_new ();
	if (camel_data_wrapper_construct_from_stream (CAMEL_DATA_WRAPPER (msg), stream) == -1) {
		camel_object_unref (CAMEL_OBJECT (msg));
		return;
	}
	
	mi = (CamelImapMessageInfo *)camel_folder_summary_info_new_from_message (folder->summary, msg);
	
	camel_object_unref (CAMEL_OBJECT (msg));
	
	if ((idate = g_datalist_get_data (&data, "INTERNALDATE")))
		mi->info.date_received = decode_internaldate ((const unsigned char *) idate);
	
	if (mi->info.date_received == -1)
		mi->info.date_received = mi->info.date_sent;

	messages->pdata[seq - first] = mi;

	return;
}

/* #define CAMEL_MESSAGE_INFO_HEADERS "DATE FROM TO CC SUBJECT REFERENCES IN-REPLY-TO MESSAGE-ID MIME-VERSION CONTENT-TYPE " */

#ifdef NON_TINYMAIL_FEATURES
#define CAMEL_MESSAGE_INFO_HEADERS "DATE FROM TO CC SUBJECT REFERENCES IN-REPLY-TO MESSAGE-ID MIME-VERSION CONTENT-TYPE X-PRIORITY X-MSMAIL-PRIORITY"
#else
#define CAMEL_MESSAGE_INFO_HEADERS "DATE FROM TO CC SUBJECT MESSAGE-ID X-PRIORITY X-MSMAIL-PRIORITY X-MS-HAS-ATTACH CONTENT-TYPE"
#endif


/* FIXME: this needs to be kept in sync with camel-mime-utils.c's list
   of mailing-list headers and so might be best if this were
   auto-generated? */
#define MAILING_LIST_HEADERS " X-MAILING-LIST X-LOOP LIST-ID LIST-POST MAILING-LIST ORIGINATOR X-LIST SENDER RETURN-PATH X-BEENTHERE"


static guint32 
imap_get_uids (CamelFolder *folder, CamelImapStore *store, CamelException *ex, GPtrArray *needheaders, int size)
{
	char *resp = NULL;
	CamelImapResponseType type;
	guint32 cnt = 0, slen;
	CamelImapFolder *imap_folder = CAMEL_IMAP_FOLDER (folder);
	gchar *uid, *str;

	while ((type = camel_imap_command_response (store, &resp, ex)) ==
			CAMEL_IMAP_RESPONSE_UNTAGGED) 
	{
		str = strstr (resp, "SEARCH");

		if (str)
		{
			str+=7;
			uid = strtok (str, " ");

			while (uid != NULL)
			{
				if (uid)
					g_ptr_array_add (needheaders, g_strdup (uid));
				uid = strtok (NULL, " ");
				cnt++;
			}
		}

		g_free (resp); 
		resp=NULL;
	}
	if (type == CAMEL_IMAP_RESPONSE_TAGGED && resp)
		g_free (resp);

	if (type == CAMEL_IMAP_RESPONSE_ERROR) 
	{
		if (resp) 
			g_free (resp);
		camel_exception_set (ex, CAMEL_EXCEPTION_USER_CANCEL,
				_("Connection canceled"));
		cnt = 0;
	}

	return cnt;

}

static void
imap_update_summary (CamelFolder *folder, int exists,
		     CamelFolderChangeInfo *changes,
		     CamelException *ex)
{
   CamelImapStore *store = CAMEL_IMAP_STORE (folder->parent_store);
   CamelImapFolder *imap_folder = CAMEL_IMAP_FOLDER (folder);
   GPtrArray *needheaders;
   guint32 flags;
   int seq=0;
   CamelImapResponseType type;
   const char *header_spec;
   CamelImapMessageInfo *mi;
   char *resp;
   GData *data;
   gboolean more = TRUE, oosync = FALSE, oldrescval = imap_folder->need_rescan;
   unsigned int nextn, cnt=0, tcnt=0, ucnt=0, rec=0, ineed = 0, allhdrs = 0;

   if (!store->ostream || !store->istream)
	return;

   if (store->server_level >= IMAP_LEVEL_IMAP4REV1)
   	header_spec = "HEADER.FIELDS (" CAMEL_MESSAGE_INFO_HEADERS ")";
   else
   	header_spec = "0";

   if( g_getenv ("TNY_IMAP_FETCH_ALL_HEADERS") )
   	header_spec = "HEADER";

   nextn = 0;
   if (folder->summary)
   	nextn = camel_folder_summary_count (folder->summary);
   if (nextn <= 0) {
   	camel_folder_summary_load (folder->summary);
   	nextn = camel_folder_summary_count (folder->summary);
   }

   ineed = (exists - nextn);
   nextn = 1;
   tcnt = 0;

   camel_operation_start (NULL, _("Fetching summary information for new messages in folder"));

   camel_folder_summary_prepare_hash (folder->summary);

   store->dontdistridlehack = TRUE;

   while (more)
   {
	gboolean did_hack = FALSE;
	gint hcnt = 0;
	CamelFolderChangeInfo *mchanges;

	imap_folder->need_rescan = TRUE;
	camel_folder_summary_save (folder->summary);
	seq = camel_folder_summary_count (folder->summary);

	if (!camel_imap_command_start (store, folder, ex,
		"UID SEARCH %d:%d ALL", seq + 1, seq + 1 + nextn)) 
		{ if (!camel_operation_cancel_check (NULL))
			g_warning ("IMAP error getting UIDs (1)"); 
		 camel_operation_end (NULL); return; }

	more = FALSE; 
	needheaders = g_ptr_array_new ();
	cnt = imap_get_uids (folder, store, ex, needheaders, (exists - seq));

	if (cnt == 0 && camel_exception_get_id (ex) == CAMEL_EXCEPTION_USER_CANCEL)
	{
		if (!camel_operation_cancel_check (NULL))
			g_warning ("IMAP error getting UIDs (1,1)");

		g_ptr_array_foreach (needheaders, (GFunc)g_free, NULL);
		g_ptr_array_free (needheaders, TRUE);
		camel_operation_end (NULL);
		more = FALSE;
		camel_folder_summary_kill_hash (folder->summary);
		store->dontdistridlehack = FALSE;
		return;
	}

	tcnt += cnt;

	/* Figure out whether we need more */
	more = (cnt < (exists - seq));
	/* If we received less than what we asked for, yet need more */
	if ((cnt < nextn) && more)
	{
		if (!camel_imap_command_start (store, folder, ex,
			"UID SEARCH %d:* ALL", seq + 1 + cnt)) 
			{ if (!camel_operation_cancel_check (NULL))
				g_warning ("IMAP error getting UIDs (2)"); 
			  camel_operation_end (NULL); 
			  store->dontdistridlehack = FALSE; return; }
		cnt = imap_get_uids (folder, store, ex, needheaders, (exists - seq) - cnt);

		if (cnt == 0 && camel_exception_get_id (ex) == CAMEL_EXCEPTION_USER_CANCEL)
		{
			if (!camel_operation_cancel_check (NULL))
				g_warning ("IMAP error getting UIDs (2,1)");

			g_ptr_array_foreach (needheaders, (GFunc)g_free, NULL);
			g_ptr_array_free (needheaders, TRUE);
			camel_operation_end (NULL);
			more = FALSE;
			camel_folder_summary_kill_hash (folder->summary);
			store->dontdistridlehack = FALSE;
			return;
		}

		tcnt += cnt;
		/* If we still received too few */
		if (tcnt < (exists - seq))
		{
			g_ptr_array_foreach (needheaders, (GFunc)g_free, NULL);
			g_ptr_array_free (needheaders, TRUE);
			needheaders = g_ptr_array_new ();
			if (!camel_imap_command_start (store, folder, ex,
				"UID SEARCH 1:* ALL"))
				{ if (!camel_operation_cancel_check (NULL)) 
					g_warning ("IMAP error getting UIDs (3)");
					camel_folder_summary_kill_hash (folder->summary);
				  camel_operation_end (NULL); 
				  store->dontdistridlehack = FALSE; return; }
			camel_folder_summary_clear (folder->summary);
			tcnt = cnt = imap_get_uids (folder, store, ex, needheaders, (exists - seq) - tcnt);

			if (cnt == 0 && camel_exception_get_id (ex) == CAMEL_EXCEPTION_USER_CANCEL)
			{
				if (!camel_operation_cancel_check (NULL))
					g_warning ("IMAP error getting UIDs (3,1)");

				g_ptr_array_foreach (needheaders, (GFunc)g_free, NULL);
				g_ptr_array_free (needheaders, TRUE);
				camel_operation_end (NULL);
				more = FALSE;
				camel_folder_summary_kill_hash (folder->summary);
				store->dontdistridlehack = FALSE;
				return;
			}
		}
		camel_operation_end (NULL);
		more = FALSE;
		did_hack = TRUE;
	}

	nextn = (nextn < 1000) ? nextn+nextn+25 : 1000;

	if (needheaders->len) 
	{
		int uid = 0;
		char *uidset;

		ucnt = 0;
		qsort (needheaders->pdata, needheaders->len,
			sizeof (void *), uid_compar);

		mchanges = camel_folder_change_info_new ();

		while (uid < needheaders->len) 
		{
			uidset = imap_uid_array_to_set (folder->summary, needheaders, uid, UID_SET_LIMIT, &uid);
			if (!camel_imap_command_start (store, folder, ex,
						       "UID FETCH %s (FLAGS RFC822.SIZE INTERNALDATE BODY.PEEK[%s])",
						       uidset, header_spec)) 
			{
				if (!camel_operation_cancel_check (NULL))
					g_warning ("IMAP error getting headers (1)");
				g_ptr_array_foreach (needheaders, (GFunc)g_free, NULL);
				g_ptr_array_free (needheaders, TRUE);
				camel_operation_end (NULL);
				g_free (uidset);
				more = FALSE;
				camel_folder_summary_kill_hash (folder->summary);
				store->dontdistridlehack = FALSE;
				return;
			}
			g_free (uidset);

			resp = NULL;
			while ((type = camel_imap_command_response (store, &resp, ex))
				== CAMEL_IMAP_RESPONSE_UNTAGGED) 
			{
				gchar *muid;
				guint32 sequence, curlen;

				data = parse_fetch_response (imap_folder, resp);
				g_free (resp); resp = NULL;

				if (!data)
					continue;

				mi = message_from_data (folder, data);

				if (mi) 
				{
				  flags = GPOINTER_TO_INT (g_datalist_get_data (&data, "FLAGS"));
				  if (flags) 
				  {
					mi->server_flags = flags;
					mi->info.flags |= flags;
					/* flags_to_label(folder, mi); */
				  }

				  muid = g_datalist_get_data (&data, "UID");
				  if (muid) 
				  {
					if (mi->info.uid && mi->info.flags & CAMEL_MESSAGE_INFO_UID_NEEDS_FREE)
						g_free (mi->info.uid);
					mi->info.uid = g_strdup (muid);
					mi->info.flags |= CAMEL_MESSAGE_INFO_UID_NEEDS_FREE;
				  }

				  ucnt++;

				  allhdrs++;
				  camel_operation_progress (NULL, allhdrs , ineed);
				  sequence = GPOINTER_TO_INT (g_datalist_get_data (&data, "SEQUENCE"));
				  curlen = camel_folder_summary_count (folder->summary);

				  if (sequence > 0 && sequence <= exists && sequence != curlen)
				  {
					int r;
					if (curlen > sequence)
					{
						for (r = curlen-1; r >= sequence -1; r--)
						{ 
							g_warning ("Problem with your local summary store (too much), correcting: curlen=%d, r=%d, seq=%d\n", curlen, r, sequence);
							CamelMessageInfo *ri = g_ptr_array_index (folder->summary->messages, r);
							if (ri) {
								/* camel_folder_change_info_remove_uid (mchange, camel_message_info_uid (mi)); */
								((CamelMessageInfoBase*)ri)->flags |= CAMEL_MESSAGE_EXPUNGED;
								((CamelMessageInfoBase*)ri)->flags |= CAMEL_MESSAGE_FREED;
								camel_folder_summary_remove (folder->summary, ri);
							}
						}
					} else {
						for (r=0; r < sequence - curlen - 1; r++)
						{
							CamelMessageInfo *ni = camel_message_info_clone (mi);
							if (ni) {
								g_warning ("Problem with your local summary store (too few), correcting: curlen=%d, r=%d, seq=%d\n", curlen, r, sequence);
								camel_folder_summary_add (folder->summary, (CamelMessageInfo *)ni);
								/* camel_folder_change_info_add_uid (mchanges, camel_message_info_uid (ni)); */
							}
						}
					}
					oosync = TRUE;
				  }

				  camel_folder_summary_add (folder->summary, (CamelMessageInfo *)mi);

				  /* printf ("Change: %s!\n", camel_message_info_uid (mi)); */
				  camel_folder_change_info_add_uid (mchanges, camel_message_info_uid (mi));
				   if ((mi->info.flags & CAMEL_IMAP_MESSAGE_RECENT))
					camel_folder_change_info_recent_uid(mchanges, camel_message_info_uid (mi));
				}

				if (did_hack) {
					hcnt++;
					if (hcnt > 1000) {
						camel_folder_summary_save (folder->summary);
						camel_folder_summary_prepare_hash (folder->summary);
						hcnt = 0;
					}
				}

				g_datalist_clear (&data);
			}

			if (resp != NULL) 
				g_free (resp);

			if (type == CAMEL_IMAP_RESPONSE_ERROR) 
			{
				if (!camel_operation_cancel_check (NULL))
					g_warning ("IMAP error getting headers (2)");
				g_ptr_array_foreach (needheaders, (GFunc)g_free, NULL);
				g_ptr_array_free (needheaders, TRUE);
				camel_operation_end (NULL);
				more = FALSE;
				camel_folder_summary_kill_hash (folder->summary);
				store->dontdistridlehack = FALSE;
				return;
			}

		}

		
		if (camel_folder_change_info_changed (mchanges)) {
			camel_object_trigger_event (CAMEL_OBJECT (folder), "folder_changed", mchanges);
			/* printf ("Changes!\n"); */
		}
		camel_folder_change_info_free (mchanges);

	}

	if (ucnt < needheaders->len)
		g_warning ("Problem while receiving headers from IMAP "
			"(I didn't receive enough headers %d vs %d)\n",
			ucnt, needheaders->len);

	g_ptr_array_foreach (needheaders, (GFunc)g_free, NULL);
	g_ptr_array_free (needheaders, TRUE);

   } /* more */

   if (oosync)
	imap_folder->need_rescan = TRUE;

   camel_folder_summary_save (folder->summary);
   camel_folder_summary_kill_hash (folder->summary);
   camel_operation_end (NULL);

   imap_folder->need_rescan = oldrescval;
   store->dontdistridlehack = FALSE;

}

typedef struct {
	guint32 id;
	guint32 flags;
} FetchIdleResponse;

typedef struct {
	guint32 exists;
	guint32 recent;
	GArray *expunged;
	GPtrArray *fetch;
	gboolean fetch_happened;
	CamelFolder *folder;
} IdleResponse;

static void 
process_idle_response (IdleResponse *idle_resp)
{
	CamelImapStore *store;
	char *resp = NULL;
	const char *header_spec;
	CamelException ex = CAMEL_EXCEPTION_INITIALISER;
	CamelImapResponseType type;
	CamelImapMessageInfo *mi, *info;

	if (!idle_resp)
		return;

	if (idle_resp->fetch && idle_resp->folder)
	{
		CamelMessageInfo *info;
		CamelImapMessageInfo *iinfo;
		CamelFolderChangeInfo *changes = NULL;

		guint i=0;
		for (i=0; i < idle_resp->fetch->len; i++)
		{
			FetchIdleResponse *fid = idle_resp->fetch->pdata[i];
			info = camel_folder_summary_index (idle_resp->folder->summary, fid->id);
			iinfo = (CamelImapMessageInfo *) info;

			if (info) 
			{
				if (fid->flags != iinfo->server_flags) 
				{
					camel_folder_summary_touch (idle_resp->folder->summary);
					guint32 server_set, server_cleared;
					server_set = fid->flags & ~iinfo->server_flags;
					server_cleared = iinfo->server_flags & ~fid->flags;
					iinfo->info.flags = (iinfo->info.flags | server_set) & ~server_cleared;
					iinfo->server_flags = fid->flags;
					if (changes == NULL)
						changes = camel_folder_change_info_new();
					camel_folder_change_info_change_uid(changes, info->uid);
				}
				camel_message_info_free (info);
			}
		}

		if (changes)
		{
			if (camel_folder_change_info_changed (changes))
				camel_object_trigger_event (CAMEL_OBJECT (idle_resp->folder), "folder_changed", changes);
			camel_folder_change_info_free (changes);
			camel_folder_summary_save (idle_resp->folder->summary);
		}

	}

	camel_imap_folder_changed_for_idle (idle_resp->folder, idle_resp->exists, 
		idle_resp->expunged, &ex);

}

static void
read_idle_response (CamelFolder *folder, char *resp, IdleResponse *idle_resp)
{
	char *ptr = strchr (resp, '*');

	if (ptr && strstr (resp, "EXISTS") != NULL)
		idle_resp->exists = strtoul (resp + 1, NULL, 10);

	if (ptr && strstr (resp, "RECENT") != NULL)
		idle_resp->recent = strtoul (resp + 1, NULL, 10);

	if (ptr && strstr (resp, "FETCH") != NULL)
	{
		char *fptr = resp;

		FetchIdleResponse *fid = g_slice_new0 (FetchIdleResponse);
		fid->id = strtoul (resp + 1, NULL, 10);
		fptr += 9;
		fid->flags = imap_parse_flag_list (&fptr);

		if (idle_resp->fetch == NULL)
			idle_resp->fetch = g_ptr_array_new ();
		g_ptr_array_add (idle_resp->fetch, fid);
	}

	if (ptr && strstr (resp, "EXPUNGE") != NULL) 
	{
		guint32 id = strtoul (resp + 1, NULL, 10);

		if (idle_resp->expunged == NULL)
			idle_resp->expunged = g_array_new (FALSE, FALSE, sizeof (int));
		g_array_append_val (idle_resp->expunged, id);
	}
}

static IdleResponse*
idle_response_new (CamelFolder *folder)
{
	IdleResponse *idle_resp = g_slice_new0 (IdleResponse);
	idle_resp->folder = folder;
	camel_object_ref (CAMEL_OBJECT (folder));
	return idle_resp;
}

static void
idle_response_free (IdleResponse *idle_resp)
{
	guint i=0;

	if (idle_resp->expunged)
		g_array_free (idle_resp->expunged, TRUE);

	if (idle_resp->fetch) 
		for (i=0 ;i < idle_resp->fetch->len; i++)
			g_slice_free (FetchIdleResponse, idle_resp->fetch->pdata[i]);

	if (idle_resp->folder)
		camel_object_unref (CAMEL_OBJECT (idle_resp->folder));

	g_slice_free (IdleResponse, idle_resp);
}

static void
idle_real_start (CamelImapStore *store)
{
	g_mutex_lock (store->stream_lock);
	store->idle_prefix = g_strdup_printf ("%c%.5u", 
		store->tag_prefix, store->command++);
	camel_stream_printf (store->ostream, "%s IDLE\r\n",
		store->idle_prefix);
	g_mutex_unlock (store->stream_lock);

}

static IdleResponse*
idle_deal_with_stuff (CamelFolder *folder, CamelImapStore *store, gboolean *had_err, gboolean needlock, gboolean *hadlock)
{
  IdleResponse *idle_resp = NULL;
  CamelException ex = CAMEL_EXCEPTION_INITIALISER;
  char *resp = NULL;
  int nwritten;
  CamelImapResponseType type;
  gboolean locked;

  if (!folder || !CAMEL_IS_IMAP_FOLDER (folder))
	return NULL;
  if (!store || !CAMEL_IS_IMAP_STORE (store))
	return NULL;
  if (!camel_disco_store_check_online ((CamelDiscoStore*)store, &ex))
	return NULL;
  if (store->istream == NULL || ((CamelObject *)store->istream)->ref_count <= 0)
	return NULL;
  if (store->ostream == NULL || ((CamelObject *)store->istream)->ref_count <= 0)
	return NULL;

  locked = CAMEL_SERVICE_REC_TRYLOCK (store, connect_lock);
  if (needlock && !locked)
  {
	CAMEL_SERVICE_REC_LOCK (store, connect_lock);
	locked = TRUE;
  }

  if (locked)
  {
	*hadlock = TRUE;

	if (store->current_folder)
	{
		GArray *expunged = NULL;
		resp = NULL;
		while (camel_imap_store_readline_nb (store, &resp, &ex) > 0)
		{
			/* printf ("resp: %s\n", resp); */
			if (strchr (resp, '*') != NULL && (strstr (resp, "EXISTS") || 
				strstr (resp, "FETCH")|| strstr (resp, "EXPUNGE") || 
				strstr (resp, "RECENT")))
			{
				if (!idle_resp) 
					idle_resp = idle_response_new (folder);
				read_idle_response (folder, resp, idle_resp);
			}
			g_free (resp); resp=NULL;
		}
		if (resp)
			g_free (resp);

		g_mutex_lock (store->stream_lock);
		if (!camel_disco_store_check_online ((CamelDiscoStore*)store, &ex))
			{ g_mutex_unlock (store->stream_lock); return NULL; }
		if (store->istream == NULL || ((CamelObject *)store->istream)->ref_count <= 0)
			{ g_mutex_unlock (store->stream_lock); return NULL; }
		if (store->ostream == NULL || ((CamelObject *)store->istream)->ref_count <= 0)
			{ g_mutex_unlock (store->stream_lock); return NULL; }
		if (store->ostream && CAMEL_IS_STREAM (store->ostream))
			nwritten = camel_stream_printf (store->ostream, "DONE\r\n");
		g_mutex_unlock (store->stream_lock);

		if (nwritten == -1) 
			goto outofhere;

		resp = NULL;
		while ((type = camel_imap_command_response_idle (store, &resp, &ex)) == CAMEL_IMAP_RESPONSE_UNTAGGED) 
		{
			/* printf ("D resp: %s\n", resp); */
			if (strchr (resp, '*') != NULL && (strstr (resp, "EXISTS") ||
				strstr (resp, "FETCH") || strstr (resp, "EXPUNGE") || 
				strstr (resp, "RECENT")))
			{
				if (!idle_resp)
					idle_resp = idle_response_new (folder);
				read_idle_response (folder, resp, idle_resp);
			}
			g_free (resp); resp=NULL;
		}

		if (type == CAMEL_IMAP_RESPONSE_ERROR)
			*had_err = TRUE;

		if (resp)
			g_free (resp);

	} else {
		g_mutex_lock (store->stream_lock);
		if (!camel_disco_store_check_online ((CamelDiscoStore*)store, &ex))
			{ g_mutex_unlock (store->stream_lock); return NULL; }
		if (store->istream == NULL || ((CamelObject *)store->istream)->ref_count <= 0)
			{ g_mutex_unlock (store->stream_lock); return NULL; }
		if (store->ostream == NULL || ((CamelObject *)store->istream)->ref_count <= 0)
			{ g_mutex_unlock (store->stream_lock); return NULL; }
		if (store->ostream && CAMEL_IS_STREAM (store->ostream))
			nwritten = camel_stream_printf (store->ostream, "DONE\r\n");
		g_mutex_unlock (store->stream_lock);
		if (nwritten == -1) 
			goto outofhere;
		resp = NULL;
		while ((type = camel_imap_command_response_idle (store, &resp, &ex)) == CAMEL_IMAP_RESPONSE_UNTAGGED) 
			{ g_free (resp); resp=NULL; }
		if (resp)
			g_free (resp);
	}

outofhere:

	CAMEL_SERVICE_REC_UNLOCK (store, connect_lock);
  } 

  return idle_resp;
}

void
camel_imap_folder_stop_idle (CamelFolder *folder)
{
	CamelImapStore *store;
	IdleResponse *idle_resp = NULL;
	GSource *src;
	gboolean had_err = FALSE;
	CamelException ex = CAMEL_EXCEPTION_INITIALISER;

	if (!folder || !CAMEL_IS_IMAP_FOLDER (folder))
		return;

	store = CAMEL_IMAP_STORE (folder->parent_store);

	if (!store || !CAMEL_IS_IMAP_STORE (store))
		return;
	if (!camel_disco_store_check_online ((CamelDiscoStore*)store, &ex))
		return;
	if (store->istream == NULL || ((CamelObject *)store->istream)->ref_count <= 0)
		return;
	if (store->ostream == NULL || ((CamelObject *)store->istream)->ref_count <= 0)
		return;

	g_static_rec_mutex_lock (((CamelImapFolder *)folder)->idle_lock);

	if (store->capabilities & IMAP_CAPABILITY_IDLE)
	{
		gboolean hadlock = FALSE;
		g_free (store->idle_prefix);
		store->idle_prefix = NULL;

		if (store->idle_signal > 0) 
			g_source_remove (store->idle_signal);

		idle_resp = idle_deal_with_stuff (folder, store, &had_err, TRUE, &hadlock);

		/* Outside of the lock of course */
		if (idle_resp && !had_err)
			process_idle_response (idle_resp);

		/* idle_real_start (store); */

		if (idle_resp) 
			idle_response_free (idle_resp);
	}

	g_static_rec_mutex_unlock (((CamelImapFolder *)folder)->idle_lock);
}

static gboolean 
idle_timeout_checker (gpointer data)
{
	CamelFolder *folder = (CamelFolder *) data;
	CamelImapFolder *imap_folder;
	CamelImapStore *store;
	gboolean had_err = FALSE, hadlock = FALSE;
	CamelException ex = CAMEL_EXCEPTION_INITIALISER;

	((CamelImapFolder *)folder)->in_idle = TRUE;

	/* printf ("idle\n"); */

	if (!folder || ((CamelObject *)data)->ref_count <= 0 && !CAMEL_IS_IMAP_FOLDER (folder))
		return FALSE;

	store = CAMEL_IMAP_STORE (folder->parent_store);
	if (!(store->capabilities & IMAP_CAPABILITY_IDLE))
		return FALSE;
	if (!camel_disco_store_check_online ((CamelDiscoStore*)store, &ex))
		return FALSE;
	if (store->istream == NULL || ((CamelObject *)store->istream)->ref_count <= 0)
		return FALSE;
	if (store->ostream == NULL || ((CamelObject *)store->istream)->ref_count <= 0)
		return FALSE;

	g_static_rec_mutex_lock (((CamelImapFolder *)folder)->idle_lock);

	imap_folder = CAMEL_IMAP_FOLDER (folder);
	if (!imap_folder->do_push_email)
	{
		g_static_rec_mutex_unlock (((CamelImapFolder *)folder)->idle_lock);
		return FALSE;
	}

	if (store->idle_prefix != NULL)
	{
		IdleResponse *idle_resp = idle_deal_with_stuff (folder, store, &had_err, FALSE, &hadlock);

		if (idle_resp && !had_err)
			process_idle_response (idle_resp);

		if (hadlock)
			idle_real_start (store);

		if (idle_resp)
			idle_response_free (idle_resp);
	}


	g_static_rec_mutex_unlock (((CamelImapFolder *)folder)->idle_lock);

	if (((CamelImapFolder *)folder)->stopping)
	{
		g_static_rec_mutex_free (((CamelImapFolder *)folder)->idle_lock);
		((CamelImapFolder *)folder)->idle_lock = NULL;
		((CamelImapFolder *)folder)->in_idle = FALSE;
		return FALSE;
	}

	((CamelImapFolder *)folder)->in_idle = FALSE;

	return TRUE;
}


void
camel_imap_folder_start_idle (CamelFolder *folder)
{
	CamelImapStore *store;
	CamelImapFolder *imap_folder = (CamelImapFolder *) folder;
	CamelException ex = CAMEL_EXCEPTION_INITIALISER;

	if (!folder || !CAMEL_IS_IMAP_FOLDER (folder))
		return;

	store = CAMEL_IMAP_STORE (folder->parent_store);
	if (!store || !CAMEL_IS_IMAP_STORE (store))
		return;
	if (!camel_disco_store_check_online ((CamelDiscoStore*)store, &ex))
		return;
	if (store->istream == NULL || ((CamelObject *)store->istream)->ref_count <= 0)
		return;
	if (store->ostream == NULL || ((CamelObject *)store->istream)->ref_count <= 0)
		return;

	g_static_rec_mutex_lock (((CamelImapFolder *)folder)->idle_lock);

	if (store->capabilities & IMAP_CAPABILITY_IDLE)
	{
		if (store->current_folder && !store->idle_prefix)
		{
			folder->folder_flags |= CAMEL_FOLDER_HAS_PUSHEMAIL_CAPABILITY;

			idle_real_start (store);

			store->idle_signal = g_timeout_add (5 * 1000, idle_timeout_checker, folder);
			imap_folder->idle_signal = store->idle_signal;
		}
	}

	g_static_rec_mutex_unlock (((CamelImapFolder *)folder)->idle_lock);

}


static void 
imap_set_push_email (CamelFolder *folder, gboolean setting)
{
	CamelImapFolder *imap_folder = CAMEL_IMAP_FOLDER (folder);

	if (imap_folder->do_push_email && !setting) {
		imap_folder->do_push_email = setting;
		camel_imap_folder_stop_idle (folder);
	}

	if (!imap_folder->do_push_email && setting) {
		imap_folder->do_push_email = setting;
		camel_imap_folder_start_idle (folder);
	}

	return;
}
/* Called with the store's connect_lock locked */
void
camel_imap_folder_changed (CamelFolder *folder, int exists,
			   GArray *expunged, CamelException *ex)
{
	CamelImapFolder *imap_folder = CAMEL_IMAP_FOLDER (folder);
	CamelFolderChangeInfo *changes;
	CamelMessageInfo *info;
	int len;
	
	changes = camel_folder_change_info_new ();
	if (expunged) {
		int i, id;
		
		for (i = 0; i < expunged->len; i++) 
		{
			id = g_array_index (expunged, int, i);
			info = camel_folder_summary_index (folder->summary, id - 1);

			if (info == NULL) {
				/* FIXME: danw: does this mean that the summary is corrupt? */
				/* I guess a message that we never retrieved got expunged? */
				continue;
			}
			
			camel_folder_change_info_remove_uid (changes, camel_message_info_uid (info));

			CAMEL_IMAP_FOLDER_REC_LOCK (imap_folder, cache_lock);
			camel_imap_message_cache_remove (imap_folder->cache, camel_message_info_uid (info));
			CAMEL_IMAP_FOLDER_REC_UNLOCK (imap_folder, cache_lock);

			((CamelMessageInfoBase *)info)->flags |= CAMEL_MESSAGE_EXPUNGED;

			camel_folder_summary_remove (folder->summary, info);
			camel_message_info_free(info);
		}
	}

	len = camel_folder_summary_count (folder->summary);

	if (exists > len) {
		camel_imap_folder_stop_idle (folder);
		imap_update_summary (folder, exists, changes, ex);
		/* camel_imap_folder_start_idle (folder); will happen later? */
	}

	if (camel_folder_change_info_changed (changes))
		camel_object_trigger_event (CAMEL_OBJECT (folder), "folder_changed", changes);
	
	camel_folder_change_info_free (changes);
	camel_folder_summary_save (folder->summary);
}


static void
camel_imap_folder_changed_for_idle (CamelFolder *folder, int exists,
			   GArray *expunged, CamelException *ex)
{
	CamelImapFolder *imap_folder = CAMEL_IMAP_FOLDER (folder);
	CamelFolderChangeInfo *changes;
	CamelMessageInfo *info;
	int len;

	/* printf ("FOR IDLE: %d vs %d\n", exists, camel_folder_summary_count (folder->summary)); */

	changes = camel_folder_change_info_new ();
	if (expunged) {
		int i, id;
		
		for (i = 0; i < expunged->len; i++) 
		{
			const char *uid;

			id = g_array_index (expunged, int, i);
			info = camel_folder_summary_index (folder->summary, id - 1);

			if (info == NULL) {
				/* FIXME: danw: does this mean that the summary is corrupt? */
				/* I guess a message that we never retrieved got expunged? */
				continue;
			}

			uid = camel_message_info_uid (info);
			camel_folder_change_info_remove_uid (changes, uid);

			CAMEL_IMAP_FOLDER_REC_LOCK (imap_folder, cache_lock);
			camel_imap_message_cache_remove (imap_folder->cache, camel_message_info_uid (info));
			CAMEL_IMAP_FOLDER_REC_UNLOCK (imap_folder, cache_lock);

			((CamelMessageInfoBase *)info)->flags |= CAMEL_MESSAGE_EXPUNGED;

			camel_folder_summary_remove (folder->summary, info);
			camel_message_info_free(info);
		}
	}
	
	len = camel_folder_summary_count (folder->summary);
	if (exists > len)
		imap_update_summary (folder, exists, changes, ex);
	
	if (camel_folder_change_info_changed (changes))
		camel_object_trigger_event (CAMEL_OBJECT (folder), "folder_changed", changes);
	
	camel_folder_change_info_free (changes);
	camel_folder_summary_save (folder->summary);
}

static void
imap_thaw (CamelFolder *folder)
{
	CamelImapFolder *imap_folder;

	CAMEL_FOLDER_CLASS (disco_folder_class)->thaw (folder);
	if (camel_folder_is_frozen (folder))
		return;

	imap_folder = CAMEL_IMAP_FOLDER (folder);
	if (imap_folder->need_refresh) {
		imap_folder->need_refresh = FALSE;
		imap_refresh_info (folder, NULL);
	}
}

static void 
handle_freeup (CamelImapStore *store, gint nread, CamelException *ex)
{
	if (nread <= 0) 
	{
		if (errno == EINTR)
			camel_exception_set (ex, CAMEL_EXCEPTION_USER_CANCEL, _("Operation cancelled"));
		else
			camel_exception_setv (ex, CAMEL_EXCEPTION_SERVICE_UNAVAILABLE,
					      _("Server unexpectedly disconnected: %s"),
					      g_strerror (errno));
		
		camel_service_disconnect (CAMEL_SERVICE (store), FALSE, NULL);
	}
}


CamelStream *
camel_imap_folder_fetch_data (CamelImapFolder *imap_folder, const char *uid,
			      const char *section_text, gboolean cache_only,
			      CamelFolderReceiveType type, gint param, CamelException *ex)
{
	CamelFolder *folder = CAMEL_FOLDER (imap_folder);
	CamelImapStore *store = CAMEL_IMAP_STORE (folder->parent_store);
	CamelStream *stream = NULL;
	gboolean connected = FALSE, idle_rt = FALSE;
	CamelException tex = CAMEL_EXCEPTION_INITIALISER, myex = CAMEL_EXCEPTION_INITIALISER;
	ssize_t nread = 0; 
	gchar *a_resp = NULL;

	/* EXPUNGE responses have to modify the cache, which means
	 * they have to grab the cache_lock while holding the
	 * connect_lock.

	 * Because getting the service lock may cause MUCH unecessary
	 * delay when we already have the data locally, we do the
	 * locking separately.  This could cause a race
	 * getting the same data from the cache, but that is only
	 * an inefficiency, and bad luck. */

	CAMEL_IMAP_FOLDER_REC_LOCK (imap_folder, cache_lock);

	connected = camel_disco_store_check_online((CamelDiscoStore *)store, &tex);

	if (!store->istream || ((CamelObject *)store->istream)->ref_count <= 0)
		connected = FALSE;
	if (!store->ostream || ((CamelObject *)store->ostream)->ref_count <= 0)
		connected = FALSE;
	
	if (connected && ((type & CAMEL_FOLDER_RECEIVE_FULL) && camel_imap_message_cache_is_partial (imap_folder->cache, uid)))
		camel_imap_message_cache_remove (imap_folder->cache, uid);
	else if (connected && ((type & CAMEL_FOLDER_RECEIVE_PARTIAL || type & CAMEL_FOLDER_RECEIVE_SIZE_LIMITED) 
			&& !camel_imap_message_cache_is_partial (imap_folder->cache, uid)))
		camel_imap_message_cache_remove (imap_folder->cache, uid);
	else
	{
	    stream = camel_imap_message_cache_get (imap_folder->cache, uid, section_text, ex);

	    if (!stream && (!strcmp (section_text, "HEADER") || !strcmp (section_text, "0"))) 
	    {
		    camel_exception_clear (ex);
		    stream = camel_imap_message_cache_get (imap_folder->cache, uid, "", ex);
	    }
	}
	CAMEL_IMAP_FOLDER_REC_UNLOCK (imap_folder, cache_lock);
	
	if (stream || cache_only)
		return stream;

	if (!camel_disco_store_check_online ((CamelDiscoStore*)store, ex)) {
		camel_exception_set (ex, CAMEL_EXCEPTION_SERVICE_UNAVAILABLE,
				     _("This message is not currently available"));
		CAMEL_IMAP_FOLDER_REC_UNLOCK (imap_folder, cache_lock);
		return NULL;
	}
	camel_exception_clear (ex);

  CAMEL_SERVICE_REC_LOCK(store, connect_lock);

	camel_exception_clear(ex);

	CAMEL_IMAP_FOLDER_REC_LOCK (imap_folder, cache_lock);

	if (type & CAMEL_FOLDER_RECEIVE_FULL)
		stream = camel_imap_message_cache_insert (imap_folder->cache, 
			uid, section_text, "", 0, NULL);
	else /* CAMEL_FOLDER_RECEIVE_PARTIAL, CAMEL_FOLDER_RECEIVE_SIZE_LIMITED or any */
		stream = camel_imap_message_cache_insert (imap_folder->cache, 
			uid, "", "", 0, NULL);

	if (stream == NULL)
		stream = camel_stream_mem_new ();

	if (type & CAMEL_FOLDER_RECEIVE_FULL)
	{
		camel_imap_message_cache_set_partial (imap_folder->cache, uid, FALSE);

		if (store->capabilities & IMAP_CAPABILITY_BINARY)
		{
			gchar line[MAX_LINE_LEN];
			gboolean err = FALSE;
			char two_bytes[2];
			char t_str [1024];
			int f = 0;
			ssize_t hread = 1;
			gint length=0, rec=0;
			long seq;
			char *pos, *ppos;

			nread = 1;

			/* Stops idle */
			camel_imap_command_start (store, folder, ex,
				"UID FETCH %s BINARY.PEEK[%s]", uid, section_text);

			g_mutex_lock (store->stream_lock);

			two_bytes [0] = ' ';

			/* a01 uid fetch 1 BINARY.PEEK[]
			 * * 1 FETCH (UID 1 BINARY[] {24693}\r\n
			 * Subject: Testing....
			 * )\r\n
			 * a01 OK .... \r\n */

			/* Read the length in the "\*.*[~|]{<LENGTH>}" */
			while (two_bytes[0] != '\n' && f < 1023 && nread > 0)
			{
				nread = camel_stream_read (store->ostream, two_bytes, 1);
				line [f] = two_bytes [0];
/* printf ("%c%c", two_bytes[0], two_bytes[1]); */
				f++;
			}
			line[f] = '\0';
/* printf ("\n"); */

			/* The first line is very unlikely going to consume 1023 bytes */
			if (f > 1023 || nread <= 0) {
				g_warning ("BINARY: Long first line, UID=%s", uid);
				goto berrorhander;
			}

			/* If the line doesn't start with "* " */
			if (*line != '*' || *(line + 1) != ' ')
				{ err = TRUE; g_warning ("BINARY: Line doesn't start with \"* \", UID=%s (%s)", uid, line); goto berrorhander; }
			pos = strchr (line, '{');

			/* If we don't find a '{' character */
			if (!pos) 
				{ err = TRUE; g_warning ("BINARY: Line doesn't contain a {, UID=%s (%s)", uid, line); goto berrorhander; }

			/* Set the '}' character to \0 */
			ppos = strchr (pos, '}');
			if (ppos) 
				*ppos = '\0';
			else /* If we didn't find it */
				{ err = TRUE; g_warning ("BINARY: Line doesn't contain a }, UID=%s (%s)", uid, line); goto berrorhander; }

			length = strtol (pos + 1, NULL, 10);

			/* If strtol failed (it's important enough to check this) */
			if (errno == ERANGE)
				{ err = TRUE; g_warning ("BINARY: strtol failed, UID=%s (%s)", uid, line); goto berrorhander; }

			/* Until we have reached the length, read 1024 at the time */
			while (hread > 0 && rec < length)
			{
				int wread = (length - rec);
				if (wread < 1 || wread > 1024)
					wread = 1024;
				hread = camel_stream_read (store->ostream, t_str, wread);
				if (hread > 0) 
				{
					/* And write them too */
					camel_stream_write (stream, t_str, hread);
					rec += hread;
				}
			}
			camel_stream_reset (stream);

			/* Read away the last two lines */
			for (f = 0; f < 2; f++) { 
				nread = 1; two_bytes[0] = 'x';
				while (two_bytes[0] != '\n' && nread > 0)
					nread = camel_stream_read (store->ostream, two_bytes, 1); 
			}
berrorhander:
			g_mutex_unlock (store->stream_lock);
			CAMEL_SERVICE_REC_UNLOCK (store, connect_lock);
			/* Starts idle */
			camel_imap_folder_start_idle ((CamelFolder *) imap_folder);
			idle_rt = TRUE;

			if (err)
				goto errorhander;
		} else 
		{
			gboolean first = TRUE, err=FALSE;
			gchar line [MAX_LINE_LEN];
			guint linenum = 0;
			CamelStreamBuffer *server_stream;
			gchar *tag;
			guint taglen;
			gboolean isnextdone = FALSE, hadr = FALSE;

			nread = 0;

			/* Stops idle */
			if (store->server_level < IMAP_LEVEL_IMAP4REV1 && !*section_text)
				camel_imap_command_start (store, folder, ex,
					"UID FETCH %s RFC822.PEEK", uid);
			else
				camel_imap_command_start (store, folder, ex,
					"UID FETCH %s BODY.PEEK[%s]", uid, section_text);

			tag = g_strdup_printf ("%c%.5u", store->tag_prefix, store->command-1);
			taglen = strlen (tag);

			g_mutex_lock (store->stream_lock);
			server_stream = (CamelStreamBuffer*) store->istream;

			if (!server_stream)
				err = TRUE;
			else
				store->command++;

			if (server_stream) 
			  while (nread = camel_stream_buffer_gets (server_stream, line, MAX_LINE_LEN) > 0)
			  {

				/* It might be the line before the last line */
				if (line[0] == ')' && (line[1] == '\n' || (line[1] == '\r' && line[2] == '\n')))
				{
					if (line[1] == '\r')
						hadr = TRUE;
					isnextdone = TRUE;
					continue;
				}

				/* It's the first line */
				if (linenum == 0 && (line [0] != '*' || line[1] != ' '))
					{ err=TRUE; break; } 
				else if (linenum == 0) 
					{ linenum++; continue; }

				/* It's the last line (isnextdone will be ignored if that is the case) */
			 	if (!strncmp (line, tag, taglen))
					break;

				camel_seekable_stream_seek (CAMEL_SEEKABLE_STREAM (stream), 0, CAMEL_STREAM_END);

				if (isnextdone)
				{
					if (hadr)
						camel_stream_write (stream, ")\n", 2);
					else
						camel_stream_write (stream, ")\r\n", 3);

					hadr = FALSE;
					isnextdone = FALSE;
				}

				camel_stream_write (stream, line, strlen (line));

				linenum++;
				memset (line, 0, MAX_LINE_LEN);
			  }
merrorhandler:
			g_mutex_unlock (store->stream_lock);
			CAMEL_SERVICE_REC_UNLOCK (store, connect_lock);
			/* Starts idle */
			camel_imap_folder_start_idle ((CamelFolder *) imap_folder);
			idle_rt = TRUE;

			if (nread <= 0) 
				err = TRUE;

			if (tag)
				g_free (tag);

			if (err)
				goto errorhander;

			camel_stream_reset (stream);

		} /* NON-BINARY */

	} else 
	{

		/* Partial message retrieval feature gets the message like this: 
		   { HEADER boundary 1.HEADER 1 boundary } */

		char *boundary = NULL;
		int t = 0, boundary_len = 0;
		const gchar *infos[2] = { "HEADER", /*"1.HEADER",*/ "1" };

		camel_imap_message_cache_set_partial (imap_folder->cache, uid, TRUE);

		for (t=0; t < 2; t++)
		{
			gboolean first = TRUE, err=FALSE;
			gchar line[MAX_LINE_LEN];
			guint linenum = 0; 
			CamelStreamBuffer *server_stream;
			gchar *tag;
			guint taglen;
			gboolean isnextdone = FALSE;

			nread = 0;

			/* Stops idle */
			camel_imap_command_start (store, folder, ex,
				"UID FETCH %s BODY.PEEK[%s]", uid, infos[t]);

			tag = g_strdup_printf ("%c%.5u", store->tag_prefix, store->command-1);
			taglen = strlen (tag);

			g_mutex_lock (store->stream_lock);

			if (!store->istream || ((CamelObject *)store->istream)->ref_count <= 0)
			{
				err = TRUE;
				nread = -1;
				goto rerrorhandler;
			}

			if (!store->ostream || ((CamelObject *)store->ostream)->ref_count <= 0)
			{
				err = TRUE;
				nread = -1;
				goto rerrorhandler;
			}
			
			if (camel_imap_store_restore_stream_buffer (store))
				server_stream = store->istream ? CAMEL_STREAM_BUFFER (store->istream) : NULL;
			else server_stream = NULL;

			if (server_stream == NULL)
				err = TRUE;
			else
				store->command++;

			if (server_stream) 
			  while (nread = camel_stream_buffer_gets (server_stream, line, MAX_LINE_LEN) > 0)
			  {

				/* It might be the line before the last line */
				if (line[0] == ')' && (line[1] == '\n' || (line[1] == '\r' && line[2] == '\n')))
					{ isnextdone = TRUE; continue; }

				/* It's the first line */
				if (linenum == 0 && (line [0] != '*' || line[1] != ' '))
					{ err=TRUE; break; } 
				else if (linenum == 0) 
					{ linenum++; continue; }

				/* It's the last line */
				if (!strncmp (line, tag, taglen))
				{
					if ((t == 0 || t == 1 /* 2 */) && boundary_len > 0)
					{
						camel_seekable_stream_seek (CAMEL_SEEKABLE_STREAM (stream), 0, CAMEL_STREAM_END);
						camel_stream_write (stream, "\n--", 3);
						camel_stream_write (stream, boundary, boundary_len);
						camel_stream_write (stream, "\n", 1);
					}
					break;
				}

				if (t == 0 && boundary_len == 0 && !boundary)
				{
					CamelContentType *ct = NULL;
					const char *bound=NULL;
					char *pstr = (char*)strcasestr (line, "Content-Type:");

					/* If it's the Content-Type line (TODO: use BODYSTRUCTURE for this) */

					if (pstr) 
					{
						pstr = strchr (pstr, ':'); 
						if (pstr) 
						{ 
							pstr++;
							ct = camel_content_type_decode(pstr); 
						} 
					}

					if (ct) 
					{ 
						bound = camel_content_type_param(ct, "boundary");
						if (bound) 
						{ 
							boundary_len = strlen (bound);
							if (boundary_len > 0) 
								boundary = g_strdup (bound);
							else 
								boundary_len = 0;
						}
					}
				}

				camel_seekable_stream_seek (CAMEL_SEEKABLE_STREAM (stream), 0, CAMEL_STREAM_END);

				if (isnextdone)
				{
					camel_stream_write (stream, ")\n", 2);
					isnextdone = FALSE;
				}

				camel_stream_write (stream, line, strlen (line));

				linenum++;
				memset (line, 0, MAX_LINE_LEN);
			  }
rerrorhandler:
			g_mutex_unlock (store->stream_lock);
			CAMEL_SERVICE_REC_UNLOCK (store, connect_lock);
			/* Starts idle */
			camel_imap_store_start_idle (store);
			idle_rt = TRUE;

			if (nread <= 0) 
				err = TRUE;
			
			if (tag)
				g_free (tag);

			if (err) 
			{
				if (boundary_len > 0)
					g_free (boundary);
				goto errorhander;
			}
		}

		if (boundary_len > 0)
			g_free (boundary);

		camel_stream_reset (stream);
	}

	CAMEL_IMAP_FOLDER_REC_UNLOCK (imap_folder, cache_lock);

  CAMEL_SERVICE_REC_UNLOCK(store, connect_lock);

	return stream;

errorhander:

	if (!idle_rt)
		camel_imap_folder_start_idle ((CamelFolder *) imap_folder);

	CAMEL_IMAP_FOLDER_REC_UNLOCK (imap_folder, cache_lock);
	camel_imap_message_cache_remove (imap_folder->cache, uid);

	camel_exception_setv (ex, CAMEL_EXCEPTION_SERVICE_UNAVAILABLE,
		    _("Could not find message body in FETCH response."));

	handle_freeup (store, nread, ex);

	if (stream && ((CamelObject *)stream)->ref_count > 0)
		camel_object_unref (CAMEL_OBJECT (stream));

  CAMEL_SERVICE_REC_UNLOCK(store, connect_lock);

	return NULL;
}





static GData *
parse_fetch_response (CamelImapFolder *imap_folder, char *response)
{
	GData *data = NULL;
	char *start, *part_spec = NULL, *body = NULL, *uid = NULL, *idate = NULL;
	gboolean cache_header = TRUE, header = FALSE;
	size_t body_len = 0;

	if (*response != '(') {
		long seq;
		
		if (*response != '*' || *(response + 1) != ' ')
			return NULL;
		seq = strtol (response + 2, &response, 10);
		if (seq == 0)
			return NULL;
		if (g_ascii_strncasecmp (response, " FETCH (", 8) != 0)
			return NULL;
		response += 7;
		
		g_datalist_set_data (&data, "SEQUENCE", GINT_TO_POINTER (seq));
	}
	
	do {
		/* Skip the initial '(' or the ' ' between elements */
		response++;
		
		if (!g_ascii_strncasecmp (response, "FLAGS ", 6)) {
			guint32 flags;
			
			response += 6;
			/* FIXME user flags */
			flags = imap_parse_flag_list (&response);
			
			g_datalist_set_data (&data, "FLAGS", GUINT_TO_POINTER (flags));
		} else if (!g_ascii_strncasecmp (response, "RFC822.SIZE ", 12)) {
			unsigned long size;
			
			response += 12;
			size = strtoul (response, &response, 10);
			g_datalist_set_data (&data, "RFC822.SIZE", GUINT_TO_POINTER (size));
		} else if (!g_ascii_strncasecmp (response, "BODY[", 5) ||
			   !g_ascii_strncasecmp (response, "RFC822 ", 7)) {
			char *p;
			
			if (*response == 'B') {
				response += 5;
				
				/* HEADER], HEADER.FIELDS (...)], or 0] */
				if (!g_ascii_strncasecmp (response, "HEADER", 6)) {
					header = TRUE;
					if (!g_ascii_strncasecmp (response + 6, ".FIELDS", 7))
						cache_header = FALSE;
				} else if (!g_ascii_strncasecmp (response, "0]", 2))
					header = TRUE;
				
				p = strchr (response, ']');
				if (!p || *(p + 1) != ' ')
					break;
				
				if (cache_header)
					part_spec = g_strndup (response, p - response);
				else
					part_spec = g_strdup ("HEADER.FIELDS");
				
				response = p + 2;
			} else {
				part_spec = g_strdup ("");
				response += 7;
				
				if (!g_ascii_strncasecmp (response, "HEADER", 6))
					header = TRUE;
			}
			
			body = imap_parse_nstring ((const char **) &response, &body_len);
			if (!response) {
				g_free (part_spec);
				break;
			}
			
			if (!body)
				body = g_strdup ("");
			g_datalist_set_data_full (&data, "BODY_PART_SPEC", part_spec, g_free);
			g_datalist_set_data_full (&data, "BODY_PART_DATA", body, g_free);
			g_datalist_set_data (&data, "BODY_PART_LEN", GINT_TO_POINTER (body_len));
		} else if (!g_ascii_strncasecmp (response, "BODY ", 5) ||
			   !g_ascii_strncasecmp (response, "BODYSTRUCTURE ", 14)) {
			response = strchr (response, ' ') + 1;
			start = response;
			imap_skip_list ((const char **) &response);
			g_datalist_set_data_full (&data, "BODY", g_strndup (start, response - start), g_free);
		} else if (!g_ascii_strncasecmp (response, "UID ", 4)) {
			int len;
			
			len = strcspn (response + 4, " )");
			uid = g_strndup (response + 4, len);
			g_datalist_set_data_full (&data, "UID", uid, g_free);
			response += 4 + len;
		} else if (!g_ascii_strncasecmp (response, "INTERNALDATE ", 13)) {
			int len;
			
			response += 13;
			if (*response == '"') {
				response++;
				len = strcspn (response, "\"");
				idate = g_strndup (response, len);
				g_datalist_set_data_full (&data, "INTERNALDATE", idate, g_free);
				response += len + 1;
			}
		} else if (!g_ascii_strncasecmp (response, "MODSEQ ", 7)) {
			char *marker = strchr (response + 7, ')');
			if (marker)
				response = marker + 1;
			
			if (!marker)
				g_warning ("Unexpected MODSEQ format: %s", response); 
		} else {
			g_warning ("Unexpected FETCH response from server: (%s", response);
			break;
		}
	} while (response && *response != ')');
	
	if (!response || *response != ')') {
		g_datalist_clear (&data);
		return NULL;
	}
	
	if (uid && body) {
		CamelStream *stream;
		
		if (header && !cache_header) {
			stream = camel_stream_mem_new_with_buffer (body, body_len);
		} else {
			CAMEL_IMAP_FOLDER_REC_LOCK (imap_folder, cache_lock);
			stream = camel_imap_message_cache_insert (imap_folder->cache,
								  uid, part_spec,
								  body, body_len, NULL);
			CAMEL_IMAP_FOLDER_REC_UNLOCK (imap_folder, cache_lock);
			if (stream == NULL)
				stream = camel_stream_mem_new_with_buffer (body, body_len);
		}
		
		if (stream)
			g_datalist_set_data_full (&data, "BODY_PART_STREAM", stream,
						  (GDestroyNotify) camel_object_unref);
	}
	
	return data;
}

