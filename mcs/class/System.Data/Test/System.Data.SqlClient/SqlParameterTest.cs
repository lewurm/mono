//
// SqlParameterTest.cs - NUnit Test Cases for testing the
//                          SqlParameter class
// Author:
//	Senganal T (tsenganal@novell.com)
//	Amit Biswas (amit@amitbiswas.com)
//	Gert Driesen (drieseng@users.sourceforge.net)
//
// Copyright (c) 2004 Novell Inc., and the individuals listed
// on the ChangeLog entries.
//
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

using System;
using System.Data;
using System.Data.Common;
using System.Data.SqlClient;
using System.Data.SqlTypes;

using NUnit.Framework;

namespace MonoTests.System.Data.SqlClient
{
	[TestFixture]
	public class SqlParameterTest
	{
		[Test]
		public void Constructor1 ()
		{
			SqlParameter p = new SqlParameter ();
			Assert.AreEqual (DbType.String, p.DbType, "DbType");
			Assert.AreEqual (ParameterDirection.Input, p.Direction, "Direction");
			Assert.IsFalse (p.IsNullable, "IsNullable");
#if NET_2_0
			Assert.AreEqual (0, p.LocaleId, "LocaleId");
#endif
			Assert.AreEqual (string.Empty, p.ParameterName, "ParameterName");
			Assert.AreEqual (0, p.Precision, "Precision");
			Assert.AreEqual (0, p.Scale, "Scale");
			Assert.AreEqual (0, p.Size, "Size");
			Assert.AreEqual (string.Empty, p.SourceColumn, "SourceColumn");
#if NET_2_0
			Assert.IsFalse (p.SourceColumnNullMapping, "SourceColumnNullMapping");
#endif
			Assert.AreEqual (DataRowVersion.Current, p.SourceVersion, "SourceVersion");
			Assert.AreEqual (SqlDbType.NVarChar, p.SqlDbType, "SqlDbType");
#if NET_2_0
			Assert.IsNull (p.SqlValue, "SqlValue");
#endif
			Assert.IsNull (p.Value, "Value");
#if NET_2_0
			Assert.AreEqual (string.Empty, p.XmlSchemaCollectionDatabase, "XmlSchemaCollectionDatabase");
			Assert.AreEqual (string.Empty, p.XmlSchemaCollectionName, "XmlSchemaCollectionName");
			Assert.AreEqual (string.Empty, p.XmlSchemaCollectionOwningSchema, "XmlSchemaCollectionOwningSchema");
#endif
		}

		[Test]
		public void Constructor2_Value_DateTime ()
		{
			DateTime value = new DateTime (2004, 8, 24);

			SqlParameter p = new SqlParameter ("address", value);
			Assert.AreEqual (DbType.DateTime, p.DbType, "B:DbType");
			Assert.AreEqual (ParameterDirection.Input, p.Direction, "B:Direction");
			Assert.IsFalse (p.IsNullable, "B:IsNullable");
#if NET_2_0
			Assert.AreEqual (0, p.LocaleId, "B:LocaleId");
#endif
			Assert.AreEqual ("address", p.ParameterName, "B:ParameterName");
			Assert.AreEqual (0, p.Precision, "B:Precision");
			Assert.AreEqual (0, p.Scale, "B:Scale");
			//Assert.AreEqual (0, p.Size, "B:Size");
			Assert.AreEqual (string.Empty, p.SourceColumn, "B:SourceColumn");
#if NET_2_0
			Assert.IsFalse (p.SourceColumnNullMapping, "B:SourceColumnNullMapping");
#endif
			Assert.AreEqual (DataRowVersion.Current, p.SourceVersion, "B:SourceVersion");
			Assert.AreEqual (SqlDbType.DateTime, p.SqlDbType, "B:SqlDbType");
#if NET_2_0
			// FIXME
			//Assert.AreEqual (new SqlDateTime (value), p.SqlValue, "B:SqlValue");
#endif
			Assert.AreEqual (value, p.Value, "B:Value");
#if NET_2_0
			Assert.AreEqual (string.Empty, p.XmlSchemaCollectionDatabase, "B:XmlSchemaCollectionDatabase");
			Assert.AreEqual (string.Empty, p.XmlSchemaCollectionName, "B:XmlSchemaCollectionName");
			Assert.AreEqual (string.Empty, p.XmlSchemaCollectionOwningSchema, "B:XmlSchemaCollectionOwningSchema");
#endif
		}

		[Test]
		public void Constructor2_Value_DBNull ()
		{
			SqlParameter p = new SqlParameter ("address", DBNull.Value);
			Assert.AreEqual (DbType.String, p.DbType, "B:DbType");
			Assert.AreEqual (ParameterDirection.Input, p.Direction, "B:Direction");
			Assert.IsFalse (p.IsNullable, "B:IsNullable");
#if NET_2_0
			Assert.AreEqual (0, p.LocaleId, "B:LocaleId");
#endif
			Assert.AreEqual ("address", p.ParameterName, "B:ParameterName");
			Assert.AreEqual (0, p.Precision, "B:Precision");
			Assert.AreEqual (0, p.Scale, "B:Scale");
			Assert.AreEqual (0, p.Size, "B:Size");
			Assert.AreEqual (string.Empty, p.SourceColumn, "B:SourceColumn");
#if NET_2_0
			Assert.IsFalse (p.SourceColumnNullMapping, "B:SourceColumnNullMapping");
#endif
			Assert.AreEqual (DataRowVersion.Current, p.SourceVersion, "B:SourceVersion");
			Assert.AreEqual (SqlDbType.NVarChar, p.SqlDbType, "B:SqlDbType");
#if NET_2_0
			// FIXME
			//Assert.AreEqual (SqlString.Null, p.SqlValue, "B:SqlValue");
#endif
			Assert.AreEqual (DBNull.Value, p.Value, "B:Value");
#if NET_2_0
			Assert.AreEqual (string.Empty, p.XmlSchemaCollectionDatabase, "B:XmlSchemaCollectionDatabase");
			Assert.AreEqual (string.Empty, p.XmlSchemaCollectionName, "B:XmlSchemaCollectionName");
			Assert.AreEqual (string.Empty, p.XmlSchemaCollectionOwningSchema, "B:XmlSchemaCollectionOwningSchema");
#endif
		}

		[Test]
		public void Constructor2_Value_Null ()
		{
			SqlParameter p = new SqlParameter ("address", (Object) null);
			Assert.AreEqual (DbType.String, p.DbType, "A:DbType");
			Assert.AreEqual (ParameterDirection.Input, p.Direction, "A:Direction");
			Assert.IsFalse (p.IsNullable, "A:IsNullable");
#if NET_2_0
			Assert.AreEqual (0, p.LocaleId, "A:LocaleId");
#endif
			Assert.AreEqual ("address", p.ParameterName, "A:ParameterName");
			Assert.AreEqual (0, p.Precision, "A:Precision");
			Assert.AreEqual (0, p.Scale, "A:Scale");
			Assert.AreEqual (0, p.Size, "A:Size");
			Assert.AreEqual (string.Empty, p.SourceColumn, "A:SourceColumn");
#if NET_2_0
			Assert.IsFalse (p.SourceColumnNullMapping, "A:SourceColumnNullMapping");
#endif
			Assert.AreEqual (DataRowVersion.Current, p.SourceVersion, "A:SourceVersion");
			Assert.AreEqual (SqlDbType.NVarChar, p.SqlDbType, "A:SqlDbType");
#if NET_2_0
			Assert.IsNull (p.SqlValue, "A:SqlValue");
#endif
			Assert.IsNull (p.Value, "A:Value");
#if NET_2_0
			Assert.AreEqual (string.Empty, p.XmlSchemaCollectionDatabase, "A:XmlSchemaCollectionDatabase");
			Assert.AreEqual (string.Empty, p.XmlSchemaCollectionName, "A:XmlSchemaCollectionName");
			Assert.AreEqual (string.Empty, p.XmlSchemaCollectionOwningSchema, "A:XmlSchemaCollectionOwningSchema");
#endif
		}

#if NET_2_0
		[Test] // .ctor (String, SqlDbType, Int32, ParameterDirection, Byte, Byte, String, DataRowVersion, Boolean, Object, String, String, String)
		public void Constructor7 ()
		{
			SqlParameter p1 = new SqlParameter ("p1Name", SqlDbType.VarChar, 20,
							    ParameterDirection.InputOutput, 0, 0,
							    "srcCol", DataRowVersion.Original, false,
							    "foo", "database", "schema", "name");
			Assert.AreEqual (DbType.AnsiString, p1.DbType, "DbType");
			Assert.AreEqual (ParameterDirection.InputOutput, p1.Direction, "Direction");
			Assert.AreEqual (false, p1.IsNullable, "IsNullable");
			//Assert.AreEqual (999, p1.LocaleId, "#");
			Assert.AreEqual ("p1Name", p1.ParameterName, "ParameterName");
			Assert.AreEqual (0, p1.Precision, "Precision");
			Assert.AreEqual (0, p1.Scale, "Scale");
			Assert.AreEqual (20, p1.Size, "Size");
			Assert.AreEqual ("srcCol", p1.SourceColumn, "SourceColumn");
			Assert.AreEqual (false, p1.SourceColumnNullMapping, "SourceColumnNullMapping");
			Assert.AreEqual (DataRowVersion.Original, p1.SourceVersion, "SourceVersion");
			Assert.AreEqual (SqlDbType.VarChar, p1.SqlDbType, "SqlDbType");
			//Assert.AreEqual (3210, p1.SqlValue, "#");
			Assert.AreEqual ("foo", p1.Value, "Value");
			Assert.AreEqual ("database", p1.XmlSchemaCollectionDatabase, "XmlSchemaCollectionDatabase");
			Assert.AreEqual ("name", p1.XmlSchemaCollectionName, "XmlSchemaCollectionName");
			Assert.AreEqual ("schema", p1.XmlSchemaCollectionOwningSchema, "XmlSchemaCollectionOwningSchema");
		}

		[Test]
		public void CompareInfo ()
		{
			SqlParameter parameter = new SqlParameter ();
			Assert.AreEqual (SqlCompareOptions.None, parameter.CompareInfo, "#1");
			parameter.CompareInfo = SqlCompareOptions.IgnoreNonSpace;
			Assert.AreEqual (SqlCompareOptions.IgnoreNonSpace, parameter.CompareInfo, "#2");
		}
#endif

		[Test]
		public void InferType_Byte ()
		{
			Byte value = 0x0a;

			SqlParameter param = new SqlParameter ();
			param.Value = value;
			Assert.AreEqual (SqlDbType.TinyInt, param.SqlDbType, "#1");
			Assert.AreEqual (DbType.Byte, param.DbType, "#2");
		}

		[Test]
		public void InferType_ByteArray ()
		{
			Byte [] value = new Byte [] { 0x0a, 0x0d };

			SqlParameter param = new SqlParameter ();
			param.Value = value;
			Assert.AreEqual (SqlDbType.VarBinary, param.SqlDbType, "#1");
			Assert.AreEqual (DbType.Binary, param.DbType, "#2");
		}

		[Test]
#if NET_2_0
		[Category ("NotWorking")]
#endif
		public void InferType_Char ()
		{
			Char value = Char.MaxValue;

			SqlParameter param = new SqlParameter ();
#if NET_2_0
			param.Value = value;
			Assert.AreEqual (SqlDbType.NVarChar, param.SqlDbType, "#1");
			Assert.AreEqual (DbType.String, param.DbType, "#2");
#else
			try {
				param.Value = value;
				Assert.Fail ("#1");
			} catch (ArgumentException ex) {
				// The parameter data type of Char is invalid
				Assert.AreEqual (typeof (ArgumentException), ex.GetType (), "#2");
				Assert.IsNull (ex.InnerException, "#3");
				Assert.IsNotNull (ex.Message, "#4");
				Assert.IsNull (ex.ParamName, "#5");
			}
#endif
		}

		[Test]
#if NET_2_0
		[Category ("NotWorking")]
#endif
		public void InferType_CharArray ()
		{
			Char [] value = new Char [] { 'A', 'X' };

			SqlParameter param = new SqlParameter ();
#if NET_2_0
			param.Value = value;
			Assert.AreEqual (SqlDbType.NVarChar, param.SqlDbType, "#1");
			Assert.AreEqual (DbType.String, param.DbType, "#2");
#else
			try {
				param.Value = value;
				Assert.Fail ("#1");
			} catch (FormatException) {
				// appears to be bug in .NET 1.1 while constructing
				// exception message
			} catch (ArgumentException ex) {
				// The parameter data type of Char[] is invalid
				Assert.AreEqual (typeof (ArgumentException), ex.GetType (), "#2");
				Assert.IsNull (ex.InnerException, "#3");
				Assert.IsNotNull (ex.Message, "#4");
				Assert.IsNull (ex.ParamName, "#5");
			}
#endif
		}

		[Test]
		public void InferType_DateTime ()
		{
			DateTime value;
			SqlParameter param;

			value = DateTime.Now;
			param = new SqlParameter ();
			param.Value = value;
			Assert.AreEqual (SqlDbType.DateTime, param.SqlDbType, "#A1");
			Assert.AreEqual (DbType.DateTime, param.DbType, "#A2");

			value = DateTime.Now;
			param = new SqlParameter ();
			param.Value = value;
			Assert.AreEqual (SqlDbType.DateTime, param.SqlDbType, "#B1");
			Assert.AreEqual (DbType.DateTime, param.DbType, "#B2");

			value = new DateTime (1973, 8, 13);
			param = new SqlParameter ();
			param.Value = value;
			Assert.AreEqual (SqlDbType.DateTime, param.SqlDbType, "#C1");
			Assert.AreEqual (DbType.DateTime, param.DbType, "#C2");
		}

		[Test]
		public void InferType_Decimal ()
		{
			Decimal value;
			SqlParameter param;
			
			value = Decimal.MaxValue;
			param = new SqlParameter ();
			param.Value = value;
			Assert.AreEqual (SqlDbType.Decimal, param.SqlDbType, "#A1");
			Assert.AreEqual (DbType.Decimal, param.DbType, "#A2");

			value = Decimal.MinValue;
			param = new SqlParameter ();
			param.Value = value;
			Assert.AreEqual (SqlDbType.Decimal, param.SqlDbType, "#B1");
			Assert.AreEqual (DbType.Decimal, param.DbType, "#B2");

			value = 214748.364m;
			param = new SqlParameter ();
			param.Value = value;
			Assert.AreEqual (SqlDbType.Decimal, param.SqlDbType, "#C1");
			Assert.AreEqual (DbType.Decimal, param.DbType, "#C2");
		}

		[Test]
		public void InferType_Double ()
		{
			Double value;
			SqlParameter param;
			
			value = Double.MaxValue;
			param = new SqlParameter ();
			param.Value = value;
			Assert.AreEqual (SqlDbType.Float, param.SqlDbType, "#A1");
			Assert.AreEqual (DbType.Double, param.DbType, "#A2");

			value = Double.MinValue;
			param = new SqlParameter ();
			param.Value = value;
			Assert.AreEqual (SqlDbType.Float, param.SqlDbType, "#B1");
			Assert.AreEqual (DbType.Double, param.DbType, "#B2");

			value = 0d;
			param = new SqlParameter ();
			param.Value = value;
			Assert.AreEqual (SqlDbType.Float, param.SqlDbType, "#C1");
			Assert.AreEqual (DbType.Double, param.DbType, "#C2");
		}

		[Test]
		public void InferType_Enum ()
		{
			SqlParameter param;

			param = new SqlParameter ();
			param.Value = ByteEnum.A;
			Assert.AreEqual (SqlDbType.TinyInt, param.SqlDbType, "#A1");
			Assert.AreEqual (DbType.Byte, param.DbType, "#A2");

			param = new SqlParameter ();
			param.Value = Int64Enum.A;
			Assert.AreEqual (SqlDbType.BigInt, param.SqlDbType, "#B1");
			Assert.AreEqual (DbType.Int64, param.DbType, "#B2");
		}

		[Test]
		public void InferType_Guid ()
		{
			Guid value = Guid.NewGuid ();

			SqlParameter param = new SqlParameter ();
			param.Value = value;
			Assert.AreEqual (SqlDbType.UniqueIdentifier, param.SqlDbType, "#1");
			Assert.AreEqual (DbType.Guid, param.DbType, "#2");
		}

		[Test]
		public void InferType_Int16 ()
		{
			Int16 value;
			SqlParameter param;
			
			value = Int16.MaxValue;
			param = new SqlParameter ();
			param.Value = value;
			Assert.AreEqual (SqlDbType.SmallInt, param.SqlDbType, "#A1");
			Assert.AreEqual (DbType.Int16, param.DbType, "#A2");

			value = Int16.MinValue;
			param = new SqlParameter ();
			param.Value = value;
			Assert.AreEqual (SqlDbType.SmallInt, param.SqlDbType, "#B1");
			Assert.AreEqual (DbType.Int16, param.DbType, "#B2");

			value = (Int16) 0;
			param = new SqlParameter ();
			param.Value = value;
			Assert.AreEqual (SqlDbType.SmallInt, param.SqlDbType, "#C1");
			Assert.AreEqual (DbType.Int16, param.DbType, "#C2");
		}

		[Test]
		public void InferType_Int32 ()
		{
			Int32 value;
			SqlParameter param;
			
			value = Int32.MaxValue;
			param = new SqlParameter ();
			param.Value = value;
			Assert.AreEqual (SqlDbType.Int, param.SqlDbType, "#A1");
			Assert.AreEqual (DbType.Int32, param.DbType, "#A2");

			value = Int32.MinValue;
			param = new SqlParameter ();
			param.Value = value;
			Assert.AreEqual (SqlDbType.Int, param.SqlDbType, "#B1");
			Assert.AreEqual (DbType.Int32, param.DbType, "#B2");

			value = 0;
			param = new SqlParameter ();
			param.Value = value;
			Assert.AreEqual (SqlDbType.Int, param.SqlDbType, "#C1");
			Assert.AreEqual (DbType.Int32, param.DbType, "#C2");
		}

		[Test]
		public void InferType_Int64 ()
		{
			Int64 value;
			SqlParameter param;
			
			value = Int64.MaxValue;
			param = new SqlParameter ();
			param.Value = value;
			Assert.AreEqual (SqlDbType.BigInt, param.SqlDbType, "#A1");
			Assert.AreEqual (DbType.Int64, param.DbType, "#A2");

			value = Int64.MinValue;
			param = new SqlParameter ();
			param.Value = value;
			Assert.AreEqual (SqlDbType.BigInt, param.SqlDbType, "#B1");
			Assert.AreEqual (DbType.Int64, param.DbType, "#B2");

			value = 0L;
			param = new SqlParameter ();
			param.Value = value;
			Assert.AreEqual (SqlDbType.BigInt, param.SqlDbType, "#C1");
			Assert.AreEqual (DbType.Int64, param.DbType, "#C2");
		}

		[Test]
#if NET_2_0
		[Category ("NotWorking")]
#endif
		public void InferType_Invalid ()
		{
			object [] notsupported = new object [] {
				UInt16.MaxValue,
				UInt32.MaxValue,
				UInt64.MaxValue,
				SByte.MaxValue,
				new SqlParameter ()
			};

			SqlParameter param = new SqlParameter ();

			for (int i = 0; i < notsupported.Length; i++) {
#if NET_2_0
				param.Value = notsupported [i];
				try {
					SqlDbType type = param.SqlDbType;
					Assert.Fail ("#A1:" + i + " (" + type + ")");
				} catch (ArgumentException ex) {
					// The parameter data type of ... is invalid
					Assert.AreEqual (typeof (ArgumentException), ex.GetType (), "#A2");
					Assert.IsNull (ex.InnerException, "#A3");
					Assert.IsNotNull (ex.Message, "#A4");
					Assert.IsNull (ex.ParamName, "#A5");
				}

				try {
					DbType type = param.DbType;
					Assert.Fail ("#B1:" + i + " (" + type + ")");
				} catch (ArgumentException ex) {
					// The parameter data type of ... is invalid
					Assert.AreEqual (typeof (ArgumentException), ex.GetType (), "#B2");
					Assert.IsNull (ex.InnerException, "#B3");
					Assert.IsNotNull (ex.Message, "#B4");
					Assert.IsNull (ex.ParamName, "#B5");
				}
#else
				try {
					param.Value = notsupported [i];
					Assert.Fail ("#A1:" + i);
				} catch (FormatException) {
					// appears to be bug in .NET 1.1 while
					// constructing exception message
				} catch (ArgumentException ex) {
					// The parameter data type of ... is invalid
					Assert.AreEqual (typeof (ArgumentException), ex.GetType (), "#A2");
					Assert.IsNull (ex.InnerException, "#A3");
					Assert.IsNotNull (ex.Message, "#A4");
					Assert.IsNull (ex.ParamName, "#A5");
				}
#endif
			}
		}

		[Test]
		public void InferType_Object ()
		{
			Object value = new Object ();

			SqlParameter param = new SqlParameter ();
			param.Value = value;
			Assert.AreEqual (SqlDbType.Variant, param.SqlDbType, "#1");
			Assert.AreEqual (DbType.Object, param.DbType, "#2");
		}

		[Test]
		public void InferType_Single ()
		{
			Single value = Single.MaxValue;

			SqlParameter param = new SqlParameter ();
			param.Value = value;
			Assert.AreEqual (SqlDbType.Real, param.SqlDbType, "#1");
			Assert.AreEqual (DbType.Single, param.DbType, "#2");
		}

		[Test]
		public void InferType_String ()
		{
			String value = "some text";

			SqlParameter param = new SqlParameter ();
			param.Value = value;
			Assert.AreEqual (SqlDbType.NVarChar, param.SqlDbType, "#1");
			Assert.AreEqual (DbType.String, param.DbType, "#2");
		}

		[Test]
#if NET_2_0
		[Category ("NotWorking")]
#endif
		public void InferType_TimeSpan ()
		{
			TimeSpan value = new TimeSpan (4, 6, 23);

			SqlParameter param = new SqlParameter ();
#if NET_2_0
			param.Value = value;
			Assert.AreEqual (SqlDbType.Time, param.SqlDbType, "#1");
			Assert.AreEqual (DbType.Time, param.DbType, "#2");
#else
			try {
				param.Value = value;
				Assert.Fail ("#1");
			} catch (FormatException) {
				// appears to be bug in .NET 1.1 while constructing
				// exception message
			} catch (ArgumentException ex) {
				Assert.AreEqual (typeof (ArgumentException), ex.GetType (), "#2");
				Assert.IsNull (ex.InnerException, "#3");
				Assert.IsNotNull (ex.Message, "#4");
				Assert.IsNull (ex.ParamName, "#5");
			}
#endif
		}

#if NET_2_0
		[Test]
		public void LocaleId ()
		{
			SqlParameter parameter = new SqlParameter ();
			Assert.AreEqual (0, parameter.LocaleId, "#1");
			parameter.LocaleId = 15;
			Assert.AreEqual(15, parameter.LocaleId, "#2");
		}
#endif

		[Test] // bug #320196
		public void ParameterNullTest ()
		{
			SqlParameter param = new SqlParameter ("param", SqlDbType.Decimal);
			Assert.AreEqual (0, param.Scale, "#A1");
			param.Value = DBNull.Value;
			Assert.AreEqual (0, param.Scale, "#A2");

			param = new SqlParameter ("param", SqlDbType.Int);
			Assert.AreEqual (0, param.Scale, "#B1");
			param.Value = DBNull.Value;
			Assert.AreEqual (0, param.Scale, "#B2");
		}

		[Test]
		public void ParameterType ()
		{
			SqlParameter p;

			// If Type is not set, then type is inferred from the value
			// assigned. The Type should be inferred everytime Value is assigned
			// If value is null or DBNull, then the current Type should be reset to NVarChar.
			p = new SqlParameter ();
			Assert.AreEqual (DbType.String, p.DbType, "#A1");
			Assert.AreEqual (SqlDbType.NVarChar, p.SqlDbType, "#A2");
			p.Value = DBNull.Value;
			Assert.AreEqual (DbType.String, p.DbType, "#B1");
			Assert.AreEqual (SqlDbType.NVarChar, p.SqlDbType, "#B2");
			p.Value = 1;
			Assert.AreEqual (DbType.Int32, p.DbType, "#C1");
			Assert.AreEqual (SqlDbType.Int, p.SqlDbType, "#C2");
			p.Value = DBNull.Value;
#if NET_2_0
			Assert.AreEqual (DbType.String, p.DbType, "#D1");
			Assert.AreEqual (SqlDbType.NVarChar, p.SqlDbType, "#D2");
#else
			Assert.AreEqual (DbType.Int32, p.DbType, "#D1");
			Assert.AreEqual (SqlDbType.Int, p.SqlDbType, "#D2");
#endif
			p.Value = new byte [] { 0x0a };
			Assert.AreEqual (DbType.Binary, p.DbType, "#E1");
			Assert.AreEqual (SqlDbType.VarBinary, p.SqlDbType, "#E2");
			p.Value = null;
#if NET_2_0
			Assert.AreEqual (DbType.String, p.DbType, "#F1");
			Assert.AreEqual (SqlDbType.NVarChar, p.SqlDbType, "#F2");
#else
			Assert.AreEqual (DbType.Binary, p.DbType, "#F1");
			Assert.AreEqual (SqlDbType.VarBinary, p.SqlDbType, "#F2");
#endif
			p.Value = DateTime.Now;
			Assert.AreEqual (DbType.DateTime, p.DbType, "#G1");
			Assert.AreEqual (SqlDbType.DateTime, p.SqlDbType, "#G2");
			p.Value = null;
#if NET_2_0
			Assert.AreEqual (DbType.String, p.DbType, "#H1");
			Assert.AreEqual (SqlDbType.NVarChar, p.SqlDbType, "#H2");
#else
			Assert.AreEqual (DbType.DateTime, p.DbType, "#H1");
			Assert.AreEqual (SqlDbType.DateTime, p.SqlDbType, "#H2");
#endif

			// If DbType is set, then the SqlDbType should not be
			// inferred from the value assigned.
			p = new SqlParameter ();
			p.DbType = DbType.DateTime;
			Assert.AreEqual (SqlDbType.DateTime, p.SqlDbType, "#I1");
			p.Value = 1;
			Assert.AreEqual (SqlDbType.DateTime, p.SqlDbType, "#I2");
			p.Value = null;
			Assert.AreEqual (SqlDbType.DateTime, p.SqlDbType, "#I3");
			p.Value = DBNull.Value;
			Assert.AreEqual (SqlDbType.DateTime, p.SqlDbType, "#I4");

			// If SqlDbType is set, then the DbType should not be
			// inferred from the value assigned.
			p = new SqlParameter ();
			p.SqlDbType = SqlDbType.VarBinary;
			Assert.AreEqual (SqlDbType.VarBinary, p.SqlDbType, "#J1");
			p.Value = 1;
			Assert.AreEqual (SqlDbType.VarBinary, p.SqlDbType, "#J2");
			p.Value = null;
			Assert.AreEqual (SqlDbType.VarBinary, p.SqlDbType, "#J3");
			p.Value = DBNull.Value;
			Assert.AreEqual (SqlDbType.VarBinary, p.SqlDbType, "#J4");
		}

		[Test]
		public void InferType_Boolean ()
		{
			Boolean value;
			SqlParameter param;

			value = false;
			param = new SqlParameter ();
			param.Value = value;
			Assert.AreEqual (SqlDbType.Bit, param.SqlDbType, "#A1");
			Assert.AreEqual (DbType.Boolean, param.DbType, "#A2");

			value = true;
			param = new SqlParameter ();
			param.Value = value;
			Assert.AreEqual (SqlDbType.Bit, param.SqlDbType, "#B1");
			Assert.AreEqual (DbType.Boolean, param.DbType, "#B2");
		}

		[Test]
		public void ParameterName ()
		{
			SqlParameter p = new SqlParameter ();
			p.ParameterName = "name";
			Assert.AreEqual ("name", p.ParameterName, "#A:ParameterName");
			Assert.AreEqual (string.Empty, p.SourceColumn, "#A:SourceColumn");

			p.ParameterName = null;
			Assert.AreEqual (string.Empty, p.ParameterName, "#B:ParameterName");
			Assert.AreEqual (string.Empty, p.SourceColumn, "#B:SourceColumn");

			p.ParameterName = " ";
			Assert.AreEqual (" ", p.ParameterName, "#C:ParameterName");
			Assert.AreEqual (string.Empty, p.SourceColumn, "#C:SourceColumn");

			p.ParameterName = " name ";
			Assert.AreEqual (" name ", p.ParameterName, "#D:ParameterName");
			Assert.AreEqual (string.Empty, p.SourceColumn, "#D:SourceColumn");

			p.ParameterName = string.Empty;
			Assert.AreEqual (string.Empty, p.ParameterName, "#E:ParameterName");
			Assert.AreEqual (string.Empty, p.SourceColumn, "#E:SourceColumn");
		}

#if NET_2_0
		[Test]
		public void ResetDbType ()
		{
			SqlParameter p;

			//Parameter with an assigned value but no DbType specified
			p = new SqlParameter ("foo", 42);
			p.ResetDbType ();
			Assert.AreEqual (DbType.Int32, p.DbType, "#A:DbType");
			Assert.AreEqual (SqlDbType.Int, p.SqlDbType, "#A:SqlDbType");
			Assert.AreEqual (42, p.Value, "#A:Value");

			p.DbType = DbType.DateTime; //assigning a DbType
			Assert.AreEqual (DbType.DateTime, p.DbType, "#B:DbType1");
			Assert.AreEqual (SqlDbType.DateTime, p.SqlDbType, "#B:SqlDbType1");
			p.ResetDbType ();
			Assert.AreEqual (DbType.Int32, p.DbType, "#B:DbType2");
			Assert.AreEqual (SqlDbType.Int, p.SqlDbType, "#B:SqlDbtype2");

			//Parameter with an assigned SqlDbType but no specified value
			p = new SqlParameter ("foo", SqlDbType.Int);
			p.ResetDbType ();
			Assert.AreEqual (DbType.String, p.DbType, "#C:DbType");
			Assert.AreEqual (SqlDbType.NVarChar, p.SqlDbType, "#C:SqlDbType");

			p.DbType = DbType.DateTime; //assigning a SqlDbType
			Assert.AreEqual (DbType.DateTime, p.DbType, "#D:DbType1");
			Assert.AreEqual (SqlDbType.DateTime, p.SqlDbType, "#D:SqlDbType1");
			p.ResetDbType ();
			Assert.AreEqual (DbType.String, p.DbType, "#D:DbType2");
			Assert.AreEqual (SqlDbType.NVarChar, p.SqlDbType, "#D:SqlDbType2");

			p = new SqlParameter ();
			p.Value = DateTime.MaxValue;
			Assert.AreEqual (DbType.DateTime, p.DbType, "#E:DbType1");
			Assert.AreEqual (SqlDbType.DateTime, p.SqlDbType, "#E:SqlDbType1");
			p.Value = null;
			p.ResetDbType ();
			Assert.AreEqual (DbType.String, p.DbType, "#E:DbType2");
			Assert.AreEqual (SqlDbType.NVarChar, p.SqlDbType, "#E:SqlDbType2");

			p = new SqlParameter ("foo", SqlDbType.VarChar);
			p.Value = DateTime.MaxValue;
			p.ResetDbType ();
			Assert.AreEqual (DbType.DateTime, p.DbType, "#F:DbType");
			Assert.AreEqual (SqlDbType.DateTime, p.SqlDbType, "#F:SqlDbType");
			Assert.AreEqual (DateTime.MaxValue, p.Value, "#F:Value");

			p = new SqlParameter ("foo", SqlDbType.VarChar);
			p.Value = DBNull.Value;
			p.ResetDbType ();
			Assert.AreEqual (DbType.String, p.DbType, "#G:DbType");
			Assert.AreEqual (SqlDbType.NVarChar, p.SqlDbType, "#G:SqlDbType");
			Assert.AreEqual (DBNull.Value, p.Value, "#G:Value");

			p = new SqlParameter ("foo", SqlDbType.VarChar);
			p.Value = null;
			p.ResetDbType ();
			Assert.AreEqual (DbType.String, p.DbType, "#G:DbType");
			Assert.AreEqual (SqlDbType.NVarChar, p.SqlDbType, "#G:SqlDbType");
			Assert.IsNull (p.Value, "#G:Value");
		}

		[Test]
		public void ResetSqlDbType ()
		{
			//Parameter with an assigned value but no SqlDbType specified
			SqlParameter p1 = new SqlParameter ("foo", 42);
			Assert.AreEqual (42, p1.Value, "#1");
			Assert.AreEqual (DbType.Int32, p1.DbType, "#2");
			Assert.AreEqual (SqlDbType.Int, p1.SqlDbType, "#3");

			p1.ResetSqlDbType ();
			Assert.AreEqual (DbType.Int32, p1.DbType, "#4 The parameter with value 42 must have DbType as Int32");
			Assert.AreEqual (SqlDbType.Int, p1.SqlDbType, "#5 The parameter with value 42 must have SqlDbType as Int");

			p1.SqlDbType = SqlDbType.DateTime; //assigning a SqlDbType
			Assert.AreEqual (DbType.DateTime, p1.DbType, "#6");
			Assert.AreEqual (SqlDbType.DateTime, p1.SqlDbType, "#7");
			p1.ResetSqlDbType (); //Resetting SqlDbType
			Assert.AreEqual (DbType.Int32, p1.DbType, "#8 Resetting SqlDbType must infer the type from the value");
			Assert.AreEqual (SqlDbType.Int, p1.SqlDbType, "#9 Resetting SqlDbType must infer the type from the value");

			//Parameter with an assigned SqlDbType but no specified value
			SqlParameter p2 = new SqlParameter ("foo", SqlDbType.Int);
			Assert.AreEqual (null, p2.Value, "#10");
			Assert.AreEqual (DbType.Int32, p2.DbType, "#11");
			Assert.AreEqual (SqlDbType.Int, p2.SqlDbType, "#12");

			//Although a SqlDbType is specified, calling ResetSqlDbType resets 
			//the SqlDbType and DbType properties to default values
			p2.ResetSqlDbType ();
			Assert.AreEqual (DbType.String, p2.DbType, "#13 Resetting SqlDbType must infer the type from the value");
			Assert.AreEqual (SqlDbType.NVarChar, p2.SqlDbType, "#14 Resetting SqlDbType must infer the type from the value");

			p2.SqlDbType = SqlDbType.DateTime; //assigning a SqlDbType
			Assert.AreEqual (DbType.DateTime, p2.DbType, "#15");
			Assert.AreEqual (SqlDbType.DateTime, p2.SqlDbType, "#16");
			p2.ResetSqlDbType (); //Resetting SqlDbType
			Assert.AreEqual (DbType.String, p2.DbType, "#17 Resetting SqlDbType must infer the type from the value");
			Assert.AreEqual (SqlDbType.NVarChar, p2.SqlDbType, "#18 Resetting SqlDbType must infer the type from the value");
		}
#endif

		[Test]
		public void SourceColumn ()
		{
			SqlParameter p = new SqlParameter ();
			p.SourceColumn = "name";
			Assert.AreEqual (string.Empty, p.ParameterName, "#A:ParameterName");
			Assert.AreEqual ("name", p.SourceColumn, "#A:SourceColumn");

			p.SourceColumn = null;
			Assert.AreEqual (string.Empty, p.ParameterName, "#B:ParameterName");
			Assert.AreEqual (string.Empty, p.SourceColumn, "#B:SourceColumn");

			p.SourceColumn = " ";
			Assert.AreEqual (string.Empty, p.ParameterName, "#C:ParameterName");
			Assert.AreEqual (" ", p.SourceColumn, "#C:SourceColumn");

			p.SourceColumn = " name ";
			Assert.AreEqual (string.Empty, p.ParameterName, "#D:ParameterName");
			Assert.AreEqual (" name ", p.SourceColumn, "#D:SourceColumn");

			p.SourceColumn = string.Empty;
			Assert.AreEqual (string.Empty, p.ParameterName, "#E:ParameterName");
			Assert.AreEqual (string.Empty, p.SourceColumn, "#E:SourceColumn");
		}

#if NET_2_0
		[Test]
		public void SourceColumnNullMapping ()
		{
			SqlParameter p = new SqlParameter ();
			Assert.IsFalse (p.SourceColumnNullMapping, "#1");
			p.SourceColumnNullMapping = true;
			Assert.IsTrue (p.SourceColumnNullMapping, "#2");
			p.SourceColumnNullMapping = false;
			Assert.IsFalse (p.SourceColumnNullMapping, "#3");
		}
#endif

		[Test]
		public void SqlDbTypeTest ()
		{
			SqlParameter p = new SqlParameter ("zipcode", 3510);
			p.SqlDbType = SqlDbType.DateTime;
			Assert.AreEqual (DbType.DateTime, p.DbType, "#A:DbType");
			Assert.AreEqual (SqlDbType.DateTime, p.SqlDbType, "#A:SqlDbType");
			Assert.AreEqual (3510, p.Value, "#A:Value");
			p.SqlDbType = SqlDbType.VarChar;
			Assert.AreEqual (DbType.AnsiString, p.DbType, "#B:DbType");
			Assert.AreEqual (SqlDbType.VarChar, p.SqlDbType, "#B:SqlDbType");
			Assert.AreEqual (3510, p.Value, "#B:Value");
		}

		[Test]
		public void SqlDbTypeTest_Value_Invalid ()
		{
			SqlParameter p = new SqlParameter ("zipcode", 3510);
			try {
				p.SqlDbType = (SqlDbType) 666;
				Assert.Fail ("#1");
			} catch (ArgumentOutOfRangeException ex) {
#if NET_2_0
				// The SqlDbType enumeration value, 666, is
				// invalid
				Assert.AreEqual (typeof (ArgumentOutOfRangeException), ex.GetType (), "#2");
				Assert.IsNull (ex.InnerException, "#3");
				Assert.IsNotNull (ex.Message, "#4");
				Assert.IsTrue (ex.Message.IndexOf ("666") != -1, "#5:" + ex.Message);
				Assert.AreEqual ("SqlDbType", ex.ParamName, "#6");
#else
				// Specified argument was out of the range of
				// valid values
				Assert.AreEqual (typeof (ArgumentOutOfRangeException), ex.GetType (), "#2");
				Assert.IsNull (ex.InnerException, "#3");
				Assert.IsNotNull (ex.Message, "#4");
				Assert.IsNotNull (ex.ParamName, "#5");
#endif
			}
		}

#if NET_2_0
		[Test]
		public void SqlValue ()
		{
			SqlParameter parameter = new SqlParameter ();
			Assert.IsNull (parameter.SqlValue, "#A1");

			parameter.SqlValue = "Char";
			Assert.AreEqual (SqlDbType.NVarChar, parameter.SqlDbType, "#B:SqlDbType");
			Assert.IsNotNull (parameter.SqlValue, "#B:SqlValue1");
			// FIXME
			//Assert.AreEqual (typeof (SqlString), parameter.SqlValue.GetType (), "#B:SqlValue2");
			//Assert.AreEqual ("Char", ((SqlString) parameter.SqlValue).Value, "#B:SqlValue3");
			Assert.AreEqual ("Char", parameter.Value, "#B:Value");

			parameter.SqlValue = 10;
			Assert.AreEqual (SqlDbType.Int, parameter.SqlDbType, "#C:SqlDbType");
			Assert.IsNotNull (parameter.SqlValue, "#C:SqlValue1");
			// FIXME
			//Assert.AreEqual (typeof (SqlInt32), parameter.SqlValue.GetType (), "#C:SqlValue2");
			//Assert.AreEqual (10, ((SqlInt32) parameter.SqlValue).Value, "#C:SqlValue3");
			Assert.AreEqual (10, parameter.Value, "#C:Value");
		}
#endif

		[Test]
		public void Value ()
		{
			SqlParameter p;

			p = new SqlParameter ("name", (Object) null);
			p.Value = 42;
			Assert.AreEqual (DbType.Int32, p.DbType, "#A:DbType");
			Assert.AreEqual (SqlDbType.Int, p.SqlDbType, "#A:SqlDbType");
			Assert.AreEqual (42, p.Value, "#A:Value");

			p.Value = DBNull.Value;
#if NET_2_0
			Assert.AreEqual (DbType.String, p.DbType, "#B:DbType");
			Assert.AreEqual (SqlDbType.NVarChar, p.SqlDbType, "#B:SqlDbType");
#else
			Assert.AreEqual (DbType.Int32, p.DbType, "#B:DbType");
			Assert.AreEqual (SqlDbType.Int, p.SqlDbType, "#B:SqlDbType");
#endif
			Assert.AreEqual (DBNull.Value, p.Value, "#B:Value");

			p.Value = DateTime.MaxValue;
			Assert.AreEqual (DbType.DateTime, p.DbType, "#C:DbType");
			Assert.AreEqual (SqlDbType.DateTime, p.SqlDbType, "#C:SqlDbType");
			Assert.AreEqual (DateTime.MaxValue, p.Value, "#C:Value");

			p.Value = null;
#if NET_2_0
			Assert.AreEqual (DbType.String, p.DbType, "#D:DbType");
			Assert.AreEqual (SqlDbType.NVarChar, p.SqlDbType, "#D:SqlDbType");
#else
			Assert.AreEqual (DbType.DateTime, p.DbType, "#D:DbType");
			Assert.AreEqual (SqlDbType.DateTime, p.SqlDbType, "#D:SqlDbType");
#endif
			Assert.IsNull (p.Value, "#D:Value");

			p = new SqlParameter ("zipcode", SqlDbType.Int);
			p.Value = DateTime.MaxValue;
			Assert.AreEqual (DbType.Int32, p.DbType, "#E:DbType");
			Assert.AreEqual (SqlDbType.Int, p.SqlDbType, "#E:SqlDbType");
			Assert.AreEqual (DateTime.MaxValue, p.Value, "#E:Value");

			p.Value = null;
			Assert.AreEqual (DbType.Int32, p.DbType, "#F:DbType");
			Assert.AreEqual (SqlDbType.Int, p.SqlDbType, "#F:SqlDbType");
			Assert.IsNull (p.Value, "#F:Value");

			p.Value = DBNull.Value;
			Assert.AreEqual (DbType.Int32, p.DbType, "#G:DbType");
			Assert.AreEqual (SqlDbType.Int, p.SqlDbType, "#G:SqlDbType");
			Assert.AreEqual (DBNull.Value, p.Value, "#G:Value");
		}

#if NET_2_0
		[Test]
		public void XmlSchemaTest ()
		{
			SqlParameter p1 = new SqlParameter ();
			
			//Testing default values
			Assert.AreEqual (String.Empty, p1.XmlSchemaCollectionDatabase,
					 "#1 Default value for XmlSchemaCollectionDatabase is an empty string");
			Assert.AreEqual (String.Empty, p1.XmlSchemaCollectionName,
					 "#2 Default value for XmlSchemaCollectionName is an empty string");
			Assert.AreEqual (String.Empty, p1.XmlSchemaCollectionOwningSchema,
					 "#3 Default value for XmlSchemaCollectionOwningSchema is an empty string");

			//Changing one property should not affect the remaining two properties
			p1.XmlSchemaCollectionDatabase = "database";
			Assert.AreEqual ("database", p1.XmlSchemaCollectionDatabase,
					 "#4 Default value for XmlSchemaCollectionDatabase is an empty string");
			Assert.AreEqual (String.Empty, p1.XmlSchemaCollectionName,
					 "#5 Default value for XmlSchemaCollectionName is an empty string");
			Assert.AreEqual (String.Empty, p1.XmlSchemaCollectionOwningSchema,
					 "#6 Default value for XmlSchemaCollectionOwningSchema is an empty string");

			p1.XmlSchemaCollectionName = "name";
			Assert.AreEqual ("database", p1.XmlSchemaCollectionDatabase,
					 "#7 Default value for XmlSchemaCollectionDatabase is an empty string");
			Assert.AreEqual ("name", p1.XmlSchemaCollectionName,
					 "#8 Default value for XmlSchemaCollectionName is an empty string");
			Assert.AreEqual (String.Empty, p1.XmlSchemaCollectionOwningSchema,
					 "#9 Default value for XmlSchemaCollectionOwningSchema is an empty string");

			p1.XmlSchemaCollectionOwningSchema = "schema";
			Assert.AreEqual ("database", p1.XmlSchemaCollectionDatabase,
					 "#10 Default value for XmlSchemaCollectionDatabase is an empty string");
			Assert.AreEqual ("name", p1.XmlSchemaCollectionName,
					 "#11 Default value for XmlSchemaCollectionName is an empty string");
			Assert.AreEqual ("schema", p1.XmlSchemaCollectionOwningSchema,
					 "#12 Default value for XmlSchemaCollectionOwningSchema is an empty string");

			//assigning null value stores default empty string
			p1.XmlSchemaCollectionDatabase = null;
			Assert.AreEqual (String.Empty, p1.XmlSchemaCollectionDatabase,
					 "#13 Default value for XmlSchemaCollectionDatabase is an empty string");
			Assert.AreEqual ("name", p1.XmlSchemaCollectionName,
					 "#14 Default value for XmlSchemaCollectionName is an empty string");
			Assert.AreEqual ("schema", p1.XmlSchemaCollectionOwningSchema,
					 "#15 Default value for XmlSchemaCollectionOwningSchema is an empty string");

			p1.XmlSchemaCollectionName = "";
			Assert.AreEqual (String.Empty, p1.XmlSchemaCollectionDatabase,
					 "#16 Default value for XmlSchemaCollectionDatabase is an empty string");
			Assert.AreEqual ("", p1.XmlSchemaCollectionName,
					 "#17 Default value for XmlSchemaCollectionName is an empty string");
			Assert.AreEqual ("schema", p1.XmlSchemaCollectionOwningSchema,
					 "#18 Default value for XmlSchemaCollectionOwningSchema is an empty string");

			//values are not trimmed
			p1.XmlSchemaCollectionOwningSchema = "  a  ";
			Assert.AreEqual (String.Empty, p1.XmlSchemaCollectionDatabase,
					 "#19 Default value for XmlSchemaCollectionDatabase is an empty string");
			Assert.AreEqual ("", p1.XmlSchemaCollectionName,
					 "#20 Default value for XmlSchemaCollectionName is an empty string");
			Assert.AreEqual ("  a  ", p1.XmlSchemaCollectionOwningSchema,
					 "#21 Default value for XmlSchemaCollectionOwningSchema is an empty string");
		}
#endif

		private enum ByteEnum : byte
		{
			A = 0x0a,
			B = 0x0d
		}

		private enum Int64Enum : long
		{
			A = long.MinValue,
			B = long.MaxValue
		}
	}
}
