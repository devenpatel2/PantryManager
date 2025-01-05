#ifndef ITEMMANAGER_H
#define ITEMMANAGER_H
#include <map>
#include <Arduino.h>

class ItemManager {
private:
    std::map<String, int> items;

public:
    void addItem(const String& name, int status);
    void removeItem(const String& name);
    void updateItem(const String& name, int status);
    std::vector<std::pair<String, int>> getSortedItems() const;
    const std::map<String, int>& getItems() const;
};

#endif
