
namespace Tny {

	using System;

	public class TError {
		IntPtr handle;

		public IntPtr Handle {
			get {
				return handle;				
			}
			set {
				handle = value;
			}
		}


		
		public TError (IntPtr Handle) {
			handle = Handle;
		}

		public static TError New (IntPtr Handle) {
			return new TError (Handle);
		}
	}
}
