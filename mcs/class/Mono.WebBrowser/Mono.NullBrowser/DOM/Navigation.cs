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
using System.Runtime.InteropServices;
using System.Text;
using Mono.WebBrowser;
using Mono.WebBrowser.DOM;

namespace Mono.NullBrowser.DOM
{
	internal class Navigation: INavigation
	{


		#region INavigation Members

		public bool CanGoBack {
			get {
				return false;
			}
		}

		public bool CanGoForward {
			get {
				return false;
			}
		}

		public bool Back ()
		{
			return false;
		}

		public bool Forward ()
		{
			return false;
		}

		public void Home ()
		{
		}

		public void Reload ()
		{
		}

		public void Reload (ReloadOption option)
		{
		}

		public void Stop ()
		{
		}
		
		
		/// <summary>
		/// Navigate to the page in the history, by index.
		/// </summary>
		/// <param name="index">
		/// A <see cref="System.Int32"/> representing an absolute index in the 
		/// history (that is, > -1 and < history length
		/// </param>
		public void Go (int index)
		{
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="index">
		/// A <see cref="System.Int32"/> representing an index in the 
		/// history, that can be relative or absolute depending on the relative argument
		/// </param>
		/// <param name="relative">
		/// A <see cref="System.Boolean"/> indicating whether the index is relative to 
		/// the current place in history or not (i.e., if relative = true, index can be
		/// positive or negative, and index=-1 means load the previous page in the history.
		/// if relative = false, index must be > -1, and index = 0 means load the first
		/// page of the history.
		/// </param>
		public void Go (int index, bool relative) {
		}
		
		public void Go (string url)
		{
		}

		public void Go (string url, LoadFlags flags) 
		{
		}

		public int HistoryCount {
			get {
				return 0;
			}
		}

		#endregion

		public override int GetHashCode () {
			return 0;
		}		
	}
}
