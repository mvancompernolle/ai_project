using System;
using System.Runtime.InteropServices;
using System.ComponentModel;
using System.Threading;

namespace Win32
{
    public class HiPerfTimer
    {
        [DllImport("Kernel32.dll")]
        private static extern bool QueryPerformanceCounter(
            out long lpPerformanceCount);

        [DllImport("Kernel32.dll")]
        private static extern bool QueryPerformanceFrequency(
            out long lpFrequency);

        private long _startTime;//, _stopTime;
        static private long _freq = GetFreq();

        static long GetFreq()
        {
            long freq;

            if (QueryPerformanceFrequency(out freq) == false)
            {
                // high-performance counter not supported

                throw new Win32Exception();
            }
            return freq;
        }


        // Constructor

        public HiPerfTimer()
        {
            _startTime = 0;
            //_stopTime = 0;

            QueryPerformanceCounter(out _startTime);
        }

        //// Start the timer

        //public void Start()
        //{
        //    // lets do the waiting threads there work

        //    Thread.Sleep(0);

        //    QueryPerformanceCounter(out _startTime);
        //}

        //// Stop the timer

        //public void Stop()
        //{
        //    QueryPerformanceCounter(out _stopTime);
        //}

        // Returns the duration of the timer (in seconds)

        public double Duration
        {
            get
            {
                long stopTime;
                QueryPerformanceCounter(out stopTime);

                return (double)(stopTime - _startTime) / (double)_freq;
            }
        }
    }
}
