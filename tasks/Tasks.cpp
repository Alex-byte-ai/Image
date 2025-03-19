#include "Tasks.h"

#include <fstream>

#include "Window.h"
#include "Context.h"
#include "Library.h"
#include "Information.h"

DLL_API void Task0( const void *, void * )
{
    Popup( Popup::Type::Info, L"Test", L"Sample Text." ).run();
}

DLL_API void Task1( const void *, void *result )
{
    auto &context = *( Context * )result;
    auto &text = context.output();
    auto s = context.scope( __FUNCTION__ );

    auto &info = context.information;
    bool showImages = info( "showImages" ).as<bool>();
    bool inputVariableData = info( "inputVariableData" ).as<bool>();
    bool outputVariableData = info( "outputVariableData" ).as<bool>();

    text << "showImages: " << showImages << L"\n";
    text << "inputVariableData: " << inputVariableData << L"\n";
    text << "outputVariableData: " << outputVariableData << L"\n";
}

DLL_API void Task2( const void *, void *result )
{
    auto &context = *( Context * )result;
    auto &text = context.output();
    auto s = context.scope( __FUNCTION__ );

    auto &info = context.information;
    bool showImages = info( "showImages" ).as<bool>();
    bool inputVariableData = info( "inputVariableData" ).as<bool>();
    bool outputVariableData = info( "outputVariableData" ).as<bool>();

    bool circle = false;

    if( inputVariableData )
    {
        Popup question( Popup::Type::Question, L"Circle vs Square", L"Would you like to see a circle?\nSquare will be presented otherwise." );
        question.run();

        if( question.answer )
            circle = *question.answer;
    }

    if( showImages )
    {
        if( inputVariableData )
        {
            if( circle )
                text << L"       ***       \n    *       *    \n  *           *  \n *             * \n *             * \n  *           *  \n    *       *    \n       ***       \n";
            else
                text << L"*********\n*       *\n*       *\n*       *\n*       *\n*       *\n*       *\n*       *\n*********\n";
        }
        else
        {
            text << L"     *     \n    * *    \n   *   *   \n  *     *  \n *       * \n***********\n";
        }
    }

    if( outputVariableData && inputVariableData )
    {
        std::ofstream file( context.Output() / L"choice.txt" );
        file << ( circle ? "User picked circle." : "User picked square." );
    }
}

extern "C"
{
    __attribute__( ( section( ".funcs" ), used ) ) Library::Function exportedFunctions[] =
    {
        {"Task0", Task0},
        {"Task1", Task1},
        {"Task2", Task2},
        {nullptr, nullptr} // Null-terminator
    };
}
