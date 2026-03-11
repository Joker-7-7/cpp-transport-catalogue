#pragma once

#include "geo.h"
#include "transport_catalogue.h"
#include "json.h"

#include <string>
#include <string_view>
#include <vector>

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

namespace reader
{
 class JsonInput {
    public:
        /**
         * Парсит строку в структуру CommandDescription и сохраняет результат в commands_
         */
        //void ParseLine(std::string_view line);

        /**
         * Наполняет данными транспортный справочник, используя команды из commands_
         */
        //void ApplyCommands(catalogue::Transport& catalogue) const;

        void ReadFrom(const json::Document& document, catalogue::Transport& catalogue);
    private:
    };
}
