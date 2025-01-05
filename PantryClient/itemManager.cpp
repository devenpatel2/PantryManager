#include <map>
#include <Arduino.h>
#include "itemManager.h"

void ItemManager::addItem(const String& name, int status) {
    Serial.print("\nAdding item "); Serial.print(name); Serial.print(":"); Serial.print(status);
    Serial.println("");
    items[name] = status;
}

void ItemManager::removeItem(const String& name) {
    items.erase(name);
}

void ItemManager::updateItem(const String& name, int status) {
    if (items.find(name) != items.end()) {
        items[name] = status;
    }
}

std::vector<std::pair<String, int>> ItemManager::getSortedItems() const {
    std::vector<std::pair<String, int>> itemVector(items.begin(), items.end());

    // Sorting logic: prioritize status 1, then group status 0 and 2 together and sort alphabetically
    std::sort(itemVector.begin(), itemVector.end(), [](const auto& a, const auto& b) {
        // Prioritize Out of Stock (status = 1)
        if (a.second == 1 && b.second != 1) {
            return true;
        } else if (b.second == 1 && a.second != 1) {
            return false;
        }

        // For other cases (0 and 2), sort alphabetically
        return a.first < b.first;
    });

    return itemVector;
}

const std::map<String, int>& ItemManager::getItems() const {
        return items;
}
