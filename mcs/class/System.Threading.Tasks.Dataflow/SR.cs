//
// Resource strings referenced by the code.
//
// Copyright 2015 Xamarin Inc
//
// Use the following script to extract strings from CoreFxSources/Resources/Strings.resx
//
// var d = XDocument.Load ("Strings.resx");
// foreach (var j in d.XPathSelectElements ("/root/data")){ var v = j.XPathSelectElement ("value"); Console.WriteLine ("\tpublic const string {0}=\"{1}\";", j.Attribute ("name").Value, v.Value); }
//
partial class SR
{
	public const string ArgumentOutOfRange_BatchSizeMustBeNoGreaterThanBoundedCapacity="Number must be no greater than the value specified in BoundedCapacity.";
	public const string ArgumentOutOfRange_GenericPositive="Number must be positive.";
	public const string ArgumentOutOfRange_NeedNonNegOrNegative1="Number must be either non-negative and less than or equal to Int32.MaxValue or -1";
	public const string Argument_BoundedCapacityNotSupported="BoundedCapacity must be Unbounded or -1 for this dataflow block.";
	public const string Argument_CantConsumeFromANullSource="The argument must be false if no source from which to consume is specified.";
	public const string Argument_InvalidMessageHeader="The DataflowMessageHeader instance does not represent a valid message header.";
	public const string Argument_InvalidMessageId="To construct a DataflowMessageHeader instance, either pass a non-zero value or use the parameterless constructor.";
	public const string Argument_InvalidSourceForFilteredLink="This block must only be used with the source from which it was created.";
	public const string Argument_NonGreedyNotSupported="Greedy must be true for this dataflow block.";
	public const string InvalidOperation_DataNotAvailableForReceive="The source completed without providing data to receive.";
	public const string InvalidOperation_FailedToConsumeReservedMessage="The target block failed to consume a message it had successfully reserved.";
	public const string InvalidOperation_MessageNotReservedByTarget="The target does not have the message reserved.";
	public const string NotSupported_MemberNotNeeded="This member is not supported on this dataflow block. The block is intended for a specific purpose that does not utilize this member.";
	public const string ConcurrentCollection_SyncRoot_NotSupported="The SyncRoot property may not be used for the synchronization of concurrent collections.";
}