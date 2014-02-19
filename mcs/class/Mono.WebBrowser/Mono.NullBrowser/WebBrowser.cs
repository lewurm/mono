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
// Copyright (c) 2013 Jo Shields
//
// Authors:
//	Andreia Gaita (avidigal@novell.com)
//	Jo Shields (directhex@apebox.org)
//

#undef debug

using System;
using System.Collections;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;
using System.Diagnostics;
using Mono.WebBrowser;
using Mono.WebBrowser.DOM;

namespace Mono.NullBrowser
{
	internal class WebBrowser : IWebBrowser
	{
		bool loaded;
		internal bool created = false;
		bool creating = false;

                internal DOM.Navigation navigation;

		internal Platform platform;
		internal Platform enginePlatform;
		System.ComponentModel.EventHandlerList events;
		System.ComponentModel.EventHandlerList domEvents;

		string statusText;

		bool streamingMode;
		
		internal Hashtable documents;

		int width;
		int height;
		bool isDirty;
		
		public WebBrowser (Platform platform)
		{
			this.platform = platform;
			loaded = Base.Init (this, platform);
			documents = new Hashtable ();
		}

		public bool Load (IntPtr handle, int width, int height)
		{
			loaded = Base.Bind (this, handle, width, height);
			return loaded;
		}

		bool Created {
			get {
				if (!creating && !created) {
					creating = true;
					created = Base.Create (this);
					if (created && isDirty) {
						isDirty = false;
						Base.Resize (this, width, height);
					}
				}
				return created;
			}
		}

		public void Shutdown ()
		{
			Base.Shutdown (this);
		}
		
		internal void Reset ()
		{
			this.DomEvents.Dispose ();
			this.domEvents = null;
			this.documents.Clear ();
		}

		public bool Initialized {
			get { return this.loaded; }
		}
		
		public IWindow Window {
			get {
				return null;
			}
		}

		public IDocument Document {
			get {
				return null;
			}
		}

		public INavigation Navigation {
			get {
                                if (!Created) return null;

                                if (navigation == null) {

                                        navigation = new DOM.Navigation ();
                                }
                                return navigation as INavigation;
			}
		}
		
		public string StatusText {
			get { return statusText; }
		}
		
		public bool Offline {
			get {
				return false;
			}
			set {
			}
		}
		
		internal System.ComponentModel.EventHandlerList DomEvents {
			get {
				if (domEvents == null)
					domEvents = new System.ComponentModel.EventHandlerList();

				return domEvents;
			}
		}

		internal System.ComponentModel.EventHandlerList Events {
			get {
				if (events == null)
					events = new System.ComponentModel.EventHandlerList();

				return events;
			}
		}
		
		#region Layout
		public void FocusIn (FocusOption focus)
		{
			if (!created) return;
			Base.Focus (this, focus);
		}
		public void FocusOut ()
		{
			if (!created) return;
			Base.Blur (this);
		}
		
		public void Activate ()
		{
			if (!Created) return;
			Base.Activate (this);
		}
		public void Deactivate ()
		{
			if (!created) return;
			Base.Deactivate (this);
		}

		public void Resize (int width, int height)
		{
			this.width = width;
			this.height = height;
			isDirty = true;
			if (!created) return;
			Base.Resize (this, width, height);			
		}

		public void Render (byte[] data)
		{
			if (!Created) return;
			if (data == null)
				throw new ArgumentNullException ("data");
			string html = System.Text.ASCIIEncoding.UTF8.GetString (data);
			Render (html);
		}

		public void Render (string html)
		{
			if (!Created) return;
			Render (html, "file:///", "text/html");
		}

				
		public void Render (string html, string uri, string contentType)
		{
			if (!Created) return;
			throw new Mono.WebBrowser.Exception (Mono.WebBrowser.Exception.ErrorCodes.Navigation);

			IntPtr native_html = Marshal.StringToHGlobalAnsi (html);
			Marshal.FreeHGlobal (native_html);


		}
		
		public void ExecuteScript (string script) {
			if (!Created) return;
			Base.EvalScript (this, script);
		}
				
		internal void AttachEvent (INode node, string eve, EventHandler handler) {
			string key = String.Intern (node.GetHashCode() + ":" + eve);
#if debug
			Console.Error.WriteLine ("Event Attached: " + key);
#endif
			DomEvents.AddHandler (key, handler);
		}

		internal void DetachEvent (INode node, string eve, EventHandler handler) {
			string key = String.Intern (node.GetHashCode() + ":" + eve);
#if debug			
			Console.Error.WriteLine ("Event Detached: " + key);
#endif			
			DomEvents.RemoveHandler (key, handler);
		}
		
		#endregion

		#region Events
		internal static object KeyDownEvent = new object ();
		internal static object KeyPressEvent = new object ();
		internal static object KeyUpEvent = new object ();
		internal static object MouseClickEvent = new object ();
		internal static object MouseDoubleClickEvent = new object ();
		internal static object MouseDownEvent = new object ();
		internal static object MouseEnterEvent = new object ();
		internal static object MouseLeaveEvent = new object ();
		internal static object MouseMoveEvent = new object ();
		internal static object MouseUpEvent = new object ();
		internal static object FocusEvent = new object ();
		internal static object BlurEvent = new object ();
		internal static object CreateNewWindowEvent = new object ();
		internal static object AlertEvent = new object ();
			
		internal static object LoadStartedEvent = new object ();
		internal static object LoadCommitedEvent = new object ();
		internal static object ProgressChangedEvent = new object ();
		internal static object LoadFinishedEvent = new object ();	
		
		internal static object LoadEvent = new object ();
		internal static object UnloadEvent = new object ();
		internal static object StatusChangedEvent = new object ();
		internal static object SecurityChangedEvent = new object ();
		internal static object ProgressEvent = new object ();
		internal static object ContextMenuEvent = new object ();
		
		internal static object NavigationRequestedEvent = new object ();
		
		public event NodeEventHandler KeyDown
		{
			add { Events.AddHandler (KeyDownEvent, value); }
			remove { Events.RemoveHandler (KeyDownEvent, value); }
		}

		public event NodeEventHandler KeyPress
		{
			add { Events.AddHandler (KeyPressEvent, value); }
			remove { Events.RemoveHandler (KeyPressEvent, value); }
		}
		public event NodeEventHandler KeyUp
		{
			add { Events.AddHandler (KeyUpEvent, value); }
			remove { Events.RemoveHandler (KeyUpEvent, value); }
		}
		public event NodeEventHandler MouseClick
		{
			add { Events.AddHandler (MouseClickEvent, value); }
			remove { Events.RemoveHandler (MouseClickEvent, value); }
		}
		public event NodeEventHandler MouseDoubleClick
		{
			add { Events.AddHandler (MouseDoubleClickEvent, value); }
			remove { Events.RemoveHandler (MouseDoubleClickEvent, value); }
		}
		public event NodeEventHandler MouseDown
		{
			add { Events.AddHandler (MouseDownEvent, value); }
			remove { Events.RemoveHandler (MouseDownEvent, value); }
		}
		public event NodeEventHandler MouseEnter
		{
			add { Events.AddHandler (MouseEnterEvent, value); }
			remove { Events.RemoveHandler (MouseEnterEvent, value); }
		}
		public event NodeEventHandler MouseLeave
		{
			add { Events.AddHandler (MouseLeaveEvent, value); }
			remove { Events.RemoveHandler (MouseLeaveEvent, value); }
		}
		public event NodeEventHandler MouseMove
		{
			add { Events.AddHandler (MouseMoveEvent, value); }
			remove { Events.RemoveHandler (MouseMoveEvent, value); }
		}
		public event NodeEventHandler MouseUp
		{
			add { Events.AddHandler (MouseUpEvent, value); }
			remove { Events.RemoveHandler (MouseUpEvent, value); }
		}
		public event EventHandler Focus
		{
			add { Events.AddHandler (FocusEvent, value); }
			remove { Events.RemoveHandler (FocusEvent, value); }
		}
		public event EventHandler Blur
		{
			add { Events.AddHandler (BlurEvent, value); }
			remove { Events.RemoveHandler (BlurEvent, value); }
		}
		public event CreateNewWindowEventHandler CreateNewWindow
		{
			add { Events.AddHandler (CreateNewWindowEvent, value); }
			remove { Events.RemoveHandler (CreateNewWindowEvent, value); }
		}
		public event AlertEventHandler Alert
		{
			add { Events.AddHandler (AlertEvent, value); }
			remove { Events.RemoveHandler (AlertEvent, value); }
		}
		public event EventHandler Loaded
		{
			add { Events.AddHandler (LoadEvent, value); }
			remove { Events.RemoveHandler (LoadEvent, value); }
		}
		public event EventHandler Unloaded
		{
			add { Events.AddHandler (UnloadEvent, value); }
			remove { Events.RemoveHandler (UnloadEvent, value); }
		}

 		public event StatusChangedEventHandler StatusChanged
		{
			add { Events.AddHandler (StatusChangedEvent, value); }
			remove { Events.RemoveHandler (StatusChangedEvent, value); }
		}
		
 		public event SecurityChangedEventHandler SecurityChanged
		{
			add { Events.AddHandler (SecurityChangedEvent, value); }
			remove { Events.RemoveHandler (SecurityChangedEvent, value); }
		}

		public event LoadStartedEventHandler LoadStarted
		{
			add { Events.AddHandler (LoadStartedEvent, value); }
			remove { Events.RemoveHandler (LoadStartedEvent, value); }
		}
		
		public event LoadCommitedEventHandler LoadCommited
		{
			add { Events.AddHandler (LoadCommitedEvent, value); }
			remove { Events.RemoveHandler (LoadCommitedEvent, value); }
		}		
		
		public event ProgressChangedEventHandler ProgressChanged
		{
			add { Events.AddHandler (ProgressChangedEvent, value); }
			remove { Events.RemoveHandler (ProgressChangedEvent, value); }
		}
		
		public event LoadFinishedEventHandler LoadFinished
		{
			add { Events.AddHandler (LoadFinishedEvent, value); }
			remove { Events.RemoveHandler (LoadFinishedEvent, value); }
		}

		public event ContextMenuEventHandler ContextMenuShown
		{
			add { Events.AddHandler (ContextMenuEvent, value); }
			remove { Events.RemoveHandler (ContextMenuEvent, value); }
		}

		public event NavigationRequestedEventHandler NavigationRequested
		{
			add { }
			remove { }
		}

		internal static object GenericEvent = new object ();
		internal event EventHandler Generic
		{
			add { Events.AddHandler (GenericEvent, value); }
			remove { Events.RemoveHandler (GenericEvent, value); }
		}

		#endregion


	}
}
