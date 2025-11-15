#include "ImageWindow.h"

#include <algorithm>
#include <cstdlib>

#include "Exception.h"
#include "Lambda.h"
#include "Basic.h"

#include "ImageData.h"

#include <windows.h>

std::optional<GraphicInterface::Window> ImageWindow::description;

ImageWindow::OutputData::OutputData( GenericWindow::OutputData& original, ImageDataBase &img ) : image( img ), x( original.x ), y( original.y ), popup( original.popup ), quit( original.quit )
{}

ImageWindow::ImageWindow( ImageDataBase &idb, HandleMsg h, std::shared_ptr<JustEdit::Entity> object, Data initData )
    : rootObject( std::move( object ) ), data( std::move( initData ) ), handler( std::move( h ) ), image( idb )
{
    updateRoot();

    if( !description )
    {
        description.emplace();
        auto& desc = *description;

        std::vector<ImageData> samples;
        ImageData::readICO( L"image.ico", samples );

        auto isEmpty = []( int i, const auto & img )
        {
            int j = 0;
            bool empty = true;
            while( empty && j < img.w() )
            {
                empty = img( j, i )->a == 0;
                ++j;
            }
            return empty;
        };

        int maxHeight = 0;
        size_t idMax = 0, id = 0;
        int lowerMax = 0, upperMax = 0;
        for( const auto &sample : samples )
        {
            int realHeight = sample.h();

            int upper = 0;
            bool empty = true;
            while( empty && upper < sample.h() )
            {
                if( ( empty = isEmpty( upper, sample ) ) )
                    ++upper;
            }

            realHeight -= upper;

            int lower = 0;
            empty = true;
            while( empty && lower < sample.h() )
            {
                if( ( empty = isEmpty( sample.h() - lower - 1, sample ) ) )
                    ++lower;
            }

            realHeight -= lower;

            if( realHeight < desc.titlebarHeight && realHeight > maxHeight )
            {
                maxHeight = realHeight;
                lowerMax = lower;
                upperMax = upper;
                idMax = id;
            }

            ++id;
        }

        if( maxHeight > 0 )
        {
            ImageData icon;
            samples[idMax].sub( icon, 0, upperMax, samples[idMax].w(), samples[idMax].h() - lowerMax );
            desc.icon.prepare( icon( 0, 0 ), icon.s(), icon.h() );
        }

        desc.title.value = title;
        desc.title.prepare( desc.titleBar.color );

        desc.self.x = -1;
        desc.self.y = -1;

        desc.content.w = image.w();
        desc.content.h = image.h();

        desc.self.w = desc.minWidth();
        desc.self.h = desc.minHeight();

        desc.update();
    }
    else
    {
        auto& desc = *description;

        desc.title.value = title;
        desc.title.prepare( desc.titleBar.color );

        desc.update();
    }
}

ImageWindow::~ImageWindow()
{}

void ImageWindow::run()
{
    GenericWindow::HandleMsg outerHandler;

    if( !rootObject )
    {
        outerHandler = [this, callback = handler]( const GenericWindow::InputData & input, GenericWindow::OutputData & output )
        {
            if( input.escape.changed() && *input.escape )
            {
                output.quit = true;
                return;
            }

            OutputData outputData( output, image );

            if( callback )
                callback( input, outputData );

            if( input.init || outputData.image.changed() )
            {
                output.image.get().prepare( image( 0, 0 ), image.s(), image.h() );
            }

            if( input.space.changed() && *input.space )
            {
                image.output();
            }

            if( input.f1.changed() && *input.f1 )
            {
                if( data.help )
                    data.help->run();
            }
        };
    }
    else
    {
        outerHandler = [this]( const GenericWindow::InputData & input, GenericWindow::OutputData & output )
        {
            auto getFreeName = [this]( const std::wstring & prefix )
            {
                unsigned freeId = 0, length = prefix.length() + 1;
                std::wstring freeIdString = L"0";

                ( *rootObject )( [&]( JustEdit::Entity * s )
                {
                    if( s->name.length() == length && s->name.substr( 0, prefix.length() ) == prefix )
                    {
                        if( s->name.substr( 0, prefix.length() ) == freeIdString )
                        {
                            ++freeId;
                            freeIdString = std::to_wstring( freeId );
                            length = prefix.length() + freeIdString.length();
                        }
                    }

                    return true;
                } );

                return prefix + freeIdString;
            };

            auto update = [&]()
            {
                image.function( image, []( int, int, int j, int i, Pixel, Pixel & out )
                {
                    out = ( i / 16 ) % 2 == ( j / 16 ) % 2 ? Pixel( 85, 85, 85 ) : Pixel( 170, 170, 170 );
                } );

                Overlap::Canvas canavs( image );
                root->draw( camera, canavs );
                selection->draw( camera, canavs );
                canavs.render( image );

                output.image.get().prepare( image( 0, 0 ), image.s(), image.h() );
            };

            auto focusCamera = [&]( const Vector2D & topLeft, const Vector2D & bottomRight, double baseScale = 1.0 )
            {
                int w = image.w();
                int h = image.h();
                double imageW = Round( baseScale * w );
                double imageH = Round( baseScale * h );

                auto sx = imageW / ( bottomRight.x - topLeft.x );
                auto sy = imageH / ( bottomRight.y - topLeft.y );

                if( sx > 16 )
                    sx = 16;
                else if( sx < 0.0625 )
                    sx = 0.0625;

                if( sy > 16 )
                    sy = 16;
                else if( sy < 0.0625 )
                    sy = 0.0625;

                if( sx > 1 )
                    sx = RoundDown( sx );

                if( sy > 1 )
                    sy = RoundDown( sy );

                auto scale = Min( sx, sy );

                imageW = scale * ( bottomRight.x - topLeft.x );
                imageH = scale * ( bottomRight.y - topLeft.y );

                camera = Affine2D( Vector2D( w * 0.5, h * 0.5 ) ) * Affine2D( Matrix2D::Scale( scale ) ) * Affine2D( -( topLeft + bottomRight ) * 0.5 );
                update();
            };

            auto fitCamera = [&]()
            {
                Vector2D topLeft, bottomRight;
                if( root->size( Affine2D( Vector2D() ), topLeft, bottomRight ) )
                    focusCamera( topLeft, bottomRight, 0.65 );
            };

            auto edit = [&]( JustEdit::Entity * target )
            {
                Settings::Parameters parameters;
                for( auto& [name, set, get, options] : target->editData() )
                    parameters.emplace_back( name, set, get, options );

                Settings settings( target->description(), parameters );
                settings.run();

                update();
            };

            auto create = [&]( Vector2D p, int itemId )
            {
                using namespace JustEdit;

                p = camera.inv()( p );

                if( itemId == 0 )
                {
                    selection->select( root->add( std::make_shared<JustEdit::Raster>( getFreeName( L"raster" ), 64, 64, Position( p ) ) ), false );
                }
                else if( itemId == 1 )
                {
                    selection->select( root->add( std::make_shared<JustEdit::Line>( getFreeName( L"line" ), p, p + Vector2D( 32, 32 ) ) ), false );
                }
                else if( itemId == 2 )
                {
                    selection->select( root->add( std::make_shared<JustEdit::Rectangle>( getFreeName( L"rectangle" ), 32, 16, Position( p ) ) ), false );
                }
                else if( itemId == 3 )
                {
                    selection->select( root->add( std::make_shared<JustEdit::Circle>( getFreeName( L"circle" ), p, 24 ) ), false );
                }
                else if( itemId == 4 )
                {
                    selection->select( root->add( std::make_shared<JustEdit::Text>( getFreeName( L"text" ), L"Lorem ipsum", Position( p ) ) ), false );
                }
                else if( itemId == 5 )
                {
                    selection->select( root->add( std::make_shared<JustEdit::Polygon>( getFreeName( L"polygon" ), Position( p ) ) ), false );
                }
                else if( itemId == 6 )
                {
                    selection->select( root->add( std::make_shared<JustEdit::Point>( getFreeName( L"point" ), 0, p ) ), false );
                }

                update();
            };

            static uint16_t toolId = 0;
            static std::optional<Vector2D> initialCanvasGrab;
            auto pickTool = [&]( uint16_t id )
            {
                toolId = id;
            };

            auto modify = [&]( JustEdit::Entity * target, int modificationId, bool test )
            {
                if( !target || target == root || target->type() != L"Raster" )
                    return false;

                if( test )
                    return true;

                if( modificationId == 0 )
                {
                    auto targetRoot = target->getRoot();
                    selection->select( targetRoot->add( std::make_shared<JustEdit::Perspective>( getFreeName( L"perspective" ), target, target->position ) ), false );
                    target->position = JustEdit::Position();
                    update();
                }

                return true;
            };

            auto deletef = [&]( const std::vector<JustEdit::Entity*>& targets, bool test )
            {
                if( targets.empty() )
                    return false;

                for( auto target : targets )
                {
                    if( !target )
                        return false;

                    auto rootContainer = root;
                    while( rootContainer )
                    {
                        if( rootContainer == target )
                            return false;
                        rootContainer = rootContainer->getRoot();
                    }
                }

                if( test )
                    return true;

                selection->select( nullptr, false );

                for( auto target : targets )
                    target->detach();

                update();
                return true;
            };

            auto group = [&]( bool f, bool test )
            {
                if( f )
                {
                    auto targets = selection->getTargets();
                    if( targets.empty() )
                        return false;

                    if( test )
                        return true;

                    selection->select( nullptr, false );

                    auto nodes = targets[0]->getRoot()->getNodes();
                    auto g = std::make_shared<JustEdit::Group>( L"group" );
                    for( auto target : nodes )
                    {
                        if( std::find( targets.begin(), targets.end(), target ) != targets.end() )
                            g->add( target->detach() );
                    }

                    root->add( g );
                    update();
                    return true;
                }

                auto g = selection->getTarget();
                if( !g || g->type() != L"Group" )
                    return false;

                auto nodes = g->getNodes();
                if( nodes.empty() )
                    return false;

                if( test )
                    return true;

                selection->select( nullptr, false );

                for( auto node : nodes )
                {
                    node->position( g->position() * node->position() );
                    root->add( node->detach() );
                }

                g->detach();
                update();
                return true;
            };

            auto open = [&]()
            {
                auto newRoot = JustEdit::Entity::load();
                if( newRoot )
                {
                    rootObject = newRoot;
                    updateRoot();
                    update();
                }
                else
                {
                    Popup( Popup::Type::Error, L"Loading", L"File can't be loaded." ).run();
                }
            };

            auto save = [&]()
            {
                auto mainRoot = root;
                auto next = mainRoot->getRoot();
                while( next )
                {
                    mainRoot = next;
                    next = next->getRoot();
                }
                mainRoot->save();
            };

            auto import = [&]( const Vector2D & p )
            {
                auto raster = std::make_shared<JustEdit::Raster>( getFreeName( L"import" ), 64, 64, JustEdit::Position( camera.inv()( p ) ) );

                raster->image->input();
                raster->w = raster->image->w();
                raster->h = raster->image->h();

                raster->position.scaleX = 64.0 / raster->w;
                raster->position.scaleY = 64.0 / raster->h;

                root->add( raster );
                selection->select( raster.get(), false );
                update();
            };

            auto exportf = [&]()
            {
                Vector2D topLeft, bottomRight;
                if( root->size( Affine2D( Vector2D() ), topLeft, bottomRight ) )
                {
                    int w = RoundUp( bottomRight.x - topLeft.x );
                    int h = RoundUp( bottomRight.y - topLeft.y );

                    ImageData img( w, h );

                    Overlap::Canvas canavs( img );
                    root->draw( Affine2D( -topLeft ), canavs );
                    canavs.render( img );

                    img.output();
                }
            };

            auto undo = [&]( bool f )
            {
                Popup( Popup::Type::Info, L"Change buffer", f ? L"Undone is not implemented" : L"Redone is not implemented" ).run();
            };

            auto view = [&]( JustEdit::Entity * target, bool test )
            {
                if( target )
                {
                    Vector2D a, b;
                    if( target && target->size( Affine2D( Vector2D() ), a, b ) )
                    {
                        if( test )
                            return true;
                        selection->select( nullptr, false );
                        root = target;
                        target->establishStructure();
                        fitCamera();
                        return true;
                    }

                    return false;
                }

                auto newRoot = root->getRoot();
                if( newRoot )
                {
                    if( test )
                        return true;
                    selection->select( newRoot, false );
                    root->dumpStructure();
                    root = newRoot;
                    fitCamera();
                    return true;
                }

                return false;
            };

            auto help = [&]()
            {
                Popup( Popup::Type::Info, L"Help", L"Some information..." ).run();
            };

            auto exit = [&]()
            {
                output.quit = true;
            };

            auto keyDown = [&]( char symbol )
            {
                auto &key = input.keys.letter( symbol );
                return key.changed() && *key;
            };

            if( input.init )
            {
                int w = RoundUp( GetSystemMetrics( SM_CXSCREEN ) * 0.65 );
                int h = RoundUp( GetSystemMetrics( SM_CYSCREEN ) * 0.65 );

                if( w % 2 == 0 )
                    --w;

                if( h % 2 == 0 )
                    --h;

                image.reset( w, h );

                fitCamera();
            }

            if( *input.ctrl && keyDown( 'G' ) )
                group( true, false );

            if( *input.ctrl && keyDown( 'U' ) )
                group( false, false );

            if( *input.ctrl && keyDown( 'O' ) )
                open();

            if( *input.ctrl && keyDown( 'S' ) )
                save();

            if( *input.ctrl && *input.up && input.up.changed() )
            {
                if( auto target = selection->getTarget() )
                    view( target, false );
            }

            if( *input.ctrl && *input.down && input.down.changed() )
            {
                view( nullptr, false );
            }

            if( *input.del && input.del.changed() )
            {
                deletef( selection->getTargets(), false );
            }

            static uint16_t forcedOutTool = 0;
            if( input.space.changed() && *input.space )
            {
                forcedOutTool = toolId;
                toolId = 2;
            }

            if( input.space.changed() && !*input.space )
                toolId = forcedOutTool;

            bool lmb = input.leftMouse.changed() && *input.leftMouse;
            bool rmb = input.rightMouse.changed() && *input.rightMouse;

            if( lmb || rmb )
            {
                Vector2D point( *input.mouseX, *input.mouseY );
                auto target = root->pointsTo( camera, point, JustEdit::SelectionMode::Part );

                bool isSelection = false;

                if( lmb )
                {
                    if( toolId == 0 )
                    {
                        if( selection->grab( camera, point ) )
                        {
                            isSelection = true;
                        }
                        else
                        {
                            if( target != root )
                                selection->select( target, *input.ctrl );
                            else
                                selection->select( nullptr, *input.ctrl );
                            update();
                        }
                    }
                    else if( toolId == 1 && dynamic_cast<JustEdit::Raster*>( root ) )
                    {
                    }
                    else if( toolId == 2 )
                    {
                        initialCanvasGrab = camera.s - Vector2D( *input.mouseX, *input.mouseY );
                    }
                    else if( toolId == 3 )
                    {
                        initialCanvasGrab = Vector2D( *input.mouseX, *input.mouseY );
                    }
                }

                if( rmb )
                {
                    auto make = []( const auto & f, const auto & arg )
                    {
                        std::function<void()> function = [f, arg]()
                        {
                            f( arg );
                        };
                        return function;
                    };

                    auto make2 = []( const auto & f, const auto & arg0, const auto & arg1 )
                    {
                        std::function<void()> function = [f, arg0, arg1]()
                        {
                            f( arg0, arg1 );
                        };
                        return function;
                    };

                    auto make3 = []( const auto & f, const auto & arg0, const auto & arg1, const auto & arg2 )
                    {
                        std::function<void()> function = [f, arg0, arg1, arg2]()
                        {
                            f( arg0, arg1, arg2 );
                        };
                        return function;
                    };

                    ContextMenu menu(
                    {
                        {L"Edit", !isSelection && target, make( edit, target )},
                        {},
                        {
                            L"Tools", true, {},
                            {
                                {L"Select", !initialCanvasGrab, make( pickTool, 0 )},
                                {L"Pixel drawing", !initialCanvasGrab, make( pickTool, 1 )},
                                {L"Move canvas (Hold Space)", !initialCanvasGrab, make( pickTool, 2 )},
                                {L"Zoom (Mouse / Ctrl+(+ / -))", !initialCanvasGrab, make( pickTool, 3 )},
                            }
                        },
                        {},
                        {
                            L"Create", true, {},
                            {
                                {L"Raster", true, make2( create, point, 0 )},
                                {L"Line", true, make2( create, point, 1 )},
                                {L"Rectangle", true, make2( create, point, 2 )},
                                {L"Circle", true, make2( create, point, 3 )},
                                {L"Text", true, make2( create, point, 4 )},
                                {L"Polygon", true, make2( create, point, 5 )},
                                {L"Point", dynamic_cast<JustEdit::Polygon*>( root ) != nullptr, make2( create, point, 6 )}
                            }
                        },
                        {
                            L"Modify", true, {},
                            {
                                {
                                    L"Transform", true, {},
                                    {
                                        {L"Perspective", modify( target, 0, true ), make3( modify, target, 0, false )}
                                    }
                                }
                            }
                        },
                        { L"Delete (Del)", !isSelection && deletef( { target }, true ), make2( deletef, std::vector<JustEdit::Entity*>{ target }, false ) },
                        {},
                        { L"Group (ctrl+G)", group( true, true ), make2( group, true, false ) },
                        { L"Ungroup (ctrl+U)", group( false, true ), make2( group, false, false ) },
                        {},
                        { L"Copy (ctrl+C)", false },
                        { L"Cut (ctrl+X)", false },
                        { L"Paste (ctrl+V)", false },
                        { L"Place (ctrl+alt+V)", false },
                        {},
                        {L"Open (ctrl+O)", true, open},
                        {L"Save (ctrl+S)", true, save},
                        {},
                        {L"Import", true, make( import, point )},
                        {L"Export", true, exportf},
                        {},
                        {L"Undo last change (ctrl+Z)", true, make( undo, true )},
                        {L"Redo last change (ctrl+Y)", true, make( undo, false )},
                        {},
                        {L"View structure (ctrl+↑)", target && view( target, true ), make2( view, target, false )},
                        {L"Return (ctrl+↓)", view( nullptr, true ), make2( view, nullptr, false )},
                        {},
                        {L"Help", true, help},
                        {},
                        {L"Exit", true, exit}
                    } );
                    menu.run();
                }
            }

            if( input.mouseX.changed() || input.mouseY.changed() )
            {
                if( toolId == 0 )
                {
                    if( selection->move( camera, Vector2D( *input.mouseX, *input.mouseY ) ) )
                        update();
                }
                else if( toolId == 2 && initialCanvasGrab )
                {
                    camera.s = *initialCanvasGrab + Vector2D( *input.mouseX, *input.mouseY );
                    update();
                }
            }

            if( input.leftMouse.changed() && !*input.leftMouse )
            {
                selection->release();
                if( toolId == 3 && initialCanvasGrab )
                {
                    auto inv = camera.inv();
                    auto leftTop = inv( *initialCanvasGrab );
                    auto bottomRight = inv( Vector2D( *input.mouseX, *input.mouseY ) );
                    focusCamera( leftTop, bottomRight );
                }
                initialCanvasGrab.reset();
            }
        };
    }

    makeException( description );
    GenericWindow self( *description, outerHandler );
    self.run();
}

void ImageWindow::updateRoot()
{
    auto find = []( JustEdit::Entity * object )
    {
        return dynamic_cast<JustEdit::Selection*>( ( *object )( []( JustEdit::Entity * s ) -> bool
        {
            return !dynamic_cast<JustEdit::Selection*>( s );
        } ) );
    };

    root = rootObject.get();
    if( root )
    {
        auto sel = find( root );
        makeException( !sel );

        selection = std::make_shared<JustEdit::Selection>();

        if( root->getNodes().empty() )
            root->add( std::make_shared<JustEdit::Text>( L"hint", L"Press right mouse button to begin.", JustEdit::Position( Vector2D( 32, 32 ) ) ) );
    }

    title = root ? L"JustEdit" : L"Canvas";
}
