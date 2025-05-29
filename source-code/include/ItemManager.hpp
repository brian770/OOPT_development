#ifndef ITEMMANAGER_H
#define ITEMMANAGER_H

#include <vector>
#include "Item.hpp"

class ItemManager {
private:
    std::vector<Item> itemList;
    int selectedItemId;
    int selectedItemNum;

public:
    // 생성자
    ItemManager();

    void showItemList() const;
    void saveSelectedItem(std::pair<int, int> saveInfo);
    int checkStock(int itemId) const;
    bool isEnough() const;
    bool isEnough(int itemId) const;
    void minusStock();
    void minusStock(int itemId, int itemNum);
    void showBuyResult();
    int getPaymentAmount();
    
    // 구글테스트용 메서드
    int getSelectedItemId();
    int getSelectedItemNum();
    std::vector<Item> getItemList();
};

#endif // ITEMMANAGER_H