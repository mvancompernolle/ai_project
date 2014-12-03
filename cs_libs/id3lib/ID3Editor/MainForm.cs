// Copyright(C) 2002-2003 Hugo Rumayor Montemayor, All rights reserved.
using System;
using System.Diagnostics;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.Data;
using System.IO;
using Id3Lib;
using Mp3Lib;
using Utils;

namespace TagEditor
{
	/// <summary>
	/// Summary description for Form1.
	/// </summary>
	public partial class MainForm : System.Windows.Forms.Form
	{
        #region fields

        private TagEditor.MainPresenter _presenter;

        public TagEditor.MainPresenter Presenter
        {
            get
            {
                return _presenter;
            }
        }

        #endregion

        public MainForm()
		{
			InitializeComponent();

            _presenter = new TagEditor.MainPresenter(this);
        }

        private void MainForm_Shown( object sender, EventArgs e )
        {
           _presenter.Start();
        }
        
		private void _scanMenuItem_Click(object sender, System.EventArgs e)
		{
            _presenter.Scan();
		}

        public virtual void SetDirectoryList( string[] files )
        {
            _mainListBox.Items.Clear();
            _mainListBox.Items.AddRange(files);
        }

        /// <summary>
        /// when right mouse button is clicked, select the item under the mouse
        /// this makes it a lot easier to use the context menu on the list of files
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void _mainListBox_MouseDown(object sender, MouseEventArgs e)
        {
            int indexover = _mainListBox.IndexFromPoint(e.X, e.Y);
            if (indexover >= 0 && indexover < _mainListBox.Items.Count)
            {
                _mainListBox.SelectedIndex = indexover;
            }
            _mainListBox.Refresh();
        }

		private void _mainListBoxMenu_EditExtendedTag(object sender, System.EventArgs e)
		{
			if(_mainListBox.SelectedIndex != -1)
                _presenter.EditExtendedTag((string)_mainListBox.Items[_mainListBox.SelectedIndex]);
        }

        private void _mainListBoxMenu_EditTag(object sender, System.EventArgs e)
        {
            if (_mainListBox.SelectedIndex != -1)
                _presenter.EditTag((string)_mainListBox.Items[_mainListBox.SelectedIndex]);
        }

        private void _mainListBox_DoubleClick(object sender, System.EventArgs e)
		{
            if(_mainListBox.SelectedIndex >= 0)
                _presenter.EditTag((string)_mainListBox.Items[_mainListBox.SelectedIndex]);
		}

        private void _mainListBoxMenu_Compact(object sender, EventArgs e)
        {
            if (_mainListBox.SelectedIndex != -1)
                _presenter.Compact((string)_mainListBox.Items[_mainListBox.SelectedIndex]);
        }

        private void _mainListBoxMenu_Launch(object sender, EventArgs e)
        {
            if (_mainListBox.SelectedIndex != -1)
                _presenter.Launch((string)_mainListBox.Items[_mainListBox.SelectedIndex]);
        }

        private void _removeV2tag_Click( object sender, EventArgs e )
        {
            if( _mainListBox.SelectedIndex != -1 )
                _presenter.RemoveV2tag((string)_mainListBox.Items[_mainListBox.SelectedIndex]);
        }
    }
}
