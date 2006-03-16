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

	TnyStreamIface *stream;

	TnyMsgMimePartIface *body = 
		TNY_MSG_MIME_PART_IFACE (tny_msg_mime_part_new (camel_mime_part_new()));

	const gchar *the_text = "This is the body";

	//tny_account_iface_set_user (TNY_ACCOUNT_IFACE (account), "");
	tny_account_iface_set_proto (TNY_ACCOUNT_IFACE (account), "smtp");
	tny_account_iface_set_hostname (TNY_ACCOUNT_IFACE (account), "adasdasd");
	tny_account_iface_set_pass_func (TNY_ACCOUNT_IFACE (account), get_pass_func);

	tny_msg_header_iface_set_to (header, "sadadsad");
	tny_msg_header_iface_set_from (header, "sadasdasd");

	tny_msg_iface_set_header (msg, header);

	stream = tny_msg_mime_part_iface_get_stream (body);

	tny_stream_iface_write (stream, the_text, strlen (the_text));

	//tny_msg_iface_add_part (msg, body);

	tny_transport_account_iface_send (account, msg);

}

int main (int argc, char **argv)
{
	g_type_init ();
	g_thread_init (NULL);
	send_test ();
}
