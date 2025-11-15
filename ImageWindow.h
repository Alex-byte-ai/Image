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
    using InputData = GenericWindow::InputData;

    class OutputData
    {
    public:
        ChangedValue<ImageDataBase &> image;
        ChangedValue<int>& x, y;
        ChangedValue<Popup>& popup;
        bool& quit;

        OutputData( GenericWindow::OutputData& original, ImageDataBase &img );
    };

    using HandleMsg = std::function<void( const InputData &, OutputData & )>;

    class Data
    {
    public:
        std::optional<Popup> help;

        Data( std::optional<Popup> h = {} ) : help( std::move( h ) )
        {}
    };

    ImageWindow( ImageDataBase &image, HandleMsg handler, std::shared_ptr<JustEdit::Entity> object = nullptr, Data initData = Data() );
    ~ImageWindow();

    void run();
private:
    void updateRoot();

    std::wstring title;

    std::shared_ptr<JustEdit::Selection> selection;
    std::shared_ptr<JustEdit::Entity> rootObject;
    JustEdit::Entity *root;
    Affine2D camera;

    Data data;
    HandleMsg handler;

    ImageDataBase &image;

    static std::optional<GraphicInterface::Window> description;
};
