#include "Tasks.h"

#include "Window.h"
#include "Context.h"
#include "Information.h"

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
