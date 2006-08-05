#include <string.h>

#include <tny-msg.h>
#include <tny-msg-iface.h>
#include <tny-mime-part.h>
#include <tny-mime-part-iface.h>
#include <tny-stream-iface.h>
#include <tny-msg-header.h>
#include <tny-msg-header-iface.h>
#include <tny-account-iface.h>
#include <tny-account-store-iface.h>
#include <tny-transport-account-iface.h>
#include <tny-transport-account.h>
#include <tny-stream-camel.h>

#include <camel/camel-folder.h>
#include <camel/camel.h>
#include <camel/camel-folder-summary.h>

/* Quick'n dirty account store implementation */

#define TNY_TYPE_ACCOUNT_STORE             (tny_account_store_get_type ())
#define TNY_ACCOUNT_STORE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TNY_TYPE_ACCOUNT_STORE, TnyAccountStore))
#define TNY_ACCOUNT_STORE_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), TNY_TYPE_ACCOUNT_STORE, TnyAccountStoreClass))
#define TNY_IS_ACCOUNT_STORE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TNY_TYPE_ACCOUNT_STORE))
#define TNY_IS_ACCOUNT_STORE_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), TNY_TYPE_ACCOUNT_STORE))
#define TNY_ACCOUNT_STORE_GET_CLASS(inst)  (G_TYPE_INSTANCE_GET_CLASS ((inst), TNY_TYPE_ACCOUNT_STORE, TnyAccountStoreClass))

typedef struct _TnyAccountStore TnyAccountStore;
typedef struct _TnyAccountStoreClass TnyAccountStoreClass;

struct _TnyAccountStore
{
	GObject parent;
};

struct _TnyAccountStoreClass
{
	GObjectClass parent;
};

GType               tny_account_store_get_type       (void);
TnyAccountStore*    tny_account_store_get_instance   (void);

static GObjectClass *parent_class = NULL;

static void 
camel_test (void)
{
	CamelMimeMessage *msg;
	gint i=0;
	GList *list=NULL;

	g_thread_init (NULL);
	camel_type_init ();

	CamelInternetAddress *addr = camel_internet_address_new ();
	camel_object_unref (CAMEL_OBJECT (addr));

	for (i=0; i<1000; i++)
	{
		msg = camel_mime_message_new ();
		list = g_list_append (list, msg);
	}

	list = g_list_first (list);

	while (list)
	{
		CamelMimeMessage *tmsg = list->data;

		camel_object_unref (CAMEL_OBJECT (tmsg));

		list = g_list_next (list);
	}

	g_list_free (list);

	return;
}

static gchar* 
get_pass_func (TnyAccountIface *account, const gchar *prompt, gboolean *cancel)
{
	g_print ("asks pass\n");
	return g_strdup ("asdasdsadsad");
}

static void
send_test (gboolean multipart)
{
	TnyTransportAccountIface *account = 
		TNY_TRANSPORT_ACCOUNT_IFACE (tny_transport_account_new ());

	TnyMsgIface *msg = TNY_MSG_IFACE (tny_msg_new ());

	TnyMsgHeaderIface *header = 
		TNY_MSG_HEADER_IFACE (tny_msg_header_new ());

	TnyStreamIface *mime_stream = TNY_STREAM_IFACE 
		(tny_stream_camel_new (camel_stream_mem_new())); 

	const gchar *body_text = "Hey Tinne,\n\n"
			"If you receive this message, it means tinymail works\n"
			"\n\nPhilip";

	TnyStreamIface *body_stream = TNY_STREAM_IFACE 
		(tny_stream_camel_new (camel_stream_mem_new_with_buffer 
				(body_text, strlen (body_text))));
	
	TnyMimePartIface *mime_part = 
		TNY_MIME_PART_IFACE (tny_mime_part_new (camel_mime_part_new()));

	TnyMimePartIface *body_part = 
		TNY_MIME_PART_IFACE (tny_mime_part_new (camel_mime_part_new()));
	
	tny_account_iface_set_account_store (TNY_ACCOUNT_IFACE (account), 
		TNY_ACCOUNT_STORE_IFACE (tny_account_store_get_instance ()));

	/* tny_account_iface_set_user (TNY_ACCOUNT_IFACE (account), ""); */
	tny_account_iface_set_proto (TNY_ACCOUNT_IFACE (account), "smtp");
	tny_account_iface_set_hostname (TNY_ACCOUNT_IFACE (account), "localhost");
	tny_account_iface_set_pass_func (TNY_ACCOUNT_IFACE (account), get_pass_func);

	tny_msg_header_iface_set_to (header, "Philip Van Hoof <spam@pvanhoof.be>");
	tny_msg_header_iface_set_from (header, "Philip Van Hoof <spam@pvanhoof.be>");
	tny_msg_header_iface_set_subject (header, "testing");

	tny_msg_iface_set_header (msg, header);

	tny_stream_iface_reset (mime_stream);

	if (multipart)
	{
		tny_stream_iface_write (mime_stream, "<pre>", 5);
		tny_stream_iface_write (mime_stream, body_text, strlen (body_text));
		tny_stream_iface_write (mime_stream, "</pre>", 6);
	
		tny_stream_iface_reset (mime_stream); 
	
		tny_mime_part_iface_construct_from_stream (mime_part, mime_stream, "text/html");
		tny_mime_part_iface_set_content_type (mime_part, "text/html"); 
	}

	tny_mime_part_iface_construct_from_stream (body_part, body_stream, "text/plain");
	tny_mime_part_iface_set_content_type (body_part, "text/plain"); 

	tny_mime_part_iface_set_content_type 
		(TNY_MIME_PART_IFACE (msg), "text/plain");

	tny_stream_iface_reset (body_stream);

	if (!multipart)
	{
		tny_mime_part_iface_construct_from_stream 
			(TNY_MIME_PART_IFACE (msg), body_stream, "text/plain");
	} else {
		tny_msg_iface_add_part (msg, body_part); 
		tny_msg_iface_add_part (msg, mime_part); 
	}
	tny_transport_account_iface_send (account, msg);

	g_object_unref (G_OBJECT (body_stream));
	g_object_unref (G_OBJECT (body_part)); 
	g_object_unref (G_OBJECT (mime_stream)); 
	g_object_unref (G_OBJECT (mime_part)); 
	g_object_unref (G_OBJECT (header));

	g_object_unref (G_OBJECT (msg));
	g_object_unref (G_OBJECT (account));

	return;
}

int main (int argc, char **argv)
{
	g_type_init ();
	g_thread_init (NULL);

	camel_type_init ();
	CamelInternetAddress *addr = camel_internet_address_new ();
	camel_object_unref (CAMEL_OBJECT (addr));

	send_test (TRUE);
	/* send_test (FALSE); */

	return;
}


/* Quick' n dirty account store implementation */

static const gchar*
tny_account_store_get_cache_dir (TnyAccountStoreIface *self)
{
	return ".tinymail";
}

static TnyAccountStore *the_singleton = NULL;

static GObject*
tny_account_store_constructor (GType type, guint n_construct_params,
			GObjectConstructParam *construct_params)
{
	GObject *object;

	/* TODO: potential problem: singleton without lock */

	if (!the_singleton)
	{
		object = G_OBJECT_CLASS (parent_class)->constructor (type,
				n_construct_params, construct_params);

		the_singleton = TNY_ACCOUNT_STORE (object);
	}
	else
	{
		object = g_object_ref (G_OBJECT (the_singleton));
		g_object_freeze_notify (G_OBJECT(the_singleton));
	}

	return object;
}


static void 
tny_account_store_class_init (TnyAccountStoreClass *class)
{
	GObjectClass *object_class;

	parent_class = g_type_class_peek_parent (class);
	object_class = (GObjectClass*) class;

	object_class->constructor = tny_account_store_constructor;

	return;
}

static void
tny_account_store_iface_init (gpointer g_iface, gpointer iface_data)
{
	TnyAccountStoreIfaceClass *klass = (TnyAccountStoreIfaceClass *)g_iface;

	klass->get_cache_dir_func = tny_account_store_get_cache_dir;

	return;
}


GType 
tny_account_store_get_type (void)
{
	static GType type = 0;

	if (type == 0) 
	{
		static const GTypeInfo info = 
		{
		  sizeof (TnyAccountStoreClass),
		  NULL,   /* base_init */
		  NULL,   /* base_finalize */
		  (GClassInitFunc) tny_account_store_class_init,   /* class_init */
		  NULL,   /* class_finalize */
		  NULL,   /* class_data */
		  sizeof (TnyAccountStore),
		  0,      /* n_preallocs */
		  NULL    /* instance_init */
		};

		static const GInterfaceInfo tny_account_store_iface_info = 
		{
		  (GInterfaceInitFunc) tny_account_store_iface_init, /* interface_init */
		  NULL,         /* interface_finalize */
		  NULL          /* interface_data */
		};

		type = g_type_register_static (G_TYPE_OBJECT,
			"TnyAccountStore",
			&info, 0);

		g_type_add_interface_static (type, TNY_TYPE_ACCOUNT_STORE_IFACE, 
			&tny_account_store_iface_info);
	}

	return type;
}

TnyAccountStore*
tny_account_store_get_instance (void)
{
	TnyAccountStore *self = g_object_new (TNY_TYPE_ACCOUNT_STORE, NULL);

	return self;
}
