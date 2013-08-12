// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
// Copyright (c) 2007, 2008 Novell, Inc.
//
// Authors:
//	Andreia Gaita (avidigal@novell.com)
//

using System;
using System.Text;
using System.Collections;
using System.Runtime.InteropServices;
using System.Diagnostics;
using Mono.WebBrowser;

namespace Mono.NullBrowser
{
	internal class Base
	{
		private static Hashtable boundControls;
		private static bool initialized;
		private static object initLock = new object ();
		private static string monoMozDir;

		private static bool isInitialized ()
		{
			if (!initialized)
				return false;
			return true;
		}

		static Base ()
		{
			boundControls = new Hashtable ();
		}

		public Base () { }

		public static bool Init (WebBrowser control, Platform platform)
		{
			lock (initLock) {
				if (!initialized) {
				
					initialized = true;
				}
			}
			return initialized;
		}

		public static bool Bind (WebBrowser control, IntPtr handle, int width, int height)
		{
			if (!isInitialized ())
				return false;

			
			return true;
		}

		public static bool Create (IWebBrowser control) {
			if (!isInitialized ())
				return false;

			return true;
		}

		public static void Shutdown (IWebBrowser control)
		{
			lock (initLock) {
				if (!initialized)
					return;
					
				
				boundControls.Remove (control);
				if (boundControls.Count == 0) {
					initialized = false;
				}
			}
		}

		// layout
		public static void Focus (IWebBrowser control, FocusOption focus)
		{
			if (!isInitialized ())
				return;

		}


		public static void Blur (IWebBrowser control)
		{
			if (!isInitialized ())
				return;

		}

		public static void Activate (IWebBrowser control)
		{
			if (!isInitialized ())
				return;

		}

		public static void Deactivate (IWebBrowser control)
		{
			if (!isInitialized ())
				return;

		}

		public static void Resize (IWebBrowser control, int width, int height)
		{
			if (!isInitialized ())
				return;

		}

		// navigation
		public static void Home (IWebBrowser control)
		{
			if (!isInitialized ())
				return;

		}


		public static IntPtr StringInit ()
		{
			return IntPtr.Zero;
		}

		public static void StringFinish (HandleRef str)
		{
		}

		public static string StringGet (HandleRef str)
		{
			return String.Empty;
		}

		public static void StringSet (HandleRef str, string text)
		{
		}


		public static object GetProxyForObject (IWebBrowser control, Guid iid, object obj)
		{
			return null;
		}

		public static string EvalScript (IWebBrowser control, string script)
		{
			return null;
		}


	}
}
