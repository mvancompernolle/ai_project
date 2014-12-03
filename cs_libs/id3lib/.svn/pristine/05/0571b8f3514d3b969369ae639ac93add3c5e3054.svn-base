using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;

namespace Utils
{
    public class CursorKeeper : IDisposable
    {
        private Cursor _originalCursor;
        private bool _isDisposed = false;

        public CursorKeeper(Cursor newCursor)
        {
            _originalCursor = Cursor.Current;
            Cursor.Current = newCursor;
        }

        #region " IDisposable Support "
        protected virtual void Dispose(bool disposing)
        {
            if (!_isDisposed)
            {
                if (disposing)
                {
                    Cursor.Current = _originalCursor;
                }
            }
            _isDisposed = true;
        }

        public void Dispose()
        {
            // Do not change this code.  Put cleanup code in Dispose(ByVal disposing As Boolean) above.
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        #endregion
    }
}

