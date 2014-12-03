using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Reflection;

namespace Id3Lib.Tests
{
    public static class Resources
    {
        public static Stream GetResource(string resource)
        {
            if (resource == null)
                throw new ArgumentNullException("resource");

            var stream = Assembly.GetExecutingAssembly().GetManifestResourceStream("Id3Lib.Tests.Resources." + resource);

            if (stream == null)
                throw new ArgumentException("resource not found");

            return stream;
        }
    }
}
