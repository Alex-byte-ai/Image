#include "ImageWindow.h"

#include <algorithm>
#include <cstdlib>

#include "Exception.h"
#include "Lambda.h"
#include "Basic.h"

#include "ImageData.h"

std::optional<ImageWindow::Frame> ImageWindow::frame;
std::optional<ImageWindow::IconData> ImageWindow::iconData;

static void fill( GraphicInterface::Node::Parameter& parameter, const JustEdit::Entity& object )
{
    parameter.name = object.name;
    parameter.open = object.hasStructure();

    if( parameter.open )
    {
        auto nodes = object.getNodes();
        parameter.parameters.resize( nodes.size() );

        size_t i = 0;
        for( auto& p : parameter.parameters )
        {
            fill( p, *nodes[i] );
            ++i;
        }
    }
}

template<typename F, typename... Args>
auto make( F&& f, Args&&... args )
{
    std::function<void()> function = [fn = std::forward<F>( f ), tup = std::make_tuple( std::forward<Args>( args )... )]()
    {
        std::apply( fn, tup );
    };
    return function;
}

ImageWindow::OutputData::OutputData( GraphicInterface::OutputData& original, ImageDataBase &img ) : image( img ), x( original.x ), y( original.y ), quit( original.quit )
{}

ImageWindow::ImageWindow( ImageDataBase &idb, HandleMsg hnd, std::shared_ptr<JustEdit::Entity> object, Data initData )
    : rootObject( std::move( object ) ), data( std::move( initData ) ), handler( std::move( hnd ) ), image( idb )
{
    updateRoot();

    if( !frame )
        frame.emplace();
    auto& f = *frame;

    auto& desc = description.emplace();

    prepareIcon( desc );

    desc.title.value = title;
    desc.title.prepare();

    desc.x = f.x;
    desc.y = f.y;
    desc.self.w = f.w;
    desc.self.h = f.h;
    desc.client.color = GraphicInterface::makeColor( 60, 70, 200, 255 );

    desc.content.w = image.w();
    desc.content.h = image.h();
}

ImageWindow::~ImageWindow()
{
    if( frame && description )
    {
        auto& desc = *description;
        auto& f = *frame;

        f.x = desc.x;
        f.y = desc.y;
        f.w = desc.self.w;
        f.h = desc.self.h;
    }
}

bool ImageWindow::run( bool lock )
{
    GraphicInterface::HandleMsg outerHandler;

    if( !rootObject )
    {
        outerHandler = [this, callback = handler]( const GraphicInterface::InputData & input, GraphicInterface::OutputData & output )
        {
            if( input.escape.changed() && *input.escape )
            {
                output.quit = true;
                return true;
            }

            OutputData outputData( output, image );

            bool response = false;
            if( callback )
                response = callback( input, outputData );

            if( input.init || outputData.image.changed() )
            {
                output.image.get().prepare( image( 0, 0 ), image.s(), image.h() );
                return true;
            }

            if( !response )
            {
                if( input.space.changed() && *input.space )
                {
                    image.output();
                    return true;
                }

                if( input.f1.changed() && *input.f1 )
                {
                    if( data.help )
                    {
                        data.help->run();
                        return true;
                    }
                }
            }

            return response;
        };
    }
    else
    {
        outerHandler = [this]( const GraphicInterface::InputData & input, GraphicInterface::OutputData & output )
        {
            auto getFreeName = [this]( const std::wstring & prefix )
            {
                unsigned freeId = 0, length = prefix.length() + 1;
                std::wstring freeIdString = L"0";

                ( *rootObject )( [&]( JustEdit::Entity * s )
                {
                    if( s->name.length() == length && s->name.substr( 0, prefix.length() ) == prefix )
                    {
                        if( s->name.substr( prefix.length() ) == freeIdString )
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

            auto update = [this, &output]()
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

            auto focusCamera = [this, update]( const Vector2D & topLeft, const Vector2D & bottomRight, double baseScale = 1.0 )
            {
                int w = image.w();
                int h = image.h();

                auto sx = baseScale * w / ( bottomRight.x - topLeft.x );
                auto sy = baseScale * h / ( bottomRight.y - topLeft.y );

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
                else if( sx < 1 )
                    sx = 1 / RoundDown( 1 / sx );

                if( sy > 1 )
                    sy = RoundDown( sy );
                else if( sy < 1 )
                    sy = 1 / RoundDown( 1 / sy );

                auto scale = Min( sx, sy );

                camera = Affine2D( Vector2D( w * 0.5, h * 0.5 ) ) * Affine2D( Matrix2D::Scale( scale ) ) * Affine2D( -( topLeft + bottomRight ) * 0.5 );

                update();
            };

            auto fitCamera = [this, focusCamera]()
            {
                Vector2D topLeft, bottomRight;
                if( root->size( Affine2D( Vector2D() ), topLeft, bottomRight ) )
                    focusCamera( topLeft, bottomRight, 0.8 );
            };

            auto minimizeAll = [this, fitCamera]()
            {
                selection->select( nullptr, false );
                root = rootObject.get();
                selection->setRoot( root );

                ( *rootObject )( []( JustEdit::Entity * o )
                {
                    o->dumpStructure();
                    return true;
                } );

                fitCamera();
            };

            auto edit = [this, update]( JustEdit::Entity * target )
            {
                Settings::Parameters parameters;
                for( auto& [name, set, get, options] : target->editData() )
                    parameters.emplace_back( name, set, get, options );

                auto& s = settings.emplace( target->description(), parameters );
                s.onClose = [this, update]()
                {
                    selection->update();
                    update();
                };
                s.run();
            };

            auto add = [this]( std::shared_ptr<JustEdit::Entity> entity )
            {
                if( hierarchy )
                {
                    auto& r = hierarchy->root;
                    r.addNode( std::make_shared<GraphicInterface::Node>( r.data, entity->name ) );
                }
                selection->select( root->add( std::move( entity ) ), false );
            };

            auto remove = [this]( JustEdit::Entity * entity )
            {
                if( hierarchy )
                {
                    auto object = hierarchy->root.getObject( entity->getPath() );
                    if( object )
                        object->detach();
                }
                auto result = entity->detach();
                selection->select( nullptr, false );
                return result;
            };

            auto create = [this, getFreeName, add, update]( Vector2D p, int itemId, bool test )
            {
                using namespace JustEdit;

                if( !root->hasStructure() )
                    return false;
                if( test )
                    return true;

                p = camera.inv()( p );

                if( itemId == 0 )
                {
                    add( std::make_shared<JustEdit::Raster>( getFreeName( L"raster" ), 64, 64, Position( p ) ) );
                }
                else if( itemId == 1 )
                {
                    add( std::make_shared<JustEdit::Line>( getFreeName( L"line" ), p, p + Vector2D( 32, 32 ) ) );
                }
                else if( itemId == 2 )
                {
                    add( std::make_shared<JustEdit::Rectangle>( getFreeName( L"rectangle" ), 32, 16, Position( p ) ) );
                }
                else if( itemId == 3 )
                {
                    add( std::make_shared<JustEdit::Circle>( getFreeName( L"circle" ), p, 24 ) );
                }
                else if( itemId == 4 )
                {
                    add( std::make_shared<JustEdit::Text>( getFreeName( L"text" ), L"Lorem ipsum", Position( p ) ) );
                }
                else if( itemId == 5 )
                {
                    add( std::make_shared<JustEdit::Polygon>( getFreeName( L"polygon" ), Position( p ) ) );
                }
                else if( itemId == 6 )
                {
                    add( std::make_shared<JustEdit::Point>( getFreeName( L"point" ), 1, p ) );
                }

                update();
                return true;
            };

            static uint16_t toolId = 0;
            static std::optional<Vector2D> initialCanvasGrab;
            auto pickTool = []( uint16_t id, bool test )
            {
                if( initialCanvasGrab )
                    return false;

                if( test )
                    return true;

                toolId = id;
                return true;
            };

            auto modify = [this, getFreeName, add, remove, update]( JustEdit::Entity * target, int modificationId, bool test )
            {
                if( !target || target == root || target->type() != L"Raster" )
                    return false;

                if( test )
                    return true;

                if( modificationId == 0 )
                {
                    auto targetRoot = target->getRoot();
                    std::swap( root, targetRoot );
                    add( std::make_shared<JustEdit::Perspective>( getFreeName( L"perspective" ), remove( target ), target->position ) );
                    std::swap( root, targetRoot );
                    target->position = JustEdit::Position();
                    update();
                }

                return true;
            };

            auto deletef = [this, remove, update]( const std::vector<JustEdit::Entity*>& targets, bool test )
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
                    remove( target );

                update();
                return true;
            };

            auto group = [this, add, remove, update]( bool f, bool test )
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
                            g->add( remove( target ) );
                    }
                    add( g );

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
                    add( remove( node ) );
                }

                remove( g );
                update();
                return true;
            };

            auto open = [this, update]()
            {
                JustEdit::Entity::load( [this, update]( auto & newRoot )
                {
                    if( newRoot )
                    {
                        rootObject = newRoot;
                        updateRoot();
                        update();
                    }
                    else
                    {
                        popup.emplace( Popup::Type::Warning, L"Loading", L"File can't be loaded." ).run();
                    }
                } );
            };

            auto save = [this, minimizeAll]()
            {
                minimizeAll();
                rootObject->save();
            };

            auto import = [this, getFreeName, add, update]( const Vector2D & p )
            {
                auto raster = std::make_shared<JustEdit::Raster>( getFreeName( L"import" ), 64, 64, JustEdit::Position( camera.inv()( p ) ) );

                raster->image->input();
                raster->w = raster->image->w();
                raster->h = raster->image->h();

                raster->position.scaleX = 64.0 / raster->w;
                raster->position.scaleY = 64.0 / raster->h;

                add( raster );
                update();
            };

            auto exportf = [this, minimizeAll]()
            {
                Vector2D topLeft, bottomRight;
                if( root->size( Affine2D( Vector2D() ), topLeft, bottomRight ) )
                {
                    minimizeAll();

                    int w = RoundUp( bottomRight.x - topLeft.x );
                    int h = RoundUp( bottomRight.y - topLeft.y );

                    ImageData img( w, h );

                    Overlap::Canvas canavs( img );
                    root->draw( Affine2D( -topLeft ), canavs );
                    canavs.render( img );

                    img.output();
                }
            };

            auto undo = [this]( bool f )
            {
                popup.emplace( Popup::Type::Info, L"Change buffer", f ? L"Undone is not implemented" : L"Redone is not implemented" ).run();
            };

            auto view = [this, fitCamera]( JustEdit::Entity * target, bool test )
            {
                if( target )
                {
                    Vector2D a, b;
                    if( target && target->size( Affine2D( Vector2D() ), a, b ) )
                    {
                        if( test )
                            return true;

                        if( !target->hasStructure() && !target->establishStructure() )
                            return false;

                        selection->select( nullptr, false );

                        root = target;
                        selection->setRoot( root );

                        fitCamera();
                        return true;
                    }

                    return false;
                }

                auto newRoot = root->getRoot();

                if( test )
                    return true;

                if( root->hasStructure() && !root->dumpStructure() )
                    return false;

                selection->select( nullptr, false );

                root = newRoot ? newRoot : root;
                selection->setRoot( root );

                fitCamera();
                return true;
            };

            auto help = [this]()
            {
                popup.emplace( Popup::Type::Info, L"Help", L"Some information..." ).run();
            };

            auto exit = [&output]()
            {
                output.quit = true;
            };

            auto openHierarchy = [this, update]()
            {
                if( hierarchy )
                    return;

                GraphicInterface::Node::Parameter parameter;
                fill( parameter, *rootObject );

                auto& h = hierarchy.emplace( parameter );

                h.title.value = L"Scene tree";
                h.title.prepare();

                h.callback = [this, &h, o = rootObject.get(), update]( const GraphicInterface::Node::ActionData & d )
                {
                    JustEdit::Entity *primary = nullptr;

                    if( d.path )
                        primary = o->getObject( *d.path );

                    if( !primary )
                        return false;

                    auto sync = [&]()
                    {
                        GraphicInterface::Node::Parameter p;
                        fill( p, *primary );

                        auto node = h.root.getObject( *d.path );
                        node->update( p );
                    };

                    if( d.action == GraphicInterface::Node::Action::Move )
                    {
                        JustEdit::Entity *secondaryRoot = nullptr, *primaryRoot = nullptr;
                        size_t secondaryId = -1;

                        primaryRoot = o->getObject( *d.path, 1 );

                        if( d.secondary )
                        {
                            secondaryId = d.secondary->front();
                            secondaryRoot = o->getObject( *d.secondary, 1 );
                        }

                        if( !secondaryRoot )
                            return false;

                        if( secondaryRoot == primaryRoot && secondaryId > primary->getId() )
                            --secondaryId;

                        auto node = primary->detach();
                        if( !node )
                            return false;

                        if( !secondaryRoot->add( node, secondaryId ) )
                            return false;

                        selection->select( nullptr, false );
                        update();
                        return true;
                    }

                    if( d.action == GraphicInterface::Node::Action::Open )
                    {
                        if( !primary->establishStructure() )
                            return false;
                        selection->select( nullptr, false );
                        sync();
                        update();
                        return true;
                    }

                    if( d.action == GraphicInterface::Node::Action::Close )
                    {
                        if( !primary->dumpStructure() )
                            return false;
                        selection->select( nullptr, false );
                        sync();
                        update();
                        return true;
                    }

                    return false;
                };

                h.onClose = [this]()
                {
                    hierarchy.reset();
                };

                h.run( false );
            };

            auto keyDown = [&]( char symbol )
            {
                auto &key = input.keys.letter( symbol );
                return key.changed() && *key;
            };

            if( input.init )
            {
                Vector2D topLeft, bottomRight;
                if( rootObject->size( Affine2D( Vector2D() ), topLeft, bottomRight ) )
                {
                    auto size = bottomRight - topLeft;
                    image.reset( Round( size.x / 0.8 ), Round( size.y / 0.8 ) );
                    fitCamera();
                }
                openHierarchy();
                return true;
            }

            if( *input.ctrl && keyDown( 'G' ) )
            {
                group( true, false );
                return true;
            }

            if( *input.ctrl && keyDown( 'U' ) )
            {
                group( false, false );
                return true;
            }

            if( *input.ctrl && keyDown( 'O' ) )
            {
                open();
                return true;
            }

            if( *input.ctrl && keyDown( 'S' ) )
            {
                save();
                return true;
            }

            if( *input.ctrl && *input.up && input.up.changed() )
            {
                if( auto target = selection->getTarget() )
                    view( target, false );
                return true;
            }

            if( *input.ctrl && *input.down && input.down.changed() )
            {
                view( nullptr, false );
                return true;
            }

            if( *input.del && input.del.changed() )
            {
                deletef( selection->getTargets(), false );
                return true;
            }

            static uint16_t forcedOutTool = 0;
            if( input.space.changed() && *input.space )
            {
                forcedOutTool = toolId;
                toolId = 2;
                return true;
            }

            if( input.space.changed() && !*input.space )
            {
                toolId = forcedOutTool;
                return true;
            }

            bool lmb = input.leftMouse.changed() && *input.leftMouse;
            bool rmb = input.rightMouse.changed() && *input.rightMouse;

            if( lmb || rmb )
            {
                Vector2D point( *input.mouseX, *input.mouseY );
                auto target = root->pointsTo( camera, point );

                bool isSelection = false;

                if( lmb )
                {
                    if( toolId == 0 )
                    {
                        if( target != root && selection->grab( camera, point ) )
                        {
                            isSelection = true;
                        }
                        else
                        {
                            selection->select( target, *input.ctrl );
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
                    menu.emplace( ContextMenu::Parameters
                    {
                        {L"Edit", !isSelection && target, make( edit, target )},
                        {},
                        {
                            L"Tools", true, {},
                            {
                                {L"Select", pickTool( 0, true ), make( pickTool, 0, false )},
                                {L"Pixel drawing", pickTool( 1, true ), make( pickTool, 1, false )},
                                {L"Move canvas (Hold Space)", pickTool( 2, true ), make( pickTool, 2, false )},
                                {L"Zoom (Mouse / Ctrl+(+ / -))", pickTool( 3, true ), make( pickTool, 3, false )},
                            }
                        },
                        {},
                        {
                            L"Create", true, {},
                            {
                                {L"Raster", create( point, 0, true ), make( create, point, 0, false )},
                                {L"Line", create( point, 1, true ), make( create, point, 1, false )},
                                {L"Rectangle", create( point, 2, true ), make( create, point, 2, false )},
                                {L"Circle", create( point, 3, true ), make( create, point, 3, false )},
                                {L"Text", create( point, 4, true ), make( create, point, 4, false )},
                                {L"Polygon", create( point, 5, true ), make( create, point, 5, false )},
                                {L"Point", create( point, 6, true ), make( create, point, 6, false )}
                            }
                        },
                        {
                            L"Modify", true, {},
                            {
                                {
                                    L"Transform", true, {},
                                    {
                                        {L"Perspective", modify( target, 0, true ), make( modify, target, 0, false )}
                                    }
                                }
                            }
                        },
                        { L"Delete (Del)", !isSelection && deletef( { target }, true ), make( deletef, std::vector<JustEdit::Entity*>{ target }, false ) },
                        {},
                        { L"Group (ctrl+G)", group( true, true ), make( group, true, false ) },
                        { L"Ungroup (ctrl+U)", group( false, true ), make( group, false, false ) },
                        {},
                        { L"Copy (ctrl+C)", false },
                        { L"Cut (ctrl+X)", false },
                        { L"Paste (ctrl+V)", false },
                        { L"Place (ctrl+alt+V)", false },
                        {},
                        {L"Scene tree", true, openHierarchy},
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
                        {L"Hide others (ctrl+↑)", target && view( target, true ), make( view, target, false )},
                        {L"Show previously hidden (ctrl+↓)", view( nullptr, true ), make( view, nullptr, false )},
                        {},
                        {L"Help", true, help},
                        {},
                        {L"Exit", true, exit}
                    } ).run();
                }

                return true;
            }

            if( input.mouseX.changed() || input.mouseY.changed() )
            {
                if( toolId == 0 )
                {
                    if( selection->move( camera, Vector2D( *input.mouseX, *input.mouseY ) ) )
                        update();
                    return true;
                }
                if( toolId == 2 && initialCanvasGrab )
                {
                    camera.s = *initialCanvasGrab + Vector2D( *input.mouseX, *input.mouseY );
                    update();
                    return true;
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
                return true;
            }

            return false;
        };
    }

    makeException( description );
    description->handleMsg = outerHandler;
    return description->run( lock );
}

void ImageWindow::prepareIcon( GraphicInterface::Window& desc )
{
    if( iconData )
    {
        auto& ico = *iconData;
        desc.icon.w = ico.w;
        desc.icon.h = ico.h;
        desc.icon.pixels = ico.image;
        return;
    }

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

        auto& ico = iconData.emplace();
        ico.w = desc.icon.w;
        ico.h = desc.icon.h;
        ico.image = desc.icon.pixels;
    }
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
