#include "Clipboard.h"
#include "Exception.h"
#include "Console.h"
#include "Window.h"
#include "Pause.h"
#include "Tests.h"

#include "tests/Test_00_save_and_load.h"
#include "tests/Test_01_screenshot_and_clipboard.h"
#include "tests/Test_02_stride_difference.h"
#include "tests/Test_03_clipboard_and_text.h"
#include "tests/Test_04_image_processing.h"
#include "tests/Test_05_interactive_window.h"
#include "tests/Test_06_composite_object.h"
#include "tests/Test_07_window_with_an_image.h"
#include "tests/Test_08_basic_shapes.h"
#include "tests/Test_09_colourful_rectangle_trace.h"
#include "tests/Test_10_resaving_transparent_image.h"
#include "tests/Test_11_tasks.h"
#include "tests/Test_12_watermelon.h"
#include "tests/Test_13_rotating_image.h"
#include "tests/Test_14_tiling.h"
#include "tests/Test_15_DDS.h"
#include "tests/Test_16_ICO.h"
#include "tests/Test_18_color_vector.h"
#include "tests/Test_19_icon_for_console.h"
#include "tests/Test_20_discrete_graphs.h"
#include "tests/Test_21_Mandelbrot_set.h"
#include "tests/Test_22_Polygon.h"
#include "tests/Test_23_Translate.h"
#include "tests/Test_24_icon_for_image.h"
#include "tests/Test_25_remove_checkered_pattern.h"
#include "tests/Test_26_Force_image_size.h"
#include "tests/Test_27_Mesh.h"
#include "tests/Test_28_Tree.h"
#include "tests/Test_29_Carpet.h"
#include "tests/Test_30_Flower.h"
#include "tests/Test_31_Procedural_textures.h"
#include "tests/Test_32_JustEdit.h"

#include "tests/Test_X0.h"
#include "tests/Test_X1.h"
#include "tests/Test_X2.h"
#include "tests/Test_X3.h"

int main()
{
    try
    {
        Console console;
        Pause pause;

        pause.prepare = [&console]( Pause::InputType category, const std::optional<std::wstring> &message )
        {
            if( category & Pause::InputType::clibpoardHasValue )
                console( L"Copy something.\n" );
            if( category & Pause::InputType::windowFocused )
                console( L"Select console window.\n" );

            switch( *category )
            {
            case Pause::InputType::any:
                console( L"Press ⌨ any key to continue...\n" );
                break;
            case Pause::InputType::enter:
                console( L"Press ↩ 'Enter' to continue...\n" );
                break;
            case Pause::InputType::shift:
                console( L"Press ⇧ 'Shift' to continue...\n" );
                break;
            case Pause::InputType::esc:
                console( L"Press ⎋ 'Escape' to continue...\n" );
                break;
            case Pause::InputType::prtSc:
                console( L"Press 🖼 'Print screen' to continue...\n" );
                break;
            default:
                makeException( false );
            }

            if( message )
                console( *message + L"\n" );

            return true;
        };

        pause.process = [&console]( Pause::InputType category ) -> std::optional<bool>
        {
            switch( category )
            {
            case Pause::InputType::windowFocused:
                return console.focused();
            default:
                break;
            }
            return {};
        };

        Information::Item info;

        info( L"readDisk" ) = true;
        info( L"writeDisk" ) = true;

        info( L"pause" ) = true;

        info( L"additional" ) = true;

        // Any input and output, not just disks
        info( L"showImages" ) = true;
        info( L"inputVariableData" ) = true;
        info( L"outputVariableData" ) = true;

        // 'blacklist' and 'whitelist'
        // info( L"whitelist" )[0]
        // info( L"blacklist" )[0]
        info( L"blacklist" )[0] = L"Test X0";
        info( L"blacklist" )[1] = L"Test X1";
        info( L"blacklist" )[2] = L"Test X2";
        info( L"blacklist" )[3] = L"Test X3";

        // info( L"whitelist" )[0] = L"Test 11 tasks";
        // info( L"whitelist" )[0] = L"Test 22 Polygon";
        // info( L"whitelist" )[0] = L"Test 23 Translate";
        // info( L"whitelist" )[0] = L"Test 27 Mesh";
        // info( L"whitelist" )[0] = L"Test 31 Procedural textures";
        info( L"whitelist" )[0] = L"Test 32 JustEdit";

        Tests tests( console, pause, info );

        tests( Test_00_save_and_load );
        tests( Test_01_screenshot_and_clipboard );
        tests( Test_02_stride_difference );
        tests( Test_03_clipboard_and_text );
        tests( Test_04_image_processing );
        tests( Test_05_interactive_window );
        tests( Test_06_composite_object );
        tests( Test_07_window_with_an_image );
        tests( Test_08_basic_shapes );
        tests( Test_09_colourful_rectangle_trace );
        tests( Test_10_resaving_transparent_image );
        tests( Test_11_tasks );
        tests( Test_12_watermelon );
        tests( Test_13_rotating_image );
        tests( Test_14_tiling );
        tests( Test_15_DDS );
        tests( Test_16_ICO );
        // tests( Test_17_CUR );
        tests( Test_18_color_vector );
        tests( Test_19_icon_for_console );
        tests( Test_20_discrete_graphs );
        tests( Test_21_Mandelbrot_set );
        tests( Test_22_Polygon );
        tests( Test_23_Translate );
        tests( Test_24_icon_for_image );
        tests( Test_25_remove_checkered_pattern );
        tests( Test_26_Force_image_size );
        tests( Test_27_Mesh );
        tests( Test_28_Tree );
        tests( Test_29_Carpet );
        tests( Test_30_Flower );
        tests( Test_31_Procedural_textures );
        tests( Test_32_JustEdit );

        tests( Test_X0 ); //Ex test10 ???
        tests( Test_X1 ); //Ex test11 ???
        tests( Test_X2 ); //Ex test12 ???
        tests( Test_X3 ); //Ex test13 ???

        tests.run();
    }
    catch( const Exception &e )
    {
        Popup( Popup::Type::Error, L"Error", e.message() ).run();
    }
    catch( const std::exception &e )
    {
        Popup( Popup::Type::Error, L"Error", Exception::extract( e.what() ) ).run();
    }
    catch( ... )
    {
        Popup( Popup::Type::Error, L"Error", L"Program failed!" ).run();
    }

    return 0;
}
