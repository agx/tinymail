#include <string.h>

#include <tny-msg.h>
#include <tny-msg-iface.h>
#include <tny-msg-mime-part.h>
#include <tny-msg-mime-part-iface.h>
#include <tny-stream-iface.h>
#include <tny-msg-header.h>
#include <tny-msg-header-iface.h>
#include <tny-account-iface.h>
#include <tny-transport-account-iface.h>
#include <tny-transport-account.h>
#include <tny-stream-camel.h>

#include <camel/camel-folder.h>
#include <camel/camel.h>
#include <camel/camel-folder-summary.h>


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
get_pass_func (TnyAccountIface *account, const gchar *prompt)
{
	g_print ("asks pass\n");
	return g_strdup ("asdasdsadsad");
}

static void
send_test (void)
{
	TnyTransportAccountIface *account = 
		TNY_TRANSPORT_ACCOUNT_IFACE (tny_transport_account_new ());

	TnyMsgIface *msg = TNY_MSG_IFACE (tny_msg_new ());

	TnyMsgHeaderIface *header = 
		TNY_MSG_HEADER_IFACE (tny_msg_header_new ());

	/* TnyStreamIface *mime_stream = TNY_STREAM_IFACE 
		(tny_stream_camel_new (camel_stream_mem_new())); */

	const gchar /* *mime_text = "This is the text of a mime part", */
		    *body_text = "Hey Tinne,\n\n"
			"If you receive this message, it means tinymail works\n"
			"\n\nPhilip";

	TnyStreamIface *body_stream = TNY_STREAM_IFACE 
		(tny_stream_camel_new (
			camel_stream_mem_new_with_buffer 
				(body_text, strlen (body_text))));

	/*
	TnyMsgMimePartIface *mime_part = 
		TNY_MSG_MIME_PART_IFACE (tny_msg_mime_part_new (camel_mime_part_new()));
	*/

	/* tny_account_iface_set_user (TNY_ACCOUNT_IFACE (account), ""); */
	tny_account_iface_set_proto (TNY_ACCOUNT_IFACE (account), "smtp");
	tny_account_iface_set_hostname (TNY_ACCOUNT_IFACE (account), "SMTPSERVER");
	tny_account_iface_set_pass_func (TNY_ACCOUNT_IFACE (account), get_pass_func);

	tny_msg_header_iface_set_to (header, "Tinne Hannes <tinne dot hannes at gmail dot com>, Philip Van Hoof <spam at pvanhoof dot be>");
	tny_msg_header_iface_set_from (header, "Philip Van Hoof <spam at pvanhoof dot be>");
	tny_msg_header_iface_set_subject (header, "A little tinymail test for my girlfriend");

	tny_msg_iface_set_header (msg, header);


	/* tny_stream_iface_reset (mime_stream);
	tny_stream_iface_write (mime_stream, mime_text, strlen (mime_text));
	tny_stream_iface_reset (mime_stream); 

	tny_msg_mime_part_iface_construct_from_stream (mime_part, mime_stream, "text/plain");
	tny_msg_mime_part_iface_set_content_type (mime_part, "text/plain"); */

	tny_msg_mime_part_iface_set_content_type 
		(TNY_MSG_MIME_PART_IFACE (msg), "text/plain");

	tny_stream_iface_reset (body_stream);

	tny_msg_mime_part_iface_construct_from_stream 
		(TNY_MSG_MIME_PART_IFACE (msg), body_stream, "text/plain");

	/* TODO: Need to copy cameldatapart to camelmultipart stuff in the lib
	tny_msg_iface_add_part (msg, mime_part); */

	tny_transport_account_iface_send (account, msg);


	g_object_unref (G_OBJECT (body_stream));
	/* g_object_unref (G_OBJECT (mime_stream)); */
	/* g_object_unref (G_OBJECT (mime_part)); */
	g_object_unref (G_OBJECT (header));
	g_object_unref (G_OBJECT (msg));
	g_object_unref (G_OBJECT (account));

	return;
}

int main (int argc, char **argv)
{
	g_type_init ();
	g_thread_init (NULL);
	send_test ();

	return;
}
