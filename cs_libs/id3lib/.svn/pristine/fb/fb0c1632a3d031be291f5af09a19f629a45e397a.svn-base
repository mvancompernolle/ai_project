using System;
using System.Windows.Forms;
using System.Windows.Forms.Design;


namespace TagEditor
{
    /// <summary>
    /// Directory Browser.
    /// </summary>
    public class DirBrowser : FolderNameEditor
    {
        #region Fields
        private string _description = "Select Directory";
        private string _directory = string.Empty;
        #endregion

        #region Properties
        public string Description
        {
            set
            {
                _description = value;
            }
            get
            {
                return _description;
            }
        }

        public string DirectoryPath
        {
            get
            {
                return _directory;
            }
        }

        #endregion

        #region Methods
        public DialogResult ShowDialog()
        {
            FolderBrowser browser = new FolderBrowser();
            browser.Description = _description;
            browser.StartLocation = FolderNameEditor.FolderBrowserFolder.MyComputer;
            DialogResult result = browser.ShowDialog();
            if (result == DialogResult.OK)
                _directory = browser.DirectoryPath;
            else
                _directory = String.Empty;
            return result;
        }
        #endregion
    }
}
