using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;                 // for MessageBox

namespace Utils
{
    public class ExceptionMessageBox
    {
        public static void Show( Exception ex, string title )
        {
            //// Add the new top-level message to the handled exception.
            //ApplicationException exTop = new ApplicationException(title, ex);
            //exTop.Source = _form.Text;

            //// Show an exception message box with an OK button (the default).
            //ExceptionMessageBox box = new ExceptionMessageBox(exTop);
            //box.Show(_form);

            Show(null, ex, title);
        }

        public static void Show( IWin32Window owner, Exception ex, string title )
        {
            DialogResult result = MessageBox.Show(owner, ex.Message + "\nShow Details?", title, MessageBoxButtons.OKCancel, MessageBoxIcon.Error, MessageBoxDefaultButton.Button2);
            if( result == DialogResult.OK )
            {
                string details = ex.Message;
                Exception iter = ex.InnerException;
                while(iter != null)
                {
                    details += "\nfrom: " + iter.Message;
                    iter = iter.InnerException;
                }
                details += "\n\n" + ex.StackTrace;

                MessageBox.Show(owner, details, title, MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }
    }
}
