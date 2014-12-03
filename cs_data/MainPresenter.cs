using System;
using System.Collections.Generic;
using System.Linq;                  // for IEnumerable<string>.ToArray()
using System.Text;
using System.IO;
using Mp3Lib;
using Utils;
using System.Windows.Forms;
using System.Diagnostics;
using System.Data;
using System.ComponentModel;        // for ListSortDirection

namespace TagEditor
{
    public class MainPresenter : IDisposable
    {
        protected MainForm _form;
        private string _rootdir;

        public string RootDir
        {
            get
            {
                return _rootdir;
            }
            set
            {
                _rootdir = value;
            }
        }

        #region IDisposable pattern
        /// <summary>
        /// Dispose of (clean up and deallocate) resources used by this class.
        /// </summary>
        /// <param name="fromUser">
        /// True if called directly or indirectly from user code.
        /// False if called from the finalizer (i.e. from the class' destructor).
        /// </param>
        /// <remarks>
        /// When called from user code, it is safe to clean up both managed and unmanaged objects.
        /// When called from the finalizer, it is only safe to dispose of unmanaged objects.
        /// This method should expect to be called multiple times without causing an exception.
        /// </remarks>
        internal void Dispose( bool fromUser )
        {
            if( fromUser )	// Called from user code rather than the garbage collector
            {
                GC.SuppressFinalize(this);	// No need for the Finalizer to do all this again.
            }
        }
        /// <summary>
        /// Dispose of all resources (both managed and unmanaged) used by this class.
        /// </summary>
        public void Dispose()
        {
            // Call our private Dispose method, indicating that the call originated from user code.
            // Diagnostics.TraceInfo("Disposed by user code.");
            this.Dispose(true);
        }
        /// <summary>
        /// Destructor, called by the finalizer during garbage collection.
        /// There is no guarantee that this method will be called. For example, if <see cref="Dispose"/> has already
        /// been called in user code for this object, then finalization may have been suppressed.
        /// </summary>
        ~MainPresenter()
        {
            // Call our private Dispose method, indicating that the call originated from the finalizer.
            // Diagnostics.TraceInfo("Finalizer is disposing MusicEntityDAL instance");
            this.Dispose(false);
        }

        #endregion

        public MainPresenter( MainForm form )
        {
            _form = form;
            _rootdir = Properties.Settings.Default.Directory;
        }

        public void Start()
        {
            if (!string.IsNullOrEmpty(_rootdir))
            {
                ScanDirectory(_rootdir);
            }
        }

        public void Scan()
        {
            DirBrowser dirBrowser = new DirBrowser();
            if (dirBrowser.ShowDialog() == DialogResult.OK)
            {
                ScanDirectory(dirBrowser.DirectoryPath);
            }
        }

        private void ScanDirectory(string scandir)
        {
            try
            {
                using( new CursorKeeper(Cursors.WaitCursor) )
                {
                    IEnumerable<string> browseEnum = FileIterator.GetFiles(scandir, "*.mp3");
                    string[] files = browseEnum.ToArray();

                    _form.SetDirectoryList(files);

                    // success; save the directory
                    _rootdir = scandir;
                    Properties.Settings.Default.Directory = _rootdir;
                    Properties.Settings.Default.Save();
                }
            }
            catch (Exception ex)
            {
                Utils.ExceptionMessageBox.Show(ex, "Directory Scan failed.");
            }
        }

        public void EditTag(string filename)
        {
            Mp3File mp3File = null;

            try
            {
                // create mp3 file wrapper; open it and read the tags
                mp3File = new Mp3File(filename);
            }
            catch (Exception e)
            {
                ExceptionMessageBox.Show(_form, e, "Error Reading Tag");
                return;
            }

            // create dialog and give it the ID3v2 block for editing
            // this is a bit sneaky; it uses the edit dialog straight out of TagScanner.exe as if it was a dll.
            TagEditor.ID3AdapterEdit id3Edit = new TagEditor.ID3AdapterEdit(mp3File);

            if (id3Edit.ShowDialog() == System.Windows.Forms.DialogResult.OK)
            {
                try
                {
                    using( new CursorKeeper(Cursors.WaitCursor) )
                    {
                        mp3File.Update();
                    }
                }
                catch (Exception e)
                {
                    ExceptionMessageBox.Show(_form, e, "Error Writing Tag");
                }
            }
        }

        public void EditExtendedTag( string filename )
        {
            Mp3File mp3File = null;

            try
            {
                // create mp3 file wrapper; open it and read the tags
                mp3File = new Mp3File(filename);
            }
            catch (Exception ex)
            {
                ExceptionMessageBox.Show(_form, ex, "Error Reading Tag");
                return;
            }

            if (mp3File.TagModel != null)
            {
                // create dialog and give it the ID3v2 block for editing
                // this is a bit sneaky; it uses the edit dialog straight out of TagScanner.exe as if it was a dll.
                TagEditor.ID3PowerEdit id3PowerEdit = new TagEditor.ID3PowerEdit();
                id3PowerEdit.TagModel = mp3File.TagModel;
                id3PowerEdit.ShowDialog();
            }
		}

        public void Compact( string filename )
        {
            try
            {
                // create mp3 file wrapper; open it and read the tags
                Mp3File mp3File = new Mp3File(filename);

                try
                {
                    using( new CursorKeeper(Cursors.WaitCursor) )
                    {
                        mp3File.UpdatePacked();
                    }
                }
                catch (Exception e)
                {
                    ExceptionMessageBox.Show(_form, e, "Error Writing Tag");
                }
            }
            catch (Exception e)
            {
                ExceptionMessageBox.Show(_form, e, "Error Reading Tag");
            }
        }

        public void Launch( string filename )
        {
            using( new CursorKeeper(Cursors.WaitCursor) )
            {
                Process winamp = new Process();
                winamp.StartInfo.FileName = filename;
                winamp.Start();
            }
        }

        internal void RemoveV2tag( string filename )
        {
            try
            {
                // create mp3 file wrapper; open it and read the tags
                Mp3File mp3File = new Mp3File(filename);

                try
                {
                    using( new CursorKeeper(Cursors.WaitCursor) )
                    {
                        mp3File.UpdateNoV2tag();
                    }
                }
                catch( Exception e )
                {
                    ExceptionMessageBox.Show(_form, e, "Error Writing Tag");
                }
            }
            catch( Exception e )
            {
                ExceptionMessageBox.Show(_form, e, "Error Reading Tag");
            }
        }
    }
}
