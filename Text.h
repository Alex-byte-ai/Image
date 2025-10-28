#pragma once

#include <optional>
#include <cstdint>
#include <string>
#include <map>

#include "ImageDataBase.h"

class TextGraphics : public TextBase
{
public:
    class Data;
private:
    std::map<std::wstring, std::wstring> parameters;
    Data *data;
public:
    TextGraphics();
    ~TextGraphics();

    void set( const std::wstring &name, const std::wstring &value );
    std::optional<std::wstring> get( const std::wstring &name ) const;

    bool measure( int &w, int &h ) const override;
    bool operator()( ImageDataBase &image ) const override;
};
