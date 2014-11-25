#include "tri_logger.hpp"

struct Foo
{
    Foo()
    {
        TRI_LOG_STR ( "Creation of the foo object." );
    }

    ~Foo()
    {
        TRI_LOG_STR ( "Destruction of the foo object." );
    }
};

// statically created and initiazed object
Foo foo;
