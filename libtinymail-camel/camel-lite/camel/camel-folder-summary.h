/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  Copyright (C) 2000 Ximian Inc.
 *
 *  Authors: Michael Zucchi <notzed@ximian.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU Lesser General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef _CAMEL_FOLDER_SUMMARY_H
#define _CAMEL_FOLDER_SUMMARY_H

#ifdef __cplusplus
extern "C" {
#pragma }
#endif /* __cplusplus */

#include <stdio.h>
#include <time.h>
#include <camel/camel-mime-parser.h>
#include <camel/camel-object.h>
#include <camel/camel-index.h>


struct _CamelFolder;

#define CAMEL_FOLDER_SUMMARY_TYPE         camel_folder_summary_get_type ()
#define CAMEL_FOLDER_SUMMARY(obj)         CAMEL_CHECK_CAST (obj, camel_folder_summary_get_type (), CamelFolderSummary)
#define CAMEL_FOLDER_SUMMARY_CLASS(klass) CAMEL_CHECK_CLASS_CAST (klass, camel_folder_summary_get_type (), CamelFolderSummaryClass)
#define CAMEL_IS_FOLDER_SUMMARY(obj)      CAMEL_CHECK_TYPE (obj, camel_folder_summary_get_type ())

/*typedef struct _CamelFolderSummary      CamelFolderSummary;*/
typedef struct _CamelFolderSummaryClass CamelFolderSummaryClass;

typedef struct _CamelMessageInfo CamelMessageInfo;
typedef struct _CamelMessageInfoBase CamelMessageInfoBase;

typedef struct _CamelFolderMetaSummary CamelFolderMetaSummary;

/* A tree of message content info structures
   describe the content structure of the message (if it has any) */
struct _CamelMessageContentInfo {
	struct _CamelMessageContentInfo *next;
	
	struct _CamelMessageContentInfo *childs;
	struct _CamelMessageContentInfo *parent;
	
	CamelContentType *type;
	char *id;
	char *description;
	char *encoding;		/* this should be an enum?? */
	guint32 size;
	gboolean needs_free;
};

/* system flag bits */
typedef enum _CamelMessageFlags {
	CAMEL_MESSAGE_ANSWERED = 1<<0, /* used */
	CAMEL_MESSAGE_DELETED = 1<<1, /* used */
	CAMEL_MESSAGE_DRAFT = 1<<2, /* used */
	CAMEL_MESSAGE_FLAGGED = 1<<3, /* used */
	CAMEL_MESSAGE_SEEN = 1<<4, /* used */
	CAMEL_MESSAGE_ATTACHMENTS = 1<<5, /* used */
	CAMEL_MESSAGE_CACHED = 1<<6, /* used */
	CAMEL_MESSAGE_PARTIAL = 1<<7, /* used */

	/* these aren't really system flag bits, but are convenience flags */
	CAMEL_MESSAGE_SECURE = 1<<8, /* free slot */
	CAMEL_MESSAGE_FREED = 1<<9, /* free slot */
	CAMEL_MESSAGE_ANSWERED_ALL = 1<<10, /* free slot */
	CAMEL_MESSAGE_JUNK = 1<<11, /* free slot */

	/* internally used */
	CAMEL_MESSAGE_FOLDER_FLAGGED = 1<<12, /* internally used */
	CAMEL_MESSAGE_INFO_NEEDS_FREE = 1<<13,/* internally used */
	CAMEL_MESSAGE_INFO_UID_NEEDS_FREE = 1<<14, /* internally used */
	CAMEL_MESSAGE_JUNK_LEARN = 1<<15,  /* free slot */
	CAMEL_MESSAGE_USER = 1<<16  /* free slot */
} CamelMessageFlags;

/* Changes to system flags will NOT trigger a folder changed event */
#define CAMEL_MESSAGE_SYSTEM_MASK (0xffff << 16)

typedef struct _CamelFlag {
	struct _CamelFlag *next;
	char name[1];		/* name allocated as part of the structure */
} CamelFlag;

typedef struct _CamelTag {
	struct _CamelTag *next;
	char *value;
	char name[1];		/* name allocated as part of the structure */
} CamelTag;

/* a summary messageid is a 64 bit identifier (partial md5 hash) */
typedef struct _CamelSummaryMessageID {
	union {
		guint64 id;
		unsigned char hash[8];
		struct {
			guint32 hi;
			guint32 lo;
		} part;
	} id;
} CamelSummaryMessageID;

#ifdef NON_TINYMAIL_FEATURES
/* summary references is a fixed size array of references */
typedef struct _CamelSummaryReferences {
	int size;
	CamelSummaryMessageID references[1];
} CamelSummaryReferences;
#endif

/* accessor id's */
enum {
	CAMEL_MESSAGE_INFO_SUBJECT,
	CAMEL_MESSAGE_INFO_FROM,
	CAMEL_MESSAGE_INFO_TO,
	CAMEL_MESSAGE_INFO_CC,
	CAMEL_MESSAGE_INFO_MLIST,

	CAMEL_MESSAGE_INFO_FLAGS,
	CAMEL_MESSAGE_INFO_SIZE,

	CAMEL_MESSAGE_INFO_DATE_SENT,
	CAMEL_MESSAGE_INFO_DATE_RECEIVED,

	CAMEL_MESSAGE_INFO_MESSAGE_ID,
	CAMEL_MESSAGE_INFO_REFERENCES,
	CAMEL_MESSAGE_INFO_USER_FLAGS,
	CAMEL_MESSAGE_INFO_USER_TAGS,

	CAMEL_MESSAGE_INFO_LAST
};

/* information about a given message, use accessors */
struct _CamelMessageInfo {
	CamelFolderSummary *summary;
	guint32 refcount;	/* ??? */
	char *uid;
};

/* For classes wishing to do the provided i/o, or for anonymous users,
 * they must subclass or use this messageinfo structure */
/* Otherwise they can do their own thing entirely */

/*                                          on x86_32 */
struct _CamelMessageInfoBase 
{
	CamelFolderSummary *summary;       /* 4 bytes */
	guint32 refcount;                  /* 4 bytes */
	char *uid;                         /* 4 bytes */
	const char *subject;               /* 4 bytes */
	const char *from;                  /* 4 bytes */
	const char *to;                    /* 4 bytes */
	const char *cc;                    /* 4 bytes */
#ifdef NON_TINYMAIL_FEATURES
	const char *mlist;
#endif

	/* tree of content description - NULL if it is not available */
	CamelMessageContentInfo *content;  /* 4 bytes */
	CamelSummaryMessageID message_id;  /* 8 bytes */

	guint32 flags;                     /* 4 bytes */
	guint32 size;                      /* 4 bytes */

	time_t date_sent;                  /* 4 bytes */
	time_t date_received;              /* 4 bytes */

                                          /* 56 bytes */
#ifdef NON_TINYMAIL_FEATURES
	CamelSummaryReferences *references;/* from parent to root */
#endif

#ifdef NON_TINYMAIL_FEATURES
	struct _CamelFlag *user_flags;
	struct _CamelTag *user_tags;
#endif

};

/* probably do this as well, removing CamelFolderChangeInfo and interfaces 
typedef struct _CamelChangeInfo CamelChangeInfo;
struct _CamelChangeInfo {
	GPtrArray *added;
	GPtrArray *removed;
	GPtrArray *changed;
	GPtrArray *recent;
};
*/

typedef enum _CamelFolderSummaryFlags {
	CAMEL_SUMMARY_DIRTY = 1<<0,
} CamelFolderSummaryFlags;


struct _CamelFolderSummary {
	CamelObject parent;

	struct _CamelFolderSummaryPrivate *priv;

	/* header info */
	guint32 version;	/* version of file loaded/loading */
	guint32 flags;		/* flags */
	guint32 nextuid;	/* next uid? */
	time_t time;		/* timestamp for this summary (for implementors to use) */
	guint32 saved_count;	/* how many were saved/loaded */
	guint32 unread_count;	/* handy totals */
	guint32 deleted_count;
	guint32 junk_count;

	/* sizes of memory objects */
	guint32 message_info_size;
	guint32 content_info_size;

	char *summary_path;
	gboolean build_content;	/* do we try and parse/index the content, or not? */

	GPtrArray *messages; /* CamelMessageInfo's */
	GHashTable *uidhash;

	struct _CamelFolder *folder; /* parent folder, for events */
	struct _CamelFolderMetaSummary *meta_summary; /* Meta summary */

	GMappedFile *file;
	unsigned char *filepos;
	GMutex *dump_lock, *hash_lock;
	gboolean in_reload;

	void (*set_extra_flags_func) (CamelFolder *folder, CamelMessageInfoBase *mi);
};

struct _CamelFolderSummaryClass {
	CamelObjectClass parent_class;

	/* load/save the global info */
	int (*summary_header_load)(CamelFolderSummary *);
	int (*summary_header_save)(CamelFolderSummary *, FILE *);

	/* create/save/load an individual message info */
	CamelMessageInfo * (*message_info_new_from_header)(CamelFolderSummary *, struct _camel_header_raw *);
	CamelMessageInfo * (*message_info_new_from_parser)(CamelFolderSummary *, CamelMimeParser *);
	CamelMessageInfo * (*message_info_new_from_message)(CamelFolderSummary *, CamelMimeMessage *);
	CamelMessageInfo * (*message_info_load)(CamelFolderSummary *, gboolean *must_add);
 	int		   (*message_info_save)(CamelFolderSummary *, FILE *, CamelMessageInfo *);
	int		   (*meta_message_info_save)(CamelFolderSummary *, FILE *, FILE *, CamelMessageInfo *);

	void		   (*message_info_free)(CamelFolderSummary *, CamelMessageInfo *);
	CamelMessageInfo * (*message_info_clone)(CamelFolderSummary *, const CamelMessageInfo *);

	/* save/load individual content info's */
	CamelMessageContentInfo * (*content_info_new_from_header)(CamelFolderSummary *, struct _camel_header_raw *);
	CamelMessageContentInfo * (*content_info_new_from_parser)(CamelFolderSummary *, CamelMimeParser *);
	CamelMessageContentInfo * (*content_info_new_from_message)(CamelFolderSummary *, CamelMimePart *);
	CamelMessageContentInfo * (*content_info_load)(CamelFolderSummary *);
	int		          (*content_info_save)(CamelFolderSummary *, FILE *, CamelMessageContentInfo *);
	void		          (*content_info_free)(CamelFolderSummary *, CamelMessageContentInfo *);

	/* get the next uid */
	char *(*next_uid_string)(CamelFolderSummary *);

	/* virtual accessors on messageinfo's */
	const void *(*info_ptr)(const CamelMessageInfo *mi, int id);
	guint32     (*info_uint32)(const CamelMessageInfo *mi, int id);
	time_t      (*info_time)(const CamelMessageInfo *mi, int id);

	gboolean    (*info_user_flag)(const CamelMessageInfo *mi, const char *id);
	const char *(*info_user_tag)(const CamelMessageInfo *mi, const char *id);

	/* set accessors for the modifyable bits */
#if 0
	void (*info_set_ptr)(CamelMessageInfo *mi, int id, const void *val);
	void (*info_set_uint32)(CamelMessageInfo *mi, int id, guint32 val);
	void (*info_set_time)(CamelMessageInfo *mi, int id, time_t val);
	void (*info_set_references)(CamelMessageInfo *mi, CamelSummaryReferences *);
#endif

	gboolean (*info_set_user_flag)(CamelMessageInfo *mi, const char *id, gboolean state);
	gboolean (*info_set_user_tag)(CamelMessageInfo *mi, const char *id, const char *val);
	gboolean (*info_set_flags)(CamelMessageInfo *mi, guint32 mask, guint32 set);

};

/* Meta-summary info */
struct _CamelFolderMetaSummary {
	guint32 major;		/* Major version of meta-summary */
	guint32 minor;		/* Minor version of meta-summary */
	guint32 uid_len;	/* Length of UID (for implementors to use) */
	gboolean msg_expunged;	/* Whether any message is expunged or not */
	char *path;		/* Path to meta-summary-file */
};

CamelType			 camel_folder_summary_get_type	(void);
CamelFolderSummary      *camel_folder_summary_new	(struct _CamelFolder *folder);

unsigned char*
decode_uint32 (unsigned char *start, guint32 *dest, gboolean is_string);

void camel_folder_summary_set_filename(CamelFolderSummary *summary, const char *filename);
void camel_folder_summary_set_index(CamelFolderSummary *summary, CamelIndex *index);
void camel_folder_summary_set_build_content(CamelFolderSummary *summary, gboolean state);

guint32  camel_folder_summary_next_uid        (CamelFolderSummary *summary);
char    *camel_folder_summary_next_uid_string (CamelFolderSummary *summary);
void 	 camel_folder_summary_set_uid	      (CamelFolderSummary *summary, guint32 uid);

/* load/save the summary in its entirety */
int camel_folder_summary_load(CamelFolderSummary *summary);
int camel_folder_summary_save(CamelFolderSummary *summary);

/* only load the header */
int camel_folder_summary_header_load(CamelFolderSummary *summary);

/* set the dirty bit on the summary */
void camel_folder_summary_touch(CamelFolderSummary *summary);

/* add a new raw summary item */
void camel_folder_summary_add(CamelFolderSummary *summary, CamelMessageInfo *info);

/* build/add raw summary items */
CamelMessageInfo *camel_folder_summary_add_from_header(CamelFolderSummary *summary, struct _camel_header_raw *headers);
CamelMessageInfo *camel_folder_summary_add_from_parser(CamelFolderSummary *summary, CamelMimeParser *parser);
CamelMessageInfo *camel_folder_summary_add_from_message(CamelFolderSummary *summary, CamelMimeMessage *message);

/* Just build raw summary items */
CamelMessageInfo *camel_folder_summary_info_new_from_header_with_uid (CamelFolderSummary *s, struct _camel_header_raw *h, const gchar *uid);
CamelMessageInfo *camel_folder_summary_info_new_from_header(CamelFolderSummary *summary, struct _camel_header_raw *headers);
CamelMessageInfo *camel_folder_summary_info_new_from_parser(CamelFolderSummary *summary, CamelMimeParser *parser);
CamelMessageInfo *camel_folder_summary_info_new_from_message(CamelFolderSummary *summary, CamelMimeMessage *message);

CamelMessageContentInfo *camel_folder_summary_content_info_new(CamelFolderSummary *summary);
void camel_folder_summary_content_info_free(CamelFolderSummary *summary, CamelMessageContentInfo *ci);

/* removes a summary item, doesn't fix content offsets */
void camel_folder_summary_remove(CamelFolderSummary *summary, CamelMessageInfo *info);
void camel_folder_summary_remove_uid(CamelFolderSummary *summary, const char *uid);
void camel_folder_summary_remove_index(CamelFolderSummary *summary, int index);
void camel_folder_summary_remove_range(CamelFolderSummary *summary, int start, int end);

/* remove all items */
void camel_folder_summary_clear(CamelFolderSummary *summary);

/* lookup functions */
int camel_folder_summary_count(CamelFolderSummary *summary);
CamelMessageInfo *camel_folder_summary_index(CamelFolderSummary *summary, int index);
CamelMessageInfo *camel_folder_summary_uid(CamelFolderSummary *summary, const char *uid);
GPtrArray *camel_folder_summary_array(CamelFolderSummary *summary);
void camel_folder_summary_array_free(CamelFolderSummary *summary, GPtrArray *array);

/* basically like strings, but certain keywords can be compressed and de-cased */
int camel_folder_summary_encode_token(FILE *out, const char *str);
int camel_folder_summary_decode_token(CamelFolderSummary *s, char **str);

/* message flag operations */
gboolean	camel_flag_get(CamelFlag **list, const char *name);
gboolean	camel_flag_set(CamelFlag **list, const char *name, gboolean state);
gboolean	camel_flag_list_copy(CamelFlag **to, CamelFlag **from);
int		camel_flag_list_size(CamelFlag **list);
void		camel_flag_list_free(CamelFlag **list);

guint32         camel_system_flag (const char *name);
gboolean        camel_system_flag_get (guint32 flags, const char *name);

/* message tag operations */
const char	*camel_tag_get(CamelTag **list, const char *name);
gboolean	camel_tag_set(CamelTag **list, const char *name, const char *value);
gboolean	camel_tag_list_copy(CamelTag **to, CamelTag **from);
int		camel_tag_list_size(CamelTag **list);
void		camel_tag_list_free(CamelTag **list);

/* Summary may be null */
/* Use anonymous pointers to avoid tons of cast crap */
void *camel_message_info_new(CamelFolderSummary *summary);
void camel_message_info_ref(void *info);
CamelMessageInfo *camel_message_info_new_from_header(CamelFolderSummary *summary, struct _camel_header_raw *header);
void camel_message_info_free(void *info);
void *camel_message_info_clone(const void *info);

/* accessors */
const void *camel_message_info_ptr(const CamelMessageInfo *mi, int id);
guint32 camel_message_info_uint32(const CamelMessageInfo *mi, int id);
time_t camel_message_info_time(const CamelMessageInfo *mi, int id);

#define camel_message_info_uid(mi) ((const char *)((const CamelMessageInfo *)mi)->uid)

#define camel_message_info_subject(mi) ((const char *)camel_message_info_ptr((const CamelMessageInfo *)mi, CAMEL_MESSAGE_INFO_SUBJECT))
#define camel_message_info_from(mi) ((const char *)camel_message_info_ptr((const CamelMessageInfo *)mi, CAMEL_MESSAGE_INFO_FROM))
#define camel_message_info_to(mi) ((const char *)camel_message_info_ptr((const CamelMessageInfo *)mi, CAMEL_MESSAGE_INFO_TO))
#define camel_message_info_cc(mi) ((const char *)camel_message_info_ptr((const CamelMessageInfo *)mi, CAMEL_MESSAGE_INFO_CC))

#ifdef NON_TINYMAIL_FEATURES
#define camel_message_info_mlist(mi) ((const char *)camel_message_info_ptr((const CamelMessageInfo *)mi, CAMEL_MESSAGE_INFO_MLIST))
#endif

#define camel_message_info_flags(mi) camel_message_info_uint32((const CamelMessageInfo *)mi, CAMEL_MESSAGE_INFO_FLAGS)
#define camel_message_info_size(mi) camel_message_info_uint32((const CamelMessageInfo *)mi, CAMEL_MESSAGE_INFO_SIZE)

#define camel_message_info_date_sent(mi) camel_message_info_time((const CamelMessageInfo *)mi, CAMEL_MESSAGE_INFO_DATE_SENT)
#define camel_message_info_date_received(mi) camel_message_info_time((const CamelMessageInfo *)mi, CAMEL_MESSAGE_INFO_DATE_RECEIVED)

#define camel_message_info_message_id(mi) ((const CamelSummaryMessageID *)camel_message_info_ptr((const CamelMessageInfo *)mi, CAMEL_MESSAGE_INFO_MESSAGE_ID))
#ifdef NON_TINYMAIL_FEATURES
#define camel_message_info_references(mi) ((const CamelSummaryReferences *)camel_message_info_ptr((const CamelMessageInfo *)mi, CAMEL_MESSAGE_INFO_REFERENCES))
#endif
#define camel_message_info_user_flags(mi) ((const CamelFlag *)camel_message_info_ptr((const CamelMessageInfo *)mi, CAMEL_MESSAGE_INFO_USER_FLAGS))
#define camel_message_info_user_tags(mi) ((const CamelTag *)camel_message_info_ptr((const CamelMessageInfo *)mi, CAMEL_MESSAGE_INFO_USER_TAGS))

gboolean camel_message_info_user_flag(const CamelMessageInfo *mi, const char *id);
const char *camel_message_info_user_tag(const CamelMessageInfo *mi, const char *id);

gboolean camel_message_info_set_flags(CamelMessageInfo *mi, guint32 mask, guint32 set);
gboolean camel_message_info_set_user_flag(CamelMessageInfo *mi, const char *id, gboolean state);
gboolean camel_message_info_set_user_tag(CamelMessageInfo *mi, const char *id, const char *val);

/* debugging functions */
void camel_content_info_dump (CamelMessageContentInfo *ci, int depth);

void camel_message_info_dump (CamelMessageInfo *mi);

void camel_folder_summary_prepare_hash (CamelFolderSummary *summary);
void camel_folder_summary_kill_hash (CamelFolderSummary *summary);

void camel_message_info_clear_normal_flags (CamelMessageInfo *min);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ! _CAMEL_FOLDER_SUMMARY_H */
