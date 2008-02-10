
namespace Tny {

	using System;
	using System.Runtime.InteropServices;

	public class Error {
		IntPtr handle;

		public IntPtr Handle {
			get {
				return handle;				
			}
			set {
				handle = value;
			}
		}

                [DllImport("libtinymail-1.0.dll")]
		static extern IntPtr tny_error_get_message (IntPtr Handle);

                [DllImport("libtinymail-1.0.dll")]
		static extern int tny_error_get_code (IntPtr Handle);

		public string Message {
			get {
				if (Handle != IntPtr.Zero) 
					return GLib.Marshaller.Utf8PtrToString (tny_error_get_message (Handle));
				else return "";
			} 
		}
		
		public ErrorEnum ErrorEnum {
			get {
				if (Handle != IntPtr.Zero) 
					return (ErrorEnum) tny_error_get_code (Handle);
				else 
					return ErrorEnum.NoError;
			}
		}

		public Error (IntPtr Handle) {
			handle = Handle;
		}

		public static Error New (IntPtr Handle) {
			return new Error (Handle);
		}
	}
}
