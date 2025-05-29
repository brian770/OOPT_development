#include "PaymentManager.hpp"
#include <iostream>

using namespace std;

PaymentManager::PaymentManager(Bank* b) 
    : bank(b) {}

string PaymentManager::getCardNum() {
    string cardNum;
    bool isAvailable = false;
    while(!isAvailable){
        cout << "카드 번호를 입력해주세요(5자리): ";
        cin >> cardNum;
        if(cardNum.length() == 5){
            isAvailable = true;
        }
        else {
            cout << "5자리 카드번호로 입력해주세요." << endl;
        }
    }
    return cardNum;
}

// 1: 성공, 2: 카드 정보 안 맞음, 3: 계좌에 돈이 부족함, -1: 예외상황 발생
int PaymentManager::requestPayment(int cost) {
    return bank->pay(getCardNum(), cost);
}