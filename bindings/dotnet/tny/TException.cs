namespace Tny {

	using System;
	using System.Runtime.InteropServices;
	
	public class TException : System.Exception
	{
		IntPtr errptr;
	
		public TException (IntPtr errptr) : base ()
		{
			this.errptr = errptr;
		}

		[DllImport("libtinymail-1.0.dll")]
		static extern IntPtr tny_error_get_message (IntPtr errptr);
		public override string Message {
			get {
				return GLib.Marshaller.Utf8PtrToString (tny_error_get_message (errptr));
			}
		}

		// [DllImport("libglib-2.0-0.dll")]
		// static extern void g_clear_error (ref IntPtr errptr);
		//
		// ~Exception ()
		// {
		// 	g_clear_error (ref errptr);
		// }
	}
}

