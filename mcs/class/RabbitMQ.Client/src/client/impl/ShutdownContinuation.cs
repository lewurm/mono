// This source code is dual-licensed under the Apache License, version
// 2.0, and the Mozilla Public License, version 1.1.
//
// The APL v2.0:
//
//---------------------------------------------------------------------------
//   Copyright (C) 2007-2009 LShift Ltd., Cohesive Financial
//   Technologies LLC., and Rabbit Technologies Ltd.
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.
//---------------------------------------------------------------------------
//
// The MPL v1.1:
//
//---------------------------------------------------------------------------
//   The contents of this file are subject to the Mozilla Public License
//   Version 1.1 (the "License"); you may not use this file except in
//   compliance with the License. You may obtain a copy of the License at
//   http://www.rabbitmq.com/mpl.html
//
//   Software distributed under the License is distributed on an "AS IS"
//   basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
//   License for the specific language governing rights and limitations
//   under the License.
//
//   The Original Code is The RabbitMQ .NET Client.
//
//   The Initial Developers of the Original Code are LShift Ltd,
//   Cohesive Financial Technologies LLC, and Rabbit Technologies Ltd.
//
//   Portions created before 22-Nov-2008 00:00:00 GMT by LShift Ltd,
//   Cohesive Financial Technologies LLC, or Rabbit Technologies Ltd
//   are Copyright (C) 2007-2008 LShift Ltd, Cohesive Financial
//   Technologies LLC, and Rabbit Technologies Ltd.
//
//   Portions created by LShift Ltd are Copyright (C) 2007-2009 LShift
//   Ltd. Portions created by Cohesive Financial Technologies LLC are
//   Copyright (C) 2007-2009 Cohesive Financial Technologies
//   LLC. Portions created by Rabbit Technologies Ltd are Copyright
//   (C) 2007-2009 Rabbit Technologies Ltd.
//
//   All Rights Reserved.
//
//   Contributor(s): ______________________________________.
//
//---------------------------------------------------------------------------
using RabbitMQ.Client;
using RabbitMQ.Util;

namespace RabbitMQ.Client.Impl
{
    public class ShutdownContinuation
    {
        public readonly BlockingCell m_cell = new BlockingCell();
        public ShutdownContinuation() { }

        // You will note there are two practically identical overloads
        // of OnShutdown() here. This is because Microsoft's C#
        // compilers do not consistently support the Liskov
        // substitutability principle. When I use
        // OnShutdown(object,ShutdownEventArgs), the compilers
        // complain that OnShutdown can't be placed into a
        // ConnectionShutdownEventHandler because object doesn't
        // "match" IConnection, even though there's no context in
        // which the program could Go Wrong were it to accept the
        // code. The same problem appears for
        // ModelShutdownEventHandler. The .NET 1.1 compiler complains
        // about these two cases, and the .NET 2.0 compiler does not -
        // presumably they improved the type checker with the new
        // release of the compiler.

        public virtual void OnShutdown(IConnection sender, ShutdownEventArgs reason)
        {
            m_cell.Value = reason;
        }

        public virtual void OnShutdown(IModel sender, ShutdownEventArgs reason)
        {
            m_cell.Value = reason;
        }

        public virtual ShutdownEventArgs Wait()
        {
            return (ShutdownEventArgs)m_cell.Value;
        }
    }
}
