#pragma once

#include <string>
#include  <vector>
#include <memory>

#include "imgui.h"

class IAutoCommpleteItem
{
public:
    virtual bool CanPassFilter(const char* search_string) = 0;
    virtual const std::string& GetItemDescription() = 0;
};

class AutoCompleteInputText
{
public:
    AutoCompleteInputText();

    void Init(std::vector<std::shared_ptr<IAutoCommpleteItem>>&& items);
    std::shared_ptr<IAutoCommpleteItem> Show(const char* id, const char* label, const char* hint);

    inline bool IsInitialized() const { return m_is_initialized; }
private:
    bool m_is_initialized = false;
    std::vector<std::shared_ptr<IAutoCommpleteItem>> _lastTypeAheadResults;
    int _selectedResultIndex = 0;
    unsigned int _activeInputId;

    std::vector<std::shared_ptr<IAutoCommpleteItem>> m_items;

    //private static bool _isSearchResultWindowOpen;
};

