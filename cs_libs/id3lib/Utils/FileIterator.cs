using System;
using System.Collections.Generic;
using System.Text;
using System.IO;

// based on examples in http://blogs.msdn.com/toub/archive/2004/10/31/250303.aspx
// (either way you should probably check each item to ensure it isn't a reparse point)
// 
// the interesting point is that every 'yield return' creates a new ienumerable class
// the iterative iterator creates less of these classes, 
// because it's using explicit iteration rather than recursion, 
// so it should run a bit quicker.

namespace Utils
{
    public static class FileIterator
    {
        /// <summary>
        /// Brad Abrams's recursive iterator for enumerating files
        /// </summary>
        /// <param name="path"></param>
        /// <returns>enumerable of string filenames</returns>
        /// <remarks>
        /// based on http://weblogs.asp.net/brada/archive/2004/03/04.aspx
        /// </remarks>
        public static IEnumerable<string> GetFilesRecursive(string path)
        {
            foreach (string s in Directory.GetFiles(path)) 
                yield return s;

            foreach (string s in Directory.GetDirectories(path))
            {
                foreach (string s1 in GetFilesRecursive(s)) 
                    yield return s1;
            }
        }

        /// <summary>
        /// adaptation of Brad Abrams's recursive iterator for enumerating files that match a pattern as fileinfo objects
        /// </summary>
        /// <param name="path"></param>
        /// <param name="pattern">e.g. *.mp3</param>
        /// <returns>enumerable of fileinfo objects</returns>
        /// <remarks>
        /// based on http://weblogs.asp.net/brada/archive/2004/03/04.aspx
        /// </remarks>
        public static IEnumerable<FileInfo> GetFileInfosRecursive(DirectoryInfo path, string pattern)
        {
            // return matching files in the current directory
            foreach (FileInfo s in path.GetFiles(pattern))
                yield return s;

            // recurse into each directory to return matching files there too
            foreach (DirectoryInfo s in path.GetDirectories())
            {
                foreach (FileInfo s1 in GetFileInfosRecursive(s, pattern))
                    yield return s1;
            }
        }

        /// <summary>
        /// Stephen Toub's iterative iterator for enumerating files
        /// </summary>
        /// <param name="path"></param>
        /// <returns>enumerable of string filenames</returns>
        /// <remarks>
        /// based on http://blogs.msdn.com/toub/archive/2004/10/31/250303.aspx
        /// </remarks>
        public static IEnumerable<string> GetFiles(string path)
        {
            Stack<string> stack = new Stack<string>();
            stack.Push(path);

            while (stack.Count > 0)
            {
                path = stack.Pop();
                foreach (string s in Directory.GetFiles(path))
                    yield return s;

                string[] dirs = Directory.GetDirectories(path);
                for (int i = dirs.Length - 1; i >= 0; i--) 
                    stack.Push(dirs[i]);
            }
        }

        /// <summary>
        /// Stephen Toub's iterative iterator for enumerating files that match a pattern
        /// </summary>
        /// <param name="path"></param>
        /// <param name="pattern">e.g. *.mp3</param>
        /// <returns>enumerable of string filenames</returns>
        /// <remarks>
        /// based on http://blogs.msdn.com/toub/archive/2004/10/31/250303.aspx
        /// </remarks>
        public static IEnumerable<string> GetFiles( string path, string pattern )
        {
            Stack<string> stack = new Stack<string>();
            stack.Push(path);

            while( stack.Count > 0 )
            {
                path = stack.Pop();
                foreach( string s in Directory.GetFiles(path, pattern) )
                    yield return s;

                string[] dirs = Directory.GetDirectories(path);
                for( int i = dirs.Length - 1; i >= 0; i-- )
                    stack.Push(dirs[i]);
            }
        }

        /// <summary>
        /// Adaptation of Stephen Toub's iterative iterator for enumerating files that match a pattern as fileinfo objects
        /// </summary>
        /// <param name="path"></param>
        /// <param name="pattern">e.g. *.mp3</param>
        /// <returns>enumerable of FileInfos</returns>
        /// <remarks>
        /// based on http://blogs.msdn.com/toub/archive/2004/10/31/250303.aspx
        /// </remarks>
        public static IEnumerable<FileInfo> GetFileInfos(string path, string pattern)
        {
            Stack<DirectoryInfo> stack = new Stack<DirectoryInfo>();
            stack.Push(new DirectoryInfo(path));

            while (stack.Count > 0)
            {
                DirectoryInfo dir = stack.Pop();

                // return matching files in the current directory
                foreach (FileInfo s in dir.GetFiles(pattern))
                    yield return s;

                // push each directory onto the stack,
                // so next time round the loop we return matching files there too
                DirectoryInfo[] dirs = dir.GetDirectories();
                for (int i = dirs.Length - 1; i >= 0; i--) 
                    stack.Push(dirs[i]);
            }
        }
    }
}
