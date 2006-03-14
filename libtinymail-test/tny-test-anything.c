#include <camel/camel-folder.h>
#include <camel/camel.h>
#include <camel/camel-folder-summary.h>

int main (int argc, char **argv)
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

	return 0;
}
