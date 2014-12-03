using Utils;
using System.IO;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace UtilsTests
{
    /// <summary>
    ///This is a test class for FileMoverTest and is intended
    ///to contain all FileMoverTest Unit Tests
    ///</summary>
    [TestClass()]
    public class FileMoverTest
    {
        private TestContext testContextInstance;

        /// <summary>
        ///Gets or sets the test context which provides
        ///information about and functionality for the current test run.
        ///</summary>
        public TestContext TestContext
        {
            get
            {
                return testContextInstance;
            }
            set
            {
                testContextInstance = value;
            }
        }

        #region Additional test attributes
        // 
        //You can use the following additional attributes as you write your tests:
        //
        //Use ClassInitialize to run code before running the first test in the class
        //[ClassInitialize()]
        //public static void MyClassInitialize(TestContext testContext)
        //{
        //}
        //
        //Use ClassCleanup to run code after all tests in a class have run
        //[ClassCleanup()]
        //public static void MyClassCleanup()
        //{
        //}
        //
        //Use TestInitialize to run code before running each test
        //[TestInitialize()]
        //public void MyTestInitialize()
        //{
        //}
        //
        //Use TestCleanup to run code after each test has run
        //[TestCleanup()]
        //public void MyTestCleanup()
        //{
        //}
        //
        #endregion
        
        /// <summary>
        ///A test for FileMove
        ///</summary>
        [TestMethod()]
        public void FileMoveTest()
        {
            FileInfo sourceLocation = new FileInfo(TestContext.TestName + ".new");
            FileInfo targetLocation = new FileInfo(TestContext.TestName + ".txt");
            FileInfo backupLocation = new FileInfo(TestContext.TestName + ".bak");

            File.WriteAllText(sourceLocation.FullName, "source");
            File.WriteAllText(targetLocation.FullName, "target");
            File.WriteAllText(backupLocation.FullName, "backup");

            FileMover.FileMove(sourceLocation, targetLocation, backupLocation);

            Assert.AreEqual(File.ReadAllText(targetLocation.FullName), "source");
            Assert.AreEqual(File.ReadAllText(backupLocation.FullName), "target");
            bool exists = sourceLocation.Exists;
            Assert.IsFalse(exists);

            targetLocation.Delete();
            backupLocation.Delete();
        }

        /// <summary>
        ///A test for FileMove where the target is locked
        ///</summary>
        [TestMethod()]
        public void FileMoveTestLockedTarget()
        {
            FileInfo sourceLocation = new FileInfo(TestContext.TestName + ".new");
            FileInfo targetLocation = new FileInfo(TestContext.TestName + ".txt");
            FileInfo backupLocation = new FileInfo(TestContext.TestName + ".bak");

            File.WriteAllText(sourceLocation.FullName, "source");
            File.WriteAllText(targetLocation.FullName, "target");
            File.WriteAllText(backupLocation.FullName, "backup");

            try
            {
                using (FileStream locker = targetLocation.OpenRead())
                {
                    FileMover.FileMove(sourceLocation, targetLocation, backupLocation);
                }
                Assert.Fail("should have thrown an exception");
            }
            catch 
            {}

            // check the original files are still there
            Assert.AreEqual(File.ReadAllText(sourceLocation.FullName), "source");
            Assert.AreEqual(File.ReadAllText(targetLocation.FullName), "target");
            Assert.AreEqual(File.ReadAllText(backupLocation.FullName), "backup");

            sourceLocation.Delete();
            targetLocation.Delete();
            backupLocation.Delete();
        }

        /// <summary>
        ///A test for FileMove where the source is locked
        ///</summary>
        [TestMethod()]
        public void FileMoveTestLockedSource()
        {
            FileInfo sourceLocation = new FileInfo(TestContext.TestName + ".new");
            FileInfo targetLocation = new FileInfo(TestContext.TestName + ".txt");
            FileInfo backupLocation = new FileInfo(TestContext.TestName + ".bak");

            File.WriteAllText(sourceLocation.FullName, "source");
            File.WriteAllText(targetLocation.FullName, "target");
            File.WriteAllText(backupLocation.FullName, "backup");

            try
            {
                using (FileStream locker = sourceLocation.OpenRead())
                {
                    FileMover.FileMove(sourceLocation, targetLocation, backupLocation);
                }
                Assert.Fail("should have thrown an exception");
            }
            catch 
            {}

            // check the original files are still there
            Assert.AreEqual(File.ReadAllText(sourceLocation.FullName), "source");
            Assert.AreEqual(File.ReadAllText(targetLocation.FullName), "target");
            Assert.AreEqual(File.ReadAllText(backupLocation.FullName), "backup");

            sourceLocation.Delete();
            targetLocation.Delete();
            backupLocation.Delete();
        }

        /// <summary>
        ///A test for NonNtfsReplace
        ///</summary>
        [TestMethod()]
        [DeploymentItem("Id3Lib.dll")]
        public void NonNtfsReplaceTest()
        {
            FileInfo sourceLocation = new FileInfo(TestContext.TestName + ".new");
            FileInfo targetLocation = new FileInfo(TestContext.TestName + ".txt");
            FileInfo backupLocation = new FileInfo(TestContext.TestName + ".bak");

            File.WriteAllText(sourceLocation.FullName, "source");
            File.WriteAllText(targetLocation.FullName, "target");
            File.WriteAllText(backupLocation.FullName, "backup");

            FileMover.NonNtfsReplace(sourceLocation, targetLocation, backupLocation);

            Assert.AreEqual(File.ReadAllText(targetLocation.FullName), "source");
            Assert.AreEqual(File.ReadAllText(backupLocation.FullName), "target");
            bool exists = sourceLocation.Exists;
            Assert.IsFalse(exists);

            targetLocation.Delete();
            backupLocation.Delete();
        }

        /// <summary>
        ///A test for FileMove where the target is locked
        ///</summary>
        [TestMethod()]
        public void NonNtfsReplaceTestLockedTarget()
        {
            FileInfo sourceLocation = new FileInfo(TestContext.TestName + ".new");
            FileInfo targetLocation = new FileInfo(TestContext.TestName + ".txt");
            FileInfo backupLocation = new FileInfo(TestContext.TestName + ".bak");

            File.WriteAllText(sourceLocation.FullName, "source");
            File.WriteAllText(targetLocation.FullName, "target");
            File.WriteAllText(backupLocation.FullName, "backup");

            try
            {
                using (FileStream locker = targetLocation.OpenRead())
                {
                    FileMover.NonNtfsReplace(sourceLocation, targetLocation, backupLocation);
                }
                Assert.Fail("should have thrown an exception");
            }
            catch 
            {}

            // check the original files are still there
            Assert.AreEqual(File.ReadAllText(sourceLocation.FullName), "source");
            Assert.AreEqual(File.ReadAllText(targetLocation.FullName), "target");
            Assert.AreEqual(File.ReadAllText(backupLocation.FullName), "backup");

            sourceLocation.Delete();
            targetLocation.Delete();
            backupLocation.Delete();
        }

        /// <summary>
        ///A test for FileMove where the source is locked
        ///</summary>
        [TestMethod()]
        public void NonNtfsReplaceTestLockedSource()
        {
            FileInfo sourceLocation = new FileInfo(TestContext.TestName + ".new");
            FileInfo targetLocation = new FileInfo(TestContext.TestName + ".txt");
            FileInfo backupLocation = new FileInfo(TestContext.TestName + ".bak");

            File.WriteAllText(sourceLocation.FullName, "source");
            File.WriteAllText(targetLocation.FullName, "target");
            File.WriteAllText(backupLocation.FullName, "backup");

            try
            {
                using (FileStream locker = sourceLocation.OpenRead())
                {
                    FileMover.NonNtfsReplace(sourceLocation, targetLocation, backupLocation);
                }
                Assert.Fail("should have thrown an exception");
            }
            catch 
            {}

            // check the original files are still there
            Assert.AreEqual(File.ReadAllText(sourceLocation.FullName), "source");
            Assert.AreEqual(File.ReadAllText(targetLocation.FullName), "target");
            Assert.AreEqual(File.ReadAllText(backupLocation.FullName), "backup");

            sourceLocation.Delete();
            targetLocation.Delete();
            backupLocation.Delete();
        }

        /// <summary>
        ///A test for FileMove where the backup is locked
        ///</summary>
        [TestMethod()]
        public void NonNtfsReplaceTestLockedBackup()
        {
            FileInfo sourceLocation = new FileInfo(TestContext.TestName + ".new");
            FileInfo targetLocation = new FileInfo(TestContext.TestName + ".txt");
            FileInfo backupLocation = new FileInfo(TestContext.TestName + ".bak");

            File.WriteAllText(sourceLocation.FullName, "source");
            File.WriteAllText(targetLocation.FullName, "target");
            File.WriteAllText(backupLocation.FullName, "backup");

            try
            {
                using (FileStream locker = backupLocation.OpenRead())
                {
                    FileMover.NonNtfsReplace(sourceLocation, targetLocation, backupLocation);
                }
                Assert.Fail("should have thrown an exception");
            }
            catch
            { }

            // check the original files are still there
            Assert.AreEqual(File.ReadAllText(sourceLocation.FullName), "source");
            Assert.AreEqual(File.ReadAllText(targetLocation.FullName), "target");
            Assert.AreEqual(File.ReadAllText(backupLocation.FullName), "backup");

            sourceLocation.Delete();
            targetLocation.Delete();
            backupLocation.Delete();
        }
    }
}

