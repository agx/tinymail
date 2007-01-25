#include <string.h>
#include <camel/camel.h>

#define TEST "This is a test to compres to compress to compres"

int main ()
{
	CamelStream *in, *out, *com, *de;
	CamelInternetAddress *addr;

	g_thread_init (NULL);
	camel_type_init ();

	addr = camel_internet_address_new ();

	out = camel_stream_fs_new_with_name ("/tmp/testing", O_CREAT, S_IRWXU);
	in = camel_stream_mem_new_with_buffer (TEST, strlen (TEST));

	com = camel_stream_gzip_new (in, 7, CAMEL_STREAM_GZIP_ZIP, CAMEL_STREAM_GZIP_ZIP);
	de = camel_stream_gzip_new (out, 7, CAMEL_STREAM_GZIP_UNZIP, CAMEL_STREAM_GZIP_UNZIP);

	camel_stream_write_to_stream (com, de);

	camel_object_unref (CAMEL_OBJECT (com));
	camel_object_unref (CAMEL_OBJECT (de));
	camel_object_unref (CAMEL_OBJECT (out));
	camel_object_unref (CAMEL_OBJECT (in));

	return 0;

}

