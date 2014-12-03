// Copyright(C) 2002-2003 Hugo Rumayor Montemayor, All rights reserved.
using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.IO;
using Id3Lib;
using Mp3Lib;
using Utils;

namespace TagEditor
{
	/// <summary>
	/// Summary description for Form1.
	/// </summary>
	public class ID3AdapterEdit : System.Windows.Forms.Form
    {
        #region fields
        private System.Windows.Forms.TabPage _tabPageGeneric;
		private System.Windows.Forms.Label _labelGenre;
		private System.Windows.Forms.Label _labelYear;
		private System.Windows.Forms.TabControl _tabControlLyrics;
		private System.Windows.Forms.ComboBox _comboBoxGenre;
		private System.Windows.Forms.TextBox _textBoxYear;
		private System.Windows.Forms.Label _labelAlbum;
		private System.Windows.Forms.Label _labelArtist;
		private System.Windows.Forms.TextBox _textBoxAlbum;
		private System.Windows.Forms.TextBox _textBoxArtist;
		private System.Windows.Forms.TextBox _textBoxTitle;
		private System.Windows.Forms.Label _labelTitle;
		private System.Windows.Forms.TabPage _tabPageLyrics;
		private System.Windows.Forms.TextBox _textBoxLyrics;
		private System.Windows.Forms.TabPage _tabPageMore;
		private System.Windows.Forms.Label _labelComposer;
		private System.Windows.Forms.TextBox _textBoxComposer;
		private System.Windows.Forms.TextBox _textBoxTrackNo;
		private System.Windows.Forms.Label _labelTrackNo;
		private System.Windows.Forms.PictureBox _artPictureBox;
		private System.Windows.Forms.TabPage _tabPageComments;
        private System.Windows.Forms.TextBox _textBoxComments;
        private IContainer components;
		private System.Windows.Forms.Button _buttonOK;
		private System.Windows.Forms.Button _buttonCancel;
		private System.Windows.Forms.Button _removePicture;
		private System.Windows.Forms.Button _addPicture;
		private System.Windows.Forms.ErrorProvider _errorProvider;
		private System.Windows.Forms.OpenFileDialog _openFileDialog;
        private Label _labelDiscNo;
        private TextBox _textBoxDiscNo;
        private Label _labelLength;
        private TextBox _textBoxPadding;
        private TextBox _textBoxLength;
        private Label _labelPadding;
        private Label _labelDetails;
        private BindingSource _tagHandlerBindingSource;
        private BindingSource _mp3FileBindingSource;
        private TextBox _textBoxKBitRateMP3;
        private Label _labelKBitRate;

        #endregion

        /// <summary>
        /// MP3.File reference
        /// encapsulates both the TagModel and the audio information
        /// </summary>
        private Mp3File _mp3File = null;
        private Label label1;
        private Label label2;
        private TextBox _textBoxSampleRate;
        private Label label4;
        private Label label3;
        private TextBox _textBoxKBitRateCalc;
        private Label label6;
        private Label label5;
        private TextBox _textBoxKBitRateVbr;
        private Label label8;
        private Label label7;
        private TextBox _textBoxKBitRate;
        private Label label10;
        private Label label9;
        private CheckBox checkBoxVbr;
        private CheckBox hasInconsistenciesCheckBox;
        private Button _buttonScanWholeFile;
        private Label _labelSHA;
        private ToolTip toolTip1;

        /// <summary>
		/// Tag Handler reference
        /// encapsulates TagModel
		/// </summary>
        private TagHandler _tagHandler = null;


        public ID3AdapterEdit(Mp3File mp3File)
		{
            _mp3File = mp3File;
            _tagHandler = new TagHandler(_mp3File.TagModel);

			InitializeComponent();
		}

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose( bool disposing )
		{
			if( disposing )
			{
				if(components != null)
				{
					components.Dispose();
				}
			}
			base.Dispose( disposing );
		}

		#region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
            this.components = new System.ComponentModel.Container();
            this._tabControlLyrics = new System.Windows.Forms.TabControl();
            this._tabPageGeneric = new System.Windows.Forms.TabPage();
            this._labelSHA = new System.Windows.Forms.Label();
            this._buttonScanWholeFile = new System.Windows.Forms.Button();
            this.hasInconsistenciesCheckBox = new System.Windows.Forms.CheckBox();
            this.checkBoxVbr = new System.Windows.Forms.CheckBox();
            this._mp3FileBindingSource = new System.Windows.Forms.BindingSource(this.components);
            this._addPicture = new System.Windows.Forms.Button();
            this._removePicture = new System.Windows.Forms.Button();
            this._artPictureBox = new System.Windows.Forms.PictureBox();
            this._tagHandlerBindingSource = new System.Windows.Forms.BindingSource(this.components);
            this._comboBoxGenre = new System.Windows.Forms.ComboBox();
            this._labelGenre = new System.Windows.Forms.Label();
            this._textBoxYear = new System.Windows.Forms.TextBox();
            this._labelYear = new System.Windows.Forms.Label();
            this._labelAlbum = new System.Windows.Forms.Label();
            this._labelArtist = new System.Windows.Forms.Label();
            this._textBoxAlbum = new System.Windows.Forms.TextBox();
            this._textBoxArtist = new System.Windows.Forms.TextBox();
            this._textBoxSampleRate = new System.Windows.Forms.TextBox();
            this._textBoxKBitRate = new System.Windows.Forms.TextBox();
            this._textBoxKBitRateVbr = new System.Windows.Forms.TextBox();
            this._textBoxKBitRateCalc = new System.Windows.Forms.TextBox();
            this._textBoxKBitRateMP3 = new System.Windows.Forms.TextBox();
            this._textBoxLength = new System.Windows.Forms.TextBox();
            this._textBoxPadding = new System.Windows.Forms.TextBox();
            this.label10 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.label8 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label9 = new System.Windows.Forms.Label();
            this.label6 = new System.Windows.Forms.Label();
            this.label7 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this._labelKBitRate = new System.Windows.Forms.Label();
            this._labelPadding = new System.Windows.Forms.Label();
            this._labelDetails = new System.Windows.Forms.Label();
            this._textBoxDiscNo = new System.Windows.Forms.TextBox();
            this._labelLength = new System.Windows.Forms.Label();
            this._textBoxTrackNo = new System.Windows.Forms.TextBox();
            this._labelDiscNo = new System.Windows.Forms.Label();
            this._labelTrackNo = new System.Windows.Forms.Label();
            this._textBoxTitle = new System.Windows.Forms.TextBox();
            this._labelTitle = new System.Windows.Forms.Label();
            this._tabPageLyrics = new System.Windows.Forms.TabPage();
            this._textBoxLyrics = new System.Windows.Forms.TextBox();
            this._tabPageComments = new System.Windows.Forms.TabPage();
            this._textBoxComments = new System.Windows.Forms.TextBox();
            this._tabPageMore = new System.Windows.Forms.TabPage();
            this._labelComposer = new System.Windows.Forms.Label();
            this._textBoxComposer = new System.Windows.Forms.TextBox();
            this._errorProvider = new System.Windows.Forms.ErrorProvider(this.components);
            this._buttonOK = new System.Windows.Forms.Button();
            this._buttonCancel = new System.Windows.Forms.Button();
            this._openFileDialog = new System.Windows.Forms.OpenFileDialog();
            this.toolTip1 = new System.Windows.Forms.ToolTip(this.components);
            this._tabControlLyrics.SuspendLayout();
            this._tabPageGeneric.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this._mp3FileBindingSource)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this._artPictureBox)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this._tagHandlerBindingSource)).BeginInit();
            this._tabPageLyrics.SuspendLayout();
            this._tabPageComments.SuspendLayout();
            this._tabPageMore.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this._errorProvider)).BeginInit();
            this.SuspendLayout();
            // 
            // _tabControlLyrics
            // 
            this._tabControlLyrics.Controls.Add(this._tabPageGeneric);
            this._tabControlLyrics.Controls.Add(this._tabPageLyrics);
            this._tabControlLyrics.Controls.Add(this._tabPageComments);
            this._tabControlLyrics.Controls.Add(this._tabPageMore);
            this._tabControlLyrics.Location = new System.Drawing.Point(8, 8);
            this._tabControlLyrics.Name = "_tabControlLyrics";
            this._tabControlLyrics.SelectedIndex = 0;
            this._tabControlLyrics.Size = new System.Drawing.Size(552, 391);
            this._tabControlLyrics.TabIndex = 0;
            // 
            // _tabPageGeneric
            // 
            this._tabPageGeneric.AutoScroll = true;
            this._tabPageGeneric.Controls.Add(this._labelSHA);
            this._tabPageGeneric.Controls.Add(this._buttonScanWholeFile);
            this._tabPageGeneric.Controls.Add(this.hasInconsistenciesCheckBox);
            this._tabPageGeneric.Controls.Add(this.checkBoxVbr);
            this._tabPageGeneric.Controls.Add(this._addPicture);
            this._tabPageGeneric.Controls.Add(this._removePicture);
            this._tabPageGeneric.Controls.Add(this._artPictureBox);
            this._tabPageGeneric.Controls.Add(this._comboBoxGenre);
            this._tabPageGeneric.Controls.Add(this._labelGenre);
            this._tabPageGeneric.Controls.Add(this._textBoxYear);
            this._tabPageGeneric.Controls.Add(this._labelYear);
            this._tabPageGeneric.Controls.Add(this._labelAlbum);
            this._tabPageGeneric.Controls.Add(this._labelArtist);
            this._tabPageGeneric.Controls.Add(this._textBoxAlbum);
            this._tabPageGeneric.Controls.Add(this._textBoxArtist);
            this._tabPageGeneric.Controls.Add(this._textBoxSampleRate);
            this._tabPageGeneric.Controls.Add(this._textBoxKBitRate);
            this._tabPageGeneric.Controls.Add(this._textBoxKBitRateVbr);
            this._tabPageGeneric.Controls.Add(this._textBoxKBitRateCalc);
            this._tabPageGeneric.Controls.Add(this._textBoxKBitRateMP3);
            this._tabPageGeneric.Controls.Add(this._textBoxLength);
            this._tabPageGeneric.Controls.Add(this._textBoxPadding);
            this._tabPageGeneric.Controls.Add(this.label10);
            this._tabPageGeneric.Controls.Add(this.label4);
            this._tabPageGeneric.Controls.Add(this.label8);
            this._tabPageGeneric.Controls.Add(this.label2);
            this._tabPageGeneric.Controls.Add(this.label9);
            this._tabPageGeneric.Controls.Add(this.label6);
            this._tabPageGeneric.Controls.Add(this.label7);
            this._tabPageGeneric.Controls.Add(this.label3);
            this._tabPageGeneric.Controls.Add(this.label5);
            this._tabPageGeneric.Controls.Add(this.label1);
            this._tabPageGeneric.Controls.Add(this._labelKBitRate);
            this._tabPageGeneric.Controls.Add(this._labelPadding);
            this._tabPageGeneric.Controls.Add(this._labelDetails);
            this._tabPageGeneric.Controls.Add(this._textBoxDiscNo);
            this._tabPageGeneric.Controls.Add(this._labelLength);
            this._tabPageGeneric.Controls.Add(this._textBoxTrackNo);
            this._tabPageGeneric.Controls.Add(this._labelDiscNo);
            this._tabPageGeneric.Controls.Add(this._labelTrackNo);
            this._tabPageGeneric.Controls.Add(this._textBoxTitle);
            this._tabPageGeneric.Controls.Add(this._labelTitle);
            this._tabPageGeneric.Location = new System.Drawing.Point(4, 22);
            this._tabPageGeneric.Name = "_tabPageGeneric";
            this._tabPageGeneric.Size = new System.Drawing.Size(544, 365);
            this._tabPageGeneric.TabIndex = 0;
            this._tabPageGeneric.Text = "Generic";
            // 
            // _labelSHA
            // 
            this._labelSHA.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this._labelSHA.Location = new System.Drawing.Point(10, 331);
            this._labelSHA.Margin = new System.Windows.Forms.Padding(2, 0, 2, 0);
            this._labelSHA.Name = "_labelSHA";
            this._labelSHA.Size = new System.Drawing.Size(164, 30);
            this._labelSHA.TabIndex = 43;
            this._labelSHA.Text = "SHA-1";
            this.toolTip1.SetToolTip(this._labelSHA, "Click to calculate SHA-1 160bit cryptographic hash of the audio");
            this._labelSHA.Click += new System.EventHandler(this.SHA_Click);
            // 
            // _buttonScanWholeFile
            // 
            this._buttonScanWholeFile.Location = new System.Drawing.Point(10, 305);
            this._buttonScanWholeFile.Margin = new System.Windows.Forms.Padding(2);
            this._buttonScanWholeFile.Name = "_buttonScanWholeFile";
            this._buttonScanWholeFile.Size = new System.Drawing.Size(67, 23);
            this._buttonScanWholeFile.TabIndex = 41;
            this._buttonScanWholeFile.Text = "Scan File";
            this.toolTip1.SetToolTip(this._buttonScanWholeFile, "parse and count audio frames and bytes; result is added to details text box");
            this._buttonScanWholeFile.UseVisualStyleBackColor = true;
            this._buttonScanWholeFile.Click += new System.EventHandler(this.OnButtonScanWholeFile);
            // 
            // hasInconsistenciesCheckBox
            // 
            this.hasInconsistenciesCheckBox.AutoCheck = false;
            this.hasInconsistenciesCheckBox.Location = new System.Drawing.Point(10, 282);
            this.hasInconsistenciesCheckBox.Margin = new System.Windows.Forms.Padding(2);
            this.hasInconsistenciesCheckBox.Name = "hasInconsistenciesCheckBox";
            this.hasInconsistenciesCheckBox.Size = new System.Drawing.Size(136, 20);
            this.hasInconsistenciesCheckBox.TabIndex = 40;
            this.hasInconsistenciesCheckBox.Text = "Has Inconsistencies";
            this.toolTip1.SetToolTip(this.hasInconsistenciesCheckBox, "VBR header size doesn\'t agree with  measured audio file size");
            this.hasInconsistenciesCheckBox.UseVisualStyleBackColor = true;
            // 
            // checkBoxVbr
            // 
            this.checkBoxVbr.AutoCheck = false;
            this.checkBoxVbr.AutoSize = true;
            this.checkBoxVbr.DataBindings.Add(new System.Windows.Forms.Binding("Checked", this._mp3FileBindingSource, "Audio.IsVbr", true));
            this.checkBoxVbr.Location = new System.Drawing.Point(11, 260);
            this.checkBoxVbr.Name = "checkBoxVbr";
            this.checkBoxVbr.Size = new System.Drawing.Size(136, 17);
            this.checkBoxVbr.TabIndex = 15;
            this.checkBoxVbr.Text = "Variable Bit Rate (VBR)";
            this.toolTip1.SetToolTip(this.checkBoxVbr, "probably VBR encoded MP3 audio");
            this.checkBoxVbr.UseVisualStyleBackColor = true;
            // 
            // _mp3FileBindingSource
            // 
            this._mp3FileBindingSource.DataSource = typeof(Mp3Lib.Mp3File);
            // 
            // _addPicture
            // 
            this._addPicture.Location = new System.Drawing.Point(410, 308);
            this._addPicture.Name = "_addPicture";
            this._addPicture.Size = new System.Drawing.Size(91, 23);
            this._addPicture.TabIndex = 14;
            this._addPicture.Text = "Add Picture";
            this._addPicture.Click += new System.EventHandler(this.addPicture_Click);
            // 
            // _removePicture
            // 
            this._removePicture.Font = new System.Drawing.Font("Microsoft Sans Serif", 7.8F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this._removePicture.Location = new System.Drawing.Point(507, 308);
            this._removePicture.Name = "_removePicture";
            this._removePicture.Size = new System.Drawing.Size(22, 23);
            this._removePicture.TabIndex = 13;
            this._removePicture.Text = "X";
            this._removePicture.Click += new System.EventHandler(this.removePicture_Click);
            // 
            // _artPictureBox
            // 
            this._artPictureBox.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this._artPictureBox.DataBindings.Add(new System.Windows.Forms.Binding("Image", this._tagHandlerBindingSource, "Picture", true));
            this._artPictureBox.Location = new System.Drawing.Point(400, 150);
            this._artPictureBox.Name = "_artPictureBox";
            this._artPictureBox.Size = new System.Drawing.Size(140, 152);
            this._artPictureBox.SizeMode = System.Windows.Forms.PictureBoxSizeMode.StretchImage;
            this._artPictureBox.TabIndex = 12;
            this._artPictureBox.TabStop = false;
            // 
            // _tagHandlerBindingSource
            // 
            this._tagHandlerBindingSource.DataSource = typeof(Id3Lib.TagHandler);
            // 
            // _comboBoxGenre
            // 
            this._comboBoxGenre.DataBindings.Add(new System.Windows.Forms.Binding("Text", this._tagHandlerBindingSource, "Genre", true));
            this._comboBoxGenre.Location = new System.Drawing.Point(71, 72);
            this._comboBoxGenre.Name = "_comboBoxGenre";
            this._comboBoxGenre.Size = new System.Drawing.Size(184, 21);
            this._comboBoxGenre.TabIndex = 11;
            // 
            // _labelGenre
            // 
            this._labelGenre.Location = new System.Drawing.Point(8, 74);
            this._labelGenre.Name = "_labelGenre";
            this._labelGenre.Size = new System.Drawing.Size(56, 16);
            this._labelGenre.TabIndex = 10;
            this._labelGenre.Text = "Genre:";
            this._labelGenre.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // _textBoxYear
            // 
            this._textBoxYear.DataBindings.Add(new System.Windows.Forms.Binding("Text", this._tagHandlerBindingSource, "Year", true));
            this._textBoxYear.Location = new System.Drawing.Point(301, 72);
            this._textBoxYear.Name = "_textBoxYear";
            this._textBoxYear.Size = new System.Drawing.Size(44, 20);
            this._textBoxYear.TabIndex = 9;
            // 
            // _labelYear
            // 
            this._labelYear.Location = new System.Drawing.Point(268, 74);
            this._labelYear.Name = "_labelYear";
            this._labelYear.Size = new System.Drawing.Size(32, 16);
            this._labelYear.TabIndex = 8;
            this._labelYear.Text = "Year:";
            this._labelYear.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // _labelAlbum
            // 
            this._labelAlbum.Location = new System.Drawing.Point(8, 50);
            this._labelAlbum.Name = "_labelAlbum";
            this._labelAlbum.Size = new System.Drawing.Size(56, 16);
            this._labelAlbum.TabIndex = 7;
            this._labelAlbum.Text = "Album:";
            this._labelAlbum.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // _labelArtist
            // 
            this._labelArtist.Location = new System.Drawing.Point(8, 28);
            this._labelArtist.Name = "_labelArtist";
            this._labelArtist.Size = new System.Drawing.Size(56, 16);
            this._labelArtist.TabIndex = 6;
            this._labelArtist.Text = "Artist:";
            this._labelArtist.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // _textBoxAlbum
            // 
            this._textBoxAlbum.DataBindings.Add(new System.Windows.Forms.Binding("Text", this._tagHandlerBindingSource, "Album", true));
            this._textBoxAlbum.Location = new System.Drawing.Point(71, 50);
            this._textBoxAlbum.Name = "_textBoxAlbum";
            this._textBoxAlbum.Size = new System.Drawing.Size(448, 20);
            this._textBoxAlbum.TabIndex = 5;
            // 
            // _textBoxArtist
            // 
            this._textBoxArtist.DataBindings.Add(new System.Windows.Forms.Binding("Text", this._tagHandlerBindingSource, "Artist", true));
            this._textBoxArtist.Location = new System.Drawing.Point(71, 28);
            this._textBoxArtist.Name = "_textBoxArtist";
            this._textBoxArtist.Size = new System.Drawing.Size(448, 20);
            this._textBoxArtist.TabIndex = 4;
            // 
            // _textBoxSampleRate
            // 
            this._textBoxSampleRate.Location = new System.Drawing.Point(78, 236);
            this._textBoxSampleRate.Name = "_textBoxSampleRate";
            this._textBoxSampleRate.ReadOnly = true;
            this._textBoxSampleRate.Size = new System.Drawing.Size(60, 20);
            this._textBoxSampleRate.TabIndex = 3;
            this._textBoxSampleRate.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            // 
            // _textBoxKBitRate
            // 
            this._textBoxKBitRate.Location = new System.Drawing.Point(78, 215);
            this._textBoxKBitRate.Name = "_textBoxKBitRate";
            this._textBoxKBitRate.ReadOnly = true;
            this._textBoxKBitRate.Size = new System.Drawing.Size(60, 20);
            this._textBoxKBitRate.TabIndex = 3;
            this._textBoxKBitRate.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            // 
            // _textBoxKBitRateVbr
            // 
            this._textBoxKBitRateVbr.Location = new System.Drawing.Point(78, 194);
            this._textBoxKBitRateVbr.Name = "_textBoxKBitRateVbr";
            this._textBoxKBitRateVbr.ReadOnly = true;
            this._textBoxKBitRateVbr.Size = new System.Drawing.Size(60, 20);
            this._textBoxKBitRateVbr.TabIndex = 3;
            this._textBoxKBitRateVbr.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            // 
            // _textBoxKBitRateCalc
            // 
            this._textBoxKBitRateCalc.Location = new System.Drawing.Point(78, 173);
            this._textBoxKBitRateCalc.Name = "_textBoxKBitRateCalc";
            this._textBoxKBitRateCalc.ReadOnly = true;
            this._textBoxKBitRateCalc.Size = new System.Drawing.Size(60, 20);
            this._textBoxKBitRateCalc.TabIndex = 3;
            this._textBoxKBitRateCalc.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            // 
            // _textBoxKBitRateMP3
            // 
            this._textBoxKBitRateMP3.Location = new System.Drawing.Point(78, 152);
            this._textBoxKBitRateMP3.Name = "_textBoxKBitRateMP3";
            this._textBoxKBitRateMP3.ReadOnly = true;
            this._textBoxKBitRateMP3.Size = new System.Drawing.Size(60, 20);
            this._textBoxKBitRateMP3.TabIndex = 3;
            this._textBoxKBitRateMP3.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            // 
            // _textBoxLength
            // 
            this._textBoxLength.DataBindings.Add(new System.Windows.Forms.Binding("Text", this._tagHandlerBindingSource, "Length", true, System.Windows.Forms.DataSourceUpdateMode.Never, null, "hh:mm:ss.fff"));
            this._textBoxLength.Location = new System.Drawing.Point(78, 131);
            this._textBoxLength.Name = "_textBoxLength";
            this._textBoxLength.ReadOnly = true;
            this._textBoxLength.Size = new System.Drawing.Size(60, 20);
            this._textBoxLength.TabIndex = 3;
            // 
            // _textBoxPadding
            // 
            this._textBoxPadding.DataBindings.Add(new System.Windows.Forms.Binding("Text", this._tagHandlerBindingSource, "PaddingSize", true, System.Windows.Forms.DataSourceUpdateMode.Never));
            this._textBoxPadding.Location = new System.Drawing.Point(78, 109);
            this._textBoxPadding.Name = "_textBoxPadding";
            this._textBoxPadding.ReadOnly = true;
            this._textBoxPadding.Size = new System.Drawing.Size(60, 20);
            this._textBoxPadding.TabIndex = 3;
            this._textBoxPadding.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            // 
            // label10
            // 
            this.label10.Location = new System.Drawing.Point(143, 216);
            this.label10.Name = "label10";
            this.label10.Size = new System.Drawing.Size(34, 17);
            this.label10.TabIndex = 2;
            this.label10.Text = "kbit/s";
            this.label10.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // label4
            // 
            this.label4.Location = new System.Drawing.Point(143, 237);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(34, 17);
            this.label4.TabIndex = 2;
            this.label4.Text = "kHz";
            this.label4.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // label8
            // 
            this.label8.Location = new System.Drawing.Point(143, 195);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(34, 17);
            this.label8.TabIndex = 2;
            this.label8.Text = "kbit/s";
            this.label8.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // label2
            // 
            this.label2.Location = new System.Drawing.Point(143, 110);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(34, 17);
            this.label2.TabIndex = 2;
            this.label2.Text = "bytes";
            this.label2.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // label9
            // 
            this.label9.Location = new System.Drawing.Point(2, 216);
            this.label9.Name = "label9";
            this.label9.Size = new System.Drawing.Size(75, 17);
            this.label9.TabIndex = 2;
            this.label9.Text = "BitRate:";
            this.label9.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // label6
            // 
            this.label6.Location = new System.Drawing.Point(143, 174);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(34, 17);
            this.label6.TabIndex = 2;
            this.label6.Text = "kbit/s";
            this.label6.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // label7
            // 
            this.label7.Location = new System.Drawing.Point(2, 195);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(75, 17);
            this.label7.TabIndex = 2;
            this.label7.Text = "VbrBitRate:";
            this.label7.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // label3
            // 
            this.label3.Location = new System.Drawing.Point(2, 237);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(75, 17);
            this.label3.TabIndex = 2;
            this.label3.Text = "Samples:";
            this.label3.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // label5
            // 
            this.label5.Location = new System.Drawing.Point(2, 174);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(75, 17);
            this.label5.TabIndex = 2;
            this.label5.Text = "CalcBitRate:";
            this.label5.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // label1
            // 
            this.label1.Location = new System.Drawing.Point(143, 154);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(34, 17);
            this.label1.TabIndex = 2;
            this.label1.Text = "kbit/s";
            this.label1.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // _labelKBitRate
            // 
            this._labelKBitRate.Location = new System.Drawing.Point(2, 154);
            this._labelKBitRate.Name = "_labelKBitRate";
            this._labelKBitRate.Size = new System.Drawing.Size(75, 17);
            this._labelKBitRate.TabIndex = 2;
            this._labelKBitRate.Text = "MP3BitRate:";
            this._labelKBitRate.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // _labelPadding
            // 
            this._labelPadding.Location = new System.Drawing.Point(2, 110);
            this._labelPadding.Name = "_labelPadding";
            this._labelPadding.Size = new System.Drawing.Size(75, 17);
            this._labelPadding.TabIndex = 2;
            this._labelPadding.Text = "Padding:";
            this._labelPadding.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // _labelDetails
            // 
            this._labelDetails.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this._labelDetails.Location = new System.Drawing.Point(192, 96);
            this._labelDetails.Name = "_labelDetails";
            this._labelDetails.Size = new System.Drawing.Size(200, 265);
            this._labelDetails.TabIndex = 2;
            this._labelDetails.Text = "Details:";
            // 
            // _textBoxDiscNo
            // 
            this._textBoxDiscNo.DataBindings.Add(new System.Windows.Forms.Binding("Text", this._tagHandlerBindingSource, "Disc", true));
            this._textBoxDiscNo.Location = new System.Drawing.Point(393, 72);
            this._textBoxDiscNo.Name = "_textBoxDiscNo";
            this._textBoxDiscNo.Size = new System.Drawing.Size(40, 20);
            this._textBoxDiscNo.TabIndex = 3;
            // 
            // _labelLength
            // 
            this._labelLength.Location = new System.Drawing.Point(2, 131);
            this._labelLength.Name = "_labelLength";
            this._labelLength.Size = new System.Drawing.Size(75, 17);
            this._labelLength.TabIndex = 2;
            this._labelLength.Text = "Length:";
            this._labelLength.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // _textBoxTrackNo
            // 
            this._textBoxTrackNo.DataBindings.Add(new System.Windows.Forms.Binding("Text", this._tagHandlerBindingSource, "Track", true));
            this._textBoxTrackNo.Location = new System.Drawing.Point(479, 72);
            this._textBoxTrackNo.Name = "_textBoxTrackNo";
            this._textBoxTrackNo.Size = new System.Drawing.Size(40, 20);
            this._textBoxTrackNo.TabIndex = 3;
            // 
            // _labelDiscNo
            // 
            this._labelDiscNo.Location = new System.Drawing.Point(346, 74);
            this._labelDiscNo.Name = "_labelDiscNo";
            this._labelDiscNo.Size = new System.Drawing.Size(46, 16);
            this._labelDiscNo.TabIndex = 2;
            this._labelDiscNo.Text = "Disc #:";
            this._labelDiscNo.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // _labelTrackNo
            // 
            this._labelTrackNo.Location = new System.Drawing.Point(413, 74);
            this._labelTrackNo.Name = "_labelTrackNo";
            this._labelTrackNo.Size = new System.Drawing.Size(68, 16);
            this._labelTrackNo.TabIndex = 2;
            this._labelTrackNo.Text = "Track #:";
            this._labelTrackNo.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // _textBoxTitle
            // 
            this._textBoxTitle.DataBindings.Add(new System.Windows.Forms.Binding("Text", this._tagHandlerBindingSource, "Title", true));
            this._textBoxTitle.Location = new System.Drawing.Point(71, 6);
            this._textBoxTitle.Name = "_textBoxTitle";
            this._textBoxTitle.Size = new System.Drawing.Size(448, 20);
            this._textBoxTitle.TabIndex = 1;
            // 
            // _labelTitle
            // 
            this._labelTitle.Location = new System.Drawing.Point(8, 6);
            this._labelTitle.Name = "_labelTitle";
            this._labelTitle.Size = new System.Drawing.Size(56, 16);
            this._labelTitle.TabIndex = 0;
            this._labelTitle.Text = "Title:";
            this._labelTitle.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // _tabPageLyrics
            // 
            this._tabPageLyrics.Controls.Add(this._textBoxLyrics);
            this._tabPageLyrics.Location = new System.Drawing.Point(4, 22);
            this._tabPageLyrics.Name = "_tabPageLyrics";
            this._tabPageLyrics.Size = new System.Drawing.Size(544, 365);
            this._tabPageLyrics.TabIndex = 1;
            this._tabPageLyrics.Text = "Lyrics";
            // 
            // _textBoxLyrics
            // 
            this._textBoxLyrics.AcceptsReturn = true;
            this._textBoxLyrics.AcceptsTab = true;
            this._textBoxLyrics.DataBindings.Add(new System.Windows.Forms.Binding("Text", this._tagHandlerBindingSource, "Lyrics", true));
            this._textBoxLyrics.Location = new System.Drawing.Point(16, 16);
            this._textBoxLyrics.Multiline = true;
            this._textBoxLyrics.Name = "_textBoxLyrics";
            this._textBoxLyrics.ScrollBars = System.Windows.Forms.ScrollBars.Both;
            this._textBoxLyrics.Size = new System.Drawing.Size(512, 288);
            this._textBoxLyrics.TabIndex = 0;
            // 
            // _tabPageComments
            // 
            this._tabPageComments.Controls.Add(this._textBoxComments);
            this._tabPageComments.Location = new System.Drawing.Point(4, 22);
            this._tabPageComments.Name = "_tabPageComments";
            this._tabPageComments.Size = new System.Drawing.Size(544, 365);
            this._tabPageComments.TabIndex = 3;
            this._tabPageComments.Text = "Comments";
            // 
            // _textBoxComments
            // 
            this._textBoxComments.AcceptsReturn = true;
            this._textBoxComments.AcceptsTab = true;
            this._textBoxComments.DataBindings.Add(new System.Windows.Forms.Binding("Text", this._tagHandlerBindingSource, "Comment", true));
            this._textBoxComments.Location = new System.Drawing.Point(16, 15);
            this._textBoxComments.Multiline = true;
            this._textBoxComments.Name = "_textBoxComments";
            this._textBoxComments.ScrollBars = System.Windows.Forms.ScrollBars.Both;
            this._textBoxComments.Size = new System.Drawing.Size(512, 288);
            this._textBoxComments.TabIndex = 1;
            // 
            // _tabPageMore
            // 
            this._tabPageMore.Controls.Add(this._labelComposer);
            this._tabPageMore.Controls.Add(this._textBoxComposer);
            this._tabPageMore.Location = new System.Drawing.Point(4, 22);
            this._tabPageMore.Name = "_tabPageMore";
            this._tabPageMore.Size = new System.Drawing.Size(544, 365);
            this._tabPageMore.TabIndex = 2;
            this._tabPageMore.Text = "More";
            // 
            // _labelComposer
            // 
            this._labelComposer.Location = new System.Drawing.Point(8, 8);
            this._labelComposer.Name = "_labelComposer";
            this._labelComposer.Size = new System.Drawing.Size(66, 16);
            this._labelComposer.TabIndex = 15;
            this._labelComposer.Text = "Composer:";
            this._labelComposer.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // _textBoxComposer
            // 
            this._textBoxComposer.DataBindings.Add(new System.Windows.Forms.Binding("Text", this._tagHandlerBindingSource, "Composer", true));
            this._textBoxComposer.Location = new System.Drawing.Point(80, 8);
            this._textBoxComposer.Name = "_textBoxComposer";
            this._textBoxComposer.Size = new System.Drawing.Size(448, 20);
            this._textBoxComposer.TabIndex = 14;
            // 
            // _errorProvider
            // 
            this._errorProvider.ContainerControl = this;
            this._errorProvider.DataMember = "";
            // 
            // _buttonOK
            // 
            this._buttonOK.Location = new System.Drawing.Point(208, 405);
            this._buttonOK.Name = "_buttonOK";
            this._buttonOK.Size = new System.Drawing.Size(72, 24);
            this._buttonOK.TabIndex = 1;
            this._buttonOK.Text = "OK";
            this._buttonOK.Click += new System.EventHandler(this.OnOkClick);
            // 
            // _buttonCancel
            // 
            this._buttonCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this._buttonCancel.Location = new System.Drawing.Point(288, 405);
            this._buttonCancel.Name = "_buttonCancel";
            this._buttonCancel.Size = new System.Drawing.Size(72, 24);
            this._buttonCancel.TabIndex = 2;
            this._buttonCancel.Text = "&Cancel";
            this._buttonCancel.Click += new System.EventHandler(this.buttonCancel_Click);
            // 
            // ID3AdapterEdit
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this._buttonCancel;
            this.ClientSize = new System.Drawing.Size(567, 439);
            this.Controls.Add(this._buttonCancel);
            this.Controls.Add(this._buttonOK);
            this.Controls.Add(this._tabControlLyrics);
            this.DataBindings.Add(new System.Windows.Forms.Binding("Text", this._mp3FileBindingSource, "FileName", true, System.Windows.Forms.DataSourceUpdateMode.Never));
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.Name = "ID3AdapterEdit";
            this.ShowInTaskbar = false;
            this.Text = "ID3 Tag Adapter Editor";
            this.Load += new System.EventHandler(this.ID3Edit_Load);
            this._tabControlLyrics.ResumeLayout(false);
            this._tabPageGeneric.ResumeLayout(false);
            this._tabPageGeneric.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this._mp3FileBindingSource)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this._artPictureBox)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this._tagHandlerBindingSource)).EndInit();
            this._tabPageLyrics.ResumeLayout(false);
            this._tabPageLyrics.PerformLayout();
            this._tabPageComments.ResumeLayout(false);
            this._tabPageComments.PerformLayout();
            this._tabPageMore.ResumeLayout(false);
            this._tabPageMore.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this._errorProvider)).EndInit();
            this.ResumeLayout(false);

		}
		#endregion

		private void ID3Edit_Load(object sender, System.EventArgs e)
		{
			// If there is no model
			if(_mp3File == null)
				throw new ApplicationException("No data to edit on load");

            // set up databindings
            this._tagHandlerBindingSource.DataSource = _tagHandler;
            this._mp3FileBindingSource.DataSource = _mp3File;

            //this._textBoxSampleRate.DataBindings.Add(new System.Windows.Forms.Binding("Text", this._mp3FileBindingSource, "Audio.AudioHeader.SamplesPerSecond", true));
            Binding bindSampleRate = new Binding("Text", this._mp3FileBindingSource, "Audio.Header.SamplesPerSecond", true, System.Windows.Forms.DataSourceUpdateMode.Never);
            // Add the delegates to the events.
            bindSampleRate.Format += new ConvertEventHandler(bit_to_kbit);
            this._textBoxSampleRate.DataBindings.Add(bindSampleRate);

            //this._textBoxKBitRateCalc.DataBindings.Add(new System.Windows.Forms.Binding("Text", this._mp3FileBindingSource, "KBitRate", true, System.Windows.Forms.DataSourceUpdateMode.Never, null, "N0"));
            Binding bindKBitRateCalc = new Binding("Text", this._mp3FileBindingSource, "Audio.BitRateCalc", true, System.Windows.Forms.DataSourceUpdateMode.Never);
            // Add the delegates to the events.
            bindKBitRateCalc.Format += new ConvertEventHandler(bit_to_kbit);
            this._textBoxKBitRateCalc.DataBindings.Add(bindKBitRateCalc);

            //this._textBoxKBitRateMP3.DataBindings.Add(new System.Windows.Forms.Binding("Text", this._mp3FileBindingSource, "KBitRate", true, System.Windows.Forms.DataSourceUpdateMode.Never, null, "N0"));
            Binding bindKBitRateMP3 = new Binding("Text", this._mp3FileBindingSource, "Audio.BitRateMp3", true, System.Windows.Forms.DataSourceUpdateMode.Never);
            // Add the delegates to the events.
            bindKBitRateMP3.Format += new ConvertEventHandler(bit_to_kbit);
            this._textBoxKBitRateMP3.DataBindings.Add(bindKBitRateMP3);

            //this._textBoxKBitRateVbr.DataBindings.Add(new System.Windows.Forms.Binding("Text", this._mp3FileBindingSource, "KBitRate", true, System.Windows.Forms.DataSourceUpdateMode.Never, null, "N0"));
            Binding bindKBitRateVbr = new Binding("Text", this._mp3FileBindingSource, "Audio.BitRateVbr", true, System.Windows.Forms.DataSourceUpdateMode.Never);
            // Add the delegates to the events.
            bindKBitRateVbr.Format += new ConvertEventHandler(bit_to_kbit);
            this._textBoxKBitRateVbr.DataBindings.Add(bindKBitRateVbr);

            //this._textBoxKBitRate.DataBindings.Add(new System.Windows.Forms.Binding("Text", this._mp3FileBindingSource, "KBitRate", true, System.Windows.Forms.DataSourceUpdateMode.Never, null, "N0"));
            Binding bindKBitRate = new Binding("Text", this._mp3FileBindingSource, "Audio.BitRate", true, System.Windows.Forms.DataSourceUpdateMode.Never);
            // Add the delegates to the events.
            bindKBitRate.Format += new ConvertEventHandler(bit_to_kbit);
            this._textBoxKBitRate.DataBindings.Add(bindKBitRate);

            //this._labelDetails.DataBindings.Add(new System.Windows.Forms.Binding("Text", this._mp3FileBindingSource, "Audio", true, System.Windows.Forms.DataSourceUpdateMode.Never));
            Binding bindDetails = new Binding("Text", this._mp3FileBindingSource, "Audio.DebugString", true, System.Windows.Forms.DataSourceUpdateMode.Never);
            this._labelDetails.DataBindings.Add(bindDetails);

            this.hasInconsistenciesCheckBox.DataBindings.Add(new System.Windows.Forms.Binding("CheckState", this._mp3FileBindingSource, "Audio.HasInconsistencies", true));
        }

		private void OnOkClick(object sender, System.EventArgs e)
		{
            // picturebox is not a control that can receive focus,
            // so it doesn't save its image with the databinding mechanism.
            _tagHandler.Picture  = this._artPictureBox.Image;

			DialogResult = DialogResult.OK;
			this.Close();
		}

		private void buttonCancel_Click(object sender, System.EventArgs e)
		{
			DialogResult = DialogResult.Cancel;
			this.Close();
		}

		private void addPicture_Click(object sender, System.EventArgs e)
		{
			_openFileDialog.Multiselect= false;
			_openFileDialog.CheckFileExists = true;
			_openFileDialog.CheckPathExists = true;
			_openFileDialog.Title = "Select a picture";
            _openFileDialog.Filter = "Picture Files(*.bmp;*.jpg;*.gif;*.png)|*.bpm;*.jpg;*.gif;*.png|Bitmap (*.bmp)|*.bmp|jpg (*.jpg)|*.jpg|jpeg (*.jpeg)|*.jpeg|gif (*.gif)|*.gif|gif (*.png)|*.png";
			if(_openFileDialog.ShowDialog() == DialogResult.OK)
			{ 
				using (FileStream stream = File.Open(_openFileDialog.FileName,FileMode.Open,FileAccess.Read,FileShare.Read))
                {
				    byte[] buffer = new Byte[stream.Length];
				    stream.Read(buffer,0,buffer.Length);
				    if(buffer != null)
				    {
					    MemoryStream memoryStream = new MemoryStream(buffer,false);
					    this._artPictureBox.Image = Image.FromStream(memoryStream);
				    }
			    }
			}
		}

		private void removePicture_Click(object sender, System.EventArgs e)
		{
			this._artPictureBox.Image = null;
		}

        private void bit_to_kbit(object sender, ConvertEventArgs cevent)
        {
            // The method converts only to string type. Test this using the DesiredType.
            if (cevent.DesiredType != typeof(string)) return;

            // Use the ToString method to format the bit/s value as kbit/s with one decimal place ("F1").
            //cevent.Value = (System.Convert.ToDouble(cevent.Value) / 1000).ToString("F1");
            double rate = System.Convert.ToDouble(cevent.Value);
            double ratek = rate / 1000;
            string result = ratek.ToString("F1");
            cevent.Value = result;
        }

        private void OnButtonScanWholeFile(object sender, EventArgs e)
        {
            using (new CursorKeeper(Cursors.WaitCursor))
            {
                _mp3File.Audio.ScanWholeFile();
            }

            foreach (Binding b in this._labelDetails.DataBindings)
                b.ReadValue();
        }


        private void SHA_Click(object sender, EventArgs e)
        {
            using (new CursorKeeper(Cursors.WaitCursor))
            {
                byte[] sha1 = _mp3File.Audio.CalculateAudioSHA1();
                string txt = System.Convert.ToBase64String(sha1);

                _labelSHA.Text = txt;
            }
        }
	}
}
