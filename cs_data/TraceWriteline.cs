using System;
using System.Collections.Generic;
using System.Text;
using System.Diagnostics;

namespace Utils
{
    public class TraceF
    {
        //
        // Summary:
        //     Writes a message to the trace listeners in the System.Diagnostics.Trace.Listeners
        //     collection.
        //
        // Parameters:
        //   message:
        //     A message to write.
        [Conditional("TRACE")]
        public static void WriteLine(string message)
        {
            System.Diagnostics.Trace.WriteLine(message);
        }

        [Conditional("TRACE")]
        public static void WriteLine(string format, params object[] args)
        {
            System.Diagnostics.Trace.WriteLine(string.Format(format, args));
        }
    }
}
