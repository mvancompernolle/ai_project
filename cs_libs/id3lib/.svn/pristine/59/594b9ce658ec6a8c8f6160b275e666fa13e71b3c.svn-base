// Copyright(C) 2002-2003 Hugo Rumayor Montemayor, All rights reserved.

using System;
using System.Collections;
using System.IO;
using Id3Lib;
using System.Collections.Generic;

namespace TagEditor
{
    public class DirScan
    {
		#region Fields
		private List<string> _files = new List<string>();
		#endregion

		#region Methods

        public string[] Browse(string directory)
        {
            DirectoryInfo dir;
			if (directory.Length == 0)
				dir = new DirectoryInfo(Directory.GetCurrentDirectory());
			else
			{

				if (directory[directory.Length - 1] == '"')
					directory = directory.Substring(0, directory.Length - 1);

				dir = new DirectoryInfo(directory);				
			}
			IterateFiles(dir);
			string[] fileArray = (string[])_files.ToArray();
			_files.Clear();
			return fileArray;
        }

		protected void IterateFiles(DirectoryInfo dir)
		{
			foreach (FileSystemInfo fileSystemInfo in dir.GetFileSystemInfos())
			{
				if (fileSystemInfo is FileInfo)
				{
					FileInfo fileInfo = (FileInfo)fileSystemInfo;
					if (fileInfo.Extension.ToLower() == ".mp3")
					{
						_files.Add(fileInfo.FullName);
					}
				}
				else if (fileSystemInfo is DirectoryInfo)
				{
					IterateFiles((DirectoryInfo)fileSystemInfo);
				}
			}
		}
		#endregion
	}
}