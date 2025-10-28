#include "Tasks.h"

#include <fstream>

#include "Window.h"
#include "Context.h"
#include "Information.h"

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
