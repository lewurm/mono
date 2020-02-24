
// Metadata version: v4.0.30319


// =============== CLASS MEMBERS DECLARATION ===================

.class public auto ansi beforefieldinit Sample
       extends [System.Runtime]System.Object
{

  .method private hidebysig static string 
          Do() cil managed noinlining
  {
    // Code size       11 (0xb)
    .maxstack  1
    .locals init (string V_0)
    IL_0000:  nop
    IL_0001:  ldstr      "REPLACED!"
    IL_0006:  stloc.0
    IL_0007:  br.s       IL_0009

    IL_0009:  ldloc.0
    IL_000a:  ret
  } // end of method Sample::Do

} // end of class Sample


