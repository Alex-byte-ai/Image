#include "Test_11_tasks.h"

#include "Library.h"

void Test_11_tasks( Context &context )
{
    auto &text = context.output();
    auto s = context.scope( __FUNCTION__ );

#ifdef _DEBUG
    Library library( L"tasks\\Debug\\bin\\Tasks.dll" );
#else
    Library library( L"tasks\\Release\\bin\\Tasks.dll" );
#endif

    auto check = [&]( const std::string & name )
    {
        if( library.call( name, nullptr, &context ) )
        {
            text << name << L" was executed.\n";
            return;
        }
        text << name << L" does not exist.\n";
    };

    for( auto &function : library.functions() )
        check( function );
}
