#pragma once

#include "geo.h"
#include "transport_catalogue.h"
#include "json.h"
#include "map_renderer.h"

#include <string>
#include <string_view>
#include <vector>

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */
namespace reader
{
    renderer::RenderSettings ParseRenderSettings(const json::Document& document);
    
    void ReadFrom(const json::Document& document, catalogue::Transport& catalogue);
}
