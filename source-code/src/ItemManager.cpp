#include "ItemManager.hpp"
#include <iostream>
#include <time.h>

using namespace std;

// 생성자
ItemManager::ItemManager()
    : selectedItemId(0), selectedItemNum(0) {
    srand((unsigned int)time(NULL)); 
    
    string name[20] = {
        "콜라", "사이다", "녹차", "홍차", "밀크티",
        "탄산수", "보리차", "캔커피", "물", "에너지드링크",
        "유자차", "식혜", "아이스티", "딸기주스", "오렌지주스",
        "포도주스", "이온음료", "아메리카노", "핫초코", "카페라떼"
    };

    int cost[20] = {
        1500, 1400, 1300, 1350, 1800,
        1200, 1100, 1600, 1000, 2000,
        1500, 1400, 1350, 1700, 1700,
        1700, 1300, 1900, 1800, 2000
    };

    int stock[20] = {0};

    // 재고 초기화: 20개 중 7개 항목에만 재고를 할당
    int count = 0;
    while (count < 7) {
        int index = rand() % 20;
        if (stock[index] == 0) { // 아직 선택되지 않은 항목이라면
            stock[index] = 1 + rand() % 99; // 1~99 사이 재고 할당
            count++;
        }
    }

    // 초기 아이템 등록
    for (int i = 0; i < 20; i++) {
        itemList.emplace_back(i + 1, name[i], cost[i], stock[i]);
    }
}

// 아이템 목록 보여주기
void ItemManager::showItemList() const {
    for(int id=0; id<itemList.size(); id++){
        cout << "ID: " << id+1
                << ", Name: " << itemList[id].getName()
                << ", Price: " << itemList[id].getCost(1)
                << ", Stock: " << itemList[id].getStock() << endl;
    }
}

// 선택된 아이템 저장
void ItemManager::saveSelectedItem(pair<int, int> saveInfo) {
    selectedItemId = saveInfo.first;
    selectedItemNum = saveInfo.second;
}

// 특정 음료수 제고 확인
int ItemManager::checkStock(int itemId) const {
    return itemList[itemId-1].getStock();
}

// 선택된 아이템의 재고가 충분한지 확인
bool ItemManager::isEnough() const {
    if(itemList[selectedItemId-1].getStock() >= selectedItemNum)
        return true;
    else
        return false;
}

// 인자로 받아오는 값의 재고가 충분한지 확인
bool ItemManager::isEnough(int itemId) const
{
    if(itemList[itemId-1].getStock() >= selectedItemNum)
        return true;
    else
        return false;
}

// 선택된 아이템 재고 감소
void ItemManager::minusStock() {
    itemList[selectedItemId-1].decreaseStock(selectedItemNum);
}

void ItemManager::minusStock(int itemId, int itemNum)
{
    itemList[itemId-1].decreaseStock(itemNum);
}

// 구매 결과 출력
void ItemManager::showBuyResult() {
    cout << itemList[selectedItemId-1].getName() << " " << selectedItemNum << "개 구매되었습니다." << endl;
}

int ItemManager::getPaymentAmount(){
    return itemList[selectedItemId-1].getCost(selectedItemNum);
}

// 구글테스트용 메서드
int ItemManager::getSelectedItemId(){
    return selectedItemId;
}
int ItemManager::getSelectedItemNum(){
    return selectedItemNum;
}

vector<Item> ItemManager::getItemList(){
    return itemList;
}