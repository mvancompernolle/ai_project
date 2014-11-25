#include <more/io/filesys.h>
#include <more/diag/debug.h>

namespace io = more::io;

int
main()
{
    using io::relative_file_name;
    using io::file_name_sans_extension;
    using io::file_name_extension;

    MORE_CHECK_EQ(relative_file_name("common/component/and/file.ext",
				     "common/component"),
		  "and/file.ext");
    MORE_CHECK_EQ(relative_file_name("common/component/and/file.ext",
				     "common/component/"),
		  "and/file.ext");
    MORE_CHECK_EQ(relative_file_name("not/common", "nott"), "../not/common");
    MORE_CHECK_EQ(relative_file_name("not/common", "nott/"), "../not/common");
    MORE_CHECK_EQ(relative_file_name("the/same", "the/same"), ".");
    MORE_CHECK_EQ(relative_file_name("the/same/", "the/same"), ".");
    MORE_CHECK_EQ(relative_file_name("the/same", "the/same/"), ".");
    MORE_CHECK_EQ(relative_file_name("the/same/", "the/same/"), ".");

    MORE_CHECK_EQ(file_name_sans_extension("base"), "base");
    MORE_CHECK_EQ(file_name_sans_extension("base.ext"), "base");
    MORE_CHECK_EQ(file_name_sans_extension("./base"), "./base");
    MORE_CHECK_EQ(file_name_sans_extension("./base.ext"), "./base");
    MORE_CHECK_EQ(file_name_sans_extension("dir.d/base"), "dir.d/base");
    MORE_CHECK_EQ(file_name_sans_extension("dir.d/.base"), "dir.d/.base");
    MORE_CHECK_EQ(file_name_sans_extension("dir.d/.base.ext"), "dir.d/.base");
    MORE_CHECK_EQ(file_name_sans_extension(".dir/base"), ".dir/base");

    MORE_CHECK_EQ(file_name_extension("base"), "");
    MORE_CHECK_EQ(file_name_extension("base.ext"), ".ext");
    MORE_CHECK_EQ(file_name_extension(".base"), "");
    MORE_CHECK_EQ(file_name_extension("dir.d/base"), "");
    MORE_CHECK_EQ(file_name_extension("dir.d/base.ext"), ".ext");
    MORE_CHECK_EQ(file_name_extension("dir.d/.base"), "");

    return more::diag::check_exit_status();
}
