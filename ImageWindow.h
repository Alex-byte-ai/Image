#pragma once

#include <functional>
#include <optional>
#include <array>

#include "ChangedValue.h"
#include "Exception.h"
#include "Window.h"

#include "ImageDataBase.h"
#include "JustEdit.h"

class ImageWindow
{
public:
    using InputData = GraphicInterface::InputData;

    class OutputData
    {
    public:
        ChangedValue<ImageDataBase &> image;
        ChangedValue<int>& x, y;
        bool& quit;

        OutputData( GraphicInterface::OutputData& original, ImageDataBase &img );
    };

    using HandleMsg = std::function<bool( const InputData &, OutputData & )>;

    class Data
    {
    public:
        std::optional<Popup> help;

        Data( std::optional<Popup> h = {} ) : help( std::move( h ) )
        {}
    };

    ImageWindow( ImageDataBase &image, HandleMsg handler, std::shared_ptr<JustEdit::Entity> object = nullptr, Data initData = Data() );
    ~ImageWindow();

    bool run( bool lock = true );
private:
    void prepareIcon( GraphicInterface::Window& desc );

    void updateRoot();

    std::wstring title;

    std::shared_ptr<JustEdit::Selection> selection;
    std::shared_ptr<JustEdit::Entity> rootObject;
    JustEdit::Entity *root;
    Affine2D camera;

    Data data;
    HandleMsg handler;

    ImageDataBase &image;

    std::optional<GraphicInterface::Window> description;
    std::optional<Hierarchy> hierarchy;
    std::optional<Settings> settings;
    std::optional<ContextMenu> menu;
    std::optional<Popup> popup;

    struct Frame
    {
        int x = -1, y = -1, w = -1, h = -1;
    };

    struct IconData
    {
        int w = 0, h = 0;
        std::vector<uint32_t> image;
    };

    static std::optional<Frame> frame;
    static std::optional<IconData> iconData;
};
